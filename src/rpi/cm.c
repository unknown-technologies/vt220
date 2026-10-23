#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cm.h"
#include "tty.h"
#include "pty.h"
#include "telnet.h"

void CMInit(CM* self, VT220* vt, char** envp)
{
	memset(self, 0, sizeof(CM));

	TTYInit(&self->tty);
	PTYInit(&self->pty);
	TELNETInit(&self->telnet);

	self->envp = envp;

	self->autoconnect = -1;

	self->connections_max = 28;
	self->connections = (CMCONN*) malloc(self->connections_max * sizeof(CMCONN));
	memset(self->connections, 0, self->connections_max * sizeof(CMCONN));

	/* default connections */
	self->connections_cnt = 1;
	self->connections[0].mode = CM_MODE_PTY;
	strcpy(self->connections[0].name, "LOCAL SHELL");

	CMiInitUI(self, vt);

	CMLoadState(self);
}

void CMDestroy(CM* self)
{
	CMDisconnect(self);

	TELNETDestroy(&self->telnet);

	SPFDestroy(&self->spf);
}

void CMUpdateHandlers(CM* self)
{
	self->tty.rx = self->rx;
	self->tty.rxe = self->rxe;
	self->pty.rx = self->rx;
	self->pty.rxe = self->rxe;
	self->telnet.rx = self->rx;
	self->telnet.rxe = self->rxe;
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
	}

	self->mode = CM_MODE_NONE;
}

void CMPoll(CM* self)
{
	switch(self->mode) {
		case CM_MODE_TTY:
			TTYPoll(&self->tty);
			break;
		case CM_MODE_PTY:
			PTYPoll(&self->pty);
			break;
		case CM_MODE_TELNET:
			TELNETPoll(&self->telnet);
			break;
	}
}

void CMSend(CM* self, unsigned char c)
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
	}
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

	if(id + 1 == self->connections_cnt) {
		self->connections_cnt--;
	} else {
		memmove(con, &con[1], (self->connections_cnt - id) * sizeof(CMCONN));
		self->connections_cnt--;
	}
}
