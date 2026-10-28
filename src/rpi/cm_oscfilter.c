#include "cm.h"

#define	STATE_BYPASS	-1
#define	STATE_NORMAL	0
#define	STATE_ESC	1
#define	STATE_OSC	2
#define	STATE_OSC_ESC	3

/* What is this and why is it necessary, you might wonder? This code filters
 * OSC sequences because systemd developers are children who have no idea what
 * they are doing. When you login to a standard systemd based Linux machine,
 * systemd checks if TERM=dumb. If this is not the case, it unconditionally
 * sends an OSC sequence with some "system information" to the terminal. They
 * never check if the terminal understands OSC at all, so on a VT220 you get a
 * line of garbage shown on every login/logout.
 *
 * This filter removes all OSC sequences (including the 8bit variant which the
 * systemd developers probably aren't even aware of, but anyway) because you
 * really don't want to see this garbage line and it's more work to remove the
 * relevant systemd misfeature on every installation which runs this program.
 *
 * If something goes wrong, you can reset the filter's state via the standard
 * VT220 "Clear Comm" function.
 *
 * This OSC filter is only used for local PTY sessions. For any remote system,
 * it is your responsibility to ensure this remote system knows how to deal
 * with a VT220.
 */

void CMInitOSCFilter(CM* self)
{
	self->osc_state = STATE_BYPASS;
}

void CMResetOSCFilter(CM* self)
{
	if(self->osc_state != STATE_BYPASS) {
		self->osc_state = STATE_NORMAL;
	}
}

void CMSetOSCEnabled(CM* self, BOOL enabled)
{
	if(enabled) {
		self->osc_state = STATE_NORMAL;
	} else {
		self->osc_state = STATE_BYPASS;
	}
}

void CMOSCReceive(CM* self, unsigned char c)
{
	switch(self->osc_state) {
		case STATE_BYPASS:
			CMUTF8Receive(self, c);
			break;
		case STATE_NORMAL:
			if(c == ESC) {
				self->osc_state = STATE_ESC;
			} else if(c == OSC && self->utf8_state == -1) {
				/* C1 codes must not be recognized in UTF-8 mode */
				self->osc_state = STATE_OSC;
			} else {
				CMUTF8Receive(self, c);
			}
			break;
		case STATE_ESC:
			if(c == ESC) {
				CMUTF8Receive(self, c);
			} else if(c == ']') {
				self->osc_state = STATE_OSC;
			} else if(self->rx) {
				CMUTF8Receive(self, ESC);
				CMUTF8Receive(self, c);
				self->osc_state = STATE_NORMAL;
			}
			break;
		case STATE_OSC:
			if(c == ST) {
				self->osc_state = STATE_NORMAL;
			} else if(c == ESC) {
				self->osc_state = STATE_OSC_ESC;
			}
			break;
		case STATE_OSC_ESC:
			if(c == ESC) {
				self->osc_state = STATE_OSC_ESC;
			} else if(c == '\\') {
				self->osc_state = STATE_NORMAL;
			} else {
				self->osc_state = STATE_OSC;
			}
			break;
	}
}
