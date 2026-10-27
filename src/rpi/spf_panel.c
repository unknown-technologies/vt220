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
	panel->flags = SPF_FLAG_ACTIVE | SPF_FLAG_OPAQUE;
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

void SPFSetPanelOffset(SPFPanel* self)
{
	SPF* spf = SPFGet(self);

	if(SPFIsDialog(self)) {
		int width = SPFGetWidth(self);
		int height = SPFGetHeight(self);

		int screen_width = spf->vt->columns;
		int screen_height = spf->vt->lines;

		int off_x = (screen_width - width) / 2;
		int off_y = (screen_height - height) / 2;

		SPFScreenSetOffset(spf, off_y, off_x);
	} else {
		SPFScreenSetOffset(spf, 2, 0);
	}
}

void SPFDrawDialog(SPFPanel* self)
{
	int lines, columns;
	SPFGetSize(self, &lines, &columns);

	SPFPutcAt(self, 1, 1, 0x0D, 0);
	SPFPutcAt(self, 1, columns, 0x0C, 0);
	SPFPutcAt(self, lines, 1, 0x0E, 0);
	SPFPutcAt(self, lines, columns, 0x0B, 0);

	SPFPutcAt(self, 2, 1, 0x19, 0);
	SPFPutcAt(self, 2, columns, 0x19, 0);

	SPFGoto(self, 1, 2);
	SPFScreenFill(SPFGet(self), 0x12, columns - 2, 0);
	SPFGoto(self, 3, 2);
	SPFScreenFill(SPFGet(self), 0x12, columns - 2, 0);

	const char* title = SPFGetPanelTitle(self);
	if(title) {
		size_t len = strlen(title);
		if(len + 4 > (size_t) columns) {
			SPFGoto(self, 1, 2);
			SPFPutc(self, 0x20, 0);
			for(int i = 0; i < columns - 4; i++) {
				SPFPutc(self, title[i], 0);
			}
			SPFPutc(self, 0x20, 0);
		} else {
			int off = (columns - len) / 2;
			SPFGoto(self, 1, off);
			SPFPutc(self, 0x20, 0);
			SPFPrint(self, title, 0);
			SPFPutc(self, 0x20, 0);
		}
	}
}

void SPFDrawPanel(SPFPanel* self)
{
	ASSERT(self);

	SPF* spf = SPFGet(self);
	SPFScreenSetOffset(spf, 0, 0);
	const char* title = self->title;
	if(!SPFIsDialog(self) && title) {
		SPFScreenPrintCenter(spf, 1, title, SGR_BOLD);
	}

	SPFScreenGoto(spf, 2, 1);
	SPFScreenFill(spf, 0x12, spf->vt->columns, 0);

	if(!SPFIsDialog(self)) {
		SPFSetSize(self, spf->vt->lines - 2, spf->vt->columns);
	}

	SPFSetPanelOffset(self);

	if(SPFIsOpaque(self)) {
		for(int line = 1; line <= SPFGetHeight(self); line++) {
			SPFScreenGoto(spf, line, 1);
			SPFScreenFill(spf, 0x20, SPFGetWidth(self), 0);
		}
	}

	if(SPFIsDialog(self)) {
		SPFDrawDialog(self);
	}

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
