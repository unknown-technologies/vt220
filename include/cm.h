#ifndef __CM_H__
#define __CM_H__

#include "types.h"
#include "vt.h"
#include "renderer.h"
#include "telnet.h"
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

typedef struct {
	u16		port;
	u16		status;
	unsigned char	mode;
	char		name[25];
	char		target[134];
	char		username[64];
} CMCONN;

typedef struct CM CM;
struct CM {
	int		mode;
	char**		envp;

	TTY		tty;
	PTY		pty;
	TELNET		telnet;

	int		osc_state;

	int		visible;
	SPF		spf;

	CMCONN*		connections;
	int		connections_cnt;
	int		connections_max;
	int		autoconnect;

	void		(*rx)(unsigned char c);
	int		(*rxe)(void);
};

void	CMInit(CM* self, VT220* vt, char** envp);
void	CMDestroy(CM* self);
void	CMUpdateHandlers(CM* self);
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

void	CMDeleteConnection(CM* self, int id);

void	CMSetAutoConnect(CM* self, int id);
int	CMGetAutoConnect(CM* self);
void	CMAutoConnect(CM* self);

void	CMLoadState(CM* cm);
void	CMSaveState(const CM* cm);

/******************************************************************************/
void	CMInitOSCFilter(CM* self);
void	CMResetOSCFilter(CM* self);
void	CMOSCReceive(CM* self, unsigned char c);

void	CMiInitUI(CM* self, VT220* vt);

void	CMiCreateConnectionMenu(CM* self);
void	CMiCreateSetupMenu(CM* self);
void	CMiCreateDiagnosticsMenu(CM* self);
void	CMiCreateConnectionCustomizationMenu(CM* self);
void	CMiCreateConnectionEditMenu(CM* self, int id);

#endif
