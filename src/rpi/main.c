#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "types.h"
#include "kms.h"
#include "vt.h"
#include "config.h"
#include "renderer.h"
#include "telnet.h"
#include "tty.h"
#include "kb.h"
#include "error.h"

static EGL egl;
static KB kb;

static VT220 vt;
static VTRenderer renderer;
static TELNET telnet;
static TTY tty;

static unsigned long current_time = 0;

#define	MODE_NONE	0
#define	MODE_TTY	1
#define	MODE_TELNET	2

static int mode = MODE_NONE;

static volatile bool quit = false;

static void sigint_handler(int signum)
{
	(void) signum;

	quit = true;
}

static unsigned long long get_time(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (unsigned long long) t.tv_sec * 1000uLL + t.tv_nsec / 1000000uLL;
}

void process(void)
{
	unsigned long long now = get_time();

	unsigned long dt = (unsigned long) (now - current_time);
	if(dt > 1000) {
		/* more than one second? Act as if it was only one second */
		dt = 1000;
	}

	switch(mode) {
		case MODE_TELNET:
			TELNETPoll(&telnet);
			break;
		case MODE_TTY:
			TTYPoll(&tty);
			break;
	}

	KBPoll(&kb);
	VT220Process(&vt, dt);
	VTProcess(&renderer, dt);

	current_time = now;
}

static void vt_rx(unsigned char c)
{
	VT220Receive(&vt, c);
}

static int vt_rxe(void)
{
	return VT220CanReceive(&vt);
}

static void vt_flowcontrol_nop(int start)
{
	(void) start;
}

#if 0
static void telnet_tx(unsigned char c)
{
	TELNETSend(&telnet, c);
}

static void telnet_brk(void)
{
	TELNETBreak(&telnet);
}
#endif

static void tty_tx(unsigned char c)
{
	TTYSend(&tty, c);
}

static void tty_brk(void)
{
	TTYBreak(&tty);
}

static void tty_resize(unsigned int width, unsigned int height)
{
	TTYResize(&tty, width, height);
}

static void tty_update_baudrate(unsigned int rx, unsigned int tx)
{
	TTYSetBaudRate(&tty, rx, tx);
}

static void tty_update_flowcontrol(int enable)
{
	TTYSetFlowControl(&tty, enable);
}

static void tty_update_format(unsigned int format, unsigned int stopbits)
{
	unsigned int data;
	unsigned int stop = stopbits == VT220_COMM_2_STOP_BITS ? 2 : 1;
	unsigned int parity;

	switch(format) {
		default:
		case VT220_COMM_8BIT_NO_PARITY:
			data = 8;
			parity = TTY_PARITY_NONE;
			break;
		case VT220_COMM_8BIT_EVEN_PARITY:
			data = 8;
			parity = TTY_PARITY_EVEN;
			break;
		case VT220_COMM_8BIT_ODD_PARITY:
			data = 8;
			parity = TTY_PARITY_ODD;
			break;
		case VT220_COMM_7BIT_NO_PARITY:
			data = 7;
			parity = TTY_PARITY_NONE;
			break;
		case VT220_COMM_7BIT_EVEN_PARITY:
			data = 7;
			parity = TTY_PARITY_EVEN;
			break;
		case VT220_COMM_7BIT_ODD_PARITY:
			data = 7;
			parity = TTY_PARITY_ODD;
			break;
		case VT220_COMM_7BIT_MARK_PARITY:
			data = 7;
			parity = TTY_PARITY_MARK;
			break;
		case VT220_COMM_7BIT_SPACE_PARITY:
			data = 7;
			parity = TTY_PARITY_SPACE;
			break;
		case VT220_COMM_7BIT_EVEN_PARITY_NO_CHECK:
			data = 7;
			parity = TTY_PARITY_EVEN_NOCHK;
			break;
		case VT220_COMM_7BIT_ODD_PARITY_NO_CHECK:
			data = 7;
			parity = TTY_PARITY_ODD_NOCHK;
			break;
		case VT220_COMM_8BIT_EVEN_PARITY_NO_CHECK:
			data = 8;
			parity = TTY_PARITY_EVEN_NOCHK;
			break;
		case VT220_COMM_8BIT_ODD_PARITY_NO_CHECK:
			data = 8;
			parity = TTY_PARITY_ODD_NOCHK;
			break;
	}

	TTYSetFormat(&tty, data, stop, parity);
}

void kb_down(KB* kb, int code)
{
	(void) kb;

	VT220KeyboardKeyDown(&vt, code);
}

void kb_up(KB* kb, int code)
{
	(void) kb;

	VT220KeyboardKeyUp(&vt, code);
}

static void print_usage(const char* self)
{
	printf("Usage: %s [OPTIONS] [/dev/tty [baud]]\n"
		"\n"
		"OPTIONS\n"
		"  -h            Show this help message\n"
		"  -v            Show version and exit\n"
		"  -g            Disable glow effect\n"
		"  -cg           Screen color: green\n"
		"  -cw           Screen color: white\n"
		"  -ca           Screen color: amber\n"
		"  -f 0.75       Electron beam focus\n"
		"  -i 1.0        Electron beam intensity (brightness)\n"
		"  -p            Use simple linear phosphor emulation model\n"
		"  -r            Raw mode, deactivates all post processing; implies -g\n"
		"  -z            Rotate screen by 180 degrees\n"
		"  -y /dev/ttyS0 Connect the terminal to the serial line /dev/ttyS0\n"
		"  -yr 9600      Set baud rate to 9600\n"
		"  -b            Enable buffering (slow processing)\n", self);
}

void print_version(void)
{
	printf("DEC VT220 emulator r" REVISION " (" BUILDDATE ", " BUILDTIME " (UTC))\n"
			"License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n");
}

int main(int argc, char** argv)
{
	const char* self = *argv;
	const char* serial = NULL;
	float focus = 0.75f;
	float intensity = 1.0f;
	bool rawmode = false;
	bool simple_phosphor = false;
	bool buffering = false;
	bool enable_glow = true;
	int baud = -1;
	bool flip = false;

	unsigned int color = VT220_SCREEN_COLOR_AMBER;

	argc--;
	argv++;
	for(int i = 0; i < argc; i++) {
		char* arg = argv[i];
		if(!strcmp(arg, "-g")) {
			enable_glow = false;
		} else if(!strcmp(arg, "-cw")) {
			color = VT220_SCREEN_COLOR_WHITE;
		} else if(!strcmp(arg, "-cg")) {
			color = VT220_SCREEN_COLOR_GREEN;
		} else if(!strcmp(arg, "-ca")) {
			color = VT220_SCREEN_COLOR_AMBER;
		} else if(!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			print_usage(self);
			return 0;
		} else if(!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
			print_version();
			return 0;
		} else if(!strcmp(arg, "-f")) {
			if(i + 1 >= argc) {
				print_usage(self);
				return 1;
			} else {
				focus = atof(argv[i + 1]);
				i += 1;
			}
		} else if(!strcmp(arg, "-i")) {
			if(i + 1 >= argc) {
				print_usage(self);
				return 1;
			} else {
				intensity = atof(argv[i + 1]);
				i += 1;
			}
		} else if(!strcmp(arg, "-r")) {
			rawmode = true;
		} else if(!strcmp(arg, "-p")) {
			simple_phosphor = true;
		} else if(!strcmp(arg, "-b")) {
			buffering = true;
		} else if(!strcmp(arg, "-z")) {
			flip = true;
		} else if(!strcmp(arg, "-y")) {
			if(i + 1 >= argc) {
				print_usage(self);
				return 1;
			}
			serial = argv[i + 1];
			i++;
		} else if(!strcmp(arg, "-yr")) {
			if(i + 1 >= argc) {
				print_usage(self);
				return 1;
			} else {
				baud = atoi(argv[i + 1]);
				i += 1;
			}
		} else {
			if(i + 1 == argc) {
				serial = argv[i];
				break;
			} else if(i + 2 > argc) {
				print_usage(self);
				return 1;
			} else {
				serial = argv[i];
				baud = atoi(argv[i + 1]);
				break;
			}
		}
	}

	/* ignore SIGPIPE which can happen if stdout/stderr are redirected */
	signal(SIGPIPE, SIG_IGN);

	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &sa, NULL) == -1) {
		printf("WARNING: failed to set SIGINT signal handler\n");
	}

	if(!KBInit(&kb)) {
		fprintf(stderr, "failed to initialize keyboard\n");
		return 1;
	}

	DRM* drm = DRMInit(NULL, NULL, 0);
	if(!drm) {
		fprintf(stderr, "failed to initialize DRM\n");
		return 1;
	}

	GBM* gbm = GBMInit(drm->fd, drm->mode->hdisplay,
			drm->mode->vdisplay, DRM_FORMAT_ARGB8888,
			DRM_FORMAT_MOD_LINEAR, false);
	if(!gbm) {
		fprintf(stderr, "failed to initialize GBM\n");
		return 1;
	}

	printf("Screen size: %dx%d\n", drm->mode->hdisplay,
			drm->mode->vdisplay);

	EGLInit(&egl, drm, gbm, 0);

	VT220Init(&vt);
	VT220SetScreenColor(&vt, color);
	VT220SetBuffering(&vt, buffering);

	VTInitRenderer(&renderer, &vt);

	CFGLoadState(&vt);

	VT220ShowInitScreen(&vt);

	VTEnableGlow(&renderer, enable_glow);
	VTSetRaw(&renderer, rawmode);
	VTSetSimplePhosphor(&renderer, simple_phosphor);
	VTSetFocus(&renderer, focus);
	VTSetIntensity(&renderer, intensity);
	VTSetFlipped(&renderer, flip);

	KBSetDown(&kb, kb_down);
	KBSetUp(&kb, kb_up);

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LEQUAL);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	if(serial) {
		mode = MODE_TTY;

		TTYInit(&tty);
		TTYOpen(&tty, serial);

		if(baud != -1) {
			VT220SetBaudRate(&vt, baud, baud);
		}

		tty.rx = vt_rx;
		tty.rxe = vt_rxe;
		vt.rx = tty_tx;
		vt.brk = tty_brk;
		vt.resize = tty_resize;
		vt.flowcontrol = vt_flowcontrol_nop;
		vt.update_baudrate = tty_update_baudrate;
		vt.update_flowcontrol = tty_update_flowcontrol;
		vt.update_format = tty_update_format;
	}

	VT220InitComm(&vt);

	/* set window size of connected device */
	if(vt.resize) {
		vt.resize(vt.columns, vt.lines);
	}

	current_time = get_time();

	GL_ERROR();

	while(!quit) {
		EGLBegin(&egl);

		GL_ERROR();

		glClear(GL_COLOR_BUFFER_BIT);

		process();

		VTRender(&renderer, drm->mode->hdisplay, drm->mode->vdisplay);

		if(!EGLEnd(&egl)) {
			break;
		}
	}

	EGLDestroy(&egl);
	GBMDestroy(gbm);
	DRMDestroy(drm);

	KBClose(&kb);

	return 0;
}
