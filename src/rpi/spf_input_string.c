#include <string.h>

#include "vt.h"
#include "spf.h"

void SPFCreateInputStr(SPFInputStr* self, unsigned int id, unsigned int line, unsigned int col, char* string, int len,
		void (*callback)(SPFInputStr*))
{
	ASSERT(self);
	ASSERT(string);

	SPFCreateWidget(&self->widget, SPF_WIDGET_INPUT_STR, id);
	self->widget.pos_x = col;
	self->widget.pos_y = line;
	self->string = string;
	self->max_len = len;
	self->len = strnlen(string, len);
	ASSERT(self->len <= self->max_len);
	self->cursor = self->len; /* cursor is at the end of the field by default */
	self->callback = callback;
}

void SPFDrawInputStr(SPFInputStr* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_STR);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFGoto(panel, self->widget.pos_y, self->widget.pos_x);
	for(int i = 0; i < self->max_len; i++) {
		char c = self->string[i];
		if(c == 0) {
			c = ' ';
		}
		SPFPutc(panel, c, SGR_BOLD | SGR_UNDERSCORE);
	}
}

void SPFFocusInputStr(SPFInputStr* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_STR);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFSetCursor(panel, self->widget.pos_y, self->widget.pos_x + self->cursor);
}

static void SPFInputStrEnter(SPFInputStr* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_STR);

	if(self->callback) {
		self->callback(self);
	}
}

BOOL SPFProcessInputStr(SPFInputStr* self, u16 key)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_STR);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPF* spf = SPFGet(panel);

	switch(key) {
		case CR:
		case VT220_KEY_KP_ENTER:
			if(self->string[0]) {
				SPFInputStrEnter(self);
			}
			return TRUE;
		case BS:
		case DEL:
			if(self->cursor > 0) {
				for(int i = self->cursor - 1; i < self->len; i++) {
					if(i + 1 == self->len) {
						self->string[i] = 0;
					} else {
						self->string[i] = self->string[i + 1];
					}
				}
				self->len--;
				self->cursor--;
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			return TRUE;
		case VT220_KEY_REMOVE:
			if(self->cursor < self->len) {
				for(int i = self->cursor; i < self->len; i++) {
					if(i + 1 == self->len) {
						self->string[i] = 0;
					} else {
						self->string[i] = self->string[i + 1];
					}
				}
				self->len--;
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			return TRUE;
		case VT220_KEY_LEFT:
			if(self->cursor > 0) {
				self->cursor--;
				SPFFocusInputStr(self);
				VT220ResetCursorTime(SPFGetVT(spf));
				return TRUE;
			} else {
				return FALSE;
			}
		case VT220_KEY_RIGHT:
			if(self->cursor < self->len) {
				self->cursor++;
				SPFFocusInputStr(self);
				VT220ResetCursorTime(SPFGetVT(spf));
				return TRUE;
			} else {
				return FALSE;
			}
		case VTCTRL('A'):
			self->cursor = 0;
			SPFFocusInputStr(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			return TRUE;
		case VTCTRL('E'):
			self->cursor = self->len;
			SPFFocusInputStr(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			return TRUE;
		case VTCTRL('U'):
			self->cursor = 0;
			self->len = 0;
			memset(self->string, 0, self->max_len);
			SPFRepaint(spf);
			return TRUE;
		case VT220_KEY_KP_0:
		case VT220_KEY_KP_1:
		case VT220_KEY_KP_2:
		case VT220_KEY_KP_3:
		case VT220_KEY_KP_4:
		case VT220_KEY_KP_5:
		case VT220_KEY_KP_6:
		case VT220_KEY_KP_7:
		case VT220_KEY_KP_8:
		case VT220_KEY_KP_9:
			key = key - VT220_KEY_KP_0 + '0';
			/* fallthrough */
		default:
			if(key >= 0x20 && key < 0x7F && self->cursor < self->max_len) {
				/* insert mode */
				int end = self->len - 1;
				if(end + 1 >= self->max_len - 2) {
					end = self->max_len - 2;
				}
				for(int i = end; i >= self->cursor; i--) {
					self->string[i + 1] = self->string[i];
				}
				self->string[self->cursor++] = (char) key;
				if(self->len < self->max_len) {
					self->len++;
				}
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			return key >= 0x20 && key < 0x7F;
	}
}
