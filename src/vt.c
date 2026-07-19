#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <math.h>

#include "types.h"
#include "vt.h"
#include "vtfont.h"

#define	CHAR_WIDTH_80		10
#define	CHAR_WIDTH_132		6
#define	CHAR_HEIGHT		10

#define	CURSOR_OFF_TIME		(2.0f / 3.0f)
#define	CURSOR_ON_TIME		(4.0f / 3.0f)
#define	CURSOR_TIME		(CURSOR_OFF_TIME + CURSOR_ON_TIME)

#define	BLINK_OFF_TIME		1.8f
#define	BLINK_ON_TIME		1.8f
#define	BLINK_TIME		(BLINK_OFF_TIME + BLINK_ON_TIME)

#define	FRAMEBUFFER_WIDTH	800
#define	FRAMEBUFFER_HEIGHT	240

#define	TEXT_WIDTH		80
#define	TEXT_WIDTH_MAX		132
#define	TEXT_HEIGHT		24

#define	STATE_TEXT		0
#define	STATE_ESC		1
#define	STATE_CSI		2
#define	STATE_DECPRIV		3
#define	STATE_ESC_HASH		4
#define	STATE_DCS		5
#define	STATE_DCS_ESC		6
#define	STATE_DCS_UNK		7
#define	STATE_DCA1		8
#define	STATE_DCA2		9
#define	STATE_G			10
#define	STATE_ESC_SP		11
#define	STATE_CSI_GT		12
#define	STATE_CSI_QUOT		13
#define	STATE_CSI_EXCL		14
#define	STATE_DECUDK		15
#define	STATE_DECUDK_ESC	16
#define	STATE_DECUDK_ODD	17
#define	STATE_DECUDK_ODD_ESC	18
#define	STATE_DECUDK_EVEN	19
#define	STATE_DECUDK_EVEN_ESC	20
#define	STATE_DECDLD		21
#define	STATE_DECDLD_ESC	22
#define	STATE_DECDLD_GLYPH	23
#define	STATE_DECDLD_GLYPH_ESC	24
#define	STATE_DECDLD_ERR	25
#define	STATE_DECDLD_ERR_ESC	26

static const VT220NVR default_config = { .tx_baud_rate = 4800, 0 };

void VT220Init(VT220* vt)
{
	vt->cursor_time = 0;
	vt->config = default_config;
	vt->screen_color = VT220_SCREEN_COLOR_GREEN;

	vt->columns = TEXT_WIDTH;
	vt->lines = TEXT_HEIGHT;

	vt->text = (VT220CELL*) malloc(TEXT_WIDTH_MAX * TEXT_HEIGHT * sizeof(VT220CELL));
	vt->line_attributes = (char*) malloc(TEXT_HEIGHT);
	vt->tabstops = (char*) malloc(TEXT_WIDTH_MAX);

	vt->buf = (unsigned char*) malloc(256); /* input buffer */
	vt->buf_r = 0;
	vt->buf_w = 0;
	vt->buf_used = 0;
	vt->buf_lost = 0;

	vt->hold_screen = 0;
	vt->enable_buffering = 0;

	vt->bell = NULL;
	vt->keyclick = NULL;
	vt->rx = NULL;
	vt->brk = NULL;
	vt->flowcontrol = NULL;

	vt->drcs = (unsigned char*) malloc(940);
	vt->drcs_dirty = 1;

	/* configure setup screens */
	vt->in_setup = 0;
	vt->setup.text = (VT220CELL*) malloc(8 * TEXT_WIDTH_MAX * sizeof(VT220CELL));
	vt->setup.line_attributes = (char*) malloc(8);
	memset(vt->setup.text, 0, 8 * TEXT_WIDTH_MAX * sizeof(VT220CELL));
	memset(vt->setup.line_attributes, 0, 8);

	/* reset config */
	vt->config.user_features = VT220_USER_FEATURES_UNLOCKED;
	vt->config.mode = VT220_MODE_VT200_MODE_7BIT_CONTROLS;
	vt->config.auto_wrap = true;
	vt->config.text_cursor = VT220_TEXT_CURSOR;
	vt->config.new_line = VT220_NO_NEW_LINE;
	vt->config.local_echo = VT220_NO_LOCAL_ECHO;
	vt->config.auto_repeat = VT220_AUTO_REPEAT;

	VT220InitKeyboard(vt);
	VT220HardReset(vt);
}

static inline unsigned int VT220GetCellWidth(VT220* vt)
{
	if(vt->mode & DECCOLM) {
		/* 132 column mode */
		return CHAR_WIDTH_132;
	} else {
		/* 80 column mode */
		return CHAR_WIDTH_80;
	}
}

static inline unsigned int VT220GetCellOffset(VT220* vt)
{
	if(vt->mode & DECCOLM) {
		/* 132 column mode */
		return 4;
	} else {
		/* 80 column mode */
		return 0;
	}
}

void VT220WriteGlyph(VT220* vt, u16 c, bool force_wrap)
{
	if(vt->cursor_x == vt->columns) {
		if(vt->mode & DECAWM || force_wrap) { /* Auto Wrap Mode: enabled */
			VT220CarriageReturn(vt);
			VT220Linefeed(vt);
		} else {
			vt->cursor_x--;
		}
	} else if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		if(vt->mode & DECAWM || force_wrap) { /* Auto Wrap Mode: enabled */
			VT220CarriageReturn(vt);
			VT220Linefeed(vt);
		} else {
			vt->cursor_x = (vt->columns - 1) / 2;
		}
	}

	int px = vt->cursor_x;
	int py = vt->cursor_y;
	int pos = py * vt->columns + px;
	int eol = (py + 1) * vt->columns - 1;

	if(vt->mode & IRM) {
		int i;
		for(i = eol - 1; i >= pos; i--) {
			vt->text[i + 1] = vt->text[i];
		}
	}

	vt->text[pos].text = c;
	vt->text[pos].attr = vt->sgr;
	vt->cursor_x++;
	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = vt->columns / 2;
	}
}

void VT220WriteControls(VT220* vt, unsigned char c)
{
	u16 glyph = c;

	if(glyph == 0x20) {
		glyph = 0;
	} else if(glyph < 0x20) {
		glyph += 0x100;
	}

	VT220WriteGlyph(vt, glyph, true);
}

void VT220Write(VT220* vt, u16 c)
{
	VT220WriteGlyph(vt, c, false);
}

void VT220WriteChar(VT220* vt, unsigned char c)
{
	unsigned char ch;
	int cs;
	if(c < 0x20) {
		VT220Write(vt, (u16) c + 0x100);
	} else if(c >= 0x80 && c < 0xA0) {
		VT220Write(vt, c);
	} else if(c < 0x80) {
		/* use GL */
		ch = c - 0x20;
		cs = vt->g[vt->gl];
		VT220WriteCS(vt, ch, cs);
	} else {
		/* use GR */
		ch = c - 0xA0;
		cs = vt->g[vt->gr];
		VT220WriteCS(vt, ch, cs);
	}
	vt->gl = vt->gl_lock;
	vt->gr = vt->gr_lock;
}

void VT220WriteCS(VT220* vt, unsigned char ch, int cs)
{
	const u16* cs_table = NULL;
	switch(cs) {
		case CHARSET_ASCII:
			cs_table = vt220_cs_ascii;
			break;
		case CHARSET_DEC_SUPPLEMENTAL:
			cs_table = vt220_cs_dec_supplemental_graphics;
			break;
		case CHARSET_DEC_SPECIAL_GRAPHICS:
			cs_table = vt220_cs_dec_special_graphics;
			break;
		case CHARSET_NRCS_BRITISH:
			cs_table = vt220_cs_british;
			break;
		case CHARSET_DRCS:
			cs_table = vt220_cs_drcs;
			break;
		case CHARSET_ASCII_DC:
			cs_table = vt220_cs_ascii_dc;
			break;
		case CHARSET_DEC_SUPPLEMENTAL_DC:
			cs_table = vt220_cs_dec_supplemental_graphics_dc;
			break;
	}

	if(cs_table) {
		VT220Write(vt, cs_table[ch]);
	} else {
		VT220Write(vt, 0x20);
	}
}

void VT220ClearLastColumnFlag(VT220* vt)
{
	if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	}
}

static inline int VT220IsLastColumnFlag(VT220* vt)
{
	if(vt->cursor_x == vt->columns) {
		return 1;
	}

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		return 1;
	}

	return 0;
}

void VT220ShiftOut(VT220* vt)
{
	vt->gl = 1;
	vt->gl_lock = 1;
}

void VT220ShiftIn(VT220* vt)
{
	vt->gl = 0;
	vt->gl_lock = 0;
}

void VT220SingleShiftG2(VT220* vt)
{
	vt->gl = 2;
}

void VT220SingleShiftG3(VT220* vt)
{
	vt->gl = 3;
}

void VT220CursorHome(VT220* vt)
{
	vt->cursor_x = 0;
	vt->cursor_y = 0;
}

void VT220CursorLeft(VT220* vt)
{
	if(VT220IsLastColumnFlag(vt)) {
		VT220ClearLastColumnFlag(vt);
	}
	if(vt->cursor_x > 0) {
		vt->cursor_x--;
	}
}

void VT220CursorLeftN(VT220* vt, int n)
{
	for(int i = 0; i < n; i++) {
		VT220CursorLeft(vt);
	}
}

void VT220CursorRight(VT220* vt)
{
	if(vt->cursor_x == vt->columns) {
		return;
	}

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		return;
	}

	vt->cursor_x++;
	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}
}

void VT220CursorRightN(VT220* vt, int n)
{
	for(int i = 0; i < n; i++) {
		VT220CursorRight(vt);
	}
}

void VT220CursorDown(VT220* vt)
{
	if(vt->cursor_y == vt->margin_bottom) {
		return;
	}

	vt->cursor_y++;
	if(vt->cursor_y == vt->lines) {
		vt->cursor_y--;
	}

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	}
}

void VT220CursorDownN(VT220* vt, int n)
{
	for(int i = 0; i < n; i++) {
		VT220CursorDown(vt);
	}
}

void VT220CursorUp(VT220* vt)
{
	if(vt->cursor_y == vt->margin_top) {
		return;
	}

	if(vt->cursor_y > 0) {
		vt->cursor_y--;
	}

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	}
}

void VT220CursorUpN(VT220* vt, int n)
{
	for(int i = 0; i < n; i++) {
		VT220CursorUp(vt);
	}
}

void VT220CheckOriginMode(VT220* vt)
{
	if(vt->mode & DECOM) {
		vt->cursor_y += vt->margin_top;
		if(vt->cursor_y < vt->margin_top) {
			vt->cursor_y = vt->margin_top;
		}
		if(vt->cursor_y > vt->margin_bottom) {
			vt->cursor_y = vt->margin_bottom;
		}
	}
}

void VT220SetOriginMode(VT220* vt)
{
	vt->mode |= DECOM;
	VT220CheckOriginMode(vt);
}

void VT220ClearOriginMode(VT220* vt)
{
	vt->mode &= ~DECOM;
	vt->cursor_y -= vt->margin_top;
	if(vt->cursor_y < 0) {
		vt->cursor_y = 0;
	}
}

void VT220SetCursor(VT220* vt, int line, int column)
{
	int x = column - 1;
	int y = line - 1;

	if(x < 0) {
		x = 0;
	} else if(x >= vt->columns) {
		x = vt->columns - 1;
	}

	if(y < 0) {
		y = 0;
	} else if(y >= vt->lines) {
		y = vt->lines - 1;
	}

	vt->cursor_x = x;
	vt->cursor_y = y;

	VT220CheckOriginMode(vt);
}

void VT220SaveCursor(VT220* vt)
{
	vt->saved_cursor_x = vt->cursor_x;
	vt->saved_cursor_y = vt->cursor_y;
	vt->saved_sgr = vt->sgr;
	vt->saved_awm = vt->mode & DECAWM;
	vt->saved_om = vt->mode & DECOM;
	vt->saved_gl = vt->gl;
	vt->saved_gr = vt->gr;
	vt->saved_gl_lock = vt->gl_lock;
	vt->saved_gr_lock = vt->gr_lock;
	vt->saved_g[0] = vt->g[0];
	vt->saved_g[1] = vt->g[1];
	vt->saved_g[2] = vt->g[2];
	vt->saved_g[3] = vt->g[3];
}

void VT220RestoreCursor(VT220* vt)
{
	vt->cursor_x = vt->saved_cursor_x;
	vt->cursor_y = vt->saved_cursor_y;
	vt->sgr = vt->saved_sgr;
	if(vt->saved_awm) {
		vt->mode |= DECAWM;
	} else {
		vt->mode &= ~DECAWM;
	}
	if(vt->saved_om) {
		vt->mode |= DECOM;
	} else {
		vt->mode &= ~DECOM;
	}
	vt->gl = vt->gl_lock;
	vt->gr = vt->gr_lock;
	vt->gl_lock = vt->saved_gl_lock;
	vt->gr_lock = vt->saved_gr_lock;
	vt->g[0] = vt->saved_g[0];
	vt->g[1] = vt->saved_g[1];
	vt->g[2] = vt->saved_g[2];
	vt->g[3] = vt->saved_g[3];

	VT220CheckOriginMode(vt);
}

void VT220Tabstop(VT220* vt)
{
	if(VT220IsLastColumnFlag(vt)) {
		return;
	}

	while(++vt->cursor_x < vt->columns) {
		if(vt->tabstops[vt->cursor_x]) {
			break;
		}
	}

	/* Since the while loop always increments cursor_x by one,
	 * it might end up in the "last column" position. Therefore,
	 * the last column flag has to be cleared if it was set by
	 * accident.
	 */
	if(VT220IsLastColumnFlag(vt)) {
		VT220ClearLastColumnFlag(vt);
	}
}

void VT220SetTabstop(VT220* vt)
{
	if(VT220IsLastColumnFlag(vt)) {
		int tmp = vt->cursor_x;
		VT220ClearLastColumnFlag(vt);
		vt->tabstops[vt->cursor_x] = 1;
		vt->cursor_x = tmp;
	} else {
		vt->tabstops[vt->cursor_x] = 1;
	}
}

void VT220ClearTabstop(VT220* vt)
{
	vt->tabstops[vt->cursor_x] = 0;
}

void VT220ClearAllTabstops(VT220* vt)
{
	int i;
	for(i = 0; i < vt->columns; i++) {
		vt->tabstops[i] = 0;
	}
}

void VT220CarriageReturn(VT220* vt)
{
	vt->cursor_x = 0;
}

void VT220Linefeed(VT220* vt)
{
	VT220Index(vt);
}

void VT220ReverseLinefeed(VT220* vt)
{
	if(vt->cursor_y > 0) {
		vt->cursor_y--;
	} else {
		VT220ScrollDown(vt);
	}
}

void VT220NextLine(VT220* vt)
{
	VT220Index(vt);
	vt->cursor_x = 0;
}

void VT220ScrollUp(VT220* vt)
{
	int start = (vt->margin_top + 1) * vt->columns;
	int end = (vt->margin_bottom + 1) * vt->columns;
	for(int i = start; i < end; i++) {
		vt->text[i - vt->columns] = vt->text[i];
	}

	for(int i = vt->margin_top + 1; i < vt->margin_bottom; i++) {
		vt->line_attributes[i - 1] = vt->line_attributes[i];
	}

	end = vt->margin_bottom * vt->columns;
	memset(&vt->text[end], 0, vt->columns * sizeof(VT220CELL));
}

void VT220ScrollDown(VT220* vt)
{
	int start = vt->margin_top * vt->columns;
	int end = vt->margin_bottom * vt->columns;
	for(int i = end - 1; i >= start; i--) {
		vt->text[i + vt->columns] = vt->text[i];
	}

	for(int i = vt->margin_bottom - 1; i >= vt->margin_top; i--) {
		vt->line_attributes[i + 1] = vt->line_attributes[i];
	}

	memset(&vt->text[start], 0, vt->columns * sizeof(VT220CELL));
}

void VT220SetTopBottomMargins(VT220* vt, int top, int bottom)
{
	int y = vt->cursor_y;
	if(vt->mode & DECOM) {
		y -= vt->margin_top;
	}

	vt->margin_top = top - 1;
	vt->margin_bottom = bottom - 1;

	if(vt->margin_top < 0) {
		vt->margin_top = 0;
	}

	if(vt->margin_bottom < 0) {
		vt->margin_bottom = vt->lines - 1;
	}

	if(vt->margin_top > vt->margin_bottom) {
		int tmp = vt->margin_top;
		vt->margin_top = vt->margin_bottom;
		vt->margin_bottom = tmp;
	}

	if(vt->margin_bottom >= vt->lines) {
		vt->margin_bottom = vt->lines - 1;
	}

	/* manual says: "the smallest scrolling region allowed is two lines" */
	if(vt->margin_top == vt->margin_bottom) {
		vt->margin_top = 0;
		vt->margin_bottom = vt->lines - 1;
	}

	if(vt->mode & DECOM) {
		y += vt->margin_top;
		if(y >= vt->margin_bottom) {
			y = vt->margin_bottom;
		}
		vt->cursor_y = y;
	}
}

void VT220EraseScreen(VT220* vt)
{
	memset(vt->text, 0, vt->lines * vt->columns * sizeof(VT220CELL));
	memset(vt->line_attributes, 0, vt->lines);
}

void VT220InsertLine(VT220* vt)
{
	if(vt->cursor_y < vt->margin_top || vt->cursor_y > vt->margin_bottom) {
		return;
	}

	int start = vt->cursor_y * vt->columns;
	int end = vt->margin_bottom * vt->columns;
	for(int i = end - 1; i >= start; i--) {
		vt->text[i + vt->columns] = vt->text[i];
	}

	for(int i = vt->margin_bottom - 2; i >= vt->margin_top; i--) {
		vt->line_attributes[i + 1] = vt->line_attributes[i];
	}

	memset(&vt->text[start], 0, vt->columns * sizeof(VT220CELL));

	vt->cursor_x = 0;
}

void VT220InsertLineN(VT220* vt, int n)
{
	/* TODO: optimize for performance */
	int i;

	if(n == 0) {
		n = 1;
	}

	for(i = 0; i < n; i++) {
		VT220InsertLine(vt);
	}
}

void VT220DeleteLine(VT220* vt)
{
	if(vt->cursor_y < vt->margin_top || vt->cursor_y > vt->margin_bottom) {
		return;
	}

	int start = (vt->cursor_y + 1) * vt->columns;
	int end = (vt->margin_bottom + 1) * vt->columns;
	for(int i = start; i < end; i++) {
		vt->text[i - vt->columns] = vt->text[i];
	}

	for(int i = vt->margin_top + 1; i < vt->margin_bottom; i++) {
		vt->line_attributes[i - 1] = vt->line_attributes[i];
	}

	end = vt->margin_bottom * vt->columns;
	memset(&vt->text[end], 0, vt->columns * sizeof(VT220CELL));

	vt->cursor_x = 0;
}

void VT220DeleteLineN(VT220* vt, int n)
{
	/* TODO: optimize for performance */
	int i;

	if(n == 0) {
		n = 1;
	}

	for(i = 0; i < n; i++) {
		VT220DeleteLine(vt);
	}
}

void VT220InsertCharacter(VT220* vt)
{
	int i;
	int cur;
	int eol = (vt->cursor_y + 1) * vt->columns - 1;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	for(i = eol - 1; i >= cur; i--) {
		vt->text[i + 1] = vt->text[i];
	}

	vt->text[cur].text = 0;
	vt->text[cur].attr = 0;
}

void VT220InsertCharacterN(VT220* vt, int n)
{
	/* TODO: optimize for performance */
	int i;

	if(n == 0) {
		n = 1;
	}

	for(i = 0; i < n; i++) {
		VT220InsertCharacter(vt);
	}
}

void VT220DeleteCharacter(VT220* vt)
{
	int i;
	int cur;
	int eol = (vt->cursor_y + 1) * vt->columns - 1;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	for(i = cur; i < eol; i++) {
		vt->text[i] = vt->text[i + 1];
	}

	vt->text[eol].text = 0;
	vt->text[eol].attr = 0;
}

void VT220DeleteCharacterN(VT220* vt, int n)
{
	/* TODO: optimize for performance */
	int i;

	if(n == 0) {
		n = 1;
	}

	for(i = 0; i < n; i++) {
		VT220DeleteCharacter(vt);
	}
}

void VT220EraseCharacter(VT220* vt, int n)
{
	int i, start;
	int cnt = n;

	if(cnt == 0) {
		cnt = 1;
	}

	if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	if(cnt > (vt->columns - vt->cursor_x)) {
		cnt = vt->columns - vt->cursor_x;
	}

	start = vt->cursor_y * vt->columns + vt->cursor_x;

	for(i = 0; i < cnt; i++) {
		vt->text[start + i].text = 0;
		vt->text[start + i].attr = 0;
	}
}

void VT220EraseInLine(VT220* vt, int type)
{
	int i;
	int cur;
	int bol = vt->cursor_y * vt->columns;
	int eol = (vt->cursor_y + 1) * vt->columns - 1;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	switch(type) {
		case EL_TO_END:
			for(i = cur; i <= eol; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}
			break;
		case EL_FROM_BOL:
			for(i = bol; i <= cur; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}
			break;
		case EL_WHOLE_LINE:
			for(i = bol; i <= eol; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}
			break;
	}
}

void VT220EraseInDisplay(VT220* vt, int type)
{
	int i;
	int cur;
	int end = vt->lines * vt->columns;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	switch(type) {
		case ED_TO_END:
			for(i = cur; i < end; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}

			for(i = vt->cursor_y + 1; i < vt->lines; i++) {
				vt->line_attributes[i] = DECSWL;
			}
			break;
		case ED_FROM_BEGIN:
			for(i = 0; i <= cur; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}

			for(i = 0; i < vt->cursor_y; i++) {
				vt->line_attributes[i] = DECSWL;
			}
			break;
		case ED_ALL:
			for(i = 0; i < end; i++) {
				vt->text[i].text = 0;
				vt->text[i].attr = 0;
			}

			for(i = 0; i < vt->lines; i++) {
				vt->line_attributes[i] = DECSWL;
			}
			break;
	}
}

void VT220SelectiveEraseInLine(VT220* vt, int type)
{
	int i;
	int cur;
	int bol = vt->cursor_y * vt->columns;
	int eol = (vt->cursor_y + 1) * vt->columns - 1;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	switch(type) {
		case EL_TO_END:
			for(i = cur; i <= eol; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
			break;
		case EL_FROM_BOL:
			for(i = bol; i <= cur; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
			break;
		case EL_WHOLE_LINE:
			for(i = bol; i <= eol; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
			break;
	}
}

void VT220SelectiveEraseInDisplay(VT220* vt, int type)
{
	int i;
	int cur;
	int end = vt->lines * vt->columns;

	if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
		vt->cursor_x = (vt->columns - 1) / 2;
	} else if(vt->cursor_x == vt->columns) {
		vt->cursor_x--;
	}

	cur = vt->cursor_y * vt->columns + vt->cursor_x;

	switch(type) {
		case ED_TO_END:
			for(i = cur; i < end; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
			break;
		case ED_FROM_BEGIN:
			for(i = 0; i <= cur; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
			break;
		case ED_ALL:
			for(i = 0; i < end; i++) {
				if(!(vt->text[i].attr & SCA_ON)) {
					vt->text[i].text = 0;
				}
			}
	}
}

void VT220SetColumnMode(VT220* vt)
{
	vt->mode |= DECCOLM;
	vt->columns = TEXT_WIDTH_MAX;
	VT220EraseInDisplay(vt, ED_ALL);
	VT220SetCursor(vt, 1, 1);
	vt->margin_top = 0;
	vt->margin_bottom = vt->lines - 1;

	if(vt->resize) {
		vt->resize(vt->columns, vt->lines);
	}
}

void VT220ClearColumnMode(VT220* vt)
{
	vt->mode &= ~DECCOLM;
	vt->columns = TEXT_WIDTH;
	VT220EraseInDisplay(vt, ED_ALL);
	VT220SetCursor(vt, 1, 1);
	vt->margin_top = 0;
	vt->margin_bottom = vt->lines - 1;

	if(vt->resize) {
		vt->resize(vt->columns, vt->lines);
	}
}

void VT220SetVT52Mode(VT220* vt)
{
	vt->mode &= ~DECANM;
	vt->state = STATE_TEXT;
	vt->gl = 0;
	vt->gl_lock = 0;
	vt->g[0] = CHARSET_ASCII;
	vt->g[1] = CHARSET_DEC_SPECIAL_GRAPHICS;
}

void VT220SetANSIMode(VT220* vt)
{
	vt->mode |= DECANM;
	vt->state = STATE_TEXT;
	vt->gl = 0;
	vt->gl_lock = 0;

	/* go to VT100 mode */
	vt->ct_7bit = 1;
	vt->vt100_mode = 1;
}

void VT220Adjustments(VT220* vt)
{
	int i;
	for(i = 0; i < vt->lines * vt->columns; i++) {
		vt->text[i].text = 'E';
	}
}

void VT220Xoff(VT220* vt)
{
	vt->xoff = 1;
}

void VT220Xon(VT220* vt)
{
	vt->xoff = 0;
}

void VT220SetLineMode(VT220* vt, int mode)
{
	int startX = vt->columns / 2;
	int count = vt->columns - startX;

	vt->line_attributes[vt->cursor_y] = mode;

	switch(mode) {
		case DECDHL_TOP:
		case DECDHL_BOTTOM:
		case DECDWL:
			memset(&vt->text[vt->cursor_y * vt->columns + startX], 0, count * sizeof(VT220CELL));
			if(2 * vt->cursor_x >= vt->columns) {
				vt->cursor_x = (vt->columns - 1) / 2;
			}
			break;
	}
}

void VT220ClearComm(VT220* vt)
{
	vt->state = 0;

	vt->xoff = 0;
	vt->sent_xoff = 0;
	vt->hold_screen = 0;

	VT220FlowControl(vt, 1);

	vt->buf_r = 0;
	vt->buf_w = 0;
	vt->buf_used = 0;
	vt->buf_lost = 0;
}

/* DECSTR: page 129 */
void VT220SoftReset(VT220* vt)
{
	int i;
	for(i = 0; i < TEXT_WIDTH_MAX; i++) {
		vt->tabstops[i] = i % 8 == 7;
	}

	VT220ClearComm(vt);

	vt->margin_top = 0;
	vt->margin_bottom = vt->lines - 1;
	vt->udk_locked = 0;
	vt->sgr = 0;

	vt->mode |= DECTCEM;
	vt->mode &= ~(IRM | DECOM | DECAWM | KAM | DECCKM);

	vt->gl = 0;
	vt->gl_lock = 0;
	vt->gr = 2;
	vt->gr_lock = 2;
	vt->g[0] = CHARSET_ASCII;
	vt->g[1] = CHARSET_DEC_SUPPLEMENTAL;
	vt->g[2] = CHARSET_DEC_SUPPLEMENTAL;
	vt->g[3] = CHARSET_DEC_SUPPLEMENTAL;

	vt->sgr = 0;

	/* VT220SaveCursor */
	vt->saved_cursor_x = 0;
	vt->saved_cursor_y = 0;
	vt->saved_sgr = 0;
	vt->saved_awm = vt->mode & DECAWM;
	vt->saved_om = vt->mode & DECOM;
	vt->saved_gl = vt->gl;
	vt->saved_gr = vt->gr;
	vt->saved_gl_lock = vt->gl_lock;
	vt->saved_gr_lock = vt->gr_lock;
	vt->saved_g[0] = vt->g[0];
	vt->saved_g[1] = vt->g[1];
	vt->saved_g[2] = vt->g[2];
	vt->saved_g[3] = vt->g[3];
}

/* RIS */
void VT220HardReset(VT220* vt)
{
	unsigned int old_columns = vt->columns;

	vt->columns = TEXT_WIDTH;
	vt->mode = 0;

	VT220SoftReset(vt);

	vt->state = 0;
	vt->mode = 0;
	vt->ct_7bit = vt->config.mode != VT220_MODE_VT200_MODE_8BIT_CONTROLS;

	if(vt->config.mode != VT220_MODE_VT52_MODE) {
		vt->mode |= DECANM;
	}

	switch(vt->config.mode) {
		case VT220_MODE_VT200_MODE_7BIT_CONTROLS:
		case VT220_MODE_VT200_MODE_8BIT_CONTROLS:
		case VT220_MODE_VT52_MODE:
			vt->vt100_mode = 0;
			break;
		case VT220_MODE_VT100_MODE:
			vt->vt100_mode = 1;
			break;
	}

	if(vt->config.auto_wrap) {
		vt->mode |= DECAWM;
	} else {
		vt->mode &= ~DECAWM;
	}

	if(vt->config.text_cursor == VT220_TEXT_CURSOR) {
		vt->mode |= DECTCEM;
	} else {
		vt->mode &= ~DECTCEM;
	}

	if(vt->config.new_line == VT220_NEW_LINE) {
		vt->mode |= LNM;
	} else {
		vt->mode &= ~LNM;
	}

	if(vt->config.local_echo == VT220_NO_LOCAL_ECHO) {
		vt->mode |= SRM;
	} else {
		vt->mode &= ~SRM;
	}

	if(vt->config.auto_repeat == VT220_AUTO_REPEAT) {
		vt->mode |= DECARM;
	} else {
		vt->mode &= DECARM;
	}

	vt->xoff = 0;
	vt->xoff_point = 64;
	vt->xon_point = 16;
	vt->use_xoff = 1;

	vt->margin_top = 0;
	vt->margin_bottom = TEXT_HEIGHT - 1;

	vt->udk_locked = 0;
	VT220ClearAllUDK(vt);

	vt->gl = 0;
	vt->gl_lock = 0;
	vt->gr = 2;
	vt->gr_lock = 2;
	vt->g[0] = CHARSET_ASCII;
	vt->g[1] = CHARSET_DEC_SUPPLEMENTAL;
	vt->g[2] = CHARSET_DEC_SUPPLEMENTAL;
	vt->g[3] = CHARSET_DEC_SUPPLEMENTAL;

	/* clear DRCS */
	memset(vt->drcs_name, 0, 3);
	vt->drcs_name[0] = ' '; /* set it to the default unregistered */
	vt->drcs_name[1] = '@'; /* character set */
	VT220ClearDRCS(vt);

	VT220CursorHome(vt);
	VT220EraseScreen(vt);
	VT220SaveCursor(vt);

	/* TODO: implement properly */
	memset(vt->answerback, 0, 31);

	if(vt->resize && vt->columns != old_columns) {
		vt->resize(vt->columns, vt->lines);
	}
}

unsigned int VT220GetUDKNumber(unsigned int key)
{
	switch(key) {
		case 17:
			return 0;
		case 18:
			return 1;
		case 19:
			return 2;
		case 20:
			return 3;
		case 21:
			return 4;
		case 23:
			return 5;
		case 24:
			return 6;
		case 25:
			return 7;
		case 26:
			return 8;
		case 28:
			return 9;
		case 29:
			return 10;
		case 31:
			return 11;
		case 32:
			return 12;
		case 33:
			return 13;
		case 34:
			return 14;
		default:
			return 0xFF;
	}
}

void VT220ClearUDK(VT220* vt, unsigned int key)
{
	if(key >= 15) {
		return;
	}

	if(vt->udk_len[key]) {
		u16 ptr = vt->udk_ptr[key];
		u16 len = vt->udk_len[key];
		u16 end = ptr + len;

		/* compact */
		memmove(&vt->udk_memory[ptr], &vt->udk_memory[end], 256 - end);

		/* update pointers */
		for(unsigned int i = 0; i < 15; i++) {
			if(vt->udk_len[key] && vt->udk_ptr[key] > ptr) {
				vt->udk_ptr[key] -= ptr;
			}
		}

		vt->udk_ptr[key] = 0;
		vt->udk_len[key] = 0;
		vt->udk_free += len;

		assert(vt->udk_free <= 256);
	}
}

void VT220ClearAllUDK(VT220* vt)
{
	vt->udk_free = 256;
	memset(vt->udk_ptr, 0, 15);
	memset(vt->udk_len, 0, 15 * sizeof(u16));
	memset(vt->udk_memory, 0, 256);
}

void VT220ClearDRCS(VT220* vt)
{
	for(unsigned int i = 0; i < 94; i++) {
		memcpy(&vt->drcs[i * 10], &vt220font[0x20 * 10], 10);
	}
	vt->drcs_dirty = 1;
}

void VT220Substitute(VT220* vt)
{
	VT220Write(vt, 0x20);
}

void VT220Index(VT220* vt)
{
	if(vt->cursor_y == vt->margin_bottom) {
		VT220ScrollUp(vt);
	} else {
		vt->cursor_y++;
		if(vt->cursor_y == vt->lines) {
			vt->cursor_y--;
		}
	}
}

void VT220ReverseIndex(VT220* vt)
{
	if(vt->cursor_y == vt->margin_top) {
		VT220ScrollDown(vt);
	} else if(vt->cursor_y > 0) {
		vt->cursor_y--;
	}
}

void VT220Destroy(VT220* vt)
{
	free(vt->buf);
	free(vt->tabstops);
	free(vt->text);
	free(vt->drcs);
}

void VT220Bell(VT220* vt)
{
	if(vt->config.bell == VT220_BELL && vt->bell) {
		vt->bell();
	}
}

void VT220Keyclick(VT220* vt)
{
	if(vt->config.keyclick == VT220_KEYCLICK && vt->keyclick) {
		vt->bell();
	}
}

void VT220Send(VT220* vt, unsigned char c)
{
	if(vt->rx) {
		if(vt->ct_7bit && c >= 0x80 && c <= 0x9F) {
			vt->rx(0x1b);
			vt->rx(c - 0x40);
		} else {
			vt->rx(c);
		}
	}
}

void VT220SendText(VT220* vt, char* c)
{
	for(; *c; c++) {
		VT220Send(vt, (unsigned char) *c);
	}
}

void VT220SendDecimal(VT220* vt, int x)
{
	int i;
	char buf[8];

	buf[6] = '0';
	buf[7] = 0;

	for(i = 6; i >= 0 && x > 0; i--, x /= 10) {
		buf[i] = (x % 10) + '0';
	}

	i++;
	if(i == 7) {
		i = 6;
	}

	VT220SendText(vt, &buf[i]);
}

void VT220SendInput(VT220* vt, unsigned char c)
{
	if(vt->config.local) {
		VT220Receive(vt, c);
	} else {
		if(vt->rx) {
			if(vt->ct_7bit && c >= 0x80 && c <= 0x9F) {
				vt->rx(0x1b);
				vt->rx(c - 0x40);
			} else {
				vt->rx(c);
			}
		}

		if(!(vt->mode & SRM)) {
			VT220Receive(vt, c);
		}
	}
}

void VT220SendAnswerback(VT220* vt)
{
	VT220SendText(vt, vt->answerback);
}

void VT220SendPrimaryDA(VT220* vt)
{
	if(vt->vt100_mode) {
		switch(vt->config.vt100_terminal_id) {
			case VT220_VT100_TERMINAL_ID_VT220:
				VT220SendText(vt, "\x9b?62;1;2;6;7;8c");
				break;
			case VT220_VT100_TERMINAL_ID_VT100:
				VT220SendText(vt, "\x1b[?1;2c");
				break;
			case VT220_VT100_TERMINAL_ID_VT101:
				VT220SendText(vt, "\x1b[?1;0c");
				break;
			case VT220_VT100_TERMINAL_ID_VT102:
				VT220SendText(vt, "\x1b[?6c");
				break;
		}
	} else {
		VT220SendText(vt, "\x9b?62;1;2;6;7;8c");
	}
}

void VT220SendSecondaryDA(VT220* vt)
{
	/* version 2.3, no options */
	VT220SendText(vt, "\x9b>1;23;0c");
}

void VT220IdentifyVT52(VT220* vt)
{
	VT220SendText(vt, "\x1b/Z");
}

void VT220ProcessCharVT220(VT220* vt, unsigned char c)
{
	int tmp;
	switch(vt->state) {
		case STATE_TEXT:
			switch(c) {
				case NUL:
					break;
				case ENQ:
					VT220SendAnswerback(vt);
					break;
				case BEL:
					VT220Bell(vt);
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case HT:
					VT220Tabstop(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case SO:
					VT220ShiftOut(vt);
					break;
				case SI:
					VT220ShiftIn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					break;
				case DC3:
					VT220Xoff(vt);
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case ESC:
					vt->state = STATE_ESC;
					break;
				case DEL:
					break;
				case IND:
					VT220Index(vt);
					break;
				case NEL:
					VT220NextLine(vt);
					break;
				case HTS:
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						VT220SetTabstop(vt);
					}
					break;
				case RI:
					VT220ReverseIndex(vt);
					break;
				case SS2:
					VT220SingleShiftG2(vt);
					break;
				case SS3:
					VT220SingleShiftG3(vt);
					break;
				case DCS:
					vt->state = STATE_DCS;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case ST:
					break;
				default:
					if((c >= 0x20 && c < 0x80) || (c >= 0xA0 && c <= 0xFF)) {
						VT220WriteChar(vt, c);
						break;
					}
			}
			break;
		case STATE_ESC:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '(':
					vt->state = STATE_G;
					vt->g_dst = 0;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case ')':
					vt->state = STATE_G;
					vt->g_dst = 1;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case '*':
					if(vt->vt100_mode) {
						vt->state = STATE_TEXT;
					} else {
						vt->state = STATE_G;
						vt->g_dst = 2;
						vt->parameter_id = 0;
						memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					}
					break;
				case '+':
					if(vt->vt100_mode) {
						vt->state = STATE_TEXT;
					} else {
						vt->state = STATE_G;
						vt->g_dst = 3;
						vt->parameter_id = 0;
						memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					}
					break;
				case '~': /* LS1R */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						vt->gr      = 1;
						vt->gr_lock = 1;
					}
					break;
				case 'n': /* LS2 */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						vt->gl      = 2;
						vt->gl_lock = 2;
					}
					break;
				case '}': /* LS2R */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						vt->gr      = 2;
						vt->gr_lock = 2;
					}
					break;
				case 'o': /* LS3 */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						vt->gl      = 3;
						vt->gl_lock = 3;
					}
					break;
				case '|': /* LS3R */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						vt->gr      = 3;
						vt->gr_lock = 3;
					}
					break;
				case ' ':
					vt->state = STATE_ESC_SP;
					break;
				case 'Z':
					vt->state= STATE_TEXT;
					VT220SendPrimaryDA(vt);
					break;
				case '=':
					vt->state= STATE_TEXT;
					vt->mode |= DECKPAM;
					break;
				case '>':
					vt->state= STATE_TEXT;
					vt->mode &= ~DECKPAM;
					break;
				case '7': /* DECSC */
					vt->state = STATE_TEXT;
					VT220SaveCursor(vt);
					break;
				case '8': /* DECRC */
					vt->state = STATE_TEXT;
					VT220RestoreCursor(vt);
					break;
				case 'H': /* HTS */
					vt->state = STATE_TEXT;
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						VT220SetTabstop(vt);
					}
					break;
				case '#':
					vt->state = STATE_ESC_HASH;
					break;
				case 'c': /* RIS */
					vt->state = STATE_TEXT;
					VT220HardReset(vt);
					break;
				default:
					vt->state = STATE_TEXT;
					if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
						VT220ProcessCharVT220(vt, c + 0x40);
					}
			}
			break;
		case STATE_G:
			vt->state = STATE_TEXT;
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
					if(c >= 0x20 && c <= 0x2F) {
						/* intermediate */
						if(vt->parameter_id < 2) {
							vt->state = STATE_G;
							vt->parameters[vt->parameter_id++] = c;
						}
					} else if(c >= 0x30 && c <= 0x7E) {
						/* final */
						vt->parameters[vt->parameter_id] = c;
						BOOL is_drcs = TRUE;
						for(int i = 0; i < 3; i++) {
							if(vt->parameters[i] != vt->drcs_name[i]) {
								is_drcs = FALSE;
								break;
							}
						}
						if(is_drcs) {
							vt->g[vt->g_dst] = CHARSET_DRCS;
						}
					}
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case 'B':
					vt->g[vt->g_dst] = CHARSET_ASCII;
					break;
				case '<':
					vt->g[vt->g_dst] = CHARSET_DEC_SUPPLEMENTAL;
					break;
				case '0':
					vt->g[vt->g_dst] = CHARSET_DEC_SPECIAL_GRAPHICS;
					break;
				case 'A':
					if(vt->vt100_mode) {
						vt->g[vt->g_dst] = CHARSET_NRCS_BRITISH;
					}
					break;
			}
			break;
		case STATE_ESC_SP:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case 'F': /* S7C1T */
					vt->state = STATE_TEXT;
					vt->ct_7bit = 1;
					break;
				case 'G': /* S8C1T */
					vt->state = STATE_TEXT;
					vt->ct_7bit = vt->vt100_mode;
					break;
			}
			break;
		case STATE_CSI:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case NUL:
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->parameters[vt->parameter_id] *= 10;
					vt->parameters[vt->parameter_id] += c - '0';
					break;
				case ';':
					vt->parameter_id++;
					if(vt->parameter_id == MAX_PARAMETERS) {
						vt->parameter_id--;
						vt->parameters[vt->parameter_id] = 0;
					}
					break;
				case '?':
					vt->state = STATE_DECPRIV;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case 'c':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						VT220SendPrimaryDA(vt);
					}
					break;
				case 'n':
					vt->state = STATE_TEXT;
					switch(vt->parameters[0]) {
						case 5:
							VT220Send(vt, CSI);
							VT220SendText(vt, "0n");
							break;
						case 6:
							tmp = vt->cursor_x;
							if(tmp == vt->columns) {
								tmp--;
							} else if(vt->line_attributes[vt->cursor_y] != DECSWL && 2 * vt->cursor_x >= vt->columns) {
								tmp = (vt->columns - 1) / 2;
							}
							VT220Send(vt, CSI);
							if(vt->mode & DECOM) {
								VT220SendDecimal(vt, vt->cursor_y + 1 - vt->margin_top);
							} else {
								VT220SendDecimal(vt, vt->cursor_y + 1);
							}
							VT220Send(vt, ';');
							VT220SendDecimal(vt, tmp + 1);
							VT220Send(vt, 'R');
							break;
					}
					break;
				case '>':
					vt->state = STATE_CSI_GT;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				case 'h':
					for(tmp = 0; tmp <= vt->parameter_id; tmp++) {
						int param = vt->parameters[tmp];
						switch(param) {
							case 2:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode |= KAM;
								}
								break;
							case 4:
								vt->mode |= IRM;
								break;
							case 12:
								vt->mode |= SRM;
								break;
							case 20:
								vt->mode |= LNM;
								break;
							default:
								tmp = MAX_PARAMETERS;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
				case 'l':
					for(tmp = 0; tmp <= vt->parameter_id; tmp++) {
						int param = vt->parameters[tmp];
						switch(param) {
							case 2:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode &= ~KAM;
								}
								break;
							case 4:
								vt->mode &= ~IRM;
								break;
							case 12:
								vt->mode &= ~SRM;
								break;
							case 20:
								vt->mode &= ~LNM;
								break;
							default:
								tmp = MAX_PARAMETERS;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
				case 'A':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						vt->parameters[0] = 1;
					}
					VT220CursorUpN(vt, vt->parameters[0]);
					break;
				case 'B':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						vt->parameters[0] = 1;
					}
					VT220CursorDownN(vt, vt->parameters[0]);
					break;
				case 'C':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						vt->parameters[0] = 1;
					}
					VT220CursorRightN(vt, vt->parameters[0]);
					break;
				case 'D':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						vt->parameters[0] = 1;
					}
					VT220CursorLeftN(vt, vt->parameters[0]);
					break;
				case 'H':
				case 'f':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						vt->parameters[0] = 1;
					}
					if(vt->parameters[1] == 0) {
						vt->parameters[1] = 1;
					}
					VT220SetCursor(vt, vt->parameters[0], vt->parameters[1]);
					break;
				case 'g': /* TBC */
					vt->state = STATE_TEXT;
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						switch(vt->parameters[0]) {
							case 0:
								VT220ClearTabstop(vt);
								break;
							case 3:
								VT220ClearAllTabstops(vt);
								break;
						}
					}
					break;
				case 'm': /* SGR */
					for(tmp = 0; tmp <= vt->parameter_id; tmp++) {
						int param = vt->parameters[tmp];
						switch(param) {
							case 0:
								vt->sgr &= ~(SGR_BOLD | SGR_UNDERSCORE | SGR_BLINKING | SGR_REVERSE);
								break;
							case 1:
								vt->sgr |= SGR_BOLD;
								break;
							case 4:
								vt->sgr |= SGR_UNDERSCORE;
								break;
							case 5:
								vt->sgr |= SGR_BLINKING;
								break;
							case 7:
								vt->sgr |= SGR_REVERSE;
								break;
							case 22:
								vt->sgr &= ~SGR_BOLD;
								break;
							case 24:
								vt->sgr &= ~SGR_UNDERSCORE;
								break;
							case 25:
								vt->sgr &= ~SGR_BLINKING;
								break;
							case 27:
								vt->sgr &= ~SGR_REVERSE;
								break;
							default:
								tmp = MAX_PARAMETERS;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
				case '"':
					vt->state = STATE_CSI_QUOT;
					break;
				case 'L': /* IL */
					vt->state = STATE_TEXT;
					VT220InsertLineN(vt, vt->parameters[0]);
					break;
				case 'M': /* DL */
					vt->state = STATE_TEXT;
					VT220DeleteLineN(vt, vt->parameters[0]);
					break;
				case '@': /* ICH */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						VT220InsertCharacterN(vt, vt->parameters[0]);
					}
					break;
				case 'P': /* DCH */
					vt->state = STATE_TEXT;
					VT220DeleteCharacterN(vt, vt->parameters[0]);
					break;
				case 'X': /* ECH */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						VT220EraseCharacter(vt, vt->parameters[0]);
					}
					break;
				case 'K': /* EL */
					vt->state = STATE_TEXT;
					VT220EraseInLine(vt, vt->parameters[0]);
					break;
				case 'J': /* ED */
					vt->state = STATE_TEXT;
					VT220EraseInDisplay(vt, vt->parameters[0]);
					break;
				case 'r': /* DECSTBM */
					vt->state = STATE_TEXT;
					VT220SetTopBottomMargins(vt, vt->parameters[0], vt->parameters[1]);
					break;
				case 'R': /* report cursor position result; ignore */
					vt->state = STATE_TEXT;
					break;
				case '!':
					vt->state = STATE_CSI_EXCL;
					break;
			}
			break;
		case STATE_CSI_GT:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->parameters[vt->parameter_id] *= 10;
					vt->parameters[vt->parameter_id] += c - '0';
					break;
				case ';':
					vt->parameter_id++;
					if(vt->parameter_id == MAX_PARAMETERS) {
						vt->parameter_id--;
						vt->parameters[vt->parameter_id] = 0;
					}
					break;
				case 'c':
					vt->state = STATE_TEXT;
					if(vt->parameters[0] == 0) {
						VT220SendSecondaryDA(vt);
					}
					break;
			}
			break;
		case STATE_DECPRIV:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->parameters[vt->parameter_id] *= 10;
					vt->parameters[vt->parameter_id] += c - '0';
					break;
				case ';':
					vt->parameter_id++;
					if(vt->parameter_id == MAX_PARAMETERS) {
						vt->parameter_id--;
						vt->parameters[vt->parameter_id] = 0;
					}
					break;
				case 'h':
					for(tmp = 0; tmp <= vt->parameter_id; tmp++) {
						int param = vt->parameters[tmp];
						switch(param) {
							case 1:
								vt->mode |= DECCKM;
								break;
							case 3:
								VT220SetColumnMode(vt);
								break;
							case 4:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode |= DECSCLM;
								}
								break;
							case 5:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode |= DECSCNM;
								}
								break;
							case 6:
								VT220SetOriginMode(vt);
								break;
							case 7:
								vt->mode |= DECAWM;
								break;
							case 8:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode |= DECARM;
								}
								break;
							case 18:
								vt->mode |= DECPFF;
								break;
							case 19:
								vt->mode |= DECPEX;
								break;
							case 25:
								vt->mode |= DECTCEM;
								break;
							case 42:
								vt->mode |= DECNRCM;
								break;
							case 43:
								vt->mode |= DECGEPM;
								break;
							case 44:
								vt->mode |= DECGPCM;
								break;
							case 45:
								vt->mode |= DECGPCS;
								break;
							case 46:
								vt->mode |= DECGPBM;
								break;
							case 47:
								vt->mode |= DECGRPM;
								break;
							default:
								tmp = MAX_PARAMETERS;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
				case 'l':
					for(tmp = 0; tmp <= vt->parameter_id; tmp++) {
						int param = vt->parameters[tmp];
						switch(param) {
							case 1:
								vt->mode &= ~DECCKM;
								break;
							case 2:
								VT220SetVT52Mode(vt);
								break;
							case 3:
								VT220ClearColumnMode(vt);
								break;
							case 4:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode &= ~DECSCLM;
								}
								break;
							case 5:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode &= ~DECSCNM;
								}
								break;
							case 6:
								VT220ClearOriginMode(vt);
								break;
							case 7:
								vt->mode &= ~DECAWM;
								break;
							case 8:
								if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
									vt->mode &= ~DECARM;
								}
								break;
							case 18:
								vt->mode &= ~DECPFF;
								break;
							case 19:
								vt->mode &= ~DECPEX;
								break;
							case 25:
								vt->mode &= ~DECTCEM;
								break;
								break;
							case 42:
								vt->mode &= ~DECNRCM;
								break;
							case 43:
								vt->mode &= ~DECGEPM;
								break;
							case 44:
								vt->mode &= ~DECGPCM;
								break;
							case 45:
								vt->mode &= ~DECGPCS;
								break;
							case 46:
								vt->mode &= ~DECGPBM;
								break;
							case 47:
								vt->mode &= ~DECGRPM;
								break;
							default:
								tmp = MAX_PARAMETERS;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
				case 'J': /* DECSEL */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						VT220SelectiveEraseInLine(vt, vt->parameters[0]);
					}
					break;
				case 'K': /* DECSED */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						VT220SelectiveEraseInDisplay(vt, vt->parameters[0]);
					}
					break;
				case 'c':
					/* ignore */
					vt->state = STATE_TEXT;
					break;
				case 'n':
					vt->state = STATE_TEXT;
					switch(vt->parameters[0]) {
						case 15:
							VT220SendText(vt, "\x9b?13n"); /* no printer */
							break;
						case 25:
							if(vt->udk_locked) {
								VT220SendText(vt, "\x9b?21n");
							} else {
								VT220SendText(vt, "\x9b?20n");
							}
							break;
						case 26:
							VT220SendText(vt, "\x9b?27;");
							VT220SendDecimal(vt, vt->config.keyboard);
							VT220Send(vt, 'n');
							break;
					}
					break;
			}
			break;
		case STATE_CSI_QUOT:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case 'p':
					switch(vt->parameters[0]) {
						case 61: /* terminal level 1 */
							vt->ct_7bit = 1;
							vt->vt100_mode = 1;
							break;
						case 62: /* terminal level 2 */
							vt->vt100_mode = 0;
							switch(vt->parameters[1]) {
								case 0:
								case 2:
									vt->ct_7bit = 0;
									break;
								default:
								case 1:
									/* VT200, 7bit */
									vt->ct_7bit = 1;
									break;
							}
							break;
					}
					vt->state = STATE_TEXT;
					break;
				case 'q': /* DECSCA */
					if(!vt->vt100_mode) {
						switch(vt->parameters[0]) {
							case 0:
							case 2:
								vt->sgr &= ~SCA_ON;
								break;
							case 1:
								vt->sgr |= SCA_ON;
								break;
						}
					}
					vt->state = STATE_TEXT;
					break;
			}
			break;
		case STATE_CSI_EXCL:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case 'p': /* DECSTR */
					vt->state = STATE_TEXT;
					if(!vt->vt100_mode) {
						VT220SoftReset(vt);
					}
					break;
			}
			break;
		case STATE_ESC_HASH:
			switch(c) {
				case ESC:
					vt->state = STATE_ESC;
					break;
				case CSI:
					vt->state = STATE_CSI;
					vt->parameter_id = 0;
					memset(vt->parameters, 0, MAX_PARAMETERS * sizeof(u16));
					break;
				default:
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '3':
					VT220SetLineMode(vt, DECDHL_TOP);
					vt->state = STATE_TEXT;
					break;
				case '4':
					VT220SetLineMode(vt, DECDHL_BOTTOM);
					vt->state = STATE_TEXT;
					break;
				case '5':
					VT220SetLineMode(vt, DECSWL);
					vt->state = STATE_TEXT;
					break;
				case '6':
					VT220SetLineMode(vt, DECDWL);
					vt->state = STATE_TEXT;
					break;
				case '8':
					VT220Adjustments(vt);
					vt->state = STATE_TEXT;
					break;
			}
			break;
		case STATE_DCS:
			switch(c) {
				case ESC:
					vt->state = STATE_DCS_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '|':
					if(!vt->vt100_mode) {
						if(vt->udk_locked || vt->udk_free == 0) {
							vt->state = STATE_TEXT;
						} else {
							if(vt->parameters[0] == 0) {
								VT220ClearAllUDK(vt);
							}
							vt->udk_key = 0;
							vt->udk_hex = 0;
							vt->state = STATE_DECUDK;
						}
					}
					break;
				case '{':
					if(!vt->vt100_mode) {
						if(vt->parameters[2] == 0 || vt->parameters[2] == 2) {
							VT220ClearDRCS(vt);
						}
						vt->state = STATE_DECDLD;
						memset(vt->decdld_dscs, 0, sizeof(vt->decdld_dscs));
						vt->decdld_dscs_pos = 0;
					}
					break;
				case ST:
					vt->state = STATE_TEXT;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->parameters[vt->parameter_id] *= 10;
					vt->parameters[vt->parameter_id] += c - '0';
					break;
				case ';':
					vt->parameter_id++;
					if(vt->parameter_id == MAX_PARAMETERS) {
						vt->parameter_id--;
						vt->parameters[vt->parameter_id] = 0;
					}
					break;
				default:
					vt->state = STATE_DCS_UNK;
					break;
			}
			break;
		case STATE_DCS_UNK:
			switch(c) {
				case ESC:
					vt->state = STATE_DCS_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case ST:
					vt->state = STATE_TEXT;
					break;
				default:
					break;
			}
			break;
		case STATE_DCS_ESC:
			switch(c) {
				case ESC:
					vt->state = STATE_DCS_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case ST:
					vt->state = STATE_TEXT;
					break;
				default:
					vt->state = STATE_DCS;
					if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
						VT220ProcessCharVT220(vt, c + 0x40);
					}
					break;
			}
			break;
		case STATE_DECUDK:
			switch(c) {
				case ESC:
					vt->state = STATE_DECUDK_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->udk_key = (vt->udk_key * 10) + (c - '0');
					break;
				case '/':
					if(VT220GetUDKNumber(vt->udk_key) >= 15) {
						/* unrecognized key number, act
						 * as if it was valid but
						 * ignore the definition */
						vt->udk_key = 15;
						vt->state = STATE_DECUDK_ODD;
					} else {
						u16 id = VT220GetUDKNumber(vt->udk_key);
						vt->udk_key = id;
						VT220ClearUDK(vt, vt->udk_key);
						vt->udk_ptr[id] = 256 - vt->udk_free;
						vt->udk_hex = 0;
						vt->state = STATE_DECUDK_ODD;
					}
					break;
				case ST:
					vt->udk_locked = vt->parameters[1] == 0;
					vt->state = STATE_TEXT;
					break;
				default:
					vt->state = STATE_TEXT;
					vt->udk_locked = 1;
					break;
			}
			break;
		case STATE_DECUDK_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECUDK;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
				vt->udk_locked = 1;
			}
			break;
		case STATE_DECUDK_ODD:
			switch(c) {
				case ESC:
					vt->state = STATE_DECUDK_ODD_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->udk_hex = c - '0';
					vt->state = STATE_DECUDK_EVEN;
					break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					vt->udk_hex = c - 'A' + 10;
					vt->state = STATE_DECUDK_EVEN;
					break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					vt->udk_hex = c - 'a' + 10;
					vt->state = STATE_DECUDK_EVEN;
					break;
				case ';':
					vt->udk_key = 0;
					vt->state = STATE_DECUDK;
					break;
				case ST:
					vt->udk_locked = vt->parameters[1] == 0;
					vt->state = STATE_TEXT;
					break;
				default:
					vt->state = STATE_TEXT;
					vt->udk_locked = 1;
					break;
			}
			break;
		case STATE_DECUDK_ODD_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECUDK_ODD;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
				vt->udk_locked = 1;
			}
			break;
		case STATE_DECUDK_EVEN:
			switch(c) {
				case ESC:
					vt->state = STATE_DECUDK_EVEN_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->udk_hex = (vt->udk_hex << 4) | (c - '0');
					if(vt->udk_free != 0) {
						unsigned int key = vt->udk_key;
						if(key < 15) {
							vt->udk_memory[vt->udk_ptr[key] + vt->udk_len[key]] = (u8) vt->udk_hex;
							vt->udk_len[key]++;
							vt->udk_free--;
						}
						vt->state = STATE_DECUDK_ODD;
					} else {
						vt->state = STATE_TEXT;
						vt->udk_locked = 1;
					}
					break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					vt->udk_hex = (vt->udk_hex << 4) | (c - 'A' + 10);
					if(vt->udk_free != 0) {
						unsigned int key = vt->udk_key;
						if(key < 15) {
							vt->udk_memory[vt->udk_ptr[key] + vt->udk_len[key]] = (u8) vt->udk_hex;
							vt->udk_len[key]++;
							vt->udk_free--;
						}
						vt->state = STATE_DECUDK_ODD;
					} else {
						vt->state = STATE_TEXT;
						vt->udk_locked = 1;
					}
					break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					vt->udk_hex = (vt->udk_hex << 4) | (c - 'a' + 10);
					if(vt->udk_free != 0) {
						unsigned int key = vt->udk_key;
						if(key < 15) {
							vt->udk_memory[vt->udk_ptr[key] + vt->udk_len[key]] = (u8) vt->udk_hex;
							vt->udk_len[key]++;
							vt->udk_free--;
						}
						vt->state = STATE_DECUDK_ODD;
					} else {
						vt->state = STATE_TEXT;
						vt->udk_locked = 1;
					}
					break;
				case ST:
					vt->udk_locked = vt->parameters[1] == 0;
					vt->state = STATE_TEXT;
					break;
				case ';':
				default:
					vt->state = STATE_TEXT;
					vt->udk_locked = 1;
					break;
			}
			break;
		case STATE_DECUDK_EVEN_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECUDK_EVEN;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
				vt->udk_locked = 1;
			}
			break;
		case STATE_DECDLD:
			switch(c) {
				case ESC:
					vt->state = STATE_DECDLD_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case ST:
					vt->state = STATE_TEXT;
					break;
				default:
					if(c >= 0x20 && c <= 0x2F) {
						/* intermediate */
						if(vt->decdld_dscs_pos < 2) {
							vt->decdld_dscs[vt->decdld_dscs_pos++] = c;
						} else {
							vt->state = STATE_DECDLD_ERR;
						}
					} else if(c >= 0x30 && c <= 0x7E) {
						/* final */
						vt->decdld_dscs[vt->decdld_dscs_pos++] = c;
						vt->state = STATE_DECDLD_GLYPH;
						vt->decdld_glyph = vt->parameters[1];
						vt->decdld_row = 0;
						vt->decdld_col = 0;
					} else {
						vt->state = STATE_DECDLD_ERR;
					}
					break;
			}
			break;
		case STATE_DECDLD_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECDLD;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
			}
			break;
		case STATE_DECDLD_GLYPH:
			switch(c) {
				case ESC:
					vt->state = STATE_DECDLD_GLYPH_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case ST:
					vt->state = STATE_TEXT;
					memcpy(vt->drcs_name, vt->decdld_dscs, sizeof(vt->decdld_dscs));
					break;
				case '/':
					/* newline */
					if(vt->decdld_row < 2) {
						vt->decdld_row++;
					}
					vt->decdld_col = 0;
					break;
				case ';':
					/* next glyph */
					if(vt->decdld_glyph < 96) {
						vt->decdld_glyph++;
					}
					vt->decdld_row = 0;
					vt->decdld_col = 0;
					break;
				default:
					if(c >= '?' && c <= '~') {
						u8 bits = c - '?';
						if(vt->decdld_row < 2 && vt->decdld_col < 8 && vt->decdld_glyph >= 1
								&& vt->decdld_glyph <= 94) {
							vt->drcs_dirty = 1;
							u8* glyph = &vt->drcs[(vt->decdld_glyph - 1) * 10];
							if(vt->decdld_row == 0  && vt->decdld_col == 0) {
								memset(glyph, 0, 10);
							}

							u8* glyph_bits = &glyph[vt->decdld_row * 6];

							unsigned int max;
							if(vt->decdld_row == 0) {
								max = 6;
							} else {
								max = 4;
							}

							for(unsigned int i = 0; i < max; i++) {
								if(bits & 1) {
									glyph_bits[i] |= 1 << (7 - vt->decdld_col);
								}
								bits >>= 1;
							}

							vt->decdld_col++;
						}
					} else {
						vt->state = STATE_DECDLD_ERR;
					}
					break;
			}
			break;
		case STATE_DECDLD_GLYPH_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECDLD_GLYPH;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
			}
			break;
		case STATE_DECDLD_ERR:
			switch(c) {
				case ESC:
					vt->state = STATE_DECDLD_ERR_ESC;
					break;
				case CAN:
					vt->state = STATE_TEXT;
					break;
				case SUB:
					vt->state = STATE_TEXT;
					VT220Substitute(vt);
					break;
				case ST:
					vt->state = STATE_TEXT;
					break;
				default:
					break;
			}
			break;
		case STATE_DECDLD_ERR_ESC:
			if((c + 0x40) >= 0x80 && (c + 0x40) < 0xA0) {
				vt->state = STATE_DECDLD_ERR;
				VT220ProcessCharVT220(vt, c + 0x40);
			} else {
				vt->state = STATE_TEXT;
			}
			break;
	}
}

void VT220ProcessCharVT52(VT220* vt, unsigned char c)
{
	switch(vt->state) {
		case STATE_TEXT:
			switch(c) {
				case NUL:
					break;
				case ENQ:
					VT220SendAnswerback(vt);
					break;
				case BEL:
					VT220Bell(vt);
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case HT:
					VT220Tabstop(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					break;
				case DC3:
					VT220Xoff(vt);
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case ESC:
					vt->state = STATE_ESC;
					break;
				case DEL:
					break;
				case IND:
					VT220Index(vt);
					break;
				case NEL:
					VT220NextLine(vt);
					break;
				case HTS:
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						VT220SetTabstop(vt);
					}
					break;
				case RI:
					VT220ReverseIndex(vt);
					break;
				case SS2:
					VT220SingleShiftG2(vt);
					break;
				case SS3:
					VT220SingleShiftG3(vt);
					break;
				default:
					if(c >= 0x20 && c < 0x7F) {
						VT220WriteChar(vt, c);
					}
					break;
			}
			break;
		case STATE_ESC:
			vt->state = STATE_TEXT;
			switch(c) {
				case NUL:
					break;
				case ENQ:
					VT220SendAnswerback(vt);
					break;
				case BEL:
					VT220Bell(vt);
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case HT:
					VT220Tabstop(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					break;
				case DC3:
					VT220Xoff(vt);
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case ESC:
					vt->state = STATE_ESC;
					break;
				case DEL:
					break;
				case IND:
					VT220Index(vt);
					break;
				case NEL:
					VT220NextLine(vt);
					break;
				case 'A':
					VT220CursorUp(vt);
					break;
				case 'B':
					VT220CursorDown(vt);
					break;
				case 'C':
					VT220CursorRight(vt);
					break;
				case 'D':
					VT220CursorLeft(vt);
					break;
				case 'F':
					vt->gl = 1;
					vt->gl_lock = 1;
					break;
				case 'G':
					vt->gl = 0;
					vt->gl_lock = 0;
					break;
				case 'H':
					VT220CursorHome(vt);
					break;
				case 'I':
					VT220ReverseLinefeed(vt);
					break;
				case 'J':
					VT220EraseInDisplay(vt, ED_TO_END);
					break;
				case 'K':
					VT220EraseInLine(vt, EL_TO_END);
					break;
				case 'Y':
					vt->state = STATE_DCA1;
					break;
				case 'Z':
					VT220IdentifyVT52(vt);
					break;
				case '=':
					vt->mode |= DECKPAM;
					break;
				case '>':
					vt->mode &= ~DECKPAM;
					break;
				case '<':
					VT220SetANSIMode(vt);
					break;
				case '^': /* Enter auto print mode */
				case '_': /* Exit auto print mode */
				case 'W': /* Enter printer controller mode */
				case 'X': /* Exit printer controller mode */
				case ']': /* Print screen */
				case 'V': /* Print cursor line */
					/* TODO: implement */
					break;
			}
			break;
		case STATE_DCA1:
			switch(c) {
				case NUL:
					break;
				case ENQ:
					VT220SendAnswerback(vt);
					break;
				case BEL:
					VT220Bell(vt);
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case HT:
					VT220Tabstop(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					break;
				case DC3:
					VT220Xoff(vt);
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case ESC:
					vt->state = STATE_ESC;
					break;
				case DEL:
					break;
				case IND:
					VT220Index(vt);
					break;
				case NEL:
					VT220NextLine(vt);
					break;
				default:
					vt->parameters[0] = c - 037;
					vt->state = STATE_DCA2;
			}
			break;
		case STATE_DCA2:
			switch(c) {
				case NUL:
					break;
				case ENQ:
					VT220SendAnswerback(vt);
					break;
				case BEL:
					VT220Bell(vt);
					break;
				case BS:
					VT220CursorLeft(vt);
					break;
				case HT:
					VT220Tabstop(vt);
					break;
				case LF:
				case VT:
				case FF:
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					break;
				case DC3:
					VT220Xoff(vt);
					break;
				case CAN:
					break;
				case SUB:
					VT220Substitute(vt);
					break;
				case ESC:
					vt->state = STATE_ESC;
					break;
				case DEL:
					break;
				case IND:
					VT220Index(vt);
					break;
				case NEL:
					VT220NextLine(vt);
					break;
				default:
					vt->parameters[1] = c - 037;
					vt->state = STATE_TEXT;

					/* Special handling:
					 * - if the line is out of range, it is not updated
					 * - if the column is out of range, it is not updated */
					/* See: DEC 070 (VSRM - VT52 Emulation EL-00070-0A, page A-28) */
					unsigned int line = vt->parameters[0];
					unsigned int column = vt->parameters[1];
					if(line < 1 || line > vt->lines) {
						line = vt->cursor_y + 1;
					}
					if(column < 1 || column > vt->columns) {
						column = vt->cursor_x + 1;
					}
					VT220SetCursor(vt, line, column);
			}
			break;
	}
}

void VT220ProcessChar(VT220* vt, unsigned char c)
{
	vt->cursor_time = 0;

	/* strip MSB in VT52 mode */
	if(!(vt->mode & DECANM)) {
		c &= 0x7F;
	}

	switch(vt->config.controls) {
		default:
		case VT220_CONTROLS_INTERPRET_CONTROLS:
			if(vt->mode & DECANM) {
				VT220ProcessCharVT220(vt, c);
			} else {
				VT220ProcessCharVT52(vt, c);
			}
			break;
		case VT220_CONTROLS_DISPLAY_CONTROLS:
			switch(c) {
				case LF:
				case VT:
				case FF:
					VT220WriteControls(vt, c);
					VT220Linefeed(vt);
					break;
				case CR:
					VT220CarriageReturn(vt);
					break;
				case DC1:
					VT220Xon(vt);
					VT220WriteControls(vt, c);
					break;
				case DC3:
					VT220Xoff(vt);
					VT220WriteControls(vt, c);
					break;
				default:
					VT220WriteControls(vt, c);
					break;
			}
	}
}

void VT220Receive(VT220* vt, unsigned char c)
{
	if(vt->enable_buffering) {
		if(vt->buf_used < 256) {
			vt->buf[vt->buf_w++] = c;
			vt->buf_w %= 256;
			vt->buf_used++;

			/* XOFF handling */
			if(vt->use_xoff) {
				if(vt->buf_used == vt->xoff_point) {
					VT220FlowControl(vt, 0);
				} else if(vt->buf_used == 220) {
					VT220FlowControl(vt, 0);
				} else if(vt->buf_used == 254) {
					VT220FlowControl(vt, 0);
				}
			}
		} else {
			vt->buf_lost++;
		}
	} else {
		VT220ProcessChar(vt, c);
	}
}

void VT220ReceiveText(VT220* vt, const char* s)
{
	for(; *s; s++) {
		VT220Receive(vt, (unsigned char) *s);
	}
}

void VT220Process(VT220* vt, unsigned long dt)
{
	vt->cursor_time = (vt->cursor_time + dt)
		% (unsigned long) roundf(CURSOR_TIME * 1000.0f);

	VT220ProcessKeys(vt, dt);

	while(vt->buf_used > 0) {
		unsigned char c = vt->buf[vt->buf_r++];
		vt->buf_used--;
		vt->buf_r %= 256;

		VT220ProcessChar(vt, c);

		if(vt->use_xoff && vt->sent_xoff && vt->buf_used == vt->xon_point - 1) {
			VT220FlowControl(vt, 1);
		}
	}

	while(vt->buf_lost) {
		vt->buf_lost--;
		VT220Write(vt, 0x20);
	}
}

void VT220ProcessKeyVT220(VT220* vt, u16 key)
{
	switch(key) {
		case CR:
			VT220SendInput(vt, key);
			if(vt->mode & LNM) {
				VT220SendInput(vt, LF);
			}
			break;
		case VT220_KEY_FIND:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_INSERT:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_REMOVE:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_SELECT:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '4');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_PREV_SCREEN:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '5');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_NEXT_SCREEN:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '6');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_UP:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'A');
			break;
		case VT220_KEY_DOWN:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'B');
			break;
		case VT220_KEY_RIGHT:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'C');
			break;
		case VT220_KEY_LEFT:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'D');
			break;
		case VT220_KEY_F6:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '7');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F7:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '8');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F8:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '9');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F9:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '0');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F10:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F11:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F12:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '4');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F13:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '5');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F14:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '6');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F15:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '8');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F16:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '9');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F17:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '1');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F18:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '2');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F19:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F20:
			VT220SendInput(vt, CSI);
			VT220SendInput(vt, '3');
			VT220SendInput(vt, '4');
			VT220SendInput(vt, '~');
			break;
		case VT220_KEY_F6_UDK:
			if(vt->udk_len[0]) {
				for(unsigned int i = 0; i < vt->udk_len[0]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[0] + i]);
				}
			}
			break;
		case VT220_KEY_F7_UDK:
			if(vt->udk_len[1]) {
				for(unsigned int i = 0; i < vt->udk_len[1]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[1] + i]);
				}
			}
			break;
		case VT220_KEY_F8_UDK:
			if(vt->udk_len[2]) {
				for(unsigned int i = 0; i < vt->udk_len[2]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[2] + i]);
				}
			}
			break;
		case VT220_KEY_F9_UDK:
			if(vt->udk_len[3]) {
				for(unsigned int i = 0; i < vt->udk_len[3]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[3] + i]);
				}
			}
			break;
		case VT220_KEY_F10_UDK:
			if(vt->udk_len[4]) {
				for(unsigned int i = 0; i < vt->udk_len[4]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[4] + i]);
				}
			}
			break;
		case VT220_KEY_F11_UDK:
			if(vt->udk_len[5]) {
				for(unsigned int i = 0; i < vt->udk_len[5]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[5] + i]);
				}
			}
			break;
		case VT220_KEY_F12_UDK:
			if(vt->udk_len[6]) {
				for(unsigned int i = 0; i < vt->udk_len[6]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[6] + i]);
				}
			}
			break;
		case VT220_KEY_F13_UDK:
			if(vt->udk_len[7]) {
				for(unsigned int i = 0; i < vt->udk_len[7]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[7] + i]);
				}
			}
			break;
		case VT220_KEY_F14_UDK:
			if(vt->udk_len[8]) {
				for(unsigned int i = 0; i < vt->udk_len[8]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[8] + i]);
				}
			}
			break;
		case VT220_KEY_F15_UDK:
			if(vt->udk_len[9]) {
				for(unsigned int i = 0; i < vt->udk_len[9]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[9] + i]);
				}
			}
			break;
		case VT220_KEY_F16_UDK:
			if(vt->udk_len[10]) {
				for(unsigned int i = 0; i < vt->udk_len[10]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[10] + i]);
				}
			}
			break;
		case VT220_KEY_F17_UDK:
			if(vt->udk_len[11]) {
				for(unsigned int i = 0; i < vt->udk_len[11]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[11] + i]);
				}
			}
			break;
		case VT220_KEY_F18_UDK:
			if(vt->udk_len[12]) {
				for(unsigned int i = 0; i < vt->udk_len[12]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[12] + i]);
				}
			}
			break;
		case VT220_KEY_F19_UDK:
			if(vt->udk_len[13]) {
				for(unsigned int i = 0; i < vt->udk_len[13]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[13] + i]);
				}
			}
			break;
		case VT220_KEY_F20_UDK:
			if(vt->udk_len[14]) {
				for(unsigned int i = 0; i < vt->udk_len[14]; i++) {
					VT220SendInput(vt, vt->udk_memory[vt->udk_ptr[14] + i]);
				}
			}
			break;
		case VT220_KEY_KP_0:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'p');
			} else {
				VT220SendInput(vt, '0');
			}
			break;
		case VT220_KEY_KP_1:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'q');
			} else {
				VT220SendInput(vt, '1');
			}
			break;
		case VT220_KEY_KP_2:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'r');
			} else {
				VT220SendInput(vt, '2');
			}
			break;
		case VT220_KEY_KP_3:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 's');
			} else {
				VT220SendInput(vt, '3');
			}
			break;
		case VT220_KEY_KP_4:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 't');
			} else {
				VT220SendInput(vt, '4');
			}
			break;
		case VT220_KEY_KP_5:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'u');
			} else {
				VT220SendInput(vt, '5');
			}
			break;
		case VT220_KEY_KP_6:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'v');
			} else {
				VT220SendInput(vt, '6');
			}
			break;
		case VT220_KEY_KP_7:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'w');
			} else {
				VT220SendInput(vt, '7');
			}
			break;
		case VT220_KEY_KP_8:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'x');
			} else {
				VT220SendInput(vt, '8');
			}
			break;
		case VT220_KEY_KP_9:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'y');
			} else {
				VT220SendInput(vt, '9');
			}
			break;
		case VT220_KEY_KP_MINUS:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'm');
			} else {
				VT220SendInput(vt, '-');
			}
			break;
		case VT220_KEY_KP_COMMA:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'l');
			} else {
				VT220SendInput(vt, ',');
			}
			break;
		case VT220_KEY_KP_PERIOD:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'n');
			} else {
				VT220SendInput(vt, '.');
			}
			break;
		case VT220_KEY_KP_ENTER:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'M');
			} else {
				VT220SendInput(vt, CR);
				if(vt->mode & LNM) {
					VT220SendInput(vt, LF);
				}
			}
			break;
		case VT220_KEY_KP_PF1:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'P');
			break;
		case VT220_KEY_KP_PF2:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'Q');
			break;
		case VT220_KEY_KP_PF3:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'R');
			break;
		case VT220_KEY_KP_PF4:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'S');
			break;
		default:
			if(key < 0x100) {
				VT220SendInput(vt, key);
			}
	}

}

void VT220ProcessKeyVT100(VT220* vt, u16 key)
{
	switch(key) {
		case CR:
			VT220SendInput(vt, key);
			if(vt->mode & LNM) {
				VT220SendInput(vt, LF);
			}
			break;
		case VT220_KEY_UP:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'A');
			break;
		case VT220_KEY_DOWN:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'B');
			break;
		case VT220_KEY_RIGHT:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'C');
			break;
		case VT220_KEY_LEFT:
			if(vt->mode & DECCKM) {
				VT220SendInput(vt, SS3);
			} else {
				VT220SendInput(vt, CSI);
			}
			VT220SendInput(vt, 'D');
			break;
		case VT220_KEY_HOLD_SCREEN:
		case VT220_KEY_PRINT_SCREEN:
			break;
		case VT220_KEY_SET_UP:
			if(vt->in_setup) {
				VT220LeaveSetup(vt);
			} else {
				VT220EnterSetup(vt);
			}
			break;
		case VT220_KEY_DATA_TALK:
		case VT220_KEY_BREAK:
			/* local function keys */
			/* TODO: implement */
			break;
		case VT220_KEY_F6:
		case VT220_KEY_F7:
		case VT220_KEY_F8:
		case VT220_KEY_F9:
		case VT220_KEY_F10:
			break;
		case VT220_KEY_F11:
			VT220SendInput(vt, ESC);
			break;
		case VT220_KEY_F12:
			VT220SendInput(vt, BS);
			break;
		case VT220_KEY_F13:
			VT220SendInput(vt, LF);
			break;
		case VT220_KEY_F14:
		case VT220_KEY_F15:
		case VT220_KEY_F16:
		case VT220_KEY_F17:
		case VT220_KEY_F18:
		case VT220_KEY_F19:
		case VT220_KEY_F20:
			break;
		case VT220_KEY_KP_0:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'p');
			} else {
				VT220SendInput(vt, '0');
			}
			break;
		case VT220_KEY_KP_1:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'q');
			} else {
				VT220SendInput(vt, '1');
			}
			break;
		case VT220_KEY_KP_2:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'r');
			} else {
				VT220SendInput(vt, '2');
			}
			break;
		case VT220_KEY_KP_3:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 's');
			} else {
				VT220SendInput(vt, '3');
			}
			break;
		case VT220_KEY_KP_4:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 't');
			} else {
				VT220SendInput(vt, '4');
			}
			break;
		case VT220_KEY_KP_5:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'u');
			} else {
				VT220SendInput(vt, '5');
			}
			break;
		case VT220_KEY_KP_6:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'v');
			} else {
				VT220SendInput(vt, '6');
			}
			break;
		case VT220_KEY_KP_7:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'w');
			} else {
				VT220SendInput(vt, '7');
			}
			break;
		case VT220_KEY_KP_8:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'x');
			} else {
				VT220SendInput(vt, '8');
			}
			break;
		case VT220_KEY_KP_9:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'y');
			} else {
				VT220SendInput(vt, '9');
			}
			break;
		case VT220_KEY_KP_MINUS:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'm');
			} else {
				VT220SendInput(vt, '-');
			}
			break;
		case VT220_KEY_KP_COMMA:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'l');
			} else {
				VT220SendInput(vt, ',');
			}
			break;
		case VT220_KEY_KP_PERIOD:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'n');
			} else {
				VT220SendInput(vt, '.');
			}
			break;
		case VT220_KEY_KP_ENTER:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, SS3);
				VT220SendInput(vt, 'M');
			} else {
				VT220SendInput(vt, CR);
				if(vt->mode & LNM) {
					VT220SendInput(vt, LF);
				}
			}
			break;
		case VT220_KEY_KP_PF1:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'P');
			break;
		case VT220_KEY_KP_PF2:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'Q');
			break;
		case VT220_KEY_KP_PF3:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'R');
			break;
		case VT220_KEY_KP_PF4:
			VT220SendInput(vt, SS3);
			VT220SendInput(vt, 'S');
			break;
		default:
			if(key < 0x80) {
				VT220SendInput(vt, key);
			}
	}

}

void VT220ProcessKeyVT52(VT220* vt, u16 key)
{
	switch(key) {
		case CR:
			VT220SendInput(vt, key);
			if(vt->mode & LNM) {
				VT220SendInput(vt, LF);
			}
			break;
		case VT220_KEY_FIND:
		case VT220_KEY_INSERT:
		case VT220_KEY_REMOVE:
		case VT220_KEY_SELECT:
		case VT220_KEY_PREV_SCREEN:
		case VT220_KEY_NEXT_SCREEN:
			break;
		case VT220_KEY_UP:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'A');
			break;
		case VT220_KEY_DOWN:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'B');
			break;
		case VT220_KEY_RIGHT:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'C');
			break;
		case VT220_KEY_LEFT:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'D');
			break;
		case VT220_KEY_F6:
		case VT220_KEY_F7:
		case VT220_KEY_F8:
		case VT220_KEY_F9:
		case VT220_KEY_F10:
			break;
		case VT220_KEY_F11:
			VT220SendInput(vt, ESC);
			break;
		case VT220_KEY_F12:
			VT220SendInput(vt, BS);
			break;
		case VT220_KEY_F13:
			VT220SendInput(vt, LF);
			break;
		case VT220_KEY_F14:
		case VT220_KEY_F15:
		case VT220_KEY_F16:
		case VT220_KEY_F17:
		case VT220_KEY_F18:
		case VT220_KEY_F19:
		case VT220_KEY_F20:
			break;
		case VT220_KEY_KP_0:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'p');
			} else {
				VT220SendInput(vt, '0');
			}
			break;
		case VT220_KEY_KP_1:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'q');
			} else {
				VT220SendInput(vt, '1');
			}
			break;
		case VT220_KEY_KP_2:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'r');
			} else {
				VT220SendInput(vt, '2');
			}
			break;
		case VT220_KEY_KP_3:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 's');
			} else {
				VT220SendInput(vt, '3');
			}
			break;
		case VT220_KEY_KP_4:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 't');
			} else {
				VT220SendInput(vt, '4');
			}
			break;
		case VT220_KEY_KP_5:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'u');
			} else {
				VT220SendInput(vt, '5');
			}
			break;
		case VT220_KEY_KP_6:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'v');
			} else {
				VT220SendInput(vt, '6');
			}
			break;
		case VT220_KEY_KP_7:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'w');
			} else {
				VT220SendInput(vt, '7');
			}
			break;
		case VT220_KEY_KP_8:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'x');
			} else {
				VT220SendInput(vt, '8');
			}
			break;
		case VT220_KEY_KP_9:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'y');
			} else {
				VT220SendInput(vt, '9');
			}
			break;
		case VT220_KEY_KP_MINUS:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'm');
			} else {
				VT220SendInput(vt, '-');
			}
			break;
		case VT220_KEY_KP_COMMA:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'l');
			} else {
				VT220SendInput(vt, ',');
			}
			break;
		case VT220_KEY_KP_PERIOD:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'n');
			} else {
				VT220SendInput(vt, '.');
			}
			break;
		case VT220_KEY_KP_ENTER:
			if(vt->mode & DECKPAM) {
				VT220SendInput(vt, ESC);
				VT220SendInput(vt, '?');
				VT220SendInput(vt, 'M');
			} else {
				VT220SendInput(vt, CR);
				if(vt->mode & LNM) {
					VT220SendInput(vt, LF);
				}
			}
			break;
		case VT220_KEY_KP_PF1:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'P');
			break;
		case VT220_KEY_KP_PF2:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'Q');
			break;
		case VT220_KEY_KP_PF3:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'R');
			break;
		case VT220_KEY_KP_PF4:
			VT220SendInput(vt, ESC);
			VT220SendInput(vt, 'S');
			break;
		default:
			if(key < 0x80) {
				VT220SendInput(vt, key);
			}
	}
}

void VT220ProcessKey(VT220* vt, u16 key)
{
	if(key == VT220_KEY_SET_UP) {
		if(vt->in_setup) {
			VT220LeaveSetup(vt);
		} else {
			VT220EnterSetup(vt);
		}
		return;
	}

	if(!vt->in_setup && vt->mode & KAM) {
		return;
	}

	VT220Keyclick(vt);

	if(vt->in_setup) {
		VT220SetupProcessKey(vt, key);
	} else if(vt->mode & DECANM) {
		switch(key) {
			/* local function keys */
			case VT220_KEY_HOLD_SCREEN:
				if(vt->use_xoff) {
					vt->hold_screen = !vt->hold_screen;
					VT220FlowControl(vt, !vt->hold_screen);
				}
				break;
			case VT220_KEY_PRINT_SCREEN:
			case VT220_KEY_DATA_TALK:
				/* TODO: implement */
				return;
			case VT220_KEY_BREAK:
				if(vt->brk) {
					vt->brk();
				}
				return;
			case VT220_KEY_ANSWERBACK:
				VT220SendAnswerback(vt);
				return;
			case VT220_KEY_DISCONNECT:
				/* TODO: implement */
				return;
			case DC1:
				if(vt->use_xoff) {
					VT220FlowControl(vt, 1);
					return;
				}
				break;
			case DC3:
				if(vt->use_xoff) {
					VT220FlowControl(vt, 0);
					return;
				}
				break;
		}

		if(vt->vt100_mode) {
			VT220ProcessKeyVT100(vt, key);
		} else {
			VT220ProcessKeyVT220(vt, key);
		}
	} else {
		VT220ProcessKeyVT52(vt, key);
	}
}

void VT220FlowControl(VT220* vt, int start)
{
	if(!vt->use_xoff) {
		vt->sent_xoff = !start;
		return;
	}

	if(start) {
		if(vt->flowcontrol) {
			vt->flowcontrol(1);
		} else {
			VT220Send(vt, DC1);
		}
		vt->sent_xoff = 0;
	} else {
		if(vt->flowcontrol) {
			vt->flowcontrol(0);
		} else {
			VT220Send(vt, DC3);
		}
		vt->sent_xoff = 1;
	}
}

int VT220CanReceive(VT220* vt)
{
	return !vt->sent_xoff;
}

void VT220SetScreenColor(VT220* vt, unsigned int color)
{
	if(color < 3) {
		vt->screen_color = color;
	}
}

void VT220SetBuffering(VT220* vt, int enable)
{
	vt->enable_buffering = enable;
}
