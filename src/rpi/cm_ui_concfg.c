#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

typedef struct {
	char		input[4];
	int		input_len;
	int		input_pos;
} CMConnectionCustomizationMenu;

void CMConnectionCustomizationMenuCreate(SPFPanel* self)
{
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) malloc(sizeof(CMConnectionCustomizationMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMConnectionCustomizationMenu));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "Connection Customization");
	SPFShowCursor(self);
}

void CMConnectionCustomizationMenuDestroy(SPFPanel* self)
{
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);

	free(this);
}

void CMConnectionCustomizationMenuUpdateCursor(SPFPanel* self)
{
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);
	SPFSetCursor(self, 19, 14 + this->input_pos);
}

void CMConnectionCustomizationMenuDraw(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);

	SPFPrintAt(self, 1, 2, "Enter a number (NUM) to edit / C to create a connection, then press ENTER", 0);

	SPFPrintAt(self, 3, 2, "NUM", SGR_BOLD);
	SPFMoveRight(self);
	SPFPrint(self, "NAME", SGR_BOLD);
	SPFPrintAt(self, 3, 32, "TYPE", SGR_BOLD);
	SPFPrintAt(self, 3, 42, "NUM", SGR_BOLD);
	SPFMoveRight(self);
	SPFPrint(self, "NAME", SGR_BOLD);
	SPFPrintAt(self, 3, 72, "TYPE", SGR_BOLD);

	/* print the connection table */
	for(int i = 0; i < cm->connections_cnt; i++) {
		CMCONN* con = &cm->connections[i];
		int base;
		int line = 4 + i;
		if(i < 14) {
			base = 2;
		} else {
			line -= 14;
			base = 42;
		}

		SPFGoto(self, line, base);
		SPFPrintNumber(self, i + 1, 3, 1, 0);
		if(i == CMGetAutoConnect(cm)) {
			SPFPutc(self, '*', 0);
		} else {
			SPFMoveRight(self);
		}
		SPFPrint(self, con->name, 0);
		SPFGoto(self, line, base + 30);
		switch(con->mode) {
			case CM_MODE_PTY:
				SPFPrint(self, "PTY", 0);
				break;
			case CM_MODE_TTY:
				SPFPrint(self, "TTY", 0);
				break;
			case CM_MODE_TELNET:
				SPFPrint(self, "TELNET", 0);
				break;
			case CM_MODE_SSH:
				SPFPrint(self, "SSH", 0);
				break;
		}
	}

	SPFPrintAt(self, 19, 2, "Select ===> ", 0);
	SPFPrint(self, this->input, 0);

	SPFPrintAt(self, 21, 2, "F4=End", 0);
	SPFGoto(self, 21, SPFGetWidth(self) - 8);
	SPFPrint(self, "F12=Back", 0);

	CMConnectionCustomizationMenuUpdateCursor(self);
}

void CMConnectionCustomizationMenuResize(SPFPanel* self, int width, int height)
{
	(void) height;
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);

	/* truncate command line to screen width */
	if(this->input_len >= width - 7) {
		this->input_len = width - 7;
		size_t len = sizeof(this->input) - this->input_len;
		memset(&this->input[this->input_len], 0, len);
		if(this->input_pos > this->input_len) {
			this->input_pos = this->input_len;
			CMConnectionCustomizationMenuUpdateCursor(self);
		}
	}
}

void CMConnectionCustomizationMenuEnter(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);

	if((this->input[0] == 'c' || this->input[0] == 'C') && !this->input[1]) {
		/* create new connection */
		if(cm->mode != CM_MODE_NONE) {
			SPFPrintAt(self, 20, 2, "Cannot create new connections while connected", 0);
			return;
		}
		if(cm->connections_cnt >= cm->connections_max) {
			SPFPrintAt(self, 20, 2, "Cannot create any more connections", 0);
			return;
		} else {
			int id = cm->connections_cnt++;
			CMCONN* con = &cm->connections[id];
			memset(con, 0, sizeof(CMCONN));
			CMiCreateConnectionEditMenu(cm, id);
		}
	} else {
		int id = 0;
		for(int i = 0; i < this->input_len; i++) {
			char c = this->input[i];
			if(c >= '0' && c <= '9') {
				id = (id * 10) + c - '0';
			} else {
				SPFPrintAt(self, 20, 2, "Invalid input", 0);
				return;
			}
		}

		if(id < 1 || id > cm->connections_cnt) {
			SPFPrintAt(self, 20, 2, "Invalid selection", 0);
			return;
		}

		CMiCreateConnectionEditMenu(cm, id - 1);
	}

	this->input_pos = 0;
	this->input_len = 0;
	memset(this->input, 0, sizeof(this->input));
	SPFRepaint(SPFGet(self));
}

void CMConnectionCustomizationMenuProcess(SPFPanel* self, u16 key)
{
	SPF* spf = SPFGet(self);
	CMConnectionCustomizationMenu* this = (CMConnectionCustomizationMenu*) SPFGetPanelUserData(self);

	const int maxlen = sizeof(this->input) - 1;

	switch(key) {
		case VT220_KEY_F12:
			SPFDestroyPanel(self);
			break;
		case CR:
		case VT220_KEY_KP_ENTER:
			if(this->input[0]) {
				CMConnectionCustomizationMenuEnter(self);
			}
			break;
		case BS:
		case DEL:
			if(this->input_pos > 0) {
				for(int i = this->input_pos - 1; i < this->input_len; i++) {
					this->input[i] = this->input[i + 1];
				}
				this->input_len--;
				this->input_pos--;
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VT220_KEY_REMOVE:
			if(this->input_pos < this->input_len) {
				for(int i = this->input_pos; i < this->input_len; i++) {
					this->input[i] = this->input[i + 1];
				}
				this->input_len--;
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VT220_KEY_LEFT:
			if(this->input_pos > 0) {
				this->input_pos--;
				CMConnectionCustomizationMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VT220_KEY_RIGHT:
			if(this->input_pos < this->input_len) {
				this->input_pos++;
				CMConnectionCustomizationMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VTCTRL('A'):
			this->input_pos = 0;
			CMConnectionCustomizationMenuUpdateCursor(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			break;
		case VTCTRL('E'):
			this->input_pos = this->input_len;
			CMConnectionCustomizationMenuUpdateCursor(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			break;
		case VTCTRL('U'):
			this->input_pos = 0;
			this->input_len = 0;
			memset(this->input, 0, sizeof(this->input));
			SPFRepaint(spf);
			break;
		case VT220_KEY_KP_0:
		case VT220_KEY_KP_1:
		case VT220_KEY_KP_2:
		case VT220_KEY_KP_3:
		case VT220_KEY_KP_4:
		case VT220_KEY_KP_5:
		case VT220_KEY_KP_6:
		case VT220_KEY_KP_7:
		case VT220_KEY_KP_8:
		case VT220_KEY_KP_9:
			key = key - VT220_KEY_KP_0 + '0';
			/* fallthrough */
		default:
			if(key >= 0x20 && key < 0x7F && this->input_pos < maxlen) {
				/* insert mode */
				int end = this->input_len - 1;
				if(end + 1 >= maxlen - 2) {
					end = maxlen - 2;
				}
				for(int i = end; i >= this->input_pos; i--) {
					this->input[i + 1] = this->input[i];
				}
				this->input[this->input_pos++] = (char) key;
				if(this->input_len < maxlen) {
					this->input_len++;
				}
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
	}
}

static SPFPanelDefinition connection_customization_menu = {
	.init = CMConnectionCustomizationMenuCreate,
	.draw = CMConnectionCustomizationMenuDraw,
	.resize = CMConnectionCustomizationMenuResize,
	.process = CMConnectionCustomizationMenuProcess,
	.destroy = CMConnectionCustomizationMenuDestroy
};

void CMiCreateConnectionCustomizationMenu(CM* self)
{
	SPFCreatePanel(&self->spf, &connection_customization_menu);
}
