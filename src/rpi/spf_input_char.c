#include <string.h>

#include "vt.h"
#include "spf.h"

void SPFCreateInputChar(SPFInputChar* self, unsigned int id, unsigned int line, unsigned int col, const char* allowed_chars,
		BOOL uppercase, void (*callback)(SPFInputChar*))
{
	SPFCreateWidget(&self->widget, SPF_WIDGET_INPUT_CHR, id);
	self->widget.pos_x = col;
	self->widget.pos_y = line;
	self->allowed_chars = allowed_chars;
	self->uppercase = uppercase;
	self->callback = callback;
	self->value = SP;
}

void SPFDrawInputChar(SPFInputChar* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_CHR);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFPutcAt(panel, self->widget.pos_y, self->widget.pos_x, self->value, SGR_BOLD | SGR_UNDERSCORE);
}

void SPFFocusInputChar(SPFInputChar* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_CHR);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFSetCursor(panel, self->widget.pos_y, self->widget.pos_x);
}

static void SPFInputCharStateChanged(SPFInputChar* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_CHR);

	if(self->callback) {
		self->callback(self);
	}
}

BOOL SPFProcessInputChar(SPFInputChar* self, u16 key)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_INPUT_CHR);

	switch(key) {
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
			if(key >= 0x20 && key < 0x7F) {
				if(self->uppercase && key >= 'a' && key <= 'z') {
					key = key - 'a' + 'A';
				}
				if(self->allowed_chars) {
					char* found = strchr(self->allowed_chars, (char) key);
					if(!found) {
						return FALSE;
					}
				}
				self->value = (char) key;
				SPFInputCharStateChanged(self);
				SPFDrawInputChar(self);
				return TRUE;
			}
			return FALSE;
	}
}
