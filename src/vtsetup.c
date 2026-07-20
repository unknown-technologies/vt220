#include <string.h>

#include "types.h"
#include "vt.h"

#define	VT220_SETUP_MOVE_NONE			0
#define	VT220_SETUP_MOVE_UP			1
#define	VT220_SETUP_MOVE_DOWN			2
#define	VT220_SETUP_MOVE_LEFT			3
#define	VT220_SETUP_MOVE_RIGHT			4
#define	VT220_SETUP_MOVE_LEFT_MARGIN		5

#define	SETUP_SCREEN_DIRECTORY	0
#define	SETUP_SCREEN_DISPLAY	1
#define	SETUP_SCREEN_GENERAL	2
#define	SETUP_SCREEN_COMM	3
#define	SETUP_SCREEN_PRINTER	4
#define	SETUP_SCREEN_KEYBOARD	5
#define	SETUP_SCREEN_TAB	6
#define	SETUP_SCREEN_COUNT	7

static const char* vt220_setup_screen_names[SETUP_SCREEN_COUNT] = {
	"Set-Up Directory",
	"Display Set-Up",
	"General Set-Up",
	"Communications Set-Up",
	"Printer Set-Up",
	"Keyboard Set-Up",
	"Tab Set-Up"
};

static const char* vt220_keyboard_languages[16] = {
	"Unknown Keyboard",
	"North American Keyboard",
	"British Keyboard",
	"Flemish Keyboard",
	"Canadian(French) Keyboard",
	"Danish Keyboard",
	"Finnish Keyboard",
	"German Keyboard",
	"Dutch Keyboard",
	"Italian Keyboard",
	"Swiss(French) Keyboard",
	"Swiss(German) Keyboard",
	"Swedish Keyboard",
	"Norwegian Keyboard",
	"French/Belgian Keyboard",
	"Spanish Keyboard"
};

static const unsigned int VT220GetNextBaudRate(unsigned int baud, int allow_zero)
{
	switch(baud) {
		case 0:
			return 75;
		case 75:
			return 110;
		case 110:
			return 150;
		case 150:
			return 300;
		case 300:
			return 600;
		case 600:
			return 1200;
		case 1200:
			return 2400;
		case 2400:
		default:
			return 4800;
		case 4800:
			return 9600;
		case 9600:
			return 19200;
		/* the following rates are for modern use, a real VT220
		 * only supports up to 19200 */
		case 19200:
			return 38400;
		case 38400:
			return 57600;
		case 57600:
			return 115200;
		case 115200:
			if(allow_zero) {
				return 0;
			} else {
				return 75;
			}
		/* Technically various UARTs support higher rates, like 230400
		 * baud, but others do not. It would therefore be more work to
		 * figure out what is/isn't supported on the local hardware. */
	}
}

const char* VT220SetupGetTitle(VT220* vt)
{
	if(vt->setup.screen < 0 || vt->setup.screen >= SETUP_SCREEN_COUNT) {
		return "???";
	} else {
		return vt220_setup_screen_names[vt->setup.screen];
	}
}

void VT220SetupEraseDisplay(VT220* vt)
{
	memset(vt->setup.text, 0, 8 * 132 * sizeof(VT220CELL));
	memset(vt->setup.line_attributes, 0, 8);
}

void VT220SetupEraseLine(VT220* vt)
{
	memset(&vt->setup.text[vt->setup.write_y * vt->columns], 0, vt->columns * sizeof(VT220CELL));
}

void VT220SetupWrite(VT220* vt, const u16 c, const int sgr, int display_controls)
{
	u16 glyph = c;
	if(display_controls) {
		if(glyph == 0x20) {
			glyph = 0;
		} else if(glyph < 0x20) {
			glyph += 0x100;
		}
	} else if(c == 0x20) {
		glyph = 0;
	}

	int idx = vt->setup.write_y * vt->columns + vt->setup.write_x;
	vt->setup.text[idx].text = glyph;
	vt->setup.text[idx].attr = sgr;

	vt->setup.write_x++;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_y++;
		if(vt->setup.write_y >= 8) {
			vt->setup.write_y = 7;
		}
	}
}

void VT220SetupSetLineAttribute(VT220* vt, int attr)
{
	vt->setup.line_attributes[vt->setup.write_y] = attr;
}

void VT220SetupGoto(VT220* vt, const int line, const int column)
{
	vt->setup.write_x = column - 1;
	vt->setup.write_y = line - 1;
}

void VT220SetupCursorRight(VT220* vt)
{
	vt->setup.write_x++;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_x = vt->columns - 1;
	}
}

void VT220SetupCursorRightN(VT220* vt, int n)
{
	vt->setup.write_x += n;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_x = vt->columns - 1;
	}
}

void VT220SetupCursorSave(VT220* vt)
{
	vt->setup.write_x_save = vt->setup.write_x;
	vt->setup.write_y_save = vt->setup.write_y;
}

void VT220SetupCursorRestore(VT220* vt)
{
	vt->setup.write_x = vt->setup.write_x_save;
	vt->setup.write_y = vt->setup.write_y_save;
}

void VT220SetupWriteString(VT220* vt, const char* s, const int sgr)
{
	for(; *s; s++) {
		VT220SetupWrite(vt, (unsigned char) *s, sgr, 0);
	}
}

void VT220SetupFill(VT220* vt, const u16 c, unsigned int count, const int sgr)
{
	for(; count; count--) {
		VT220SetupWrite(vt, c, sgr, 0);
	}
}

void VT220SetupWriteNumber(VT220* vt, int val, const int width, const int align, const int sgr)
{
	char buf[8];
	int i, j;
	for(i = 7; i > 0 && val > 0; i--) {
		buf[i] = val % 10;
		val /= 10;
	}
	i++;
	j = (7 - i);

	if(align) {
		for(; j < width - 1; j++) {
			VT220SetupWrite(vt, ' ', sgr, 0);
		}
	}

	for(; i < 8; i++) {
		VT220SetupWrite(vt, buf[i] + '0', sgr, 0);
	}

	for(; j < width; j++) {
		VT220SetupWrite(vt, ' ', sgr, 0);
	}
}

void VT220SetupShowTitle(VT220* vt)
{
	const char* title = VT220SetupGetTitle(vt);

	VT220SetupGoto(vt, 1, 1);
	VT220SetupEraseLine(vt);
	VT220SetupSetLineAttribute(vt, DECDWL);
	VT220SetupWriteString(vt, title, SGR_BOLD | SGR_BLINKING);

	VT220SetupGoto(vt, 1, 31);
	if(vt->mode & DECSCNM) {
		VT220SetupWriteString(vt, "VT220 V2.3", SGR_UNDERSCORE);
	} else {
		VT220SetupWriteString(vt, "VT220 V2.3", SGR_BOLD | SGR_UNDERSCORE);
	}
}

void VT220SetupShowStatus(VT220* vt)
{
	VT220SetupGoto(vt, 7, 1);
	VT220SetupFill(vt, '-', 132, 0);
	VT220SetupGoto(vt, 8, 5);
	VT220SetupEraseLine(vt);
	if(vt->setup.in_enq >= 0) {
		VT220SetupGoto(vt, 8, 15);
		VT220SetupWriteString(vt, "Enter Answerback=", 0);
		for(unsigned int i = 0; i < 30; i++) {
			if(vt->setup.enq[i]) {
				int bold = i == vt->setup.in_enq || (i == 29 && vt->setup.in_enq == 30);
				int sgr = bold ? (SGR_REVERSE | SGR_BOLD) : SGR_REVERSE;
				VT220SetupWrite(vt, (unsigned char) vt->setup.enq[i], sgr, 1);
			} else {
				for(; i < 30; i++) {
					int bold = i == vt->setup.in_enq || (i == 29 && vt->setup.in_enq == 30);
					int sgr = bold ? (SGR_REVERSE | SGR_BOLD) : SGR_REVERSE;
					VT220SetupWrite(vt, ' ', sgr, 1);
				}
				break;
			}
		}
	} else {
		if(vt->mode & IRM) {
			VT220SetupWriteString(vt, "Insert Mode", 0);
		} else {
			VT220SetupWriteString(vt, "Replace Mode", 0);
		}

		VT220SetupGoto(vt, 8, 28);
		VT220SetupWriteString(vt, "Printer: None", 0);
	}
}

void VT220SetupShowHint(VT220* vt)
{
	VT220SetupGoto(vt, 8, 2);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, "Press ENTER to take this action - Press Cursor Keys to move", 0);
	VT220Bell(vt);
}

void VT220SetupShowDone(VT220* vt)
{
	VT220SetupGoto(vt, 8, 37);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, "Done", 0);
}

static inline int VT220SetupGetSGR(VT220* vt, int x, int y, int cursor_x, int cursor_y)
{
	if(x == cursor_x && y == cursor_y) {
		if(vt->mode & DECSCNM) {
			return SGR_BOLD;
		} else {
			return SGR_REVERSE | SGR_BOLD;
		}
	} else {
		return SGR_REVERSE;
	}
}

#define GET_SGR(y, x)	VT220SetupGetSGR(vt, x, y, vt->setup.cursor_x, vt->setup.cursor_y)

const char* VT220SetupGetKeyboardLanguage(VT220* vt)
{
	return vt220_keyboard_languages[(int) vt->config.keyboard];
}

void VT220SetupShowDirectory(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 5) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 5;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 5) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 5;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 5;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 3) {
				vt->setup.cursor_x = 3;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 5;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " Display ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " General ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Comm ", GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Printer ", GET_SGR(0, 3));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Keyboard ", GET_SGR(0, 4));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Tab ", GET_SGR(0, 5));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->config.local) {
		VT220SetupWriteString(vt, " Local   ", GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " On Line ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Clear Display ", GET_SGR(1, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Clear Comm ", GET_SGR(1, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Reset Terminal ", GET_SGR(1, 3));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Recall ", GET_SGR(1, 4));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Save ", GET_SGR(1, 5));

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " Set-Up=English ", GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " ", GET_SGR(2, 1));
	VT220SetupCursorSave(vt);
	VT220SetupWriteString(vt, "                          ", GET_SGR(2, 1));
	VT220SetupCursorRestore(vt);
	VT220SetupWriteString(vt, VT220SetupGetKeyboardLanguage(vt), GET_SGR(2, 1));
	VT220SetupCursorRestore(vt);
	VT220SetupCursorRightN(vt, 27);
	VT220SetupWriteString(vt, " Default ", GET_SGR(2, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Exit ", GET_SGR(2, 3));
}

void VT220SetupShowDisplay(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 3) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 3;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 2) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 2;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 3;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 1) {
				vt->setup.cursor_x = 1;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 2;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(vt->mode & DECCOLM) {
		VT220SetupWriteString(vt, " 132 Columns ", GET_SGR(0, 2));
	} else {
		VT220SetupWriteString(vt, " 80 Columns  ", GET_SGR(0, 2));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.controls) {
		case VT220_CONTROLS_INTERPRET_CONTROLS:
			VT220SetupWriteString(vt, " Interpret Controls ", GET_SGR(0, 3));
			break;
		case VT220_CONTROLS_DISPLAY_CONTROLS:
			VT220SetupWriteString(vt, " Display Controls   ", GET_SGR(0, 3));
			break;
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECAWM) {
		VT220SetupWriteString(vt, " Auto Wrap    ", GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " No Auto Wrap ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECSCLM) {
		VT220SetupWriteString(vt, " Smooth Scroll ", GET_SGR(1, 1));
	} else {
		VT220SetupWriteString(vt, " Jump Scroll   ", GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECSCNM) {
		VT220SetupWriteString(vt, " Dark Text, Light Screen ", GET_SGR(1, 2));
	} else {
		VT220SetupWriteString(vt, " Light Text, Dark Screen ", GET_SGR(1, 2));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECTCEM) {
		VT220SetupWriteString(vt, " Text Cursor    ", GET_SGR(2, 0));
	} else {
		VT220SetupWriteString(vt, " No Text Cursor ", GET_SGR(2, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.cursor_style == VT220_CURSOR_STYLE_BLOCK_CURSOR) {
		VT220SetupWriteString(vt, " Block Cursor Style     ", GET_SGR(2, 1));
	} else {
		VT220SetupWriteString(vt, " Underline Cursor Style ", GET_SGR(2, 1));
	}
}

void VT220SetupShowGeneral(VT220* vt)
{
	if(vt->vt100_mode) {
		if(vt->setup.cursor_y == 0 && vt->setup.cursor_x > 3) {
			if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y = 1;
			} else {
				vt->setup.cursor_x = 3;
			}
		} else if(vt->setup.cursor_y > 0 && vt->setup.cursor_x > 2) {
			if(vt->setup.cursor_y < 2 && vt->setup.cursor_x > 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y++;
			} else {
				vt->setup.cursor_x = 2;
			}
		} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
			vt->setup.cursor_y--;
			if(vt->setup.cursor_y == 0) {
				vt->setup.cursor_x = 3;
			} else {
				vt->setup.cursor_x = 2;
			}
		}
	} else if(vt->setup.cursor_x > 2) {
		if(vt->setup.cursor_y < 2 && vt->setup.cursor_x > 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 2;
		}
	} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 2;
		vt->setup.cursor_y--;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(!(vt->mode & DECANM)) {
		VT220SetupWriteString(vt, " VT52 Mode                  ", GET_SGR(0, 2));
	} else if(vt->vt100_mode) {
		VT220SetupWriteString(vt, " VT100 Mode                 ", GET_SGR(0, 2));
	} else if(vt->ct_7bit) {
		VT220SetupWriteString(vt, " VT200 Mode, 7 Bit Controls ", GET_SGR(0, 2));
	} else {
		VT220SetupWriteString(vt, " VT200 Mode, 8 Bit Controls ", GET_SGR(0, 2));
	}

	if(vt->vt100_mode) {
		VT220SetupCursorRight(vt);
		switch(vt->config.vt100_terminal_id) {
			case VT220_VT100_TERMINAL_ID_VT220:
				VT220SetupWriteString(vt, " VT220 ID ", GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT100:
				VT220SetupWriteString(vt, " VT100 ID ", GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT101:
				VT220SetupWriteString(vt, " VT101 ID ", GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT102:
				VT220SetupWriteString(vt, " VT102 ID ", GET_SGR(0, 3));
				break;
		}
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->udk_locked) {
		VT220SetupWriteString(vt, " User Defined Keys Locked   ", GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " User Defined Keys Unlocked ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
		VT220SetupWriteString(vt, " User Features Unlocked ", GET_SGR(1, 1));
	} else {
		VT220SetupWriteString(vt, " User Features Locked   ", GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.character_set_mode == VT220_CHARACTER_SET_MODE_MULTINATIONAL) {
		VT220SetupWriteString(vt, " Multinational ", GET_SGR(1, 2));
	} else {
		VT220SetupWriteString(vt, " National      ", GET_SGR(1, 2));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & KAM) {
		VT220SetupWriteString(vt, " Application Keypad ", GET_SGR(2, 0));
	} else {
		VT220SetupWriteString(vt, " Numeric Keypad     ", GET_SGR(2, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECCKM) {
		VT220SetupWriteString(vt, " Application Cursor Keys ", GET_SGR(2, 1));
	} else {
		VT220SetupWriteString(vt, " Normal Cursor Keys      ", GET_SGR(2, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & LNM) {
		VT220SetupWriteString(vt, " New Line    ", GET_SGR(2, 2));
	} else {
		VT220SetupWriteString(vt, " No New Line ", GET_SGR(2, 2));
	}
}

void VT220SetupShowComm(VT220* vt)
{
	if(vt->setup.cursor_x > 3) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 3;
		}
	} else if(vt->setup.cursor_y > 0  && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 3;
		vt->setup.cursor_y--;
	}
	if(vt->setup.cursor_y == 2 && vt->setup.cursor_x > 2) {
		vt->setup.cursor_x = 2;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Transmit=", GET_SGR(0, 2));
	VT220SetupWriteNumber(vt, vt->config.tx_baud_rate, 6, 1, GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Receive=", GET_SGR(0, 3));
	if(vt->config.rx_baud_rate == 0) {
		VT220SetupWriteString(vt, "Transmit ", GET_SGR(0, 3));
	} else {
		VT220SetupWriteNumber(vt, vt->config.rx_baud_rate, 8, 1, GET_SGR(0, 3));
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->use_xoff) {
		VT220SetupWriteString(vt, " XOFF at ", GET_SGR(1, 0));
		VT220SetupWriteNumber(vt, vt->xoff_point, 4, 0, GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " No XOFF      ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.format) {
		default:
		case VT220_COMM_8BIT_NO_PARITY:
			VT220SetupWriteString(vt, " 8 Bits, No Parity             ", GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_EVEN_PARITY:
			VT220SetupWriteString(vt, " 8 Bits, Even Parity           ", GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_ODD_PARITY:
			VT220SetupWriteString(vt, " 8 Bits, Odd Parity            ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_NO_PARITY:
			VT220SetupWriteString(vt, " 7 Bits, No Parity             ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_EVEN_PARITY:
			VT220SetupWriteString(vt, " 7 Bits, Even Parity           ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_ODD_PARITY:
			VT220SetupWriteString(vt, " 7 Bits, Odd Parity            ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_MARK_PARITY:
			VT220SetupWriteString(vt, " 7 Bits, Mark Parity           ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_SPACE_PARITY:
			VT220SetupWriteString(vt, " 7 Bits, Space Parity          ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_EVEN_PARITY_NO_CHECK:
			VT220SetupWriteString(vt, " 7 Bits, Even Parity, No Check ", GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_ODD_PARITY_NO_CHECK:
			VT220SetupWriteString(vt, " 7 Bits, Odd Parity, No Check  ", GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_EVEN_PARITY_NO_CHECK:
			VT220SetupWriteString(vt, " 8 Bits, Even Parity, No Check ", GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_ODD_PARITY_NO_CHECK:
			VT220SetupWriteString(vt, " 8 Bits, Odd Parity, No Check  ", GET_SGR(1, 1));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->config.stop_bits == VT220_COMM_1_STOP_BIT) {
		VT220SetupWriteString(vt, " 1 Stop Bit  ", GET_SGR(1, 2));
	} else {
		VT220SetupWriteString(vt, " 2 Stop Bits ", GET_SGR(1, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & SRM) {
		VT220SetupWriteString(vt, " No Local Echo ", GET_SGR(1, 3));
	} else {
		VT220SetupWriteString(vt, " Local Echo    ", GET_SGR(1, 3));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	switch(vt->config.port) {
		default:
		case VT220_COMM_EIA_PORT_DATA_LEADS_ONLY:
			VT220SetupWriteString(vt, " EIA Port, Data Leads Only ", GET_SGR(2, 0));
			break;
		case VT220_COMM_EIA_PORT_MODEM_CONTROL:
			VT220SetupWriteString(vt, " EIA Port, Modem Control   ", GET_SGR(2, 0));
			break;
		case VT220_COMM_20MA_PORT:
			VT220SetupWriteString(vt, " 20 mA Port                ", GET_SGR(2, 0));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->config.delay == VT220_COMM_2S_DELAY) {
		VT220SetupWriteString(vt, " Disconnect, 2 s Delay   ", GET_SGR(2, 1));
	} else {
		VT220SetupWriteString(vt, " Disconnect, 60 ms Delay ", GET_SGR(2, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.transmit == VT220_TRANSMIT_LIMITED) {
		VT220SetupWriteString(vt, " Limited Transmit   ", GET_SGR(2, 2));
	} else {
		VT220SetupWriteString(vt, " Unlimited Transmit ", GET_SGR(2, 2));
	}
}

void VT220SetupShowPrinter(VT220* vt)
{
	if(vt->setup.cursor_x > 3) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 3;
		}
	} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 3;
		vt->setup.cursor_y--;
	}
	if((vt->setup.cursor_y == 0 || vt->setup.cursor_y == 2) && vt->setup.cursor_x > 2) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y = 1;
		} else {
			vt->setup.cursor_x = 2;
		}
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Speed=4800  ", GET_SGR(0, 2));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " Normal Print Mode ", GET_SGR(1, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " XOFF    ", GET_SGR(1, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " 7 Bits, Space Parity ", GET_SGR(1, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " 1 Stop Bit  ", GET_SGR(1, 3));

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " Print Full Page     ", GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Print National Only       ", GET_SGR(2, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " No Terminator   ", GET_SGR(2, 2));
}

void VT220SetupShowKeyboard(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 3) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 3;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 4) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 4;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 3;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 2) {
				vt->setup.cursor_x = 2;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 4;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(vt->config.keys == VT220_KEYS_TYPEWRITER) {
		VT220SetupWriteString(vt, " Typewriter Keys      ", GET_SGR(0, 2));
	} else {
		VT220SetupWriteString(vt, " Data Processing Keys ", GET_SGR(0, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.lock == VT220_LOCK_CAPS_LOCK) {
		VT220SetupWriteString(vt, " Caps Lock  ", GET_SGR(0, 3));
	} else {
		VT220SetupWriteString(vt, " Shift Lock ", GET_SGR(0, 3));
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECARM) {
		VT220SetupWriteString(vt, " Auto Repeat    ", GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " No Auto Repeat ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.keyclick == VT220_KEYCLICK) {
		VT220SetupWriteString(vt, " Keyclick    ", GET_SGR(1, 1));
	} else {
		VT220SetupWriteString(vt, " No Keyclick ", GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.margin_bell == VT220_MARGIN_BELL) {
		VT220SetupWriteString(vt, " Margin Bell    ", GET_SGR(1, 2));
	} else {
		VT220SetupWriteString(vt, " No Margin Bell ", GET_SGR(1, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.bell == VT220_BELL) {
		VT220SetupWriteString(vt, " Warning Bell    ", GET_SGR(1, 3));
	} else {
		VT220SetupWriteString(vt, " No Warning Bell ", GET_SGR(1, 3));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.brk == VT220_BREAK) {
		VT220SetupWriteString(vt, " Break    ", GET_SGR(1, 4));
	} else {
		VT220SetupWriteString(vt, " No Break ", GET_SGR(1, 4));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " No Auto Answerback ", GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
		VT220SetupWriteString(vt, " Answerback=<Concealed>                   ", GET_SGR(2, 1));
	} else {
		VT220SetupWriteString(vt, " Answerback=", GET_SGR(2, 1));
		for(unsigned int i = 0; i < 30; i++) {
			if(vt->answerback[i]) {
				VT220SetupWrite(vt, (unsigned char) vt->answerback[i], GET_SGR(2, 1), 1);
			} else {
				for(; i < 30; i++) {
					VT220SetupWrite(vt, ' ', GET_SGR(2, 1), 1);
				}
				break;
			}
		}
	}
	VT220SetupWriteString(vt, " ", GET_SGR(2, 1));
	VT220SetupCursorRight(vt);
	if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
		VT220SetupWriteString(vt, " Concealed     ", GET_SGR(2, 2));
	} else {
		VT220SetupWriteString(vt, " Not Concealed ", GET_SGR(2, 2));
	}
}

void VT220SetupShowTab(VT220* vt)
{
	int i;

	if(vt->setup.cursor_y == 0) {
		if(vt->setup.move == VT220_SETUP_MOVE_UP) {
			vt->setup.cursor_x = 0;
		} else if(vt->setup.cursor_x > 3) {
			if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y = 1;
			} else {
				vt->setup.cursor_x = 3;
			}
		}
	} else if(vt->setup.move != VT220_SETUP_MOVE_LEFT_MARGIN) {
		if(vt->setup.move == VT220_SETUP_MOVE_DOWN) {
			if(vt->setup.cursor_y == 1) {
				vt->setup.cursor_x = 0;
			} else {
				vt->setup.cursor_y = 1;
			}
		} else if(vt->setup.move == VT220_SETUP_MOVE_UP) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y = 0;
		} else {
			vt->setup.cursor_y = 1;
			if(vt->setup.cursor_x >= vt->columns) {
				vt->setup.cursor_x = vt->columns - 1;
			}
		}
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Clear All Tabs ", GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Set 8 Column Tabs ", GET_SGR(0, 3));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	for(i = 0; i < vt->columns; i++) {
		VT220SetupGoto(vt, 4, i + 1);
		if(i > 0 && vt->tabstops[i - 1]) {
			VT220SetupWriteString(vt, "T", GET_SGR(1, i));
		} else {
			VT220SetupWriteString(vt, " ", GET_SGR(1, i));
		}
		VT220SetupGoto(vt, 5, i + 1);
		VT220SetupWrite(vt, '0' + (i + 1) % 10, ((i / 10) % 2) ? SGR_REVERSE : 0, 0);
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
}

void VT220SetupShowScreen(VT220* vt)
{
	if(vt->setup.cursor_y < 0) {
		vt->setup.cursor_y = 0;
	}
	if(vt->setup.cursor_y > 2) {
		vt->setup.cursor_y = 2;
	}

	VT220SetupShowTitle(vt);

	/* clear extra stuff from tab setup screen */
	VT220SetupGoto(vt, 5, 1);
	VT220SetupEraseLine(vt);

	switch(vt->setup.screen) {
		case SETUP_SCREEN_DIRECTORY:
			VT220SetupShowDirectory(vt);
			break;
		case SETUP_SCREEN_DISPLAY:
			VT220SetupShowDisplay(vt);
			break;
		case SETUP_SCREEN_GENERAL:
			VT220SetupShowGeneral(vt);
			break;
		case SETUP_SCREEN_COMM:
			VT220SetupShowComm(vt);
			break;
		case SETUP_SCREEN_PRINTER:
			VT220SetupShowPrinter(vt);
			break;
		case SETUP_SCREEN_KEYBOARD:
			VT220SetupShowKeyboard(vt);
			break;
		case SETUP_SCREEN_TAB:
			VT220SetupShowTab(vt);
			break;
	}

	vt->setup.move = VT220_SETUP_MOVE_NONE;
}

void VT220SetupShow(VT220* vt)
{
	vt->setup.move = VT220_SETUP_MOVE_NONE;
	VT220SetupEraseDisplay(vt);
	VT220SetupShowScreen(vt);
	VT220SetupShowStatus(vt);
}

void VT220SetupSetScreen(VT220* vt, int screen)
{
	vt->setup.screen = screen;
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	VT220SetupShowScreen(vt);
}

void VT220SetupNextScreen(VT220* vt)
{
	vt->setup.screen++;
	vt->setup.screen %= SETUP_SCREEN_COUNT;
	if(vt->setup.screen == 0) {
		vt->setup.screen++;
	}
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	VT220SetupShowScreen(vt);
}

void VT220EnterSetup(VT220* vt)
{
	vt->in_setup = 1;
	vt->setup.in_enq = -1;
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	vt->setup.screen = SETUP_SCREEN_DIRECTORY;
	VT220FlowControl(vt, 0);

	VT220SetupShow(vt);
}

void VT220LeaveSetup(VT220* vt)
{
	vt->in_setup = 0;
	if(!vt->hold_screen) {
		VT220FlowControl(vt, 1);
	}
}

void VT220SetupDirectoryEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0: /* Display */
					VT220SetupSetScreen(vt, SETUP_SCREEN_DISPLAY);
					break;
				case 1: /* General */
					VT220SetupSetScreen(vt, SETUP_SCREEN_GENERAL);
					break;
				case 2: /* Comm */
					VT220SetupSetScreen(vt, SETUP_SCREEN_COMM);
					break;
				case 3: /* Printer */
					VT220SetupSetScreen(vt, SETUP_SCREEN_PRINTER);
					break;
				case 4: /* Keyboard */
					VT220SetupSetScreen(vt, SETUP_SCREEN_KEYBOARD);
					break;
				case 5: /* Tab */
					VT220SetupSetScreen(vt, SETUP_SCREEN_TAB);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0: /* On Line/Local */
					vt->config.local = !vt->config.local;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* Clear Display */
					VT220EraseInDisplay(vt, 2);
					VT220SetCursor(vt, 1, 1);
					VT220SetupShowDone(vt);
					break;
				case 2:
					/* Clear Comm */
					VT220ClearComm(vt);
					VT220SetupShowDone(vt);
					break;
				case 3: /* Reset */
					VT220SoftReset(vt);
					VT220SetupShowDone(vt);
					break;
				case 4: /* Recall */
					VT220HardReset(vt);
					VT220SetupShow(vt);
					VT220SetupShowDone(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 1: /* Keyboard language */
					vt->config.keyboard = (vt->config.keyboard + 1) % VT220_KEYBOARD_COUNT;
					VT220SetupShow(vt);
					break;
				case 3: /* Exit */
					VT220LeaveSetup(vt);
					break;
			}
	}
}

void VT220SetupDisplayEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(vt->mode & DECCOLM) {
						VT220ClearColumnMode(vt);
					} else {
						VT220SetColumnMode(vt);
					}
					VT220SetupShow(vt);
					break;
				case 3:
					switch(vt->config.controls) {
						case VT220_CONTROLS_INTERPRET_CONTROLS:
							vt->config.controls = VT220_CONTROLS_DISPLAY_CONTROLS;
							break;
						case VT220_CONTROLS_DISPLAY_CONTROLS:
							vt->config.controls = VT220_CONTROLS_INTERPRET_CONTROLS;
							break;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0: /* auto wrap mode */
					vt->mode ^= DECAWM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* scroll mode */
					vt->mode ^= DECSCLM;
					VT220SetupShowScreen(vt);
					break;
				case 2: /* inverse display */
					vt->mode ^= DECSCNM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0: /* text cursor */
					vt->mode ^= DECTCEM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* cursor style */
					if(vt->config.cursor_style == VT220_CURSOR_STYLE_BLOCK_CURSOR) {
						vt->config.cursor_style = VT220_CURSOR_STYLE_UNDERLINE_CURSOR;
					} else {
						vt->config.cursor_style = VT220_CURSOR_STYLE_BLOCK_CURSOR;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
	}
}

void VT220SetupGeneralEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(!(vt->mode & DECANM)) {
						/* was VT52, go to VT100 */
						vt->mode |= DECANM;
						vt->vt100_mode = 1;
						vt->ct_7bit = 1;
					} else if(vt->vt100_mode) {
						/* was VT100, go to (tek) VT200, 7bit */
						vt->mode |= DECANM;
						vt->ct_7bit = 1;
						vt->vt100_mode = 0;
					} else if(vt->ct_7bit) {
						vt->ct_7bit = 0;
					} else {
						vt->mode &= ~DECANM;
					}
					break;
				case 3:
					switch(vt->config.vt100_terminal_id) {
						case VT220_VT100_TERMINAL_ID_VT220:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT100;
							break;
						case VT220_VT100_TERMINAL_ID_VT100:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT101;
							break;
						case VT220_VT100_TERMINAL_ID_VT101:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT102;
							break;
						case VT220_VT100_TERMINAL_ID_VT102:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT220;
							break;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					vt->udk_locked = !vt->udk_locked;
					VT220SetupShowScreen(vt);
					break;
				case 1:
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						vt->config.user_features = VT220_USER_FEATURES_LOCKED;
					} else {
						vt->config.user_features = VT220_USER_FEATURES_UNLOCKED;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0: /* Keypad */
					vt->mode ^= KAM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* Cursor keys */
					vt->mode ^= DECCKM;
					VT220SetupShowScreen(vt);
					break;
				case 2: /* New line */
					vt->mode ^= LNM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
	}
}

void VT220SetupCommEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					vt->config.tx_baud_rate = VT220GetNextBaudRate(vt->config.tx_baud_rate, 0);
					break;
				case 3:
					vt->config.rx_baud_rate = VT220GetNextBaudRate(vt->config.rx_baud_rate, 1);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					switch(vt->xoff_point) {
						case 64:
							vt->xoff_point = 128;
							break;
						case 128:
							vt->use_xoff = 0;
							vt->xoff_point = 0;
							break;
						case 0:
							vt->use_xoff = 1;
							vt->xoff_point = 64;
							break;
					}
					VT220SetupShowScreen(vt);
					break;
				case 1:
					vt->config.format = (vt->config.format + 1) % 12;
					break;
				case 2:
					if(vt->config.stop_bits == VT220_COMM_1_STOP_BIT) {
						vt->config.stop_bits = VT220_COMM_2_STOP_BITS;
					} else {
						vt->config.stop_bits = VT220_COMM_1_STOP_BIT;
					}
					break;
				case 3:
					vt->mode ^= SRM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0:
					switch(vt->config.port) {
						case VT220_COMM_EIA_PORT_DATA_LEADS_ONLY:
							vt->config.port = VT220_COMM_EIA_PORT_MODEM_CONTROL;
							break;
						case VT220_COMM_EIA_PORT_MODEM_CONTROL:
							vt->config.port = VT220_COMM_20MA_PORT;
							break;
						default:
						case VT220_COMM_20MA_PORT:
							vt->config.port = VT220_COMM_EIA_PORT_DATA_LEADS_ONLY;
							break;
					}
					break;
				case 1:
					if(vt->config.delay == VT220_COMM_2S_DELAY) {
						vt->config.delay = VT220_COMM_60MS_DELAY;
					} else {
						vt->config.delay = VT220_COMM_2S_DELAY;
					}
					break;
				case 2:
					if(vt->config.transmit == VT220_TRANSMIT_LIMITED) {
						vt->config.transmit = VT220_TRANSMIT_UNLIMITED;
					} else {
						vt->config.transmit = VT220_TRANSMIT_LIMITED;
					}
					break;
			}
			break;
	}
}

void VT220SetupPrinterEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
			}
			break;
	}
}

void VT220SetupKeyboardEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(vt->config.keys == VT220_KEYS_TYPEWRITER) {
						vt->config.keys = VT220_KEYS_DATA_PROCESSING;
					} else {
						vt->config.keys = VT220_KEYS_TYPEWRITER;
					}
					break;
				case 3:
					if(vt->config.lock == VT220_LOCK_CAPS_LOCK) {
						vt->config.lock = VT220_LOCK_SHIFT_LOCK;
					} else {
						vt->config.lock = VT220_LOCK_CAPS_LOCK;
					}
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					vt->mode ^= DECARM;
					VT220SetupShowScreen(vt);
					break;
				case 1:
					if(vt->config.keyclick == VT220_KEYCLICK) {
						vt->config.keyclick = VT220_NO_KEYCLICK;
					} else {
						vt->config.keyclick = VT220_KEYCLICK;
					}
					break;
				case 2:
					if(vt->config.margin_bell == VT220_MARGIN_BELL) {
						vt->config.margin_bell = VT220_NO_MARGIN_BELL;
					} else {
						vt->config.margin_bell = VT220_MARGIN_BELL;
					}
					break;
				case 3:
					if(vt->config.bell == VT220_BELL) {
						vt->config.bell = VT220_NO_BELL;
					} else {
						vt->config.bell = VT220_BELL;
					}
					VT220Bell(vt);
					break;
				case 4:
					if(vt->config.brk == VT220_BREAK) {
						vt->config.brk = VT220_NO_BREAK;
					} else {
						vt->config.brk = VT220_BREAK;
					}
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 1:
					if(vt->setup.in_enq >= 0) {
						/* commit */
						memcpy(vt->answerback, vt->setup.enq, 30);
						vt->setup.in_enq = -1;
						vt->config.concealed = VT220_ANSWERBACK_NOT_CONCEALED;
					} else {
						if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
							memset(vt->setup.enq, 0, 30);
						} else {
							memcpy(vt->setup.enq, vt->answerback, 30);
						}

						/* set edit cursor to end of answerback message */
						for(vt->setup.in_enq = 0; vt->setup.in_enq < 30; vt->setup.in_enq++) {
							if(!vt->setup.enq[vt->setup.in_enq]) {
								break;
							}
						}
					}
					VT220SetupShowStatus(vt);
					break;
				case 2:
					vt->config.concealed = VT220_ANSWERBACK_CONCEALED;
					break;
			}
			break;
	}
}

void VT220SetupTabEnter(VT220* vt)
{
	unsigned int i;

	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					VT220ClearAllTabstops(vt);
					VT220SetupShowScreen(vt);
					break;
				case 3:
					for(i = 0; i < vt->columns; i++) {
						vt->tabstops[i] = i % 8 == 7;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 0) {
				vt->tabstops[vt->setup.cursor_x - 1] = !vt->tabstops[vt->setup.cursor_x - 1];
			}
			VT220SetupShowScreen(vt);
			break;
	}
}

void VT220SetupProcessEnter(VT220* vt)
{
	switch(vt->setup.screen) {
		case SETUP_SCREEN_DIRECTORY:
			VT220SetupDirectoryEnter(vt);
			break;
		case SETUP_SCREEN_DISPLAY:
			VT220SetupDisplayEnter(vt);
			break;
		case SETUP_SCREEN_GENERAL:
			VT220SetupGeneralEnter(vt);
			break;
		case SETUP_SCREEN_COMM:
			VT220SetupCommEnter(vt);
			break;
		case SETUP_SCREEN_PRINTER:
			VT220SetupPrinterEnter(vt);
			break;
		case SETUP_SCREEN_KEYBOARD:
			VT220SetupKeyboardEnter(vt);
			break;
		case SETUP_SCREEN_TAB:
			VT220SetupTabEnter(vt);
			break;
	}
}

unsigned int VT220SetupEncodeAnswerback(u16 key, unsigned char* buf)
{
	switch(key) {
		case VT220_KEY_FIND:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '~';
			return 3;
		case VT220_KEY_INSERT:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '~';
			return 3;
		case VT220_KEY_REMOVE:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '~';
			return 3;
		case VT220_KEY_SELECT:
			buf[0] = CSI;
			buf[1] = '4';
			buf[2] = '~';
			return 3;
		case VT220_KEY_PREV_SCREEN:
			buf[0] = CSI;
			buf[1] = '5';
			buf[2] = '~';
			return 3;
		case VT220_KEY_NEXT_SCREEN:
			buf[0] = CSI;
			buf[1] = '6';
			buf[2] = '~';
			return 3;
		case VT220_KEY_F6:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '7';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F7:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '8';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F8:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '9';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F9:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '0';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F10:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '1';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F11:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '3';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F12:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '4';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F13:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '5';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F14:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '6';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F15:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '8';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F16:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '9';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F17:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '1';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F18:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '2';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F19:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '3';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F20:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '4';
			buf[3] = '~';
			return 4;
		case VT220_KEY_KP_0:
			*buf = '0';
			return 1;
		case VT220_KEY_KP_1:
			*buf = '1';
			return 1;
		case VT220_KEY_KP_2:
			*buf = '2';
			return 1;
		case VT220_KEY_KP_3:
			*buf = '3';
			return 1;
		case VT220_KEY_KP_4:
			*buf = '4';
			return 1;
		case VT220_KEY_KP_5:
			*buf = '5';
			return 1;
		case VT220_KEY_KP_6:
			*buf = '6';
			return 1;
		case VT220_KEY_KP_7:
			*buf = '7';
			return 1;
		case VT220_KEY_KP_8:
			*buf = '8';
			return 1;
		case VT220_KEY_KP_9:
			*buf = '9';
			return 1;
		case VT220_KEY_KP_MINUS:
			*buf = '-';
			return 1;
		case VT220_KEY_KP_COMMA:
			*buf = ',';
			return 1;
		case VT220_KEY_KP_PERIOD:
			*buf = '.';
			return 1;
		case VT220_KEY_KP_PF1:
			buf[0] = SS3;
			buf[1] = 'P';
			return 2;
		case VT220_KEY_KP_PF2:
			buf[0] = SS3;
			buf[1] = 'Q';
			return 2;
		case VT220_KEY_KP_PF3:
			buf[0] = SS3;
			buf[1] = 'R';
			return 2;
		case VT220_KEY_KP_PF4:
			buf[0] = SS3;
			buf[1] = 'S';
			return 2;
	}
	return 0;
}

void VT220SetupProcessAnswerback(VT220* vt, u16 key)
{
	if(key == DEL) {
		if(vt->setup.in_enq > 0) {
			vt->setup.in_enq--;
			vt->setup.enq[vt->setup.in_enq] = 0;
		} else {
			/* DEL in empty answerback = exit the field */
			vt->setup.in_enq = -1;
		}
	} else {
		if(vt->setup.in_enq <= 30) {
			unsigned int cursor = vt->setup.in_enq;
			if(cursor >= 30) {
				cursor = 29;
			}

			/* The real VT220 encodes special keys as 8-bit control
			 * sequences using CSI (9B) or SS3 (8F). If the encoded
			 * sequence does not fit into the answerback string
			 * anymore, the error bell sounds and the end of the
			 * sequence is truncated. */

			if(key > 0xFF) {
				unsigned char buf[4]; /* the longest sequence is 4 characters */
				unsigned int len = VT220SetupEncodeAnswerback(key, buf);
				if(vt->setup.in_enq + len > 30) {
					if(vt->setup.in_enq == 30) {
						/* only copy the first character */
						vt->setup.enq[29] = buf[0];
					} else {
						/* truncate string */
						len = 30 - vt->setup.in_enq;
						memcpy(&vt->setup.enq[vt->setup.in_enq], buf, len);
						vt->setup.in_enq = 30;
					}
					VT220Bell(vt);
				} else {
					memcpy(&vt->setup.enq[vt->setup.in_enq], buf, len);
					vt->setup.in_enq += len;
				}
			} else {
				vt->setup.enq[cursor] = (unsigned char) key;
				vt->setup.in_enq++;
				if(vt->setup.in_enq > 30) {
					vt->setup.in_enq = 30;
				}
			}
		}
	}

	VT220SetupShowStatus(vt);
}

void VT220SetupProcessKey(VT220* vt, u16 key)
{
	vt->setup.move = VT220_SETUP_MOVE_NONE;
	switch(key) {
		case VT220_KEY_KP_ENTER:
			VT220SetupProcessEnter(vt);
			break;
		case VT220_KEY_UP:
			if(vt->setup.cursor_y > 0) {
				vt->setup.move = VT220_SETUP_MOVE_UP;
				vt->setup.cursor_y--;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_DOWN:
			if(vt->setup.cursor_y < 2) {
				vt->setup.move = VT220_SETUP_MOVE_DOWN;
				vt->setup.cursor_y++;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_RIGHT:
			if(vt->setup.cursor_x < vt->columns) {
				vt->setup.move = VT220_SETUP_MOVE_RIGHT;
				vt->setup.cursor_x++;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_LEFT:
			if(vt->setup.cursor_x > 0) {
				vt->setup.move = VT220_SETUP_MOVE_LEFT;
				vt->setup.cursor_x--;
			} else {
				vt->setup.move = VT220_SETUP_MOVE_LEFT_MARGIN;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		default:
			if(vt->setup.in_enq >= 0) {
				VT220SetupProcessAnswerback(vt, key);
			} else if(key == CR) {
				/* On the real VT220, ENTER (keypad) is not the
				 * same as Return (CR). Only ENTER is accepted
				 * in Setup. However, not every PC keyboard has
				 * a numeric keypad, therefore it is extremely
				 * convenient to have Return = ENTER here.
				 * This is NOT true for the Answerback field:
				 * that field treats ENTER and Return as
				 * separate keys, just like the real VT220. */
				VT220SetupProcessEnter(vt);
			} else {
				VT220SetupShowHint(vt);
			}
			break;
	}

	VT220SetupShowScreen(vt);
}
