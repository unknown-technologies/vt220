#include <string.h>
#include <stdlib.h>

#include "vt.h"
#include "renderer.h"
#include "spf.h"

#define	SPF_MAX_LINES	24
#define	SPF_MAX_COLUMNS	132

void SPFInit(SPF* self, VT220* vt)
{
	ASSERT(self);
	ASSERT(vt);

	memset(self, 0, sizeof(SPF));

	self->vt = vt;

	self->screen.text = (VT220CELL*) malloc(SPF_MAX_COLUMNS * SPF_MAX_LINES * sizeof(VT220CELL));
	self->screen.lineattr = malloc(SPF_MAX_LINES);

	memset(self->screen.text, 0, SPF_MAX_COLUMNS * SPF_MAX_LINES * sizeof(VT220CELL));
	memset(self->screen.lineattr, 0, SPF_MAX_LINES);

	self->current_panel = -1;

	self->dirty = 1;
}

void SPFDestroy(SPF* self)
{
	ASSERT(self);
	ASSERT(self->screen.text);
	ASSERT(self->screen.lineattr);

	for(int i = self->current_panel; i >= 0; i--) {
		SPFDestroyPanel(&self->panels[i]);
	}

	free(self->screen.text);
	free(self->screen.lineattr);
}

void SPFDraw(SPF* self)
{
	ASSERT(self);

	self->dirty = 0;

	memset(self->screen.text, 0, SPF_MAX_COLUMNS * SPF_MAX_LINES * sizeof(VT220CELL));
	memset(self->screen.lineattr, 0, SPF_MAX_LINES);

	SPFPanel* panel = SPFGetCurrentPanel(self);
	if(panel) {
		if(SPFIsDialog(panel)) {
			/* draw all panels below this one first */
			for(int i = 0; i < self->current_panel; i++) {
				/* clear title line again */
				SPFScreenSetOffset(self, 0, 0);
				SPFScreenGoto(self, 1, 1);
				SPFScreenFill(self, 0x20, self->vt->columns, 0);
				SPFDrawPanel(&self->panels[i]);
			}
		}

		SPFDrawPanel(panel);
	}
}

void SPFProcess(SPF* self)
{
	SPFProcessKey(self, 0xFFFF);
}

void SPFProcessKey(SPF* self, u16 key)
{
	ASSERT(self);

	SPFPanel* panel = SPFGetCurrentPanel(self);
	if(!panel) {
		return;
	}

	if(panel->flags & SPF_FLAG_DESTROY) {
		return;
	}

	SPFSetPanelOffset(panel);

	BOOL done = FALSE;
	if(!done && panel->widgets && panel->focus) {
		done = SPFProcessWidget(panel->focus, key);
	}

	if(!done && panel->proto->process) {
		panel->proto->process(panel, key);
	}

	SPFScreenSetCursor(self, panel->cursor_y, panel->cursor_x);

	if(self->dirty) {
		SPFDraw(self);
	}
}

void SPFResize(SPF* self, int width, int height)
{
	ASSERT(self);
	ASSERT(width > 0 && height > 0);
	ASSERT(width * height <= SPF_MAX_LINES * SPF_MAX_COLUMNS);

	for(int i = 0; i <= self->current_panel; i++) {
		SPFPanel* panel = &self->panels[i];
		if(!SPFIsDialog(panel)) {
			SPFSetSize(panel, height - 2, width);
			if(panel->proto->resize) {
				panel->proto->resize(panel, width, height - 2);
			}
		}
	}

	SPFRepaint(self);
}

void SPFRepaint(SPF* self)
{
	ASSERT(self);

	self->dirty = 1;
}
