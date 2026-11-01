#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

#include "config.h"

/* TODO: on Windows, the state should probably be stored somewhere in the registry */

#ifndef _WIN32
/* on Linux we try /var/lib/vt220/nvr, $XDG_STATE_HOME/vt220.nvr, $HOME/.vt220.nvr */
static char* get_state_path(void)
{
	if(!access("/var/lib/vt220/nvr", R_OK | W_OK) || !access("/var/lib/vt220", R_OK | W_OK)) {
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
			/* XDG_CONFIG_HOME does not exist */
			free(xdg_state_home);

			/* use a dot file in $HOME instead, if $HOME is set */
			const char* home = getenv("HOME");
			if(!home) {
				return NULL;
			} else {
				char* state_path = (char*) malloc(strlen(home) + strlen("/.vt220.nvr") + 1);
				sprintf(state_path, "%s/.vt220.nvr", home);
				if(access(home, R_OK | W_OK) && access(state_path, R_OK | W_OK)) {
					/* cannot read/write $HOME nor the file, give up */
					free(state_path);
					return NULL;
				} else {
					return state_path;
				}
			}
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

#ifndef _WIN32
static bool write_all(int fd, const void* data, size_t len)
{
	while(len > 0) {
		ssize_t n = write(fd, data, len);
		if(n == -1) {
			return false;
		} else if(n == 0) {
			return false;
		} else {
			len -= n;
			data += n;
		}
	}

	return true;
}
#endif

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

	char* tmpfile = (char*) malloc(strlen(state_path) + 5);
	if(!tmpfile) {
		printf("[CFG] cannot create temporary file, using non-atomic write instead\n");
		tmpfile = state_path;
	} else {
		strcpy(tmpfile, state_path);
		strcat(tmpfile, ".tmp");
	}

	/* O_TRUNC is used to automatically "clean up" any left-over temporary
	 * files in case there was a power loss / system crash at exactly the
	 * wrong time (after the write operation, before the rename operation).
	 *
	 * Technically, S_IRUSR is sufficient for atomic updates, but in case
	 * we cannot use that mechanism for some reason, we still want to be
	 * able to perform a non-atomic update. To make this work, we also set
	 * the S_IWUSR permission so we can directly write to the file in case
	 * it is ever necessary. */
	int file = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(file == -1) {
		printf("[CFG] failed to open file %s: %s\n", tmpfile, strerror(errno));
		if(tmpfile != state_path) {
			free(tmpfile);
		}
		free(state_path);
		return;
	}

	if(!write_all(file, nvr, sizeof(VT220NVR))) {
		printf("[CFG] failed to write NVR: %s\n", strerror(errno));
		goto error;
	}

	/* make sure everything is written to disk before performing the atomic rename */
	if(fsync(file) == -1) {
		printf("[CFG] failed to write NVR to disk: %s\n", strerror(errno));
		goto error;
	}

	close(file);

	/* only perform rename if there is in fact a temporary file */
	if(tmpfile != state_path) {
		if(rename(tmpfile, state_path) == -1) {
			printf("[CFG] failed to perform atomic NVR update: %s\n", strerror(errno));
			if(unlink(tmpfile) == -1) {
				printf("[CFG] failed to remove temporary NVR file: %s\n", strerror(errno));
			}
		}

		free(tmpfile);
	}

	free(state_path);
	return;

error:
	close(file);

	if(tmpfile != state_path) {
		/* if there is a temporary file, remove it since it is most likely broken */
		if(unlink(tmpfile) == -1) {
			printf("[CFG] failed to remove temporary NVR file: %s\n", strerror(errno));
		}
		free(tmpfile);
	}

	free(state_path);
#endif
}
