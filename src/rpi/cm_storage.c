#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "cm.h"

#define	CM_CURRENT_VERSION	0x0001

static const char MAGIC[4] = { 'C', 'M', 'C', 'D' }; /* CM Customization Data */

typedef struct {
	char	magic[4];
	int	version;
	int	autoconnect;
	int	connection_count;
} CMHDR;

/* we try /var/lib/vt220/conmgr, $XDG_STATE_HOME/vt220.cm, $HOME/.vt220.cm */
static char* get_state_path(void)
{
	if(!access("/var/lib/vt220/conmgr", R_OK | W_OK)) {
		return strdup("/var/lib/vt220/conmgr");
	} else {
		char* xdg_state_home = getenv("XDG_STATE_HOME");
		if(!xdg_state_home) {
			const char* home = getenv("HOME");
			if(!home) {
				return NULL;
			} else {
				xdg_state_home = (char*) malloc(strlen(home) + strlen("/.local/state") + 1);
				sprintf(xdg_state_home, "%s/.local/state", home);
			}
		} else {
			xdg_state_home = strdup(xdg_state_home);
		}

		if(access(xdg_state_home, F_OK)) {
			/* XDG_CONFIG_HOME does not exist */
			free(xdg_state_home);

			/* use a dot file in $HOME instead, if $HOME is set */
			const char* home = getenv("HOME");
			if(!home) {
				return NULL;
			} else {
				char* state_path = (char*) malloc(strlen(home) + strlen("/.vt220.cm") + 1);
				sprintf(state_path, "%s/.vt220.cm", home);
				if(access(home, R_OK | W_OK) && access(state_path, R_OK | W_OK)) {
					/* cannot read/write $HOME nor the file, give up */
					free(state_path);
					return NULL;
				} else {
					return state_path;
				}
			}
		}

		char* state_path = (char*) malloc(strlen(xdg_state_home) + strlen("/vt220.cm") + 1);
		sprintf(state_path, "%s/vt220.cm", xdg_state_home);

		free(xdg_state_home);

		return state_path;
	}
	return NULL;
}

void CMLoadState(CM* cm)
{
	char* state_path = get_state_path();
	if(!state_path) {
		return;
	}

	printf("Loading connection manager config from %s\n", state_path);

	FILE* file = fopen(state_path, "rb");
	if(!file) {
		if(errno != ENOENT) {
			printf("[CM] failed to open file %s: %s\n", state_path, strerror(errno));
		}
		free(state_path);
		return;
	}

	CMHDR hdr;
	if(fread(&hdr, sizeof(CMHDR), 1, file) == 1) {
		if(memcmp(hdr.magic, MAGIC, 4)) {
			printf("[CM] corrupt config file, ignored\n");
		} else if(hdr.version != CM_CURRENT_VERSION) {
			printf("[CM] unknown config file version, ignored\n");
		} else if(hdr.connection_count > cm->connections_max) {
			printf("[CM] too many connections in config file, ignored\n");
		} else {
			CMCONN* conns = (CMCONN*) malloc(hdr.connection_count * sizeof(CMCONN));
			if(!conns) {
				printf("[CM] failed to allocate memory\n");
			} else if(fread(conns, hdr.connection_count * sizeof(CMCONN), 1, file) == 1) {
				memset(cm->connections, 0, cm->connections_max * sizeof(CMCONN));
				memcpy(cm->connections, conns, hdr.connection_count * sizeof(CMCONN));
				cm->connections_cnt = hdr.connection_count;
				free(conns);
				if(hdr.autoconnect != -1 && (hdr.autoconnect < 0 || hdr.autoconnect >= hdr.connection_count)) {
					printf("[CM] invalid autoconnect id in config file, ignored\n");
				} else {
					CMSetAutoConnect(cm, hdr.autoconnect);
				}
			} else {
				printf("[CM] failed to read config file: %s\n", strerror(errno));
			}
		}
	} else {
		printf("[CM] failed to read config file: %s\n", strerror(errno));
	}

	fclose(file);

	free(state_path);
}

void CMSaveState(const CM* cm)
{
	char* state_path = get_state_path();
	if(!state_path) {
		return;
	}

#ifdef DEBUG
	printf("Saving connection manager config to %s\n", state_path);
#endif

	/* TODO: the mode should be 0600 but fopen sets it to 0644 */
	FILE* file = fopen(state_path, "wb");
	if(!file) {
		printf("[CM] failed to open file %s: %s\n", state_path, strerror(errno));
		free(state_path);
		return;
	}

	CMHDR hdr = {
		.magic = { MAGIC[0], MAGIC[1], MAGIC[2], MAGIC[3] },
		.version = CM_CURRENT_VERSION,
		.autoconnect = cm->autoconnect,
		.connection_count = cm->connections_cnt
	};

	if(fwrite(&hdr, sizeof(CMHDR), 1, file) != 1) {
		printf("[CM] failed to write config file: %s\n", strerror(errno));
		goto end;
	}

	if(fwrite(cm->connections, cm->connections_cnt * sizeof(CMCONN), 1, file) != 1) {
		printf("[CM] failed to write config file: %s\n", strerror(errno));
		goto end;
	}

end:
	fclose(file);

	free(state_path);
}
