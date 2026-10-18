#include <string.h>

#include "types.h"
#include "vt.h"
#include "cm.h"

void CMiInitUI(CM* self, VT220* vt)
{
	SPFInit(&self->spf, vt);
	SPFSetUserData(&self->spf, self);
}

void CMShow(CM* self)
{
	if(!SPFGetCurrentPanel(&self->spf)) {
		CMiCreateConnectionMenu(self);
	}

	SPFDraw(&self->spf);
}

void CMClearUI(CM* self)
{
	/* close all open panels */
	SPFPanel* panel;
	while((panel = SPFGetCurrentPanel(&self->spf)) != NULL) {
		SPFDestroyPanel(panel);
	}
}

void CMProcessKey(CM* self, u16 key)
{
	SPFProcessKey(&self->spf, key);
}

void CMProcess(CM* self)
{
	SPFProcess(&self->spf);
}
