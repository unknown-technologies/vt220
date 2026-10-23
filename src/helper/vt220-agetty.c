#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	setreuid(0, 0);

	char* argv[] = { "/bin/agetty", "-8", "-s", "--local-line", "--noreset", "--noclear", "-",
		"vt220", NULL };
	char* envp[] = { NULL };

	if(execve(*argv, argv, envp) == -1) {
		printf("execve failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
