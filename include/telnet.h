#ifndef __TELNET_H__
#define __TELNET_H__

#define	TELNET_BUFFER_SIZE	32768

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

typedef struct {
	int		socket;
	int		connected;
	int		state;
	int		naws;

	/* tx ringbuffer */
	unsigned char*	buf;
	int		read_ptr;
	int		write_ptr;
	int		count;

	/* sb buffer */
	unsigned char*	sb_buf;
	int		sb_write_ptr;

	/* rx buffer */
	unsigned char*	rxbuf;
	unsigned int	bufsz;
	unsigned int	bufrd;

	void		(*rx)(unsigned char c);
	int		(*rxe)(void);
} TELNET;

void TELNETInit(TELNET* telnet);
void TELNETDestroy(TELNET* telnet);
void TELNETConnect(TELNET* telnet, const char* hostname, int port);
void TELNETConnectTo(TELNET* telnet, struct sockaddr* target, int addr_size);
void TELNETDisconnect(TELNET* telnet);
void TELNETSend(TELNET* telnet, unsigned char c);
void TELNETBreak(TELNET* telnet);
void TELNETResize(TELNET* telnet, unsigned int width, unsigned int height);
void TELNETSendRaw(TELNET* telnet, unsigned char c, int immediate);
void TELNETPoll(TELNET* telnet);

#endif
