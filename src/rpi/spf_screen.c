#include <string.h>

#include "types.h"
#include "vt.h"
#include "spf.h"

void SPFScreenShowCursor(SPF* self)
{
	ASSERT(self);

	self->screen.cursor = 1;
}

void SPFScreenHideCursor(SPF* self)
{
	ASSERT(self);

	self->screen.cursor = 0;
}

void SPFScreenSetCursor(SPF* self, const int line, const int column)
{
	ASSERT(self);
	ASSERT(self->offset_x + column - 1 >= 0);
	ASSERT(self->offset_y + line - 1 >= 0);
	ASSERT(self->offset_x + column - 1 < self->vt->columns);
	ASSERT(self->offset_y + line - 1 < self->vt->lines);

	self->screen.cursor_x = self->offset_x + column - 1;
	self->screen.cursor_y = self->offset_y + line - 1;
}

void SPFScreenPutc(SPF* self, const u16 c, const int sgr, BOOL display_controls)
{
	ASSERT(self);

	u16 glyph = c;
	if(display_controls) {
		if(glyph == 0x20) {
			glyph = 0;
		} else if(glyph < 0x20) {
			glyph += 0x100;
		}
	} else if(c == 0x20) {
		glyph = 0;
	}

	int idx = self->write_y * self->vt->columns + self->write_x;
	self->screen.text[idx].text = glyph;
	self->screen.text[idx].attr = sgr;

	self->write_x++;
	if(self->write_x >= self->vt->columns) {
		self->write_x = self->offset_x;
		self->write_y++;
		if(self->write_y >= self->vt->lines) {
			self->write_y = self->vt->lines - 1;
		}
	}
}

void SPFScreenPrintNumber(SPF* self, int val, const int width, const int align, const int sgr)
{
	ASSERT(self);
	ASSERT(val >= 0 && val < 10000000);
	ASSERT(width > 0);

	char buf[8];
	int i, j;
	if(val == 0) {
		buf[7] = 0;
		i = 7;
	} else {
		for(i = 7; i > 0 && val > 0; i--) {
			buf[i] = val % 10;
			val /= 10;
		}
		i++;
	}
	j = (7 - i);

	if(align) {
		for(; j < width - 1; j++) {
			SPFScreenPutc(self, ' ', sgr, 0);
		}
	}

	for(; i < 8; i++) {
		SPFScreenPutc(self, buf[i] + '0', sgr, 0);
	}

	for(; j < width - 1; j++) {
		SPFScreenPutc(self, ' ', sgr, 0);
	}
}

void SPFScreenPrintCenter(SPF* self, const int line, const char* s, const int sgr)
{
	ASSERT(self);
	ASSERT(line);

	size_t len = strlen(s);

	ASSERT((int) len < self->vt->columns);

	int start_x = (self->vt->columns - len) / 2;
	int idx = (self->offset_y + line - 1) * self->vt->columns + start_x;

	for(const char* ptr = s; *ptr; ptr++, idx++) {
		char c = *ptr;
		u16 glyph;
		if(c == 0x20) {
			glyph = 0;
		} else {
			glyph = (unsigned char) c;
		}

		self->screen.text[idx].text = glyph;
		self->screen.text[idx].attr = sgr;
	}
}

void SPFScreenSetLineAttribute(SPF* self, int attr)
{
	ASSERT(self);
	ASSERT(self->write_y >= 0 && self->write_y < self->vt->lines);

	self->screen.lineattr[self->write_y] = attr;
}

void SPFScreenGoto(SPF* self, const int line, const int column)
{
	ASSERT(self);
	ASSERT(self->offset_x + column - 1 >= 0);
	ASSERT(self->offset_y + line - 1 >= 0);
	ASSERT(self->offset_x + column - 1 < self->vt->columns);
	ASSERT(self->offset_y + line - 1 < self->vt->lines);

	self->write_x = self->offset_x + column - 1;
	self->write_y = self->offset_y + line - 1;
}

void SPFScreenCursorRight(SPF* self)
{
	ASSERT(self);

	self->write_x++;
	if(self->write_x >= self->vt->columns) {
		self->write_x = self->vt->columns - 1;
	}
}

void SPFScreenCursorRightN(SPF* self, int n)
{
	ASSERT(self);

	self->write_x += n;
	if(self->write_x >= self->vt->columns) {
		self->write_x = self->vt->columns - 1;
	}
}

void SPFScreenPrint(SPF* self, const char* s, const int sgr)
{
	ASSERT(self);
	ASSERT(s);

	for(; *s; s++) {
		SPFScreenPutc(self, (unsigned char) *s, sgr, 0);
	}
}

void SPFScreenFill(SPF* self, const u16 c, unsigned int count, const int sgr)
{
	ASSERT(self);

	for(; count; count--) {
		SPFScreenPutc(self, c, sgr, 0);
	}
}

void SPFScreenSetOffset(SPF* self, int line, int column)
{
	ASSERT(self);
	ASSERT(line >= 0 && line < self->vt->lines);
	ASSERT(column >= 0 && column < self->vt->columns);

	self->offset_x = column;
	self->offset_y = line;
}
