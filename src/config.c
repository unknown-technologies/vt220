#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "config.h"

/* TODO: on Windows, the state should probably be stored somewhere in the registry */

#ifndef _WIN32
static char* get_state_path(void)
{
	if(!access("/var/lib/vt220/nvr", R_OK | W_OK)) {
		return strdup("/var/lib/vt220/nvr");
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
			/* XDG_CONFIG_HOME does not exist; TODO: create it? */
			free(xdg_state_home);
			return NULL;
		}

		char* state_path = (char*) malloc(strlen(xdg_state_home) + strlen("/vt220.nvr") + 1);
		sprintf(state_path, "%s/vt220.nvr", xdg_state_home);

		free(xdg_state_home);

		return state_path;
	}
	return NULL;
}
#endif

void CFGLoadState(VT220* vt)
{
#ifdef _WIN32
	(void) vt;
#else
	char* state_path = get_state_path();
	if(!state_path) {
		return;
	}

#ifdef DEBUG
	printf("Loading config from %s\n", state_path);
#endif

	FILE* file = fopen(state_path, "rb");
	if(!file) {
		if(errno != ENOENT) {
			printf("[CFG] failed to open file %s: %s\n", state_path, strerror(errno));
		}
		free(state_path);
		return;
	}

	VT220NVR nvr;
	if(fread(&nvr, sizeof(VT220NVR), 1, file) == 1) {
		if(!VT220LoadConfig(vt, &nvr)) {
			printf("[CFG] corrupt config file, ignored\n");
		}
	} else {
		printf("[CFG] failed to read config file\n");
	}

	fclose(file);

	free(state_path);
#endif
}

void CFGSaveState(const VT220NVR* nvr)
{
#ifdef _WIN32
	(void) nvr;
#else
	char* state_path = get_state_path();
	if(!state_path) {
		return;
	}

#ifdef DEBUG
	printf("Saving config to %s\n", state_path);
#endif

	/* TODO: the mode should be 0600 but fopen sets it to 0644 */
	FILE* file = fopen(state_path, "wb");
	if(!file) {
		printf("[CFG] failed to open file %s: %s\n", state_path, strerror(errno));
		free(state_path);
		return;
	}

	fwrite(nvr, sizeof(VT220NVR), 1, file);

	fclose(file);

	free(state_path);
#endif
}
