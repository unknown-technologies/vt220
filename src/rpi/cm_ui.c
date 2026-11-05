#include <string.h>

#include "types.h"
#include "vt.h"
#include "cm.h"

bool CMiInitUI(CM* self, VT220* vt)
{
	if(!SPFInit(&self->spf, vt)) {
		return false;
	}

	SPFSetUserData(&self->spf, self);

	return true;
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
