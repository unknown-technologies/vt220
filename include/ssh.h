#ifndef	__SSH_H__
#define	__SSH_H__

#include <libssh/libssh.h>

#include "types.h"

#define	SSH_BUFFER_SIZE	32768

typedef struct {
	int		state;
	ssh_session	session;
	ssh_channel	pty;

	int		width;
	int		height;

	/* password input buffer */
	char*		input_buffer;
	int		input_end;

	/* rx buffer */
	unsigned char*	rxbuf;
	unsigned int	bufsz;
	unsigned int	bufrd;

	void*		userdata;

	unsigned char*	hostkey_hash;
	size_t		hostkey_len;

	void		(*rx)(void* userdata, unsigned char c);
	int		(*rxe)(void* userdata);
	BOOL		(*hostkey)(void* userdata, unsigned char* key, int len);
	void		(*add_hostkey)(void* userdata, unsigned char* key, int len);
} SSH;

void	SSHInit(SSH* self);
void	SSHDestroy(SSH* self);
void	SSHConnect(SSH* self, const char* hostname, int port, const char* username);
void	SSHDisconnect(SSH* self);
BOOL	SSHIsConnected(SSH* self);
void	SSHSetUserData(SSH* self, void* userdata);

void	SSHSend(SSH* self, unsigned char c);
void	SSHBreak(SSH* self);
void	SSHResize(SSH* self, unsigned int width, unsigned int height);
void	SSHPoll(SSH* self);

void	SSHRxString(SSH* self, const char* s);
void	SSHRxUnixString(SSH* self, const char* s);
void	SSHRxError(SSH* self, const char* what, const char* msg);

#endif
