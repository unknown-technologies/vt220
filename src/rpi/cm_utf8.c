#include <stdio.h>

#include "cm.h"

/* This module decodes incoming UTF-8 sequences and re-encodes them to DEC MCS
 * for the VT220. In addition, it encodes outgoing DEC MCS chracters to UTF-8
 * for the target process.
 *
 * Since UTF-8 cannot encode C1 characters, this module automatically
 * translates C1 characters to S7C1T encoding. This is never a problem for the
 * VT220 since it always accepts 7-bit escaped C1 characters and software
 * should support reception of 7-bit escaped C1 characters too.
 *
 * This translation ONLY works if the default character set mapping is left
 * unchanged, which means: G0 = ASCII, G1 = DEC Supplemental, GL = G0, GR = G1
 *
 * TODO: since virtually no UTF-8 program uses DRCS, up to 94 additional glyphs
 * could be dynamically loaded into the terminal via DRCS to emulate additional
 * Unicode glyphs.
 */

#define	STATE_BYPASS	-1

void CMResetUTF8(CM* self)
{
	if(self->utf8_state != STATE_BYPASS) {
		self->utf8_state = 0;
	}
	self->utf8_char = 0;
}

void CMSetUTF8Enabled(CM* self, BOOL enabled)
{
	if(enabled) {
		self->utf8_state = 0;
	} else {
		self->utf8_state = STATE_BYPASS;
	}
}

BOOL CMIsUTF8Enabled(CM* self)
{
	return self->utf8_state != STATE_BYPASS;
}

static inline void CMiFinishUTF8(CM* self)
{
	u32 utf8_value = self->utf8_char;

	if(!self->rx) {
		return;
	}

	unsigned char box_value = 0;

	switch(utf8_value) {
		case 0xA4:
			utf8_value = 0xA8;
			break;
		case 0x0152:
			utf8_value = 0xD7;
			break;
		case 0x0178:
			utf8_value = 0xDD;
			break;
		case 0x0153:
			utf8_value = 0xF7;
			break;
		case 0xFF:
			utf8_value = 0xFD;
			break;
		case 0xA6:
			utf8_value = '|'; /* approximation */
			break;
		case 0xD7:
			utf8_value = 'x'; /* approximation */
			break;
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xB4:
		case 0xB8:
		case 0xBE:
		case 0xD0:
		case 0xDE:
		case 0xF0:
		case 0xFE:
			utf8_value = 0;
			break;
		/* NOTE: there are multiple box drawing characters which look
		 * similar but cannot be represented with DEC Special Graphics.
		 * Those similar characters are all mapped to what CAN be
		 * represented by DEC Special Graphics. */
		case 0x2500: /* this is the real one */
		case 0x2501:
		case 0x2504:
		case 0x2505:
		case 0x2508:
		case 0x2509:
			box_value = 0x71;
			break;
		case 0x2502: /* this is the real one */
		case 0x2503:
		case 0x2506:
		case 0x2507:
		case 0x250A:
		case 0x250B:
			box_value = 0x78;
			break;
		case 0x250C: /* this is the real one */
		case 0x250D:
		case 0x250E:
		case 0x250F:
			box_value = 0x6C;
			break;
		case 0x2510: /* this is the real one */
		case 0x2511:
		case 0x2512:
		case 0x2513:
			box_value = 0x6B;
			break;
		case 0x2514: /* this is the real one */
		case 0x2515:
		case 0x2516:
		case 0x2517:
			box_value = 0x6D;
			break;
		case 0x2518: /* this is the real one */
		case 0x2519:
		case 0x251A:
		case 0x251B:
			box_value = 0x6A;
			break;
		case 0x251C: /* this is the real one */
		case 0x251D:
		case 0x251E:
		case 0x251F:
		case 0x2520:
		case 0x2521:
		case 0x2522:
		case 0x2523:
			box_value = 0x74;
			break;
		case 0x2524: /* this is the real one */
		case 0x2525:
		case 0x2526:
		case 0x2527:
		case 0x2528:
		case 0x2529:
		case 0x252A:
		case 0x252B:
			box_value = 0x75;
			break;
		case 0x252C: /* this is the real one */
		case 0x252D:
		case 0x252E:
		case 0x252F:
		case 0x2530:
		case 0x2531:
		case 0x2532:
		case 0x2533:
			box_value = 0x77;
			break;
		case 0x2534: /* this is the real one */
		case 0x2535:
		case 0x2536:
		case 0x2537:
		case 0x2538:
		case 0x2539:
		case 0x253A:
		case 0x253B:
			box_value = 0x76;
			break;
		case 0x253C: /* this is the real one */
		case 0x253D:
		case 0x253E:
		case 0x253F:
		case 0x2540:
		case 0x2541:
		case 0x2542:
		case 0x2543:
		case 0x2544:
		case 0x2545:
		case 0x2546:
		case 0x2547:
		case 0x2548:
		case 0x2549:
		case 0x254A:
		case 0x254B:
			box_value = 0x6E;
			break;
		/* the following characters are mapped 1:1 from DEC Special Graphics */
		case 0x25C6:
			box_value = 0x60;
			break;
		case 0x2592:
			box_value = 0x61;
			break;
		case 0x2409:
			box_value = 0x62;
			break;
		case 0x240C:
			box_value = 0x63;
			break;
		case 0x240D:
			box_value = 0x64;
			break;
		case 0x240A:
			box_value = 0x65;
			break;
		case 0x2424:
			box_value = 0x68;
			break;
		case 0x240B:
			box_value = 0x69;
			break;
		case 0x23BA:
			box_value = 0x6F;
			break;
		case 0x23BB:
			box_value = 0x70;
			break;
		case 0x23BC:
			box_value = 0x72;
			break;
		case 0x23BD:
			box_value = 0x73;
			break;
		case 0x2264:
			box_value = 0x79;
			break;
		case 0x2265:
			box_value = 0x7A;
			break;
		case 0x03C0:
			box_value = 0x7B;
			break;
		case 0x2260:
			box_value = 0x7C;
			break;
		/* the following two code points are for left/right double quotes */
		case 0x201C:
		case 0x201D:
			utf8_value = '"';
			break;
		case 0x2022: /* bullet */
		case 0x25CF: /* black circle */
			utf8_value = 0xB7; /* middle dot */
			break;
		case 0x2010: /* hyphen */
		case 0x2011: /* non-breaking hyphen */
		case 0x2012: /* figure dash */
		case 0x2013: /* en dash */
		case 0x2014: /* em dash */
		case 0x2015: /* horizontal bar */
		case 0x2212: /* minus sign */
		case 0x2E3A: /* two-em dash */
		case 0x2E3B: /* three-em dash */
		case 0xFE63: /* small hyphen-minus */
		case 0xFF0D: /* fullwidth hyphen-minus */
			utf8_value = '-';
			break;
		case 0x25C0: /* black left-pointing triangle */
		case 0x25C1: /* white left-pointing triangle */
		case 0x25C2: /* black left-pointing small triangle */
		case 0x25C3: /* white left-pointing small triangle */
		case 0x25C4: /* black left-pointing pointer */
		case 0x25C5: /* white left-pointing pointer */
		case 0x2190: /* leftwards arrow */
			utf8_value = '<';
			break;
		case 0x25B6: /* black right-pointing triangle */
		case 0x25B7: /* white right-pointing triangle */
		case 0x25B8: /* black right-pointing small triangle */
		case 0x25B9: /* white right-pointing small triangle */
		case 0x25BA: /* black right-pointing pointer */
		case 0x25BB: /* white right-pointing pointer */
		case 0x2192: /* rightwards arrow */
			utf8_value = '>';
			break;
		case 0x25B2: /* black up-pointing triangle */
		case 0x25B3: /* white up-pointing triangle */
		case 0x25B4: /* black up-pointing small triangle */
		case 0x25B5: /* white up-pointing small triangle */
		case 0x2191: /* upwards arrow */
			utf8_value = '^';
			break;
		case 0x25BC: /* black down-pointing triangle */
		case 0x25BD: /* white down-pointing triangle */
		case 0x25BE: /* black down-pointing small triangle */
		case 0x25BF: /* white down-pointing small triangle */
		case 0x2193: /* downwards arrow */
			utf8_value = 'v';
			break;
		case 0x25CB: /* white circle */
			utf8_value = 'o';
			break;
	}

	if(box_value > 0) {
		/* DEC Special Graphics */
		self->rx(ESC);
		self->rx('(');
		self->rx('0');
		self->rx(box_value);

		/* ASCII */
		self->rx(ESC);
		self->rx('(');
		self->rx('B');
	} else if(utf8_value >= 0x20 && utf8_value < 0x7F) {
		self->rx((unsigned char) utf8_value);
	} else if(utf8_value > 0xA0 && utf8_value < 0xFF) {
		self->rx((unsigned char) utf8_value);
	} else {
#ifdef DEBUG
		printf("[UTF8] unknown character: 0x%04X [0x%04X]\n", utf8_value, self->utf8_char);
#endif
		self->rx(SUB);
	}
}

void CMUTF8Receive(CM* self, unsigned char c)
{
	if(self->utf8_state == STATE_BYPASS) {
		if(self->rx) {
			self->rx(c);
		}
		return;
	}

	if(c < 0x80) {
#ifdef DEBUG
		if(self->utf8_state != 0) {
			printf("[UTF8] broken character (c=%02X, state=%d, char=%04X)\n", c, self->utf8_state, self->utf8_char);
		}
#endif
		self->utf8_state = 0;
		if(self->rx) {
			self->rx(c);
		}
	} else {
		if((c & 0xC0) == 0x80) {
			if(self->utf8_state > 1) {
				self->utf8_state--;
				self->utf8_char <<= 6;
				self->utf8_char |= c & 0x3F;
				if(self->utf8_state == 1) {
					CMiFinishUTF8(self);
					self->utf8_state = 0;
				}
			} else {
				if(self->rx) {
					self->rx(SUB);
				}
				self->utf8_state = 0;
#ifdef DEBUG
				printf("[UTF8] unexpected continuation byte\n");
#endif
			}
		} else if((c & 0xE0) == 0xC0) {
			/* 2 byte character */
			self->utf8_state = 2;
			self->utf8_char = c & 0x1F;
		} else if((c & 0xF0) == 0xE0) {
			/* 3 byte character */
			self->utf8_state = 3;
			self->utf8_char = c & 0x0F;
		} else if((c & 0xF8) == 0xF0) {
			/* 4 byte character */
			self->utf8_state = 4;
			self->utf8_char = c & 0x07;
		} else {
			/* invalid UTF-8 character, ignore */
#ifdef DEBUG
			printf("[UTF8] invalid UTF-8 byte (c=%02X, state=%d, char=%04X)\n", c, self->utf8_state,
					self->utf8_char);
#endif
			self->utf8_state = 0;
			self->utf8_char = 0;
			if(self->rx) {
				self->rx(SUB);
			}
		}
	}
}

void CMUTF8Transmit(CM* self, unsigned char c)
{
	u16 result;
	switch(c) {
		case 0xA8:
			result = 0xA4;
			break;
		case 0xD7:
			result = 0x152;
			break;
		case 0xDD:
			result = 0x178;
			break;
		case 0xF7:
			result = 0x153;
			break;
		case 0xFD:
			result = 0xFF;
			break;
		default:
			if(c >= 0x80 && c <= 0x9F) {
				/* encode as S7C1T */
				CMiSend(self, 0x1B);
				CMiSend(self, c - 0x40);
				return;
			}
			result = c;
			break;
	}

	if(result < 0x80) {
		CMiSend(self, result);
	} else {
		unsigned char high = (result >> 6) | 0xC0;
		unsigned char low = (result & 0x3F) | 0x80;
		CMiSend(self, high);
		CMiSend(self, low);
	}
}
