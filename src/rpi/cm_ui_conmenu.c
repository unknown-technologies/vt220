#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

typedef struct {
	char		input[132];
	int		input_len;
	int		input_pos;
} CMConnectionMenu;

void CMConnectionMenuCreate(SPFPanel* self)
{
	CMConnectionMenu* this = (CMConnectionMenu*) malloc(sizeof(CMConnectionMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMConnectionMenu));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "CONNECTION MENU");
	SPFShowCursor(self);
}

void CMConnectionMenuDestroy(SPFPanel* self)
{
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);
	free(this);
}

void CMConnectionMenuUpdateCursor(SPFPanel* self)
{
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);
	SPFSetCursor(self, 21, 7 + this->input_pos);
}

void CMConnectionMenuDraw(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);

	SPFPrintAt(self, 1, 2, "Enter a number (NUM) or a name on the Command Line, then press ENTER", 0);

	SPFPrintAt(self, 3, 2, "NUM", SGR_BOLD);
	SPFMoveRight(self);
	SPFPrint(self, "NAME", SGR_BOLD);
	SPFPrintAt(self, 3, 32, "STATUS", SGR_BOLD);
	SPFPrintAt(self, 3, 42, "NUM", SGR_BOLD);
	SPFMoveRight(self);
	SPFPrint(self, "NAME", SGR_BOLD);
	SPFPrintAt(self, 3, 72, "STATUS", SGR_BOLD);

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
		SPFMoveRight(self);
		SPFPrint(self, con->name, 0);
		SPFGoto(self, line, base + 30);
		switch(con->mode) {
			case CM_MODE_PTY:
				SPFPrint(self, "UP", 0);
				break;
			case CM_MODE_TTY:
				if(!access(con->target, R_OK | W_OK)) {
					SPFPrint(self, "UP", 0);
				} else {
					SPFPrint(self, "DOWN", 0);
				}
				break;
			case CM_MODE_TELNET:
			case CM_MODE_SSH:
				SPFPrint(self, "?", 0);
				break;
		}
	}

	SPFPrintAt(self, 19, 2, "F4=End", 0);
	if(cm->mode != CM_MODE_NONE) {
		SPFPrintCenter(self, 19, "SHIFT-F5=Host Disconnect", 0);
	}
	SPFPrintAt(self, 19, SPFGetWidth(self) - 9, "F12=Setup", 0);

	SPFPrintAt(self, 21, 2, "===> ", 0);
	SPFPrint(self, this->input, 0);
	CMConnectionMenuUpdateCursor(self);
}

void CMConnectionMenuResize(SPFPanel* self, int width, int height)
{
	(void) height;
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);

	/* truncate command line to screen width */
	if(this->input_len >= width - 7) {
		this->input_len = width - 7;
		size_t len = sizeof(this->input) - this->input_len;
		memset(&this->input[this->input_len], 0, len);
		if(this->input_pos > this->input_len) {
			this->input_pos = this->input_len;
			CMConnectionMenuUpdateCursor(self);
		}
	}
}

void CMConnectionMenuCommand(SPFPanel* self, const char* cmd)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	for(int i = 0; i < cm->connections_cnt; i++) {
		CMCONN* con = &cm->connections[i];
		if(!strcmp(con->name, cmd)) {
			CMConnect(cm, i);

			/* no need to destroy the panel here, this happens automatically in CMConnect */
			return;
		}
	}

	SPFPrintAt(self, 22, 2, "Invalid command", 0);
	CMConnectionMenuUpdateCursor(self);
}

void CMConnectionMenuEnter(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);

	int id = 0;
	for(int i = 0; i < this->input_len; i++) {
		char c = this->input[i];
		if(c >= '0' && c <= '9') {
			id = (id * 10) + c - '0';
		} else {
			/* treat this as a command */
			CMConnectionMenuCommand(self, this->input);
			return;
		}
	}

	if(id < 1 || id > cm->connections_cnt) {
		SPFPrintAt(self, 22, 2, "Invalid connection number", 0);
		CMConnectionMenuUpdateCursor(self);
		return;
	}

	CMConnect(cm, id - 1);

	/* no need to destroy the panel here, this happens automatically in CMConnect */
}

void CMConnectionMenuProcess(SPFPanel* self, u16 key)
{
	SPF* spf = SPFGet(self);
	CM* cm = (CM*) SPFGetUserData(spf);
	CMConnectionMenu* this = (CMConnectionMenu*) SPFGetPanelUserData(self);

	switch(key) {
		case VT220_KEY_F12:
			CMiCreateSetupMenu(cm);
			break;
		case CR:
		case VT220_KEY_KP_ENTER:
			if(this->input[0]) {
				CMConnectionMenuEnter(self);
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
				CMConnectionMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VT220_KEY_RIGHT:
			if(this->input_pos < this->input_len) {
				this->input_pos++;
				CMConnectionMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VTCTRL('A'):
			this->input_pos = 0;
			CMConnectionMenuUpdateCursor(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			break;
		case VTCTRL('E'):
			this->input_pos = this->input_len;
			CMConnectionMenuUpdateCursor(self);
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
			if(key >= 0x20 && key < 0x7F && this->input_pos < SPFGetWidth(self) - 7) {
				/* insert mode */
				int end = this->input_len - 1;
				if(end + 1 >= SPFGetWidth(self) - 9) {
					end = SPFGetWidth(self) - 9;
				}
				for(int i = end; i >= this->input_pos; i--) {
					this->input[i + 1] = this->input[i];
				}
				this->input[this->input_pos++] = (char) key;
				if(this->input_len < SPFGetWidth(self) - 7) {
					this->input_len++;
				}
				SPFRepaint(spf);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
	}
}

static SPFPanelDefinition connection_menu = {
	.init = CMConnectionMenuCreate,
	.draw = CMConnectionMenuDraw,
	.resize = CMConnectionMenuResize,
	.process = CMConnectionMenuProcess,
	.destroy = CMConnectionMenuDestroy
};

void CMiCreateConnectionMenu(CM* self)
{
	SPFCreatePanel(&self->spf, &connection_menu);
}
