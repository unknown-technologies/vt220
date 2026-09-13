#ifndef _WIN32
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "tty.h"

void TTYInit(TTY* tty)
{
	memset(tty, 0, sizeof(TTY));
}

void TTYOpen(TTY* tty, const char* path)
{
	tty->fd = open(path, O_RDWR | O_NONBLOCK | O_NOCTTY | O_CLOEXEC);
	if(tty->fd == -1) {
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		fflush(stderr);
		_Exit(1);
	}

	if(tcgetattr(tty->fd, &tty->ttyattrs)) {
		if(errno == ENOTTY) {
			fprintf(stderr, "Failed to open %s: not a TTY\n", path);
		} else {
			perror("tcgetattr");
		}
		fflush(stderr);
		_Exit(1);
	}

	memcpy(&tty->tio, &tty->ttyattrs, sizeof(struct termios));

	/* set terminal to 8N1, raw mode, disable signals */
	tty->tio.c_iflag &= ~(ICRNL | INPCK | ISTRIP);
	tty->tio.c_iflag |= IGNBRK | IGNPAR;
	tty->tio.c_oflag &= ~OPOST;
	tty->tio.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
	tty->tio.c_cflag |= CS8 | CREAD;
	tty->tio.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	if(tcsetattr(tty->fd, TCSANOW, &tty->tio)) {
		/* this is not good but also not fatal */
		TTYError(tty, "tcsetattr");
	}
}

static speed_t TTYiGetBaudRate(unsigned int rate)
{
	switch(rate) {
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		default:
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;
		case 460800:
			return B460800;
		case 500000:
			return B500000;
		case 576000:
			return B576000;
		case 921600:
			return B921600;
		case 1000000:
			return B1000000;
		case 1152000:
			return B1152000;
		case 1500000:
			return B1500000;
		case 2000000:
			return B2000000;
	}
}

void TTYSetBaudRate(TTY* tty, unsigned int rx_rate, unsigned int tx_rate)
{
	if(tty->fd == -1) {
		return;
	}

	if(cfsetispeed(&tty->tio, TTYiGetBaudRate(rx_rate))) {
		/* not good but not fatal either */
		TTYError(tty, "cfsetispeed");
	}

	if(cfsetospeed(&tty->tio, TTYiGetBaudRate(tx_rate))) {
		/* not good but not fatal either */
		TTYError(tty, "cfsetospeed");
	}

	if(tcsetattr(tty->fd, TCSANOW, &tty->tio)) {
		/* this is not good but also not fatal */
		TTYError(tty, "tcsetattr");
	}
}

void TTYSetFlowControl(TTY* tty, bool enable_xoff)
{
	if(tty->fd == -1) {
		return;
	}

	/* Is this correct here? The VT220 will send XON/XOFF too */
	if(enable_xoff) {
		tty->tio.c_cc[VSTART] = 021; /* DC1 */
		tty->tio.c_cc[VSTOP] = 023;  /* DC3 */
		tty->tio.c_iflag |= IXON | IXOFF;
	} else {
		tty->tio.c_iflag &= ~(IXON | IXOFF);
	}

	if(tcsetattr(tty->fd, TCSANOW, &tty->tio)) {
		/* this is not good but also not fatal */
		TTYError(tty, "tcsetattr");
	}
}

void TTYSetFormat(TTY* tty, unsigned int data, unsigned int stop, unsigned int parity)
{
	if(tty->fd == -1) {
		return;
	}

	switch(data) {
		case 5:
			tty->tio.c_cflag = (tty->tio.c_cflag & ~CSIZE) | CS5;
			break;
		case 6:
			tty->tio.c_cflag = (tty->tio.c_cflag & ~CSIZE) | CS6;
			break;
		case 7:
			tty->tio.c_cflag = (tty->tio.c_cflag & ~CSIZE) | CS7;
			break;
		case 8:
			tty->tio.c_cflag = (tty->tio.c_cflag & ~CSIZE) | CS8;
			break;
	}

	if(stop == 1) {
		tty->tio.c_cflag &= ~CSTOPB;
	} else {
		tty->tio.c_cflag |= CSTOPB;
	}

	switch(parity) {
		case TTY_PARITY_NONE:
			tty->tio.c_iflag &= ~INPCK;
			tty->tio.c_cflag &= ~(PARENB | CMSPAR);
			break;
		case TTY_PARITY_EVEN:
			tty->tio.c_iflag |= INPCK;
			tty->tio.c_cflag |= PARENB;
			tty->tio.c_cflag &= ~(PARODD | CMSPAR);
			break;
		case TTY_PARITY_ODD:
			tty->tio.c_iflag |= INPCK;
			tty->tio.c_cflag |= PARODD | PARENB;
			tty->tio.c_cflag &= ~CMSPAR;
			break;
		case TTY_PARITY_MARK:
			tty->tio.c_iflag |= INPCK;
			tty->tio.c_cflag |= PARENB | CMSPAR;
			tty->tio.c_cflag &= ~PARODD;
			break;
		case TTY_PARITY_SPACE:
			tty->tio.c_iflag |= INPCK;
			tty->tio.c_cflag |= PARODD | PARENB | CMSPAR;
			break;
		case TTY_PARITY_EVEN_NOCHK:
			tty->tio.c_iflag &= ~INPCK;
			tty->tio.c_cflag |= PARENB;
			tty->tio.c_cflag &= ~(PARODD | CMSPAR);
			break;
		case TTY_PARITY_ODD_NOCHK:
			tty->tio.c_iflag &= ~INPCK;
			tty->tio.c_cflag |= PARODD | PARENB;
			tty->tio.c_cflag &= ~CMSPAR;
			break;
	}

	if(tcsetattr(tty->fd, TCSANOW, &tty->tio)) {
		/* this is not good but also not fatal */
		TTYError(tty, "tcsetattr");
	}
}

void TTYRxString(TTY* tty, const char* s)
{
	for(; *s; s++) {
		tty->rx(*s);
	}
}

void TTYRxError(TTY* tty, const char* what, const char* msg)
{
	TTYRxString(tty, what);
	TTYRxString(tty, " failed: ");
	TTYRxString(tty, msg);
	TTYRxString(tty, "\r\n");

	printf("[TTY] %s failed: %s\n", what, msg);
}

void TTYError(TTY* tty, const char* what)
{
	char* msg = strerror(errno);
	TTYRxError(tty, what, msg);
}

void TTYEnqueueTX(TTY* tty, unsigned char c)
{
	if(tty->count >= TTY_TX_BUFFER_SIZE) {
		/* the TX buffer is full; drop this byte */
		return;
	}

	tty->count++;
	tty->txbuf[tty->write_ptr++] = c;
	tty->write_ptr %= TTY_TX_BUFFER_SIZE;
}

int TTYDrainTX(TTY* tty)
{
	if(tty->fd == -1) {
		return 0;
	}

	while(tty->count > 0) {
		unsigned char ch = tty->txbuf[tty->read_ptr];

		if(write(tty->fd, &ch, 1) == -1) {
			if(errno == EWOULDBLOCK || errno == EINTR) {
				/* buffer full or we got interrupted by a signal, try again in the next tick */
				return 0;
			}

			TTYError(tty, "write");
			close(tty->fd);
			tty->fd = -1;

			return 0;
		} else {
			tty->read_ptr = (tty->read_ptr + 1) % TTY_TX_BUFFER_SIZE;
			tty->count--;
		}
	}

	return 1;
}

void TTYSend(TTY* tty, unsigned char c)
{
	if(tty->fd == -1) {
		return;
	}

	if(tty->count > 0) {
		if(!TTYDrainTX(tty)) {
			TTYEnqueueTX(tty, c);
			return;
		}
	}

	if(write(tty->fd, &c, 1) == -1) {
		if(errno == EWOULDBLOCK) {
			/* the TTY's TX queue is full, put this byte into our own queue */
			TTYEnqueueTX(tty, c);
			return;
		} else if(errno == EINTR) {
			/* write got interrupted by a signal, put this byte into the queue */
			TTYEnqueueTX(tty, c);
			return;
		}

		TTYError(tty, "write");
		close(tty->fd);
		tty->fd = -1;
	}
}

void TTYBreak(TTY* tty)
{
	if(tty->fd == -1) {
		return;
	}

	if(tcsendbreak(tty->fd, 0) == -1) {
		TTYError(tty, "tcsendbreak");
	}
}

void TTYPoll(TTY* tty)
{
	TTYDrainTX(tty);

	if(tty->rxe && tty->rx) {
		while(tty->bufrd < tty->bufsz) {
			if(tty->rxe()) {
				tty->rx(tty->buf[tty->bufrd++]);
			} else {
				break;
			}
		}
		if(tty->bufrd >= tty->bufsz) {
			tty->bufrd = 0;
			tty->bufsz = 0;
		} else {
			return;
		}
	}

	struct pollfd fds = {
		.fd = tty->fd,
		.events = POLLIN
	};

	if(tty->fd == -1) {
		return;
	}

	int result = poll(&fds, 1, 0);
	if(result == -1) {
		TTYError(tty, "poll");
		close(tty->fd);
		tty->fd = -1;
	} else if(fds.revents & POLLIN) {
		ssize_t n = read(tty->fd, tty->buf, TTY_BUFFER_SIZE);
		if(n == -1) {
			if(errno == EWOULDBLOCK) {
				/* this should never happen, but it did happen. Ignore it */
				return;
			} else if(errno == EINTR) {
				/* our read got interrupted by a signal, ignore this too */
				return;
			}
			TTYError(tty, "read");
			close(tty->fd);
			tty->fd = -1;
		} else if(n > 0 && tty->rx) {
			if(tty->rxe) {
				tty->bufsz = n;
				tty->bufrd = 0;
				while(tty->bufrd < tty->bufsz) {
					if(tty->rxe()) {
						tty->rx(tty->buf[tty->bufrd++]);
					} else {
						break;
					}
				}
			} else {
				for(ssize_t i = 0; i < n; i++) {
					tty->rx(tty->buf[i]);
				}
			}
		}
	}
}

void TTYResize(TTY* tty, unsigned int width, unsigned int height)
{
	struct winsize ws = {
		.ws_col = width,
		.ws_row = height
	};

	if(tty->fd == -1) {
		return;
	}

	if(ioctl(tty->fd, TIOCSWINSZ, &ws) == -1) {
		printf("[TTY] failed to set console window size: %s\n", strerror(errno));
		return;
	}

	/* for a real TTY there is no SIGWINCH one could send */
}

void TTYClose(TTY* tty)
{
	if(tty->fd != -1) {
		if(tcsetattr(tty->fd, TCSANOW, &tty->ttyattrs)) {
			perror("tcsetattr");
		}

		close(tty->fd);
		tty->fd = -1;
	}
}
#endif
