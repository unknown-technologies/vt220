#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include "types.h"
#include "kb.h"

static void KBiProcess(KB* kb, struct input_event* event);

BOOL KBInit(KB* kb)
{
	char buf[256];
	FILE* f = fopen("/proc/bus/input/devices", "rt");
	if(!f) {
		printf("Failed to open device list: %s\n", strerror(errno));
		return FALSE;
	}

	char name[256] = { 0 };
	char handler[256] = { 0 };
	char dev[256] = { 0 };

	while(fgets(buf, sizeof(buf), f)) {
		buf[255] = 0;
		switch(*buf) {
			case 'I':
				/* new descriptor, ignore */
				memset(name, 0, sizeof(name));
				memset(handler, 0, sizeof(handler));
				break;
			case 'N':
				/* name */
				memset(name, 0, sizeof(name));
				char* start = strstr(buf, "Name=");
				if(start) {
					memcpy(name, start + 6,
					strlen(start + 6));
					char* c = strchr(name, '"');
					if(c) {
						*c = 0;
					}
				}
				break;
			case 'H':
				/* handler */
				char* event = strstr(buf, "event");
				if(strstr(buf, "kbd") && strstr(buf, "leds") && event) {
					char* c = strchr(event, ' ');
					if(c) {
						*c = 0;
					} else {
						c = strchr(event, '\n');
						if(c) {
							*c = 0;
						}
					}
					snprintf(dev, sizeof(dev), "/dev/input/%s", event);
					goto found;
				}
				break;
		}
	}

	fclose(f);
	return FALSE;

found:
	fclose(f);

#ifdef DEBUG
	printf("name: \"%s\", device=%s\n", name, dev);
#endif
	return KBOpen(kb, dev);
}

BOOL KBOpen(KB* kb, const char* filename)
{
	memset(kb, 0, sizeof(KB));

	kb->fd = open(filename, O_RDWR | O_NONBLOCK);
	if(kb->fd < 0) {
		printf("Failed to open input device: %s\n", strerror(errno));
		return FALSE;
	}

	if(ioctl(kb->fd, EVIOCGRAB, 1) == -1) {
		printf("Failed to grab keyboard: %s\n", strerror(errno));
		close(kb->fd);
		return FALSE;
	}

	KBSetLEDs(kb);

	return TRUE;
}

void KBClose(KB* kb)
{
	if(kb->fd >= 0) {
		/* reset LEDs on exit */
		kb->leds = 0;
		KBSetLEDs(kb);

		ioctl(kb->fd, EVIOCGRAB, 0);

		close(kb->fd);
		kb->fd = -1;
	}
}

#define	BUFSZ	128
void KBPoll(KB* kb)
{
	if(kb->fd < 0) {
		return;
	}

	struct input_event events[BUFSZ];

	errno = 0;
	ssize_t sz = read(kb->fd, events, sizeof(events));
	if(sz == 0) {
		return;
	} else if(sz == -1 && errno == EAGAIN) {
		return;
	} else if(sz < 0) {
		printf("read failed: %s\n", strerror(errno));
		return;
	}

	size_t eventcnt = sz / sizeof(struct input_event);
	for(size_t i = 0; i < eventcnt; i++) {
		KBiProcess(kb, &events[i]);
	}
}

static void KBiProcess(KB* kb, struct input_event* event)
{
	if(event->type != EV_KEY) {
		return;
	}

	/* ignore key repeat */
	switch(event->value) {
		case 0:
			if(kb->up) {
				kb->up(kb, event->code);
			}
			break;
		case 1:
			if(kb->down) {
				kb->down(kb, event->code);
			}
			break;
		case 2:
			/* key repeat */
			break;
	}
}

/* LEDs are mapped as follows:
 * - Compose = Num Lock
 * - Lock = Caps Lock
 * - Hold = Scroll Lock
 */
void KBSetLEDs(KB* kb)
{
	struct input_event event[4];

	event[0].type = EV_LED;
	event[0].code = LED_NUML;
	event[0].value = (kb->leds & KB_LED_COMP) ? 1 : 0;

	event[1].type = EV_LED;
	event[1].code = LED_CAPSL;
	event[1].value = (kb->leds & KB_LED_LOCK) ? 1 : 0;

	event[2].type = EV_LED;
	event[2].code = LED_SCROLLL;
	event[2].value = (kb->leds & KB_LED_HOLD) ? 1 : 0;

	write(kb->fd, &event, sizeof(event));
}
