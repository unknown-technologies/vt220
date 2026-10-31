#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libssh/libssh.h>

#include "cm.h"
#include "tty.h"
#include "pty.h"
#include "ssh.h"
#include "telnet.h"

static int CMRXE(void* ptr)
{
	CM* self = (CM*) ptr;
	if(self->rxe) {
		return self->rxe();
	} else {
		return 0;
	}
}

static void CMRX(void* ptr, unsigned char c)
{
	CM* self = (CM*) ptr;
	CMOSCReceive(self, c);
}

static int CMHostKeyCheck(void* ptr, unsigned char* hostkey, int len)
{
	ASSERT(ptr);
	CM* self = (CM*) ptr;
	CMCONN* con = &self->connections[self->current_connection];
	ASSERT(con->mode == CM_MODE_SSH);
	if(!(con->flags & CM_HAS_HOSTKEY)) {
		return SSH_KNOWN_HOSTS_UNKNOWN;
	} else if(len != 32) {
		return SSH_KNOWN_HOSTS_OTHER;
	} else if(!memcmp(con->hostkey, hostkey, len)) {
		return SSH_KNOWN_HOSTS_OK;
	} else {
		return SSH_KNOWN_HOSTS_CHANGED;
	}
}

static void CMHostKeyAdd(void* ptr, unsigned char* hostkey, int len)
{
	ASSERT(ptr);
	CM* self = (CM*) ptr;
	CMCONN* con = &self->connections[self->current_connection];
	ASSERT(con->mode == CM_MODE_SSH);
	if(len != 32) {
		return;
	}
	memcpy(con->hostkey, hostkey, len);
	con->flags |= CM_HAS_HOSTKEY;

	/* important: save the new host key to disk */
	CMSaveState(self);
}

void CMInit(CM* self, VT220* vt, char** envp)
{
	memset(self, 0, sizeof(CM));

	TTYInit(&self->tty);
	PTYInit(&self->pty);
	TELNETInit(&self->telnet);
	SSHInit(&self->ssh);

	self->envp = envp;

	self->flags = CM_FLAG_CLOSE | CM_FLAG_CAMBER;
	self->autoconnect = -1;

	self->connections_max = 28;
	self->connections = (CMCONN*) malloc(self->connections_max * sizeof(CMCONN));
	memset(self->connections, 0, self->connections_max * sizeof(CMCONN));

	/* default connections */
	self->connections_cnt = 1;
	self->connections[0].mode = CM_MODE_PTY;
	strcpy(self->connections[0].name, "LOCAL SHELL");
	self->connections[0].flags = CM_FLAG_OSC | CM_FLAG_UTF8;

	PTYSetUserData(&self->pty, (void*) self);
	self->pty.rx = CMRX;
	self->pty.rxe = CMRXE;

	TTYSetUserData(&self->tty, (void*) self);
	self->tty.rx = CMRX;
	self->tty.rxe = CMRXE;

	TELNETSetUserData(&self->telnet, (void*) self);
	self->telnet.rx = CMRX;
	self->telnet.rxe = CMRXE;

	SSHSetUserData(&self->ssh, (void*) self);
	self->ssh.rx = CMRX;
	self->ssh.rxe = CMRXE;
	self->ssh.hostkey = CMHostKeyCheck;
	self->ssh.add_hostkey = CMHostKeyAdd;

	CMiInitUI(self, vt);
	CMInitOSCFilter(self);

	CMLoadState(self);
}

void CMDestroy(CM* self)
{
	CMDisconnect(self);

	SSHDestroy(&self->ssh);
	TELNETDestroy(&self->telnet);

	SPFDestroy(&self->spf);
}

void CMDisconnect(CM* self)
{
	switch(self->mode) {
		case CM_MODE_TTY:
			TTYClose(&self->tty);
			break;
		case CM_MODE_PTY:
			PTYClose(&self->pty);
			break;
		case CM_MODE_TELNET:
			TELNETDisconnect(&self->telnet);
			break;
		case CM_MODE_SSH:
			SSHDisconnect(&self->ssh);
			break;
	}

	self->mode = CM_MODE_NONE;
}

void CMiDisconnected(CM* self)
{
	self->mode = CM_MODE_NONE;

	if(self->input_received && self->flags & CM_FLAG_CLOSE) {
		CMSetVisible(self, 1);
	}
}

void CMPoll(CM* self)
{
	switch(self->mode) {
		case CM_MODE_TTY:
			TTYPoll(&self->tty);
			if(!TTYIsOpen(&self->tty)) {
				CMiDisconnected(self);
			}
			break;
		case CM_MODE_PTY:
			PTYPoll(&self->pty);
			if(!PTYIsAlive(&self->pty)) {
				CMiDisconnected(self);
			}
			break;
		case CM_MODE_TELNET:
			TELNETPoll(&self->telnet);
			if(!TELNETIsConnected(&self->telnet)) {
				CMiDisconnected(self);
			}
			break;
		case CM_MODE_SSH:
			SSHPoll(&self->ssh);
			if(!SSHIsConnected(&self->ssh)) {
				CMiDisconnected(self);
			}
			break;
	}
}

void CMiSend(CM* self, unsigned char c)
{
	switch(self->mode) {
		case CM_MODE_TTY:
			TTYSend(&self->tty, c);
			break;
		case CM_MODE_PTY:
			PTYSend(&self->pty, c);
			break;
		case CM_MODE_TELNET:
			TELNETSend(&self->telnet, c);
			break;
		case CM_MODE_SSH:
			SSHSend(&self->ssh, c);
			break;
	}
}

void CMSend(CM* self, unsigned char c)
{
	self->input_received = TRUE;

	if(CMIsUTF8Enabled(self)) {
		CMUTF8Transmit(self, c);
	} else {
		CMiSend(self, c);
	}
}

void CMBreak(CM* self)
{
	switch(self->mode) {
		case CM_MODE_TTY:
			TTYBreak(&self->tty);
			break;
		case CM_MODE_PTY:
			PTYBreak(&self->pty);
			break;
		case CM_MODE_TELNET:
			TELNETBreak(&self->telnet);
			break;
		case CM_MODE_SSH:
			SSHBreak(&self->ssh);
			break;
	}
}

void CMResize(CM* self, unsigned int width, unsigned int height)
{
	SPFResize(&self->spf, (int) width, (int) height);

	switch(self->mode) {
		case CM_MODE_TTY:
			TTYResize(&self->tty, width, height);
			break;
		case CM_MODE_PTY:
			PTYResize(&self->pty, width, height);
			break;
		case CM_MODE_TELNET:
			TELNETResize(&self->telnet, width, height);
			break;
		case CM_MODE_SSH:
			SSHResize(&self->ssh, width, height);
			break;
	}
}

void CMClearComm(CM* self)
{
	CMResetOSCFilter(self);
	CMResetUTF8(self);
}

void CMSetBaudRate(CM* self, unsigned int rx_rate, unsigned int tx_rate)
{
	TTYSetBaudRate(&self->tty, rx_rate, tx_rate);
}

void CMSetFlowControl(CM* self, bool enable_xoff)
{
	TTYSetFlowControl(&self->tty, enable_xoff);
}

void CMSetFormat(CM* self, unsigned int data, unsigned int stop, unsigned int parity)
{
	TTYSetFormat(&self->tty, data, stop, parity);
}

void CMOpenTTY(CM* self, const char* path)
{
	TTYClose(&self->tty);
	TTYOpen(&self->tty, path);
}

void CMSetVisible(CM* self, int visible)
{
	self->visible = visible;
	if(visible) {
		CMShow(self);
	} else {
		CMClearUI(self);
	}

	VT220SetKeyboardGrabbed(SPFGetVT(&self->spf), visible);
}

int CMIsVisible(CM* self)
{
	return self->visible;
}

VTScreen* CMGetScreen(CM* self)
{
	return SPFGetScreen(&self->spf);
}

void CMConnect(CM* self, int id)
{
	ASSERT(self);
	ASSERT(id >= 0 && id < self->connections_cnt);

	CMCONN* con = &self->connections[id];
	VT220* vt = SPFGetVT(&self->spf);

	CMDisconnect(self);

	self->current_connection = id;
	self->input_received = 0;

	CMResetUTF8(self);
	CMSetUTF8Enabled(self, !!(con->flags & CM_FLAG_UTF8));

	CMResetOSCFilter(self);
	CMSetOSCEnabled(self, !!(con->flags & CM_FLAG_OSC));

	switch(con->mode) {
		case CM_MODE_PTY: {
			char* argv_agetty[] = { "/bin/agetty", "-8", "-s", "--local-line", "--noreset", "--noclear", "-",
				"vt220", NULL };
			char* argv_helper[] = { "/bin/vt220-agetty", NULL };

			char** argv;

			if(!access(*argv_helper, X_OK)) {
				argv = argv_helper;
			} else {
				argv = argv_agetty;
			}

			self->mode = CM_MODE_PTY;

			VT220CursorHome(vt);
			VT220EraseScreen(vt);
			VT220SaveCursor(vt);
			VT220ClearComm(vt);

			PTYOpen(&self->pty, argv, self->envp);
			CMSetVisible(self, 0);
			break;
		}
		case CM_MODE_TTY:
			self->mode = CM_MODE_TTY;

			VT220CursorHome(vt);
			VT220EraseScreen(vt);
			VT220SaveCursor(vt);
			VT220ClearComm(vt);

			if(TTYOpen(&self->tty, con->target)) {
				VT220ReceiveText(vt, "READY\r\n");
			}
			CMSetVisible(self, 0);
			break;
		case CM_MODE_TELNET:
			self->mode = CM_MODE_TELNET;

			VT220CursorHome(vt);
			VT220EraseScreen(vt);
			VT220SaveCursor(vt);
			VT220ClearComm(vt);

			TELNETConnect(&self->telnet, con->target, con->port);
			CMSetVisible(self, 0);
			break;
		case CM_MODE_SSH:
			self->mode = CM_MODE_SSH;

			VT220CursorHome(vt);
			VT220EraseScreen(vt);
			VT220SaveCursor(vt);
			VT220ClearComm(vt);

			SSHConnect(&self->ssh, con->target, con->port, con->username);
			CMSetVisible(self, 0);
			break;
		default:
			CMSetVisible(self, 1);
			break;
	}
}

void CMSetAutoConnect(CM* self, int id)
{
	ASSERT(self);
	ASSERT(id == -1 || (id >= 0 && id < self->connections_cnt));

	self->autoconnect = id;
}

int CMGetAutoConnect(CM* self)
{
	ASSERT(self);

	return self->autoconnect;
}

void CMAutoConnect(CM* self)
{
	ASSERT(self);

	if(self->autoconnect >= 0 && self->autoconnect < self->connections_cnt) {
		CMConnect(self, self->autoconnect);
	}
}

void CMDeleteConnection(CM* self, int id)
{
	ASSERT(self);

	if(id < 0 || id >= self->connections_cnt) {
		return;
	}

	CMCONN* con = &self->connections[id];
	memset(con, 0, sizeof(CMCONN));

	/* adjust autoconnect marker */
	if(self->autoconnect == id) {
		self->autoconnect = -1;
	} else if(self->autoconnect >= 0 && self->autoconnect < self->connections_cnt && self->autoconnect > id) {
		self->autoconnect--;
	}

	self->connections_cnt--;
	if(id < self->connections_cnt) {
		/* NOTE: this works because the original connections_cnt - id - 1 happens to be the current
		 * connections_cnt - id now; without the -1 we would copy one element too much */
		memmove(con, &con[1], (self->connections_cnt - id) * sizeof(CMCONN));
	}
}

void CMUpdateScreenRotation(CM* self)
{
	if(self->screen_rotation) {
		self->screen_rotation((self->flags & CM_FLAG_FLIP) ? CM_ROTATION_180 : CM_ROTATION_0);
	}
}

void CMSetFlipScreen(CM* self, BOOL flipped)
{
	if(flipped) {
		self->flags |= CM_FLAG_FLIP;
	} else {
		self->flags &= ~CM_FLAG_FLIP;
	}

	CMUpdateScreenRotation(self);
}

void CMUpdatePhosphorColor(CM* self)
{
	int color;
	switch(self->flags & CM_FLAG_CMASK) {
		default:
		case CM_FLAG_CAMBER:
			color = VT220_SCREEN_COLOR_AMBER;
			break;
		case CM_FLAG_CGREEN:
			color = VT220_SCREEN_COLOR_GREEN;
			break;
		case CM_FLAG_CWHITE:
			color = VT220_SCREEN_COLOR_WHITE;
			break;
	}
	VT220SetScreenColor(SPFGetVT(&self->spf), color);
}

void CMSetPhosphorColor(CM* self, int color)
{
	self->flags = (self->flags & ~CM_FLAG_CMASK) | (color & CM_FLAG_CMASK);

	CMUpdatePhosphorColor(self);
}

void CMUpdateRawScreen(CM* self)
{
	if(self->screen_raw) {
		self->screen_raw(!!(self->flags & CM_FLAG_RAW));
	}
}

void CMSetRawScreen(CM* self, BOOL raw)
{
	if(raw) {
		self->flags |= CM_FLAG_RAW;
	} else {
		self->flags &= ~CM_FLAG_RAW;
	}

	CMUpdateRawScreen(self);
}
