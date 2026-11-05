#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	if(setreuid(0, 0) == -1) {
		/* this is not fatal but agetty will probably not work correctly either */
		printf("setreuid failed: %s\n", strerror(errno));
		fflush(stdout);
	}

	char* argv[] = { "/bin/agetty", "-8", "-s", "--local-line", "--noreset", "--noclear", "-", "vt220", NULL };
	char* envp[] = { NULL };

	if(execve(*argv, argv, envp) == -1) {
		printf("execve failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
