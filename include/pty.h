#ifndef __PTY_H__
#define __PTY_H__

#include <sys/types.h>

#define	PTY_BUFFER_SIZE		32768

typedef struct {
	int		master;
	pid_t		pid;
	unsigned char	buf[PTY_BUFFER_SIZE];
	unsigned int	bufsz;
	unsigned int	bufrd;

	void		(*rx)(unsigned char c);
	int		(*rxe)(void);
} PTY;

void	PTYInit(PTY* pty);
void	PTYOpen(PTY* pty, char** argv, char** envp);
void	PTYSend(PTY* pty, unsigned char c);
void	PTYBreak(PTY* pty);
void	PTYPoll(PTY* pty);
void	PTYResize(PTY* pty, unsigned int width, unsigned int height);

void	PTYRxString(PTY* pty, const char* s);
void	PTYRxError(PTY* pty, const char* what, const char* msg);
void	PTYError(PTY* pty, const char* what);

#endif
