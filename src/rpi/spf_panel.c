#include <string.h>

#include "spf.h"

SPFPanel* SPFCreatePanel(SPF* self, const SPFPanelDefinition* def)
{
	ASSERT(self);
	ASSERT(def);

	if(self->current_panel + 1 >= SPF_MAX_PANELS) {
		return NULL;
	}

	SPFPanel* panel = &self->panels[++self->current_panel];

	memset(panel, 0, sizeof(SPFPanel));
	panel->proto = def;
	panel->spf = self;
	panel->flags = SPF_FLAG_ACTIVE;
	panel->width = self->vt->columns;
	panel->height = self->vt->lines - 2;
	panel->cursor_x = 1;
	panel->cursor_y = 1;

	if(def->init) {
		def->init(panel);
	}

	SPFRepaint(self);

	return panel;
}

void SPFDestroyPanel(SPFPanel* self)
{
	ASSERT(self);

	SPF* spf = SPFGet(self);
	ASSERT(SPFGetCurrentPanel(spf) == self);

	if(self->flags & SPF_FLAG_DESTROY) {
		return;
	} else {
		self->flags |= SPF_FLAG_DESTROY;
	}

	if(self->proto->destroy) {
		self->proto->destroy(self);
	}

	self->flags &= ~SPF_FLAG_ACTIVE;

	spf->current_panel--;
	SPFRepaint(spf);
}

void SPFDrawPanel(SPFPanel* self)
{
	ASSERT(self);

	SPF* spf = SPFGet(self);
	SPFScreenSetOffset(spf, 0, 0);
	const char* title = self->title;
	if(title) {
		SPFScreenPrintCenter(spf, 1, title, SGR_BOLD);
	}

	SPFScreenGoto(spf, 2, 1);
	SPFScreenFill(spf, 0x12, spf->vt->columns, 0);

	SPFSetSize(self, spf->vt->lines - 2, spf->vt->columns);
	SPFScreenSetOffset(spf, 2, 0);

	if(self->proto->draw) {
		self->proto->draw(self);
	}

	if(self->widgets) {
		for(SPFWidget** widget = self->widgets; *widget; widget++) {
			SPFDrawWidget(*widget);
		}

		if(self->focus) {
			SPFFocusWidget(self->focus);
		}
	}

	SPFScreenSetCursor(spf, self->cursor_y, self->cursor_x);
	if(self->flags & SPF_FLAG_CURSOR) {
		SPFScreenShowCursor(spf);
	} else {
		SPFScreenHideCursor(spf);
	}
}

void SPFSetWidgets(SPFPanel* self, SPFWidget** widgets)
{
	ASSERT(self);

	self->widgets = widgets;
	self->focus = NULL;

	if(widgets) {
		for(SPFWidget** widget = widgets; *widget; widget++) {
			SPFSetWidgetPanel(*widget, self);
			if(!self->focus && SPFCanFocusWidget(*widget)) {
				self->focus = *widget;
			}
		}
	}

	SPFRepaint(SPFGet(self));
}
