#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

#include <math.h>

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "types.h"
#include "vt.h"
#include "renderer.h"
#include "telnet.h"
#include "pty.h"

#define	FPS			60

#define	SCREEN_WIDTH		800
#define	SCREEN_HEIGHT		480

static bool enable_glow = true;
static bool use_telnet = false;
static bool use_pty = false;
static bool is_fullscreen = false;

static int screen_width;
static int screen_height;

static int window_width = SCREEN_WIDTH;

static int window_pos_x;
static int window_pos_y;

static GLFWwindow* window;

static VT220 vt;
static VTRenderer renderer;
static TELNET telnet;
static PTY pty;

static unsigned long current_time = 0;

#ifdef _WIN32
PFNGLGENFRAMEBUFFERSPROC	glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC	glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC	glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC	glCheckFramebufferStatus;

static void load_gl_extensions(void)
{
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
}
#endif

#ifdef NDEBUG
#define GL_ERROR()
#else
#define	GL_ERROR()	check_error(__FILE__, __LINE__)

void check_error(const char* filename, unsigned int line)
{
	GLenum error = glGetError();
	switch(error) {
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			printf("%s:%u: Error: GL_INVALID_ENUM\n", filename, line);
			break;
		case GL_INVALID_VALUE:
			printf("%s:%u: Error: GL_INVALID_VALUE\n", filename, line);
			break;
		case GL_INVALID_OPERATION:
			printf("%s:%u: Error: GL_INVALID_OPERATION\n", filename, line);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			printf("%s:%u: Error: GL_INVALID_FRAMEBUFFER_OPERATION\n", filename, line);
			break;
		case GL_OUT_OF_MEMORY:
			printf("%s:%u: Error: GL_OUT_OF_MEMORY\n", filename, line);
			exit(1);
			break;
		case GL_STACK_UNDERFLOW:
			printf("%s:%u: Error: GL_STACK_UNDERFLOW\n", filename, line);
			break;
		case GL_STACK_OVERFLOW:
			printf("%s:%u: Error: GL_STACK_OVERFLOW\n", filename, line);
			break;
		default:
			printf("%s:%u: Unknown error 0x%X\n", filename, line, error);
	}
}
#endif

static unsigned long get_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

void process(void)
{
	unsigned long now = get_time();

	unsigned long dt = now - current_time;

	if(use_telnet) {
		TELNETPoll(&telnet);
	}

	if(use_pty) {
		PTYPoll(&pty);
	}

	VT220Process(&vt, dt);
	VTProcess(&renderer, dt);

	current_time = now;
}

static int get_monitor(GLFWmonitor** monitor, GLFWwindow* window)
{
	int window_x;
	int window_y;

	glfwGetWindowPos(window, &window_x, &window_y);

	int window_w;
	int window_h;

	glfwGetWindowSize(window, &window_w, &window_h);

	int monitor_count;
	GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

	int window_x1 = window_x + window_w;
	int window_y1 = window_y + window_h;

	int max_overlap = 0;
	GLFWmonitor* closest = NULL;

	for(int i = 0; i < monitor_count; i++) {
		GLFWmonitor* mon = monitors[i];

		int pos_x;
		int pos_y;

		glfwGetMonitorPos(mon, &pos_x, &pos_y);

		const GLFWvidmode* mode = glfwGetVideoMode(mon);

		int pos_x1 = pos_x + mode->width;
		int pos_y1 = pos_y + mode->height;

		/* overlap? */
		if(!(
			(window_x1 < pos_x) ||
			(window_x > pos_x1) ||
			(window_y < pos_y) ||
			(window_y > pos_y1)
		)) {
			int overlap_w = 0;
			int overlap_h = 0;

			/* x, width */
			if(window_x < pos_x) {
				if(window_x1 < pos_x1) {
					overlap_w = window_x1 - pos_x;
				} else {
					overlap_w = mode->width;
				}
			} else {
				if(pos_x1 < window_x1) {
					overlap_w = pos_x1 - window_x;
				} else {
					overlap_w = window_w;
				}
			}

			// y, height
			if(window_y < pos_y) {
				if(window_y1 < pos_y1) {
					overlap_h = window_y1 - pos_y;
				} else {
					overlap_h = mode->height;
				}
			} else {
				if(pos_y1 < window_y1) {
					overlap_h = pos_y1 - window_y;
				} else {
					overlap_h = window_h;
				}
			}

			int overlap = overlap_w * overlap_h;
			if (overlap > max_overlap) {
				closest = monitors[i];
				max_overlap = overlap;
			}
		}
	}

	if(closest) {
		*monitor = closest;
		return 1;
	} else {
		return 0;
	}
}

static void enter_fullscreen(void)
{
	glfwGetWindowPos(window, &window_pos_x, &window_pos_y);

	GLFWmonitor* mon;
	if(get_monitor(&mon, window)) {
		int pos_x;
		int pos_y;

		glfwGetMonitorPos(mon, &pos_x, &pos_y);
		const GLFWvidmode* mode = glfwGetVideoMode(mon);

		glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowAttrib(window, GLFW_FLOATING, GLFW_TRUE);

		glfwSetWindowPos(window, pos_x, pos_y);
		glfwSetWindowSize(window, mode->width, mode->height);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		is_fullscreen = true;
	}
}

static void exit_fullscreen(void)
{
	glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
	glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
	glfwSetWindowAttrib(window, GLFW_FLOATING, GLFW_FALSE);

	glfwSetWindowSize(window, window_width, SCREEN_HEIGHT);
	glfwSetWindowPos(window, window_pos_x, window_pos_y);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	is_fullscreen = false;
}

static void toggle_fullscreen(void)
{
	if(is_fullscreen) {
		exit_fullscreen();
	} else {
		enter_fullscreen();
	}

	glfwSwapInterval(1);
}

void key_handler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS) {
		switch(key) {
			case GLFW_KEY_F4:
				if(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) {
					VT220KeyboardKeyDown(&vt, key);
				} else {
					toggle_fullscreen();
				}
				break;
			default:
				VT220KeyboardKeyDown(&vt, key);
				break;
		}
	} else if(action == GLFW_RELEASE) {
		/* One might wonder why F4 has no special handling here: since
		 * only F4 without modifiers is intercepted, this could result
		 * in a situation where the modifier is released before F4 is
		 * released. This would casue a stuck F4 key. Therefore, all
		 * keys are always released here. */
		VT220KeyboardKeyUp(&vt, key);
	}
}

void char_handler(GLFWwindow* window, unsigned int code)
{
	VT220KeyboardChar(&vt, code);
}

static void error_handler(int error, const char* description)
{
	printf("Error 0x%X: %s\n", error, description);
}

void print_ch(unsigned char c)
{
	// ignore XON/XOFF
	if(c == DC1 || c == DC3) {
		return;
	}

	putc(c, stdout);
	fflush(stdout);

	VT220Receive(&vt, c);
}

void display_func(void)
{
	GL_ERROR();
	process();

	VTRender(&renderer, screen_width, screen_height);
	GL_ERROR();
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

static void telnet_tx(unsigned char c)
{
	TELNETSend(&telnet, c);
}

static void telnet_brk(void)
{
	TELNETBreak(&telnet);
}

static void resize(unsigned int width, unsigned int height)
{
	window_width = width == 132 ? (9 * 132) : (10 * 80);
	if(!is_fullscreen) {
		glfwSetWindowSize(window, window_width, SCREEN_HEIGHT);
	}
}

static void pty_tx(unsigned char c)
{
	PTYSend(&pty, c);
}

static void pty_brk(void)
{
	PTYBreak(&pty);
}

static void pty_resize(unsigned int width, unsigned int height)
{
	resize(width, height);
	PTYResize(&pty, width, height);
}

static void print_usage(const char* self)
{
	printf("Usage: %s [OPTIONS] [-l | -s command | -t hostname port]\n"
		"\n"
		"OPTIONS\n"
		"  -h            Show this help message\n"
		"  -g            Disable glow effect\n"
		"  -cg           Screen color: green\n"
		"  -cw           Screen color: white\n"
		"  -ca           Screen color: amber\n"
		"  -f 0.75       Electron beam focus\n"
		"  -i 1.0        Electron beam intensity (brightness)\n"
		"  -p            Use simple linear phosphor emulation model\n"
		"  -r            Raw mode, deactivates all post processing; implies -g\n"
		"  -l            Loopback / local mode\n"
		"  -s /bin/sh    Execute /bin/sh in the terminal\n"
		"  -t host port  Establish TELNET connection to host:port\n"
		"  -u            Unlimited FPS\n"
		"  -b            Enable buffering (slow processing)\n"
		"\n"
		"If no option is provided, -s $(getent passwd $UID | cut -d: -f7) is assumed.\n", self);
}

char** get_default_argv(void)
{
	struct passwd* pwd;
	errno = 0;
	char* shell = NULL;
	uid_t uid = getuid();
	while((pwd = getpwent())) {
		if(pwd->pw_uid == uid) {
			shell = strdup(pwd->pw_shell);
			break;
		} else {
			errno = 0;
		}
	}
	if(errno) {
		perror("getpwent");
		exit(1);
	}
	endpwent();

	if(!shell) {
		printf("Failed to get default shell\n");
		exit(1);
	}

	char** argv = (char**) malloc(2 * sizeof(char*));
	argv[0] = shell;
	argv[1] = NULL;
	return argv;
}

int main(int argc, char** argv, char** envp)
{
	const char* self = *argv;
	char** shell = NULL;
	const char* hostname = NULL;
	int port;
	bool loopback = false;
	float focus = 0.75f;
	float intensity = 1.0f;
	bool rawmode = false;
	bool simple_phosphor = false;
	bool unlimited_fps = false;
	bool buffering = false;

	unsigned int color = VT220_SCREEN_COLOR_GREEN;

	argc--;
	argv++;
	for(int i = 0; i < argc; i++) {
		char* arg = argv[i];
		if(!strcmp(arg, "-g")) {
			enable_glow = false;
		} else if(!strcmp(arg, "-s")) {
			if(i + 1 >= argc) {
				// use default shell
				shell = get_default_argv();
			} else {
				shell = &argv[i + 1];
				i = argc;
			}
			break;
		} else if(!strcmp(arg, "-t")) {
			if(i + 2 >= argc) {
				print_usage(self);
				return 1;
			} else {
				hostname = argv[i + 1];
				port = atoi(argv[i + 2]);
				i += 2;
			}
			break;
		} else if(!strcmp(arg, "-l")) {
			loopback = true;
		} else if(!strcmp(arg, "-cw")) {
			color = VT220_SCREEN_COLOR_WHITE;
		} else if(!strcmp(arg, "-cg")) {
			color = VT220_SCREEN_COLOR_GREEN;
		} else if(!strcmp(arg, "-ca")) {
			color = VT220_SCREEN_COLOR_AMBER;
		} else if(!strcmp(arg, "-u")) {
			unlimited_fps = true;
		} else if(!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			print_usage(self);
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
		} else {
			if(i + 2 > argc) {
				print_usage(self);
				return 1;
			}
			hostname = argv[i];
			port = atoi(argv[i + 1]);
			break;
		}
	}

	if(!loopback && !hostname && !shell) {
		// no args => default shell
		shell = get_default_argv();
	}

	glfwSetErrorCallback(error_handler);
	if(!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		return 1;
	}

	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	glfwWindowHint(GLFW_AUTO_ICONIFY, 0);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VT220", NULL, NULL);
	if(!window) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return 1;
	}

	glfwSetKeyCallback(window, key_handler);
	glfwSetCharCallback(window, char_handler);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

#ifndef NDEBUG
	const unsigned char* gl_vendor = glGetString(GL_VENDOR);
	const unsigned char* gl_renderer = glGetString(GL_RENDERER);
	const unsigned char* gl_version = glGetString(GL_VERSION);
	const unsigned char* gl_glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("GL Vendor:    %s\n", gl_vendor);
	printf("GL Renderer:  %s\n", gl_renderer);
	printf("GL Version:   %s\n", gl_version);
	printf("GLSL Version: %s\n", gl_glsl_version);
#endif

#ifdef _WIN32
	load_gl_extensions();
#endif

#ifndef NDEBUG
	int num_ext = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	for(int i = 0; i < num_ext; i++) {
		if(!strcmp((const char*) glGetStringi(GL_EXTENSIONS, i), "GL_ARB_compatibility")) {
			printf("GL Compatiblity Profile\n");
			break;
		}
	}
#endif

	GL_ERROR();

	VT220Init(&vt);
	VT220SetScreenColor(&vt, color);
	VT220SetBuffering(&vt, buffering);
	vt.rx = print_ch;
	vt.resize = resize;

	VTInitRenderer(&renderer, &vt);
	VTEnableGlow(&renderer, enable_glow);
	VTSetRaw(&renderer, rawmode);
	VTSetSimplePhosphor(&renderer, simple_phosphor);
	VTSetFocus(&renderer, focus);
	VTSetIntensity(&renderer, intensity);

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LEQUAL);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	GL_ERROR();

	if(hostname) {
		char buf[256];
		use_telnet = true;

		VT220ReceiveText(&vt, "\x9b" "2J\x9bH\x9b" "12h\x9b?7h");
		snprintf(buf, 256, "Connecting to %s on port %d\r\n", hostname, port);
		buf[255] = 0;
		VT220ReceiveText(&vt, buf);

		TELNETInit(&telnet);
		telnet.rx = vt_rx;
		telnet.rxe = vt_rxe;
		vt.rx = telnet_tx;
		vt.brk = telnet_brk;
		vt.flowcontrol = vt_flowcontrol_nop;

		TELNETConnect(&telnet, hostname, port);
	} else if(shell) {
		use_pty = true;

		VT220ReceiveText(&vt, "\x9b" "2J\x9bH\x9b" "12h\x9b?7h");

		PTYInit(&pty);
		PTYOpen(&pty, shell, envp);

		pty.rx = vt_rx;
		pty.rxe = vt_rxe;
		vt.rx = pty_tx;
		vt.brk = pty_brk;
		vt.resize = pty_resize;
		vt.flowcontrol = vt_flowcontrol_nop;
	}

	current_time = get_time();

#define	FRAME_TIME	((unsigned int) (1000.0 / FPS))
	unsigned long last_time = get_time();
	unsigned long last_frame = get_time();
	unsigned long next = last_frame + FRAME_TIME;
	unsigned long frames = 0;
	unsigned long fps = 0;
	bool fps_limit = false;
	while(!glfwWindowShouldClose(window)) {
		/* compute current FPS */
		unsigned long now = get_time();
		unsigned long dt = now - last_time;
		if(dt >= 1000) {
			last_time = now;
			fps = frames;
			frames = 0;

#ifdef SHOW_FPS
			printf("\r\x1b[2KFPS: %lu", fps);
			fflush(stdout);
#endif

			if(!unlimited_fps && fps > FPS * 4) {
				printf("\r\x1b[2KLimiting FPS to %u\n", FPS);
				fps_limit = true;
			}
		} else {
			frames++;
		}

		if(fps_limit && !unlimited_fps) {
			/* sleep between frames to roughly get the target FPS,
			 * this is relevant on GPUs which ignore vsync like
			 * NVIDIA T4 cards */
			unsigned long unow = get_time();
			if(next < unow) {
				next = last_frame + FRAME_TIME;
				if(next < unow) {
					next = unow + FRAME_TIME;
				}
			}
			unsigned long udelay = next - unow;
			struct timespec dly = {
				.tv_sec = udelay / 1000,
				.tv_nsec = (udelay % 1000) * 1000000
			};
			nanosleep(&dly, NULL);

			last_frame = unow;
		}

		glfwGetFramebufferSize(window, &screen_width, &screen_height);

		glViewport(0, 0, screen_width, screen_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		display_func();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* VERY IMPORTANT!
	 * If this extra handling is missing, KDE5 / Plasma will shit itself on
	 * certain NVIDIA cards when using real fullscreen mode! */
	if(is_fullscreen) {
		exit_fullscreen();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
