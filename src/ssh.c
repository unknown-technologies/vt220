#if defined(RPI) && !defined(USE_SSH)
#define	USE_SSH
#endif

#ifdef USE_SSH

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libssh/libssh.h>

#include "vt.h"
#include "ssh.h"

#define	BUFFER_SIZE		512

#define	VTCTRL(x)		((x) - 0x40)

#define	STATE_DISCONNECTED	0
#define	STATE_CONNECTING	1
#define	STATE_SERVER_PUBKEY	2
#define	STATE_AUTH_NONE		3
#define	STATE_AUTH_PASS_INPUT	4
#define	STATE_AUTH_PASSWORD	5
#define	STATE_CHANNEL_NEW	6
#define	STATE_CHANNEL_OPEN	7
#define	STATE_PTY_REQUEST	8
#define	STATE_SHELL_REQUEST	9
#define	STATE_CONNECTED		10

static const unsigned char pty_modes[] = {
};

void SSHInit(SSH* self)
{
	memset(self, 0, sizeof(SSH));
	self->input_buffer = (char*) malloc(BUFFER_SIZE);
	self->input_end = 0;

	self->rxbuf = (unsigned char*) malloc(SSH_BUFFER_SIZE);
	self->bufsz = 0;
	self->bufrd = 0;

	self->width = 80;
	self->height = 24;
}

void SSHDestroy(SSH* self)
{
	SSHDisconnect(self);
	free(self->rxbuf);
	free(self->input_buffer);
}

void SSHConnect(SSH* self, const char* hostname, int port, const char* username)
{
	SSHDisconnect(self);

	ASSERT(self->session == NULL);
	ASSERT(self->pty == NULL);
	ASSERT(self->state == STATE_DISCONNECTED);

	self->session = ssh_new();
	if(!self->session) {
		SSHRxError(self, "ssh_new", strerror(errno));
		return;
	}

	int nodelay = 1;
	int timeout = 10;

	ssh_options_set(self->session, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(self->session, SSH_OPTIONS_PORT, &port);
	ssh_options_set(self->session, SSH_OPTIONS_USER, username);
	ssh_options_set(self->session, SSH_OPTIONS_NODELAY, &nodelay);
	ssh_options_set(self->session, SSH_OPTIONS_ADDRESS_FAMILY, SSH_ADDRESS_FAMILY_ANY);
	ssh_options_set(self->session, SSH_OPTIONS_TIMEOUT, &timeout);
	ssh_set_blocking(self->session, 0);

	self->state = STATE_CONNECTING;
	SSHRxString(self, "Connecting to ");
	SSHRxString(self, username);
	SSHRxString(self, "@");
	SSHRxString(self, hostname);
	SSHRxString(self, "\r\n");
}

void SSHDisconnect(SSH* self)
{
	if(self->session) {
		if(self->state != STATE_DISCONNECTED) {
			if(self->pty) {
				ssh_channel_close(self->pty);
				ssh_channel_send_eof(self->pty);
				ssh_channel_free(self->pty);
				self->pty = NULL;
			}
			ssh_disconnect(self->session);
			self->state = STATE_DISCONNECTED;
		}

		ssh_free(self->session);
		self->session = NULL;

		SSHRxString(self, "Disconnected\r\n");
	}
}

void SSHRxString(SSH* self, const char* s)
{
	for(; *s; s++) {
		self->rx(*s);
	}
}

void SSHRxUnixString(SSH* self, const char* s)
{
	for(; *s; s++) {
		if(*s == '\n') {
			self->rx('\r');
			self->rx('\n');
		} else {
			self->rx(*s);
		}
	}
}

void SSHRxError(SSH* self, const char* what, const char* msg)
{
	SSHRxString(self, what);
	SSHRxString(self, " failed: ");
	SSHRxString(self, msg);
	SSHRxString(self, "\r\n");

	printf("[SSH] %s failed: %s\n", what, msg);
}

void SSHSend(SSH* self, unsigned char c)
{
	int result;
	switch(self->state) {
		case STATE_AUTH_PASS_INPUT:
			switch(c) {
				case VTCTRL('U'):
					memset(self->input_buffer, 0, BUFFER_SIZE);
					self->input_end = 0;
					break;
				case DEL:
				case BS:
					if(self->input_end > 0) {
						self->input_buffer[--self->input_end] = 0;
					}
					break;
				case CR:
				case LF:
					SSHRxString(self, "\r\n");
					self->state = STATE_AUTH_PASSWORD;
					break;
				default:
					if(c >= 0x20 && c < 0x7F) {
						if(self->input_end + 1 < BUFFER_SIZE) {
							self->input_buffer[self->input_end++] = c;
						}
					} else if(c >= 0x7F) {
						SSHRxString(self, "?");
					}
					break;
			}
			break;
		case STATE_CONNECTED:
			if(!ssh_channel_is_open(self->pty) || ssh_channel_is_eof(self->pty)) {
				SSHDisconnect(self);
				return;
			}

			result = ssh_channel_write(self->pty, &c, 1);
			if(result != 1) {
				SSHRxError(self, "ssh_channel_write", ssh_get_error(self->session));
				SSHDisconnect(self);
				return;
			}
			break;
	}
}

void SSHBreak(SSH* self)
{
	if(self->state == STATE_CONNECTED) {
		int result = ssh_channel_request_send_break(self->pty, 500);
		if(result == SSH_ERROR) {
			SSHRxError(self, "ssh_channel_request_send_break", ssh_get_error(self->session));
			SSHDisconnect(self);
			return;
		}
	}
}

void SSHResize(SSH* self, unsigned int width, unsigned int height)
{
	self->width = width;
	self->height = height;

	if(self->state == STATE_CONNECTED) {
		int result = ssh_channel_change_pty_size(self->pty, width, height);
		if(result == SSH_ERROR) {
			SSHRxError(self, "ssh_channel_change_pty_size", ssh_get_error(self->session));
			SSHDisconnect(self);
			return;
		}
	}
}

void SSHPoll(SSH* self)
{
	if(self->rxe && self->rx) {
		while(self->bufrd < self->bufsz) {
			if(self->rxe()) {
				self->rx(self->rxbuf[self->bufrd++]);
			} else {
				break;
			}
		}
		if(self->bufrd >= self->bufsz) {
			self->bufrd = 0;
			self->bufsz = 0;
		} else {
			return;
		}
	}

	int result;
	switch(self->state) {
		case STATE_CONNECTING:
			result = ssh_connect(self->session);
			if(result == SSH_ERROR) {
				SSHRxError(self, "ssh_connect", ssh_get_error(self->session));
				ssh_free(self->session);
				self->session = NULL;
				self->state = STATE_DISCONNECTED;
			} else if(result == SSH_OK) {
#ifdef DEBUG
				printf("[SSH] connected\n");
#endif
				self->state = STATE_SERVER_PUBKEY;
				ssh_key key;
				if(ssh_get_server_publickey(self->session, &key) != SSH_OK) {
					SSHRxError(self, "ssh_connect", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				} else {
#ifdef DEBUG
					printf("[SSH] host key retrieved\n");
#endif
					unsigned char* hash;
					size_t hlen;
					if(ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen) != SSH_OK) {
						SSHRxError(self, "ssh_get_publickey_hash", ssh_get_error(self->session));
						ssh_key_free(key);
						SSHDisconnect(self);
						return;
					} else {
						SSHRxString(self, "Server key: ");
						for(size_t i = 0; i < hlen; i++) {
							char buf[4];
							sprintf(buf, "%02X", hash[i]);
							SSHRxString(self, buf);
						}
						SSHRxString(self, "\r\n");
						ssh_clean_pubkey_hash(&hash);
					}
					ssh_key_free(key);
				}
			}
			/* SSH_AGAIN means we'll retry in the next frame */
			break;
		case STATE_SERVER_PUBKEY:
			/* we wait here until we are allowed to proceed */
			self->state = STATE_AUTH_NONE;
			break;
		case STATE_AUTH_NONE:
			result = ssh_userauth_none(self->session, NULL);
			if(result != SSH_AUTH_ERROR) {
				char* banner = ssh_get_issue_banner(self->session);
				if(banner) {
					SSHRxUnixString(self, banner);
					free(banner);
				}
			}
			switch(result) {
				case SSH_AUTH_SUCCESS:
					self->state = STATE_CHANNEL_NEW;
#ifdef DEBUG
					printf("[SSH] authenticated without credentials\n");
#endif
					break;
				case SSH_AUTH_ERROR:
					SSHRxError(self, "ssh_userauth_none", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				case SSH_AUTH_DENIED:
				case SSH_AUTH_PARTIAL: {
					self->state = STATE_AUTH_PASS_INPUT;
					memset(self->input_buffer, 0, BUFFER_SIZE);
					self->input_end = 0;

					int methods = ssh_userauth_list(self->session, NULL);
					if(methods & SSH_AUTH_METHOD_PASSWORD) {
#ifdef DEBUG
						printf("[SSH] trying password authentication\n");
#endif
						SSHRxString(self, "Password: ");
					} else {
						SSHRxString(self, "No supported authentication methods\r\n");
						SSHDisconnect(self);
					}
					break;
				}
				case SSH_AUTH_AGAIN:
					break;
			}
			break;
		case STATE_AUTH_PASS_INPUT:
			/* waiting for user input ... */
			break;
		case STATE_AUTH_PASSWORD:
			/* TODO: the password is encoded in DEC MCS, but we need UTF-8 here */
			result = ssh_userauth_password(self->session, NULL, self->input_buffer);
			if(result != SSH_AUTH_AGAIN) {
				/* clear the password from memory again */
				memset(self->input_buffer, 0, BUFFER_SIZE);
				self->input_end = 0;
			}
			switch(result) {
				case SSH_AUTH_SUCCESS:
					self->state = STATE_CHANNEL_NEW;
#ifdef DEBUG
					printf("[SSH] authenticated with password\n");
#endif
					break;
				case SSH_AUTH_ERROR:
					SSHRxError(self, "ssh_userauth_password", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				case SSH_AUTH_DENIED:
					self->state = STATE_AUTH_PASS_INPUT;
#ifdef DEBUG
					printf("[SSH] authentication denied\n");
#endif
					SSHRxString(self, "Password: ");
					break;
				case SSH_AUTH_PARTIAL:
					SSHRxString(self, "Password authentication was insufficient\r\n");
					SSHDisconnect(self);
					return;
				case SSH_AUTH_AGAIN:
					break;
			}
			break;
		case STATE_CHANNEL_NEW:
			self->pty = ssh_channel_new(self->session);
			if(!self->pty) {
				SSHRxString(self, "Failed to allocate SSH channel\r\n");
				SSHDisconnect(self);
				return;
			} else {
				self->state = STATE_CHANNEL_OPEN;
			}
			break;
		case STATE_CHANNEL_OPEN:
			result = ssh_channel_open_session(self->pty);
			switch(result) {
				case SSH_OK:
					self->state = STATE_PTY_REQUEST;
#ifdef DEBUG
					printf("[SSH] channel opened\n");
#endif
					break;
				case SSH_ERROR:
					SSHRxError(self, "ssh_channel_open_session", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				case SSH_AGAIN:
					/* try again in the next frame */
					break;
			}
			break;
		case STATE_PTY_REQUEST:
			result = ssh_channel_request_pty_size_modes(self->pty, "vt220", self->width, self->height,
					pty_modes, sizeof(pty_modes));
			switch(result) {
				case SSH_OK:
					self->state = STATE_SHELL_REQUEST;
#ifdef DEBUG
					printf("[SSH] pty opened\n");
#endif
					break;
				case SSH_ERROR:
					SSHRxError(self, "ssh_channel_request_pty_size_modes", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				case SSH_AGAIN:
					/* try again in the next frame */
					break;
			}
			break;
		case STATE_SHELL_REQUEST:
			result = ssh_channel_request_shell(self->pty);
			switch(result) {
				case SSH_OK:
					self->state = STATE_CONNECTED;
#ifdef DEBUG
					printf("[SSH] shell opened\n");
#endif
					break;
				case SSH_ERROR:
					SSHRxError(self, "ssh_channel_request_shell", ssh_get_error(self->session));
					SSHDisconnect(self);
					return;
				case SSH_AGAIN:
					/* try again in the next frame */
					break;
			}
			break;
		case STATE_CONNECTED:
			if(!ssh_channel_is_open(self->pty) || ssh_channel_is_eof(self->pty)) {
				SSHDisconnect(self);
			} else {
				result = ssh_channel_read_nonblocking(self->pty, self->rxbuf, SSH_BUFFER_SIZE, 0);
				switch(result) {
					case SSH_EOF:
						SSHDisconnect(self);
						break;
					case SSH_ERROR:
						SSHRxError(self, "ssh_channel_read_nonblocking", ssh_get_error(self->session));
						SSHDisconnect(self);
						return;
					case SSH_AGAIN:
						/* try again in the next frame */
						break;
					default:
						if(self->rxe && self->rx) {
							self->bufsz = result;
							self->bufrd = 0;
							while(self->bufrd < self->bufsz) {
								if(self->rxe()) {
									self->rx(self->rxbuf[self->bufrd++]);
								} else {
									break;
								}
							}
						}
						break;
				}
			}
			break;
	}
}

#endif
