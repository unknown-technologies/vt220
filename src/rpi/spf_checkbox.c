#include "vt.h"
#include "spf.h"

void SPFCreateCheckbox(SPFCheckbox* self, unsigned int id, unsigned int line, unsigned int col,
		void (*callback)(SPFCheckbox*))
{
	SPFCreateWidget(&self->widget, SPF_WIDGET_CHECKBOX, id);
	self->widget.pos_x = col;
	self->widget.pos_y = line;
	self->callback = callback;
	self->state = FALSE;
}

void SPFDrawCheckbox(SPFCheckbox* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_CHECKBOX);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFPrintAt(panel, self->widget.pos_y, self->widget.pos_x, self->state ? "/" : " ", SGR_BOLD | SGR_UNDERSCORE);
}

void SPFFocusCheckbox(SPFCheckbox* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_CHECKBOX);

	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	SPFSetCursor(panel, self->widget.pos_y, self->widget.pos_x);
}

static void SPFCheckboxStateChanged(SPFCheckbox* self)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_CHECKBOX);

	if(self->callback) {
		self->callback(self);
	}
}

BOOL SPFProcessCheckbox(SPFCheckbox* self, u16 key)
{
	ASSERT(self);
	ASSERT(self->widget.type == SPF_WIDGET_CHECKBOX);

	switch(key) {
		case '/':
			self->state = TRUE;
			SPFCheckboxStateChanged(self);
			SPFDrawCheckbox(self);
			return TRUE;
		case BS:
		case DEL:
			self->state = FALSE;
			SPFCheckboxStateChanged(self);
			SPFDrawCheckbox(self);
			return TRUE;
		case SP:
			self->state = !self->state;
			SPFCheckboxStateChanged(self);
			SPFDrawCheckbox(self);
			return TRUE;
		default:
			return FALSE;
	}
}
