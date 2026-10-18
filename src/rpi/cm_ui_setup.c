#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

typedef struct {
	char		input[3];
	int		input_len;
	int		input_pos;
} CMSetupMenu;

void CMSetupMenuCreate(SPFPanel* self)
{
	CMSetupMenu* this = (CMSetupMenu*) malloc(sizeof(CMSetupMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMSetupMenu));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "Setup Menu");
	SPFShowCursor(self);
}

void CMSetupMenuDestroy(SPFPanel* self)
{
	CM* cm = (CM*) SPFGetUserData(SPFGet(self));
	CMSetupMenu* this = (CMSetupMenu*) SPFGetPanelUserData(self);

	free(this);

	CMSaveState(cm);
}

void CMSetupMenuUpdateCursor(SPFPanel* self)
{
	CMSetupMenu* this = (CMSetupMenu*) SPFGetPanelUserData(self);
	SPFSetCursor(self, 17, 14 + this->input_pos);
}

void CMSetupMenuDraw(SPFPanel* self)
{
	CMSetupMenu* this = (CMSetupMenu*) SPFGetPanelUserData(self);

	SPFPrintCenter(self, 2, "CM MICROCODE \xA9 COPYRIGHT UNKNOWN TECHNOLOGIES 2026", 0);
	SPFPrintAt(self, 5, 2, "Select option; press ENTER", 0);

	SPFPrintAt(self, 7, 3, "Option", SGR_BOLD);
	SPFMoveRightN(self, 3);
	SPFPrint(self, "Description", SGR_BOLD);
	SPFPrintAt(self,  8, 5,   "1      Customize Connections", 0);
	SPFPrintAt(self,  9, 5,   "2      Key Management", 0);
	SPFPrintAt(self, 10, 5,   "3      Diagnostics", 0);

	SPFPrintAt(self, 17, 2, "Select ===> ", 0);
	SPFPrint(self, this->input, 0);

	SPFPrintAt(self, 21, 2, "F4=End", 0);
	SPFGoto(self, 21, SPFGetWidth(self) - 14);
	SPFPrint(self, "F12=Exit Setup", 0);

	CMSetupMenuUpdateCursor(self);
}

void CMSetupMenuResize(SPFPanel* self, int width, int height)
{
	(void) height;
	CMSetupMenu* this = (CMSetupMenu*) SPFGetPanelUserData(self);

	/* truncate command line to screen width */
	if(this->input_len >= width - 7) {
		this->input_len = width - 7;
		size_t len = sizeof(this->input) - this->input_len;
		memset(&this->input[this->input_len], 0, len);
		if(this->input_pos > this->input_len) {
			this->input_pos = this->input_len;
			CMSetupMenuUpdateCursor(self);
		}
	}
}

void CMSetupMenuProcess(SPFPanel* self, u16 key)
{
	SPF* spf = SPFGet(self);
	CM* cm = (CM*) SPFGetUserData(spf);
	CMSetupMenu* this = (CMSetupMenu*) SPFGetPanelUserData(self);

	const int maxlen = sizeof(this->input) - 1;

	switch(key) {
		case VT220_KEY_F12:
			SPFDestroyPanel(self);
			break;
		case CR:
		case VT220_KEY_KP_ENTER:
			if(this->input[0]) {
				if(this->input[1] || this->input[0] < '1' || this->input[0] > '3') {
					SPFPrintAt(self, 19, 2, "Invalid option", 0);
				} else {
					int id = this->input[0] - '0';
					switch(id) {
						case 1:
							CMiCreateConnectionCustomizationMenu(cm);
							break;
						case 3:
							CMiCreateDiagnosticsMenu(cm);
							break;
					}

					this->input_pos = 0;
					this->input_len = 0;
					memset(this->input, 0, sizeof(this->input));
					SPFRepaint(spf);
				}
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
				CMSetupMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VT220_KEY_RIGHT:
			if(this->input_pos < this->input_len) {
				this->input_pos++;
				CMSetupMenuUpdateCursor(self);
				VT220ResetCursorTime(SPFGetVT(spf));
			}
			break;
		case VTCTRL('A'):
			this->input_pos = 0;
			CMSetupMenuUpdateCursor(self);
			VT220ResetCursorTime(SPFGetVT(spf));
			break;
		case VTCTRL('E'):
			this->input_pos = this->input_len;
			CMSetupMenuUpdateCursor(self);
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

static SPFPanelDefinition setup_menu = {
	.init = CMSetupMenuCreate,
	.draw = CMSetupMenuDraw,
	.resize = CMSetupMenuResize,
	.process = CMSetupMenuProcess,
	.destroy = CMSetupMenuDestroy
};

void CMiCreateSetupMenu(CM* self)
{
	SPFCreatePanel(&self->spf, &setup_menu);
}
