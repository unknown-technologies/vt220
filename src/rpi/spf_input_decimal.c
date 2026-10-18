#include <stdio.h>
#include <string.h>

#include "vt.h"
#include "spf.h"

void SPFCreateInputDec(SPFInputDec* self, unsigned int id, unsigned int line, unsigned int col, int len, int min, int max,
		int value, void (*callback)(SPFInputDec*))
{
	ASSERT(self);
	ASSERT(value >= min && value <= max);
	ASSERT((size_t) len < sizeof(self->input));

	SPFCreateWidget(&self->widget, SPF_WIDGET_INPUT_DEC, id);
	self->widget.pos_x = col;
	self->widget.pos_y = line;
	sprintf(self->input, "%d", value);
	self->max_len = len;
	self->len = strnlen(self->input, len);
	ASSERT(self->len <= self->max_len);
	self->cursor = self->len; /* cursor is at the end of the field by default */
	self->callback = callback;
}

void SPFDrawInputDec(SPFInputDec* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_DEC);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFGoto(panel, self->widget.pos_y, self->widget.pos_x);
	for(int i = 0; i < self->max_len; i++) {
		char c = self->input[i];
		if(c == 0) {
			c = ' ';
		}
		SPFPutc(panel, c, SGR_BOLD | SGR_UNDERSCORE);
	}
}

void SPFFocusInputDec(SPFInputDec* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_DEC);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFSetCursor(panel, self->widget.pos_y, self->widget.pos_x + self->cursor);
}

static void SPFInputDecEnter(SPFInputDec* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_DEC);

	if(self->callback) {
		self->callback(self);
	}
}

BOOL SPFProcessInputDec(SPFInputDec* self, u16 key)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_DEC);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPF* spf = SPFGet(panel);

	switch(key) {
		case CR:
		case VT220_KEY_KP_ENTER:
			if(self->input[0]) {
				SPFInputDecEnter(self);
			}
			return TRUE;
		case BS:
		case DEL:
			if(self->cursor > 0) {
				for(int i = self->cursor - 1; i < self->len; i++) {
					if(i + 1 == self->len) {
						self->input[i] = 0;
					} else {
						self->input[i] = self->input[i + 1];
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
						self->input[i] = 0;
					} else {
						self->input[i] = self->input[i + 1];
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
				SPFFocusInputDec(self);
				VT220ResetCursorTime(SPFGetVT(spf));
				return TRUE;
			} else {
				return FALSE;
			}
		case VT220_KEY_RIGHT:
			if(self->cursor < self->len) {
				self->cursor++;
				SPFFocusInputDec(self);
				VT220ResetCursorTime(SPFGetVT(spf));
				return TRUE;
			} else {
				return FALSE;
			}
		case '^':
		case VTCTRL('A'):
			self->cursor = 0;
			SPFFocusInputDec(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			return TRUE;
		case '$':
		case VTCTRL('E'):
			self->cursor = self->len;
			SPFFocusInputDec(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			return TRUE;
		case VTCTRL('U'):
			self->cursor = 0;
			self->len = 0;
			memset(self->input, 0, self->max_len);
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
			if(key >= 0x30 && key <= 0x39 && self->cursor < self->max_len) {
				/* insert mode */
				int end = self->len - 1;
				if(end + 1 >= self->max_len - 2) {
					end = self->max_len - 2;
				}
				for(int i = end; i >= self->cursor; i--) {
					self->input[i + 1] = self->input[i];
				}
				self->input[self->cursor++] = (char) key;
				if(self->len < self->max_len) {
					self->len++;
				}
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			return key >= 0x30 && key <= 0x39;
	}
}

int SPFGetInputDecValue(SPFInputDec* self)
{
	ASSERT(self);

	int value = 0;
	for(int i = 0; i < self->max_len; i++) {
		char c = self->input[i];
		if(c == 0) {
			break;
		} else {
			value *= 10;
			value += c - '0';
		}
	}
	return value;
}

void SPFSetInputDecValue(SPFInputDec* self, int value)
{
	ASSERT(self);
	ASSERT(value >= self->min && value <= self->max);

	memset(self->input, 0, sizeof(self->input));
	sprintf(self->input, "%d", value);
	self->cursor = 0;
	self->len = strlen(self->input);

	/* panel can be NULL if the widget is not yet on a panel */
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	if(panel) {
		SPFRepaint(SPFGet(panel));
	}
}
