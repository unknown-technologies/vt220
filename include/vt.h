#ifndef __VT_H__
#define __VT_H__

#include <GL/gl.h>

#define	VT220_SCREEN_COLOR_GREEN		0
#define	VT220_SCREEN_COLOR_WHITE		1
#define	VT220_SCREEN_COLOR_AMBER		2

#define	VT220_KEY_HOLD_SCREEN			301
#define	VT220_KEY_PRINT_SCREEN			302
#define	VT220_KEY_SET_UP			303
#define	VT220_KEY_DATA_TALK			304
#define	VT220_KEY_BREAK				305
#define	VT220_KEY_F6				306
#define	VT220_KEY_F7				307
#define	VT220_KEY_F8				308
#define	VT220_KEY_F9				309
#define	VT220_KEY_F10				310
#define	VT220_KEY_F11				311
#define	VT220_KEY_F12				312
#define	VT220_KEY_F13				313
#define	VT220_KEY_F14				314
#define	VT220_KEY_F15				315
#define	VT220_KEY_F16				316
#define	VT220_KEY_F17				317
#define	VT220_KEY_F18				318
#define	VT220_KEY_F19				319
#define	VT220_KEY_F20				320
#define	VT220_KEY_F6_UDK			321
#define	VT220_KEY_F7_UDK			322
#define	VT220_KEY_F8_UDK			323
#define	VT220_KEY_F9_UDK			324
#define	VT220_KEY_F10_UDK			325
#define	VT220_KEY_F11_UDK			326
#define	VT220_KEY_F12_UDK			327
#define	VT220_KEY_F13_UDK			328
#define	VT220_KEY_F14_UDK			329
#define	VT220_KEY_F15_UDK			330
#define	VT220_KEY_F16_UDK			331
#define	VT220_KEY_F17_UDK			332
#define	VT220_KEY_F18_UDK			333
#define	VT220_KEY_F19_UDK			334
#define	VT220_KEY_F20_UDK			335

#define	VT220_KEY_KP_0				336
#define	VT220_KEY_KP_1				337
#define	VT220_KEY_KP_2				338
#define	VT220_KEY_KP_3				339
#define	VT220_KEY_KP_4				340
#define	VT220_KEY_KP_5				341
#define	VT220_KEY_KP_6				342
#define	VT220_KEY_KP_7				343
#define	VT220_KEY_KP_8				344
#define	VT220_KEY_KP_9				345
#define	VT220_KEY_KP_MINUS			346
#define	VT220_KEY_KP_COMMA			347
#define	VT220_KEY_KP_PERIOD			348
#define	VT220_KEY_KP_ENTER			349
#define	VT220_KEY_KP_PF1			350
#define	VT220_KEY_KP_PF2			351
#define	VT220_KEY_KP_PF3			352
#define	VT220_KEY_KP_PF4			353

#define	VT220_KEY_FIND				354
#define	VT220_KEY_INSERT			355
#define	VT220_KEY_REMOVE			356
#define	VT220_KEY_SELECT			357
#define	VT220_KEY_PREV_SCREEN			358
#define	VT220_KEY_NEXT_SCREEN			359
#define	VT220_KEY_UP				360
#define	VT220_KEY_DOWN				361
#define	VT220_KEY_LEFT				362
#define	VT220_KEY_RIGHT				363

#define	VT220_KEY_CTRL				364
#define	VT220_KEY_SHIFT				365

#define	VT220_MODIFIER_SHIFT_L			_BV(0)
#define	VT220_MODIFIER_SHIFT_R			_BV(1)
#define	VT220_MODIFIER_CTRL_L			_BV(2)
#define	VT220_MODIFIER_CTRL_R			_BV(3)
#define	VT220_MODIFIER_ALT_L			_BV(4)
#define	VT220_MODIFIER_ALT_R			_BV(5)

#define	MAX_PARAMETERS				16

#define	VT220_LANGUAGE_ENGLISH			0
#define	VT220_LANGUAGE_FRANCAIS			1
#define	VT220_LANGUAGE_DEUTSCH			2

#define	VT220_KEYBOARD_UNKNOWN			0
#define	VT220_KEYBOARD_NORTH_AMERICAN		1
#define	VT220_KEYBOARD_BRITISH			2
#define	VT220_KEYBOARD_FLEMISH			3
#define	VT220_KEYBOARD_CANADIAN_FRENCH		4
#define	VT220_KEYBOARD_DANISH			5
#define	VT220_KEYBOARD_FINNISH			6
#define	VT220_KEYBOARD_GERMAN			7
#define	VT220_KEYBOARD_DUTCH			8
#define	VT220_KEYBOARD_ITALIAN			9
#define	VT220_KEYBOARD_SWISS_FRENCH		10
#define	VT220_KEYBOARD_SWISS_GERMAN		11
#define	VT220_KEYBOARD_SWEDISH			12
#define	VT220_KEYBOARD_NORWEGIAN		13
#define	VT220_KEYBOARD_FRENCH			14
#define	VT220_KEYBOARD_SPANISH			15
#define	VT220_KEYBOARD_COUNT			16

#define	VT220_CONTROLS_INTERPRET_CONTROLS	0
#define	VT220_CONTROLS_DISPLAY_CONTROLS		1

#define	VT220_SCROLL_SMOOTH_SCROLL		0
#define	VT220_SCROLL_JUMP_SCROLL		1
#define	VT220_SCROLL_NO_SCROLL			2

#define	VT220_TEXT_LIGHT_TEXT			0
#define	VT220_TEXT_DARK_TEXT			1

#define	VT220_TEXT_CURSOR			0
#define	VT220_NO_TEXT_CURSOR			1

#define	VT220_CURSOR_STYLE_BLOCK_CURSOR		0
#define	VT220_CURSOR_STYLE_UNDERLINE_CURSOR	1

#define	VT220_MODE_VT200_MODE_7BIT_CONTROLS	0
#define	VT220_MODE_VT200_MODE_8BIT_CONTROLS	1
#define	VT220_MODE_VT52_MODE			2
#define	VT220_MODE_VT100_MODE			3

#define	VT220_VT100_TERMINAL_ID_VT220		0
#define	VT220_VT100_TERMINAL_ID_VT100		1
#define	VT220_VT100_TERMINAL_ID_VT101		2
#define	VT220_VT100_TERMINAL_ID_VT102		3

#define	VT220_USER_DEFINED_KEYS_UNLOCKED	0
#define	VT220_USER_DEFINED_KEYS_LOCKED		1

#define	VT220_USER_FEATURES_UNLOCKED		0
#define	VT220_USER_FEATURES_LOCKED		1

#define	VT220_CHARACTER_SET_MODE_MULTINATIONAL	0
#define	VT220_CHARACTER_SET_MODE_NATIONAL	1

#define	VT220_KEYPAD_NUMERIC			0
#define	VT220_KEYPAD_APPLICATION		1

#define	VT220_CURSOR_KEYS_NORMAL		0
#define	VT220_CURSOR_KEYS_APPLICATION		1

#define	VT220_NO_NEW_LINE			0
#define	VT220_NEW_LINE				1

#define	VT220_XOFF_64				0
#define	VT220_XOFF_128				1
#define	VT220_XOFF_OFF				2

#define	VT220_COMM_8BIT_NO_PARITY		0
#define	VT220_COMM_8BIT_EVEN_PARITY		1
#define	VT220_COMM_8BIT_ODD_PARITY		2
#define	VT220_COMM_7BIT_NO_PARITY		3
#define	VT220_COMM_7BIT_EVEN_PARITY		4
#define	VT220_COMM_7BIT_ODD_PARITY		5
#define	VT220_COMM_7BIT_MARK_PARITY		6
#define	VT220_COMM_7BIT_SPACE_PARITY		7
#define	VT220_COMM_7BIT_EVEN_PARITY_NO_CHECK	8
#define	VT220_COMM_7BIT_ODD_PARITY_NO_CHECK	9
#define	VT220_COMM_8BIT_EVEN_PARITY_NO_CHECK	10
#define	VT220_COMM_8BIT_ODD_PARITY_NO_CHECK	11

#define	VT220_COMM_1_STOP_BIT			0
#define	VT220_COMM_2_STOP_BITS			1

#define	VT220_NO_LOCAL_ECHO			0
#define	VT220_LOCAL_ECHO			1

#define	VT220_COMM_EIA_PORT_DATA_LEADS_ONLY	0
#define	VT220_COMM_EIA_PORT_MODEM_CONTROL	1
#define	VT220_COMM_20MA_PORT			2

#define	VT220_COMM_2S_DELAY			0
#define	VT220_COMM_60MS_DELAY			1

#define	VT220_TRANSMIT_LIMITED			0
#define	VT220_TRANSMIT_UNLIMITED		1

#define	VT220_KEYS_TYPEWRITER			0
#define	VT220_KEYS_DATA_PROCESSING		1

#define	VT220_LOCK_CAPS_LOCK			0
#define	VT220_LOCK_SHIFT_LOCK			1

#define	VT220_AUTO_REPEAT			0
#define	VT220_NO_AUTO_REPEAT			1

#define	VT220_KEYCLICK				0
#define	VT220_NO_KEYCLICK			1

#define	VT220_MARGIN_BELL			0
#define	VT220_NO_MARGIN_BELL			1

#define	VT220_BELL				0
#define	VT220_NO_BELL				1

#define	VT220_BREAK				0
#define	VT220_NO_BREAK				1

#define	VT220_SETUP_MOVE_NONE			0
#define	VT220_SETUP_MOVE_UP			1
#define	VT220_SETUP_MOVE_DOWN			2
#define	VT220_SETUP_MOVE_LEFT			3
#define	VT220_SETUP_MOVE_RIGHT			4
#define	VT220_SETUP_MOVE_LEFT_MARGIN		5

typedef struct {
	unsigned int	rx_baud_rate;
	unsigned int	tx_baud_rate;
	char		stop_bits;
	char		format;
	char		port;
	char		delay;

	char		local;
	char		language;
	char		keyboard;
	char		columns;
	char		controls;
	char		auto_wrap;
	char		scroll;
	char		text;
	char		display;
	char		text_cursor;
	char		cursor_style;
	char		mode;
	char		vt100_terminal_id;
	char		user_features;
	char		character_set_mode;
	char		keypad;
	char		cursor_keys;
	char		new_line;
	char		xoff;
	char		local_echo;
	char		transmit;
	char		keys;
	char		lock;
	char		auto_repeat;
	char		keyclick;
	char		margin_bell;
	char		bell;
	char		brk;
} VT220NVR;

typedef struct {
	u16		text;
	u16		attr;
} VT220CELL;

typedef struct {
	int		cursor_x;
	int		cursor_y;
	int		move;
	int		screen;
	int		status;
	int		state;
	int		write_x;
	int		write_y;
	int		write_x_save;
	int		write_y_save;
	VT220CELL*	text;
	char*		line_attributes;
} VT220Setup;

typedef struct {
	int		lines;
	int		columns;
	VT220CELL*	text;
	char*		line_attributes;
	unsigned char*	buf;
	int		buf_r;
	int		buf_w;
	int		buf_used;
	int		buf_lost;
	int		state;
	u16		parameters[MAX_PARAMETERS];
	int		parameter_id;
	int		cursor_x;
	int		cursor_y;
	int		g[4];
	int		gl;
	int		gr;
	int		gl_lock;
	int		gr_lock;
	int		g_dst;
	char*		tabstops;
	int		ct_7bit;

	int		vt100_mode;

	int		use_xoff;
	int		xoff;
	int		xoff_point;
	int		xon_point;
	int		sent_xoff;
	u32		mode;

	char		answerback[31];

	int		margin_top;
	int		margin_bottom;

	unsigned long	cursor_time;

	int		sgr;

	/* saved state */
	int		saved_cursor_x;
	int		saved_cursor_y;
	int		saved_sgr;
	int		saved_awm;
	int		saved_om;
	int		saved_gl;
	int		saved_gr;
	int		saved_gl_lock;
	int		saved_gr_lock;
	int		saved_g[4];

	/* keyboard */
	unsigned long	repeat_time;
	int		repeat_state;

	u32		modifiers;
	int		last_scancode;
	u16		repeat_scancode;
	u16		repeat_char;

	/* user defined keys */
	u8		udk_key;
	u8		udk_hex;

	int		udk_locked;
	u8		udk_ptr[15];
	u16		udk_len[15];
	char		udk_memory[256];
	u16		udk_free;

	/* DRCS */
	unsigned char*	drcs;
	char		drcs_name[4];
	char		decdld_dscs[4];
	unsigned int	decdld_dscs_pos;
	unsigned int	decdld_glyph;
	unsigned int	decdld_row;
	unsigned int	decdld_col;
	int		drcs_dirty;

	/* configuration */
	VT220NVR	config;
	unsigned int	screen_color;

	/* setup screens  */
	int		in_setup;
	VT220Setup	setup;

	/* callbacks */
	void		(*bell)(void);
	void		(*keyclick)(void);
	void		(*rx)(unsigned char);
	void		(*brk)(void);
	void		(*resize)(unsigned int width, unsigned int height);
} VT220;

#define	CHARSET_DMCS	0 /* Multilangual */
#define	CHARSET_DEC	1 /* DEC Special Characters */
#define	CHARSET_CNTL	2 /* Display Controls Font */

#define	CHARSET_ASCII			0
#define	CHARSET_DEC_SUPPLEMENTAL	1
#define	CHARSET_DEC_SPECIAL_GRAPHICS	2
#define	CHARSET_NRCS_BRITISH		3
#define	CHARSET_DRCS			4
#define	CHARSET_ASCII_DC		5
#define	CHARSET_DEC_SUPPLEMENTAL_DC	6

#define	NUL		0x00
#define	SOH		0x01
#define	STX		0x02
#define	ETX		0x03
#define	EOT		0x04
#define	ENQ		0x05
#define	ACK		0x06
#define	BEL		0x07
#define	BS		0x08
#define	HT		0x09
#define	LF		0x0A
#define	VT		0x0B
#define	FF		0x0C
#define	CR		0x0D
#define	SO		0x0E
#define	SI		0x0F
#define	DLE		0x10
#define	DC1		0x11
#define	DC2		0x12
#define	DC3		0x13
#define	DC4		0x14
#define	NAK		0x15
#define	SYN		0x16
#define	ETB		0x17
#define	CAN		0x18
#define	EM		0x19
#define	SUB		0x1A
#define	ESC		0x1B
#define	FS		0x1C
#define	GS		0x1D
#define	RS		0x1E
#define	US		0x1F
#define	DEL		0x7F
#define	IND		0x84
#define	NEL		0x85
#define	HTS		0x88
#define	RI		0x8D
#define	SS2		0x8E
#define	SS3		0x8F
#define	DCS		0x90
#define	CSI		0x9B
#define	ST		0x9C

#define _BV(x)		(1 << (x))
#define	KAM		_BV(0)
#define	IRM		_BV(1)
#define	SRM		_BV(2)
#define	LNM		_BV(3)
#define	DECCKM		_BV(4)
#define	DECANM		_BV(5)
#define	DECCOLM		_BV(6)
#define	DECSCLM		_BV(7)
#define	DECSCNM		_BV(8)
#define	DECOM		_BV(9)
#define	DECAWM		_BV(10)
#define	DECARM		_BV(11)
#define	DECPFF		_BV(11)
#define	DECPEX		_BV(12)
#define	DECTCEM		_BV(13)
#define	DECKPAM		_BV(14)
#define	DECTEK		_BV(15)
#define	DECNRCM		_BV(16)
#define	DECGEPM		_BV(17)
#define	DECGPCM		_BV(18)
#define	DECGPCS		_BV(19)
#define	DECGPBM		_BV(20)
#define	DECGRPM		_BV(21)

#define	SGR_BOLD	_BV(0)
#define	SGR_UNDERSCORE	_BV(1)
#define	SGR_BLINKING	_BV(2)
#define	SGR_REVERSE	_BV(3)
#define	SCA_ON		_BV(4)

#define	DECSWL		0
#define	DECDHL_TOP	1
#define	DECDHL_BOTTOM	2
#define	DECDWL		3

#define	EL_TO_END	0
#define	EL_FROM_BOL	1
#define	EL_WHOLE_LINE	2

#define	ED_TO_END	0
#define	ED_FROM_BEGIN	1
#define	ED_ALL		2

int* VT220CreateTextureSheet(void);

void VT220Init(VT220* vt);
void VT220Destroy(VT220* vt);
void VT220Process(VT220* vt, unsigned long dt);
void VT220Receive(VT220* vt, unsigned char c);
void VT220ReceiveText(VT220* vt, const char* s);
void VT220Draw(VT220* vt);
void VT220SetScreenColor(VT220* vt, unsigned int color);

/* private functions */
void VT220Write(VT220* vt, u16 c);
void VT220WriteChar(VT220* vt, unsigned char c);
void VT220WriteCS(VT220* vt, unsigned char c, int cs);
void VT220CursorHome(VT220* vt);
void VT220CursorLeft(VT220* vt);
void VT220CursorLeftN(VT220* vt, int n);
void VT220CursorRight(VT220* vt);
void VT220CursorRightN(VT220* vt, int n);
void VT220CursorDown(VT220* vt);
void VT220CursorDownN(VT220* vt, int n);
void VT220CursorUp(VT220* vt);
void VT220CursorUpN(VT220* vt, int n);
void VT220SetCursor(VT220* vt, int line, int column);
void VT220SaveCursor(VT220* vt);
void VT220RestoreCursor(VT220* vt);
void VT220Tabstop(VT220* vt);
void VT220SetTabstop(VT220* vt);
void VT220ClearTabstop(VT220* vt);
void VT220ClearAllTabstops(VT220* vt);
void VT220CarriageReturn(VT220* vt);
void VT220Linefeed(VT220* vt);
void VT220ReverseLinefeed(VT220* vt);
void VT220InsertLine(VT220* vt);
void VT220InsertLineN(VT220* vt, int n);
void VT220DeleteLine(VT220* vt);
void VT220DeleteLineN(VT220* vt, int n);
void VT220InsertCharacter(VT220* vt);
void VT220InsertCharacterN(VT220* vt, int n);
void VT220DeleteCharacter(VT220* vt);
void VT220DeleteCharacterN(VT220* vt, int n);
void VT220EraseCharacter(VT220* vt, int n);
void VT220EraseInLine(VT220* vt, int type);
void VT220EraseInDisplay(VT220* vt, int type);
void VT220SelectiveEraseInLine(VT220* vt, int type);
void VT220SetColumnMode(VT220* vt);
void VT220ClearColumnMode(VT220* vt);
void VT220SetVT52Mode(VT220* vt);
void VT220SetANSIMode(VT220* vt);
void VT220ShiftOut(VT220* vt);
void VT220ShiftIn(VT220* vt);
void VT220Xon(VT220* vt);
void VT220Xoff(VT220* vt);
void VT220Substitute(VT220* vt);
void VT220Index(VT220* vt);
void VT220NextLine(VT220* vt);
void VT220ReverseIndex(VT220* vt);
void VT220SetLineMode(VT220* vt, int mode);
void VT220SingleShiftG2(VT220* vt);
void VT220SingleShiftG3(VT220* vt);
void VT220SendAnswerback(VT220* vt);
void VT220SendPrimaryDA(VT220* vt);
void VT220SendSecondaryDA(VT220* vt);
void VT220Send(VT220* vt, unsigned char c);
void VT220SendText(VT220* vt, char* c);
void VT220SendDecimal(VT220* vt, int x);
void VT220Bell(VT220* vt);
void VT220Keyclick(VT220* vt);
void VT220ScrollUp(VT220* vt);
void VT220ScrollDown(VT220* vt);
void VT220SetTopBottomMargins(VT220* vt, int top, int bottom);
void VT220EraseScreen(VT220* vt);
void VT220ClearComm(VT220* vt);
void VT220SoftReset(VT220* vt);
void VT220HardReset(VT220* vt);
void VT220ClearUDK(VT220* vt, unsigned int key);
void VT220ClearAllUDK(VT220* vt);
void VT220ClearDRCS(VT220* vt);
int  VT220GetCharset(VT220* vt, unsigned char c);
void VT220ProcessChar(VT220* vt, unsigned char c);
void VT220ProcessKey(VT220* vt, u16 key);
void VT220ProcessKeys(VT220* vt, unsigned long dt);

/* keyboard */
void VT220InitKeyboard(VT220* vt);
void VT220KeyboardKeyDown(VT220* vt, int key);
void VT220KeyboardKeyUp(VT220* vt, int key);
void VT220KeyboardChar(VT220* vt, unsigned int code);
void VT220KeyboardProcess(VT220* vt, unsigned long dt);

/* setup functions */
void VT220EnterSetup(VT220* vt);
void VT220LeaveSetup(VT220* vt);
void VT220SetupProcessKey(VT220* vt, unsigned char c);

#endif
