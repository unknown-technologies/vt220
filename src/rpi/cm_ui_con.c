#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

#define	FIELD_NAME	0
#define	FIELD_MODE	1
#define	FIELD_TARGET	2
#define	FIELD_PORT	3
#define	FIELD_USER	4

typedef struct {
	int		id;

	SPFLabel	lbl_name;
	SPFInputStr	con_name;
	SPFLabel	lbl_mode;
	SPFLabel	lbl_mode_info;
	SPFInputChar	con_mode;
	SPFLabel	lbl_target;
	SPFInputStr	con_target;
	SPFLabel	lbl_port;
	SPFInputDec	con_port;
	SPFLabel	lbl_port_info;
	SPFLabel	lbl_user;
	SPFInputStr	con_user;

	SPFWidget*	widgets[13];
} CMConnectionEditMenu;

void CMConnectionEditMenuCreate(SPFPanel* self)
{
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) malloc(sizeof(CMConnectionEditMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMConnectionEditMenu));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "Edit Connection");

	SPFShowCursor(self);
}

static void CMConnectionEditorMenuUpdateWidgets(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(self);
	CMCONN* con = &cm->connections[this->id];

	SPFWidget* focus = SPFGetFocus(self);

	int id = 0;
	this->widgets[id++] = (SPFWidget*) &this->lbl_name;
	this->widgets[id++] = (SPFWidget*) &this->con_name;
	this->widgets[id++] = (SPFWidget*) &this->lbl_mode;
	this->widgets[id++] = (SPFWidget*) &this->con_mode;
	this->widgets[id++] = (SPFWidget*) &this->lbl_mode_info;
	if(con->mode == CM_MODE_TTY || con->mode == CM_MODE_TELNET || con->mode == CM_MODE_SSH) {
		this->widgets[id++] = (SPFWidget*) &this->lbl_target;
		this->widgets[id++] = (SPFWidget*) &this->con_target;
		if(con->mode == CM_MODE_TELNET || con->mode == CM_MODE_SSH) {
			this->widgets[id++] = (SPFWidget*) &this->lbl_port;
			this->widgets[id++] = (SPFWidget*) &this->con_port;
			this->widgets[id++] = (SPFWidget*) &this->lbl_port_info;
			if(con->mode == CM_MODE_SSH) {
				this->widgets[id++] = (SPFWidget*) &this->lbl_user;
				this->widgets[id++] = (SPFWidget*) &this->con_user;
			}
		}
	}
	this->widgets[id++] = NULL;

	SPFSetWidgets(self, this->widgets);
	if(focus) {
		SPFSetFocus(self, focus);
	}
}

void CMConnectionEditorMenuModeChange(SPFInputChar* self)
{
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	CM* cm = (CM*) SPFGetUserData(SPFGet(panel));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(panel);
	CMCONN* con = &cm->connections[this->id];

	char value = SPFGetInputCharValue(self);
	switch(value) {
		case 'Y':
			con->mode = CM_MODE_TTY;
			break;
		case 'P':
			con->mode = CM_MODE_PTY;
			break;
		case 'T':
			con->mode = CM_MODE_TELNET;
			break;
		case 'S':
			con->mode = CM_MODE_SSH;
			break;
	}

	CMConnectionEditorMenuUpdateWidgets(panel);
}

void CMConnectionEditMenuInitWidgets(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(self);
	CMCONN* con = &cm->connections[this->id];

	if(con->port == 0) {
		con->port = 1;
	}

	SPFCreateLabel(&this->lbl_name, 3, 2, "Name:", 0);
	SPFCreateInputStr(&this->con_name, FIELD_NAME, 3, 10, con->name, 24, NULL);
	SPFCreateLabel(&this->lbl_mode, 4, 2, "Type:", 0);
	SPFCreateLabel(&this->lbl_mode_info, 4, 17, "(Y=TTY, P=PTY, T=TELNET, S=SSH)", 0);
	SPFCreateInputChar(&this->con_mode, FIELD_MODE, 4, 10, "YPTS", TRUE, CMConnectionEditorMenuModeChange);
	SPFCreateLabel(&this->lbl_target, 5, 2, "Target:", 0);
	SPFCreateInputStr(&this->con_target, FIELD_TARGET, 5, 10, con->target, 70, NULL);
	SPFCreateLabel(&this->lbl_port, 6, 2, "Port:", 0);
	SPFCreateLabel(&this->lbl_port_info, 6, 17, "(1-65535)", 0);
	SPFCreateInputDec(&this->con_port, FIELD_PORT, 6, 10, 5, 1, 65535, con->port, NULL);
	SPFCreateLabel(&this->lbl_user, 7, 2, "User:", 0);
	SPFCreateInputStr(&this->con_user, FIELD_USER, 7, 10, con->username, 60, NULL);

	char mode = '?';
	switch(con->mode) {
		case CM_MODE_TTY:
			mode = 'Y';
			break;
		case CM_MODE_PTY:
			mode = 'P';
			break;
		case CM_MODE_TELNET:
			mode = 'T';
			break;
		case CM_MODE_SSH:
			mode = 'S';
			break;
	}

	SPFSetInputCharValue(&this->con_mode, mode);

	CMConnectionEditorMenuUpdateWidgets(self);
}

void CMConnectionEditMenuDestroy(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(self);
	CMCONN* con = &cm->connections[this->id];

	if(con->mode == CM_MODE_NONE) {
		/* delete this connection */
		CMDeleteConnection(cm, this->id);
	} else {
		int port = SPFGetInputDecValue(&this->con_port);
		if(port > 65535) {
			port = 65535;
		} else if(port < 1) {
			port = 1;
		}
		con->port = port;
	}

	free(this);
}

void CMConnectionEditMenuDraw(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(self);
	CMCONN* con = &cm->connections[this->id];

	SPFPrintAt(self, 1, 2, "Connection ID=", 0);
	SPFPrintNumber(self, this->id + 1, 3, 0, 0);

	if(con->mode == CM_MODE_SSH) {
		SPFPrintAt(self, 9, 2, "Key:", 0);
		if(con->status & CM_HAS_HOSTKEY) {
			char buf[65];
			for(int i = 0; i < 32; i++) {
				unsigned char c = con->hostkey[i];
				char a = c >> 4;
				if(a < 10) {
					a += '0';
				} else {
					a += 'A' - 10;
				}
				char b = c & 0x0F;
				if(b < 10) {
					b += '0';
				} else {
					b += 'A' - 10;
				}
				buf[2 * i + 0] = a;
				buf[2 * i + 1] = b;
			}
			buf[64] = 0;
			SPFFieldAt(self, 9, 10, buf, 64, SGR_UNDERSCORE);
		} else {
			SPFFieldAt(self, 9, 10, "<not set>", 64, SGR_UNDERSCORE);
		}
	}

	int onequarter = SPFGetWidth(self) / 4;
	int twoquarters = SPFGetWidth(self) * 2 / 4;
	int threequarters = SPFGetWidth(self) * 3 / 4;

	SPFPrintAt(self, 21, 2, "F4=End", 0);
	if(CMGetAutoConnect(cm) == this->id) {
		SPFPrintAt(self, 21, onequarter - 11, "F9=Unmark Autoconnect", 0);
	} else {
		SPFPrintAt(self, 21, onequarter - 10, "F9=Mark Autoconnect", 0);
	}
	if(cm->mode == CM_MODE_NONE) {
		SPFPrintAt(self, 21, twoquarters - 5, "F10=Delete", 0);
	}
	if(con->mode == CM_MODE_SSH && (con->status & CM_HAS_HOSTKEY)) {
		SPFPrintAt(self, 21, threequarters - 7, "F11=Clear Key", 0);
	}
	SPFPrintAt(self, 21, SPFGetWidth(self) - 8, "F12=Back", 0);
}

void CMConnectionEditMenuProcess(SPFPanel* self, u16 key)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionEditMenu* this = (CMConnectionEditMenu*) SPFGetPanelUserData(self);
	CMCONN* con = &cm->connections[this->id];

	switch(key) {
		case VT220_KEY_F9:
			if(CMGetAutoConnect(cm) == this->id) {
				CMSetAutoConnect(cm, CM_NONE);
			} else {
				CMSetAutoConnect(cm, this->id);
			}
			SPFRepaint(SPFGet(self));
			break;
		case VT220_KEY_F10: {
			if(cm->mode == CM_MODE_NONE) {
				int id = this->id;
				SPFDestroyPanel(self);
				CMDeleteConnection(cm, id);
			}
			break;
		}
		case VT220_KEY_F11:
			if(con->mode == CM_MODE_SSH && (con->status & CM_HAS_HOSTKEY)) {
				con->status &= ~CM_HAS_HOSTKEY;
				memset(con->hostkey, 0, sizeof(con->hostkey));
				SPFRepaint(SPFGet(self));
			}
			break;
		case VT220_KEY_F12:
			SPFDestroyPanel(self);
			break;
		default:
			break;
	}
}

static SPFPanelDefinition connection_edit_menu = {
	.init = CMConnectionEditMenuCreate,
	.draw = CMConnectionEditMenuDraw,
	.resize = NULL,
	.process = CMConnectionEditMenuProcess,
	.destroy = CMConnectionEditMenuDestroy
};

void CMiCreateConnectionEditMenu(CM* self, int id)
{
	SPFPanel* panel = SPFCreatePanel(&self->spf, &connection_edit_menu);
	CMConnectionEditMenu* menu = (CMConnectionEditMenu*) SPFGetPanelUserData(panel);
	menu->id = id;

	CMConnectionEditMenuInitWidgets(panel);
}
