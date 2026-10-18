#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "types.h"
#include "vt.h"
#include "spf.h"
#include "cm.h"

typedef struct {
	struct ifaddrs*	addrs;
} CMDiagnosticsMenu;

void CMDiagnosticsMenuCreate(SPFPanel* self)
{
	CMDiagnosticsMenu* this = (CMDiagnosticsMenu*) malloc(sizeof(CMDiagnosticsMenu));
	ASSERT(this);
	memset(this, 0, sizeof(CMDiagnosticsMenu));

	getifaddrs(&this->addrs);

	SPFSetPanelUserData(self, (void*) this);

	SPFSetPanelTitle(self, "Diagnostics");
	SPFHideCursor(self);
}

void CMDiagnosticsMenuDestroy(SPFPanel* self)
{
	CMDiagnosticsMenu* this = (CMDiagnosticsMenu*) SPFGetPanelUserData(self);
	if(this->addrs) {
		freeifaddrs(this->addrs);
	}
	free(this);
}

void CMDiagnosticsMenuDraw(SPFPanel* self)
{
	CMDiagnosticsMenu* this = (CMDiagnosticsMenu*) SPFGetPanelUserData(self);

	struct sysinfo info;
	sysinfo(&info);

	long uptime = info.uptime;
	int uptime_s = uptime % 60; uptime /= 60;
	int uptime_m = uptime % 60; uptime /= 60;
	int uptime_h = uptime % 24; uptime /= 24;
	int uptime_d = uptime;

	float load_scale = 1.0f / (1 << SI_LOAD_SHIFT);

	char buf[256];

	SPFPrintAt(self, 2, 2, "System Information:", 0);

	sprintf(buf, "Total memory:   %luMB", info.totalram * (size_t) info.mem_unit / (1024 * 1024));
	SPFPrintAt(self, 4, 4, buf, 0);
	sprintf(buf, "Free memory:    %luMB", info.freeram * (size_t) info.mem_unit / (1024 * 1024));
	SPFPrintAt(self, 5, 4, buf, 0);
	sprintf(buf, "Load average:   %.2f %.2f %.2f", info.loads[0] * load_scale, info.loads[1] * load_scale,
			info.loads[2] * load_scale);
	SPFPrintAt(self, 6, 4, buf, 0);
	sprintf(buf, "CPU cores:      %d", get_nprocs());
	SPFPrintAt(self, 7, 4, buf, 0);
	sprintf(buf, "Uptime:         %d days, %02d:%02d:%02d", uptime_d, uptime_h, uptime_m, uptime_s);
	SPFPrintAt(self, 8, 4, buf, 0);

	SPFPrintAt(self, 10, 2, "Network Interfaces:", 0);

	int pos_y = 12;
	for(struct ifaddrs* ifa = this->addrs; ifa && pos_y < 20; ifa = ifa->ifa_next) {
		/* no address => uninteresting for us */
		if(!ifa->ifa_addr) {
			continue;
		}

		/* loopback interface => uninteresting for us */
		if(ifa->ifa_name && !strcmp(ifa->ifa_name, "lo")) {
			continue;
		}

		/* we only care about IPv4/IPv6 */
		int family = ifa->ifa_addr->sa_family;
		if(family != AF_INET && family != AF_INET6) {
			continue;
		}

		SPFPrintAt(self, pos_y, 4, ifa->ifa_name, 0);

		int color = 0;
		int err = getnameinfo(ifa->ifa_addr, (family == AF_INET) ?  sizeof(struct sockaddr_in)
				: sizeof(struct sockaddr_in6), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
		if(err) {
			sprintf(buf, "getnameinfo(): %s", gai_strerror(err));
			color = SGR_BOLD;
		}

		SPFPrintAt(self, pos_y, 20, buf, color);
		pos_y++;
	}

	SPFPrintAt(self, 21, 2, "F4=End", 0);
	SPFGoto(self, 21, SPFGetWidth(self) - 8);
	SPFPrint(self, "F12=Done", 0);
}

void CMDiagnosticsMenuProcess(SPFPanel* self, u16 key)
{
	switch(key) {
		case VT220_KEY_F12:
			SPFDestroyPanel(self);
			break;
		default:
			SPFRepaint(SPFGet(self));
			break;
	}
}

static SPFPanelDefinition diagnostics_menu = {
	.init = CMDiagnosticsMenuCreate,
	.draw = CMDiagnosticsMenuDraw,
	.resize = NULL,
	.process = CMDiagnosticsMenuProcess,
	.destroy = CMDiagnosticsMenuDestroy
};

void CMiCreateDiagnosticsMenu(CM* self)
{
	SPFCreatePanel(&self->spf, &diagnostics_menu);
}
