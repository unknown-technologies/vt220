#ifndef __TTY_H__
#define __TTY_H__

#include <termios.h>
#include <sys/types.h>

#include "types.h"

#define	TTY_BUFFER_SIZE		32768
#define	TTY_TX_BUFFER_SIZE	256

#define	TTY_PARITY_NONE		0
#define	TTY_PARITY_EVEN		1
#define	TTY_PARITY_ODD		2
#define	TTY_PARITY_MARK		3
#define	TTY_PARITY_SPACE	4
#define	TTY_PARITY_EVEN_NOCHK	5
#define	TTY_PARITY_ODD_NOCHK	6

typedef struct {
	int		fd;

	unsigned char	buf[TTY_BUFFER_SIZE];
	unsigned int	bufsz;
	unsigned int	bufrd;

	/* tx ringbuffer */
	unsigned char	txbuf[TTY_TX_BUFFER_SIZE];
	int		read_ptr;
	int		write_ptr;
	int		count;

	/* original termios config */
	struct termios	ttyattrs;
	struct termios	tio;

	unsigned int	baud_rx;
	unsigned int	baud_tx;
	unsigned int	flowctl;
	unsigned int	fmt_data;
	unsigned int	fmt_stop;
	unsigned int	fmt_parity;

	void*		userdata;
	void		(*rx)(void* userdata, unsigned char c);
	int		(*rxe)(void* userdata);
} TTY;

void	TTYInit(TTY* tty);
BOOL	TTYOpen(TTY* tty, const char* path);
void	TTYSend(TTY* tty, unsigned char c);
void	TTYBreak(TTY* tty);
void	TTYPoll(TTY* tty);
void	TTYResize(TTY* tty, unsigned int width, unsigned int height);
void	TTYClose(TTY* tty);
void	TTYSetUserData(TTY* tty, void* userdata);

void	TTYSetBaudRate(TTY* tty, unsigned int rx_rate, unsigned int tx_rate);
void	TTYSetFlowControl(TTY* tty, bool enable_xoff);
void	TTYSetFormat(TTY* tty, unsigned int data, unsigned int stop, unsigned int parity);

void	TTYRxString(TTY* tty, const char* s);
void	TTYRxError(TTY* tty, const char* what, const char* msg);
void	TTYError(TTY* tty, const char* what);
void	TTYOpenError(TTY* tty, const char* path, const char* what);

#endif
