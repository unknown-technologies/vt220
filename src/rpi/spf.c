#include <string.h>
#include <stdlib.h>

#include "vt.h"
#include "renderer.h"
#include "spf.h"

#define	SPF_MAX_LINES	24
#define	SPF_MAX_COLUMNS	132

bool SPFInit(SPF* self, VT220* vt)
{
	ASSERT(self);
	ASSERT(vt);

	memset(self, 0, sizeof(SPF));

	self->vt = vt;

	self->screen.text = (VT220CELL*) malloc(SPF_MAX_COLUMNS * SPF_MAX_LINES * sizeof(VT220CELL));
	if(!self->screen.text) {
		return false;
	}

	self->screen.lineattr = malloc(SPF_MAX_LINES);
	if(!self->screen.lineattr) {
		free(self->screen.text);
		return false;
	}

	memset(self->screen.text, 0, SPF_MAX_COLUMNS * SPF_MAX_LINES * sizeof(VT220CELL));
	memset(self->screen.lineattr, 0, SPF_MAX_LINES);

	self->current_panel = -1;

	self->dirty = 1;

	return true;
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

	if(self->error_message) {
		int columns = strlen(self->error_message) + 4;
		int lines = 3;

		int screen_columns = self->vt->columns;
		int screen_lines = self->vt->lines;

		int off_x = (screen_columns - columns) / 2;
		int off_y = (screen_lines - lines) / 2;

		SPFScreenSetOffset(self, off_y, off_x);

		SPFScreenGoto(self, 1, 1);
		SPFScreenPutc(self, 0x0D, 0, FALSE);
		SPFScreenGoto(self, 1, columns);
		SPFScreenPutc(self, 0x0C, 0, FALSE);
		SPFScreenGoto(self, lines, 1);
		SPFScreenPutc(self, 0x0E, 0, FALSE);
		SPFScreenGoto(self, lines, columns);
		SPFScreenPutc(self, 0x0B, 0, FALSE);

		SPFScreenGoto(self, 2, 1);
		SPFScreenPutc(self, 0x19, 0, FALSE);
		SPFScreenGoto(self, 2, columns);
		SPFScreenPutc(self, 0x19, 0, FALSE);

		SPFScreenGoto(self, 1, 2);
		SPFScreenFill(self, 0x12, columns - 2, 0);
		SPFScreenGoto(self, 3, 2);
		SPFScreenFill(self, 0x12, columns - 2, 0);

		/* for now we have a fixed title */
		const char* title = "Error";
		if(title) {
			size_t len = strlen(title);
			if(len + 4 > (size_t) columns) {
				SPFScreenGoto(self, 1, 2);
				SPFScreenPutc(self, 0x20, 0, FALSE);
				for(int i = 0; i < columns - 4; i++) {
					SPFScreenPutc(self, title[i], 0, FALSE);
				}
				SPFScreenPutc(self, 0x20, 0, FALSE);
			} else {
				int off = (columns - len) / 2;
				SPFScreenGoto(self, 1, off);
				SPFScreenPutc(self, 0x20, 0, FALSE);
				SPFScreenPrint(self, title, 0);
				SPFScreenPutc(self, 0x20, 0, FALSE);
			}
		}

		SPFScreenGoto(self, 2, 3);
		SPFScreenPrint(self, self->error_message, 0);

		SPFScreenHideCursor(self);
	}
}

void SPFProcess(SPF* self)
{
	SPFProcessKey(self, 0xFFFF);
}

void SPFProcessKey(SPF* self, u16 key)
{
	ASSERT(self);

	if(self->error_message) {
		if(key != 0xFFFF) {
			self->error_message = NULL;
			SPFRepaint(self);
		}

		if(self->dirty) {
			SPFDraw(self);
		}

		return;
	}

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
