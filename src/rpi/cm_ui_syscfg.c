#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

#define	FIELD_CLOSE	0
#define	FIELD_FLIP	1
#define	FIELD_PHOSPHOR	2
#define	FIELD_RAW	3

typedef struct {
	SPFLabel	lbl_close;
	SPFCheckbox	sys_close;
	SPFLabel	lbl_flip;
	SPFCheckbox	sys_flip;
	SPFLabel	lbl_phosphor;
	SPFLabel	lbl_phosphor_info;
	SPFInputChar	sys_phosphor;
	SPFLabel	lbl_raw;
	SPFCheckbox	sys_raw;

	SPFWidget*	widgets[10];
} CMSystemConfigMenu;

static void CMiSystemConfigMenuInitWidgets(SPFPanel* self);

void CMSystemConfigMenuCreate(SPFPanel* self)
{
	CMSystemConfigMenu* this = (CMSystemConfigMenu*) malloc(sizeof(CMSystemConfigMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMSystemConfigMenu));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "System Configuration");

	SPFShowCursor(self);

	CMiSystemConfigMenuInitWidgets(self);
}

void CMSystemConfigMenuCloseChange(SPFCheckbox* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	CM* cm = (CM*) SPFGetUserData(SPFGet(panel));
	BOOL state = SPFGetCheckboxState(self);

	if(state) {
		cm->flags |= CM_FLAG_CLOSE;
	} else {
		cm->flags &= ~CM_FLAG_CLOSE;
	}
}

void CMSystemConfigMenuFlipChange(SPFCheckbox* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	CM* cm = (CM*) SPFGetUserData(SPFGet(panel));
	BOOL state = SPFGetCheckboxState(self);

	CMSetFlipScreen(cm, state);
}

void CMSystemConfigMenuPhosphorChange(SPFInputChar* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	CM* cm = (CM*) SPFGetUserData(SPFGet(panel));
	char value = SPFGetInputCharValue(self);

	int color;
	switch(value) {
		default:
		case 'A':
			color = CM_FLAG_CAMBER;
			break;
		case 'G':
			color = CM_FLAG_CGREEN;
			break;
		case 'W':
			color = CM_FLAG_CWHITE;
			break;
	}
	CMSetPhosphorColor(cm, color);
}

void CMSystemConfigMenuRawChange(SPFCheckbox* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	CM* cm = (CM*) SPFGetUserData(SPFGet(panel));
	BOOL state = SPFGetCheckboxState(self);

	CMSetRawScreen(cm, state);
}

static void CMiSystemConfigMenuInitWidgets(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMSystemConfigMenu* this = (CMSystemConfigMenu*) SPFGetPanelUserData(self);

	SPFCreateLabel(&this->lbl_close, 2, 2, "Show menu on disconnect:", 0);
	SPFCreateCheckbox(&this->sys_close, FIELD_CLOSE, 2, 30, CMSystemConfigMenuCloseChange);
	SPFCreateLabel(&this->lbl_flip, 3, 2, "Flip screen:", 0);
	SPFCreateCheckbox(&this->sys_flip, FIELD_FLIP, 3, 30, CMSystemConfigMenuFlipChange);
	SPFCreateLabel(&this->lbl_phosphor, 4, 2, "Phosphor color:", 0);
	SPFCreateLabel(&this->lbl_phosphor_info, 4, 35, "(A=Amber, G=Green, W=White)", 0);
	SPFCreateInputChar(&this->sys_phosphor, FIELD_PHOSPHOR, 4, 30, "AWG", TRUE, CMSystemConfigMenuPhosphorChange);
	SPFCreateLabel(&this->lbl_raw, 5, 2, "Raw screen:", 0);
	SPFCreateCheckbox(&this->sys_raw, FIELD_RAW, 5, 30, CMSystemConfigMenuRawChange);

	SPFSetCheckboxState(&this->sys_close, !!(cm->flags & CM_FLAG_CLOSE));
	SPFSetCheckboxState(&this->sys_flip, !!(cm->flags & CM_FLAG_FLIP));
	SPFSetCheckboxState(&this->sys_raw, !!(cm->flags & CM_FLAG_RAW));

	char color;
	switch(cm->flags & CM_FLAG_CMASK) {
		default:
		case CM_FLAG_CAMBER:
			color = 'A';
			break;
		case CM_FLAG_CGREEN:
			color = 'G';
			break;
		case CM_FLAG_CWHITE:
			color = 'W';
			break;
	}

	SPFSetInputCharValue(&this->sys_phosphor, color);

	int id = 0;
	this->widgets[id++] = (SPFWidget*) &this->lbl_close;
	this->widgets[id++] = (SPFWidget*) &this->sys_close;
	this->widgets[id++] = (SPFWidget*) &this->lbl_flip;
	this->widgets[id++] = (SPFWidget*) &this->sys_flip;
	this->widgets[id++] = (SPFWidget*) &this->lbl_phosphor;
	this->widgets[id++] = (SPFWidget*) &this->sys_phosphor;
	this->widgets[id++] = (SPFWidget*) &this->lbl_phosphor_info;
	this->widgets[id++] = (SPFWidget*) &this->lbl_raw;
	this->widgets[id++] = (SPFWidget*) &this->sys_raw;
	this->widgets[id++] = NULL;

	SPFSetWidgets(self, this->widgets);
}

void CMSystemConfigMenuDestroy(SPFPanel* self)
{
	CMSystemConfigMenu* this = (CMSystemConfigMenu*) SPFGetPanelUserData(self);

	free(this);
}

void CMSystemConfigMenuDraw(SPFPanel* self)
{
	SPFPrintAt(self, 21, 2, "F4=End", 0);
	SPFPrintAt(self, 21, SPFGetWidth(self) - 8, "F12=Back", 0);
}

void CMSystemConfigMenuProcess(SPFPanel* self, u16 key)
{
	switch(key) {
		case VT220_KEY_F12:
			SPFDestroyPanel(self);
			break;
		default:
			break;
	}
}

static SPFPanelDefinition system_config_menu = {
	.init = CMSystemConfigMenuCreate,
	.draw = CMSystemConfigMenuDraw,
	.resize = NULL,
	.process = CMSystemConfigMenuProcess,
	.destroy = CMSystemConfigMenuDestroy
};

void CMiCreateSystemConfigMenu(CM* self)
{
	SPFCreatePanel(&self->spf, &system_config_menu);
}
