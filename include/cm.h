#ifndef __CM_H__
#define __CM_H__

#include "types.h"
#include "vt.h"
#include "renderer.h"
#include "telnet.h"
#include "ssh.h"
#include "tty.h"
#include "pty.h"
#include "spf.h"

#define	VTCTRL(x)	((x) - 0x40)

#define	CM_MODE_NONE	0
#define	CM_MODE_TTY	1
#define	CM_MODE_PTY	2
#define	CM_MODE_TELNET	3
#define	CM_MODE_SSH	4

#define	CM_NONE		-1

#define	CM_HAS_HOSTKEY	_BV(0)
#define	CM_FLAG_OSC	_BV(1)
#define	CM_FLAG_UTF8	_BV(2)
#define	CM_FLAG_CLOSE	_BV(3)
#define	CM_FLAG_FLIP	_BV(4)
#define	CM_FLAG_CMASK	(_BV(5) | _BV(6))
#define	CM_FLAG_CAMBER	0
#define	CM_FLAG_CGREEN	_BV(5)
#define	CM_FLAG_CWHITE	_BV(6)
#define	CM_FLAG_RAW	_BV(7)

#define	CM_ROTATION_0	0
#define	CM_ROTATION_90	1
#define	CM_ROTATION_180	2
#define	CM_ROTATION_270	3

typedef struct {
	u16		port;
	u16		flags;
	unsigned char	mode;
	char		name[25];
	char		target[134];
	char		username[64];
	unsigned char	hostkey[32];
} CMCONN;

typedef struct CM CM;
struct CM {
	int		mode;
	int		flags;
	char**		envp;

	TTY		tty;
	PTY		pty;
	SSH		ssh;
	TELNET		telnet;

	int		osc_state;
	int		utf8_state;
	u32		utf8_char;

	int		visible;
	SPF		spf;

	BOOL		input_received;

	int		current_connection;

	CMCONN*		connections;
	int		connections_cnt;
	int		connections_max;
	int		autoconnect;

	void		(*rx)(unsigned char c);
	int		(*rxe)(void);

	void		(*screen_rotation)(int rotation);
	void		(*screen_raw)(BOOL raw);
};

void	CMInit(CM* self, VT220* vt, char** envp);
void	CMDestroy(CM* self);
void	CMDisconnect(CM* self);
void	CMPoll(CM* self);
void	CMProcess(CM* self);
void	CMOpenTTY(CM* self, const char* path);
void	CMConnect(CM* self, int id);

void	CMSend(CM* self, unsigned char c);
void	CMBreak(CM* self);
void	CMResize(CM* self, unsigned int width, unsigned int height);
void	CMClearComm(CM* self);

void	CMSetBaudRate(CM* self, unsigned int rx_rate, unsigned int tx_rate);
void	CMSetFlowControl(CM* self, bool enable_xoff);
void	CMSetFormat(CM* self, unsigned int data, unsigned int stop, unsigned int parity);

void	CMShow(CM* self);
void	CMClearUI(CM* self);
void	CMProcessKey(CM* self, u16 key);

void	CMSetVisible(CM* self, int visible);
int	CMIsVisible(CM* self);
VTScreen* CMGetScreen(CM* self);

void	CMSetFlipScreen(CM* self, BOOL flipped);
void	CMUpdateScreenRotation(CM* self);
void	CMSetPhosphorColor(CM* self, int color);
void	CMUpdatePhosphorColor(CM* self);
void	CMSetRawScreen(CM* self, BOOL raw);
void	CMUpdateRawScreen(CM* self);

void	CMDeleteConnection(CM* self, int id);

void	CMSetAutoConnect(CM* self, int id);
int	CMGetAutoConnect(CM* self);
void	CMAutoConnect(CM* self);

void	CMLoadState(CM* self);
void	CMSaveState(const CM* self);

/******************************************************************************/
void	CMiSend(CM* self, unsigned char c);

void	CMInitOSCFilter(CM* self);
void	CMResetOSCFilter(CM* self);
void	CMSetOSCEnabled(CM* self, BOOL enabled);
void	CMOSCReceive(CM* self, unsigned char c);

void	CMResetUTF8(CM* self);
void	CMSetUTF8Enabled(CM* self, BOOL enabled);
BOOL	CMIsUTF8Enabled(CM* self);
void	CMUTF8Receive(CM* self, unsigned char c);
void	CMUTF8Transmit(CM* self, unsigned char c);

void	CMiInitUI(CM* self, VT220* vt);

void	CMiCreateConnectionMenu(CM* self);
void	CMiCreateSetupMenu(CM* self);
void	CMiCreateConnectionCustomizationMenu(CM* self);
void	CMiCreateSystemConfigMenu(CM* self);
void	CMiCreateDiagnosticsMenu(CM* self);
void	CMiCreateConnectionEditMenu(CM* self, int id);
void	CMiCreateMessageBox(CM* self, const char* title, const char* message, void (*callback)(void*, BOOL), void* userdata);

#endif
