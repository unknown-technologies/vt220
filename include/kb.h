#ifndef __KB_H__
#define __KB_H__

#include "types.h"

#define	KB_LED_HOLD	_BV(0)
#define	KB_LED_LOCK	_BV(1)
#define	KB_LED_WAIT	_BV(2)
#define	KB_LED_COMP	_BV(3)

typedef struct KB KB;

struct KB {
	int		fd;

	unsigned int	modifiers;
	unsigned int	leds;

	void*		user;

	void		(*down)(KB* kb, int code);
	void		(*up)(KB* kb, int code);
};

static inline void KBSetUser(KB* kb, void* user)
{
	kb->user = user;
}

static inline void* KBGetUser(KB* kb)
{
	return kb->user;
}

static inline void KBSetDown(KB* kb, void (*cb)(KB*, int)) {
	kb->down = cb;
}

static inline void KBSetUp(KB* kb, void (*cb)(KB*, int)) {
	kb->up = cb;
}

BOOL KBInit(KB* kb);
BOOL KBOpen(KB* kb, const char* filename);
void KBClose(KB* kb);

void KBSetLEDs(KB* kb);
void KBPoll(KB* kb);

#endif
