#ifndef __TELNET_H__
#define __TELNET_H__

#define	TELNET_BUFFER_SIZE	32768

typedef struct {
	int		socket;
	int		connected;
	int		state;

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
void TELNETConnect(TELNET* telnet, const char* hostname, int port);
void TELNETDisconnect(TELNET* telnet);
void TELNETSend(TELNET* telnet, unsigned char c);
void TELNETBreak(TELNET* telnet);
void TELNETSendRaw(TELNET* telnet, unsigned char c);
void TELNETPoll(TELNET* telnet);

#endif
