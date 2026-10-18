#include "vt.h"
#include "spf.h"

void SPFCreateLabel(SPFLabel* self, unsigned int line, unsigned int col, const char* text, unsigned int flags)
{
	SPFCreateWidget(&self->widget, SPF_WIDGET_LABEL, 0);
	self->widget.pos_x = col;
	self->widget.pos_y = line;
	self->text = text;
	self->flags = flags;
}

void SPFDrawLabel(SPFLabel* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);

	int sgr = 0;
	if(self->flags & SPF_LABEL_BOLD) {
		sgr |= SGR_BOLD;
	}
	if(self->flags & SPF_LABEL_UNDERLINE) {
		sgr |= SGR_UNDERSCORE;
	}
	if(self->flags & SPF_LABEL_REVERSE) {
		sgr |= SGR_REVERSE;
	}
	if(self->flags & SPF_LABEL_BLINK) {
		sgr |= SGR_BLINKING;
	}

	SPFPrintAt(panel, self->widget.pos_y, self->widget.pos_x, self->text, sgr);
}
