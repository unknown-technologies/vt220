#include <string.h>

#include "types.h"
#include "vt.h"

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

static const char* vt220_keyboard_languages[15] = {
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

const char* VT220SetupGetTitle(VT220* vt)
{
	if(vt->setup.screen < 0 || vt->setup.screen > 9) {
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

void VT220SetupWrite(VT220* vt, const unsigned char c, const int sgr)
{
	int idx = vt->setup.write_y * vt->columns + vt->setup.write_x;
	vt->setup.text[idx].text = c == ' ' ? 0 : c;
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
		VT220SetupWrite(vt, *s, sgr);
	}
}

void VT220SetupWriteNumber(VT220* vt, int val, const int width, const int sgr)
{
	char buf[8];
	int i, j;
	for(i = 7; i > 0 && val > 0; i--) {
		buf[i] = val % 10;
		val /= 10;
	}
	i++;
	j = (7 - i);
	for(; i < 8; i++) {
		VT220SetupWrite(vt, buf[i] + '0', sgr);
	}
	for(; j < width; j++) {
		VT220SetupWrite(vt, ' ', sgr);
	}
}

void VT220SetupShowTitle(VT220* vt)
{
	const char* title = VT220SetupGetTitle(vt);

	VT220SetupGoto(vt, 1, 1);
	VT220SetupEraseLine(vt);
	VT220SetupSetLineAttribute(vt, DECDWL);
	VT220SetupWriteString(vt, title, SGR_BLINKING);

	VT220SetupGoto(vt, 1, 31);
	if(vt->mode & DECSCNM) {
		VT220SetupWriteString(vt, "VT220 V2.0", SGR_UNDERSCORE);
	} else {
		VT220SetupWriteString(vt, "VT220 V2.0", SGR_BOLD | SGR_UNDERSCORE);
	}
}

void VT220SetupShowStatus(VT220* vt)
{
	VT220SetupGoto(vt, 7, 1);
	VT220SetupWriteString(vt, "________________________________________"
				  "________________________________________", 0);
	VT220SetupGoto(vt, 8, 2);
	VT220SetupEraseLine(vt);
	if(vt->mode & IRM) {
		VT220SetupWriteString(vt, "Insert Mode", 0);
	} else {
		VT220SetupWriteString(vt, "Replace Mode", 0);
	}

	VT220SetupGoto(vt, 8, 18);
	VT220SetupWriteString(vt, "Printer: None", 0);
}

void VT220SetupShowHint(VT220* vt)
{
	VT220SetupGoto(vt, 8, 2);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, "Press ENTER to take this action - Press Cursor Keys to move", 0);
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
				vt->setup.cursor_x = 5;
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 5) {
				vt->setup.cursor_x = 5;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 3) {
				vt->setup.cursor_x = 3;
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
				vt->setup.cursor_x = 3;
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 2) {
				vt->setup.cursor_x = 2;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 1) {
				vt->setup.cursor_x = 1;
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
	if(vt->setup.cursor_x > 2) {
		vt->setup.cursor_x = 2;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " To Next Set-Up ", GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " To Directory ", GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	/* TODO: add VT100 mode + identifier */
	if(!(vt->mode & DECANM)) {
		VT220SetupWriteString(vt, " VT52 Mode                  ", GET_SGR(0, 2));
	} else if(vt->ct_7bit) {
		VT220SetupWriteString(vt, " VT200 Mode, 7 Bit Controls ", GET_SGR(0, 2));
	} else {
		VT220SetupWriteString(vt, " VT200 Mode, 8 Bit Controls ", GET_SGR(0, 2));
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->config.user_defined_keys == VT220_USER_DEFINED_KEYS_UNLOCKED) {
		VT220SetupWriteString(vt, " User Defined Keys Unlocked ", GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " User Defined Keys Locked   ", GET_SGR(1, 0));
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
		vt->setup.cursor_x = 3;
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
	VT220SetupWriteNumber(vt, 9600, 5, GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Receive=Transmit ", GET_SGR(0, 3));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->use_xoff) {
		VT220SetupWriteString(vt, " XOFF at ", GET_SGR(1, 0));
		VT220SetupWriteNumber(vt, vt->xoff_point, 4, GET_SGR(1, 0));
	} else {
		VT220SetupWriteString(vt, " No XOFF      ", GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " 8 Bits, No Parity             ", GET_SGR(1, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " 1 Stop Bit  ", GET_SGR(1, 2));
	VT220SetupCursorRight(vt);
	if(vt->mode & SRM) {
		VT220SetupWriteString(vt, " No Local Echo ", GET_SGR(1, 3));
	} else {
		VT220SetupWriteString(vt, " Local Echo    ", GET_SGR(1, 3));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " EIA Port, Data Leads Only ", GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Disconnect, 2 s Delay   ", GET_SGR(2, 1));
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
		vt->setup.cursor_x = 3;
	}
	if((vt->setup.cursor_y == 0 || vt->setup.cursor_y == 2) && vt->setup.cursor_x > 2) {
		vt->setup.cursor_x = 2;
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
				vt->setup.cursor_x = 3;
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 4) {
				vt->setup.cursor_x = 4;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 2) {
				vt->setup.cursor_x = 2;
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
	VT220SetupWriteString(vt, " No Margin Bell ", GET_SGR(1, 2));
	VT220SetupCursorRight(vt);
	if(vt->config.bell == VT220_BELL) {
		VT220SetupWriteString(vt, " Warning Bell    ", GET_SGR(1, 3));
	} else {
		VT220SetupWriteString(vt, " No Warning Bell ", GET_SGR(1, 3));
	}
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Break    ", GET_SGR(1, 4));

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteString(vt, " No Auto Answerback ", GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Answerback=                               ", GET_SGR(2, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteString(vt, " Not Concealed ", GET_SGR(2, 2));
}

void VT220SetupShowTab(VT220* vt)
{
	int i;

	if(vt->setup.cursor_y == 0) {
		if(vt->setup.cursor_x > 3) {
			vt->setup.cursor_x = 3;
		}
	} else {
		vt->setup.cursor_y = 1;
		if(vt->setup.cursor_x >= vt->columns) {
			vt->setup.cursor_x = vt->columns - 1;
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
		VT220SetupWrite(vt, '0' + (i + 1) % 10, ((i / 10) % 2) ? SGR_REVERSE : 0);
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
}

void VT220SetupShow(VT220* vt)
{
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
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	vt->setup.screen = SETUP_SCREEN_DIRECTORY;
	vt->setup.state = 0;
	VT220Send(vt, DC3);

	VT220SetupShow(vt);
}

void VT220LeaveSetup(VT220* vt)
{
	vt->in_setup = 0;
	VT220Send(vt, DC1);
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
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
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
					VT220SetupEraseDisplay(vt);
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
						vt->mode |= DECANM;
						vt->ct_7bit = 1;
					} else if(vt->ct_7bit) {
						vt->ct_7bit = 0;
					} else {
						vt->mode &= ~DECANM;
					}
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
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					switch(vt->xoff_point) {
						case 64:
							vt->xoff_point = 256;
							break;
						case 256:
							vt->xoff_point = 512;
							break;
						case 512:
							vt->xoff_point = 1024;
							break;
						case 1024:
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
				case 3:
					vt->mode ^= SRM;
					VT220SetupShowScreen(vt);
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
				case 3:
					if(vt->config.bell == VT220_BELL) {
						vt->config.bell = VT220_NO_BELL;
					} else {
						vt->config.bell = VT220_BELL;
					}
					VT220Bell(vt);
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

#define	STATE_TEXT	0
#define	STATE_ESC	1
#define	STATE_CSI	2
#define	STATE_SS3	3

void VT220SetupProcessKey(VT220* vt, unsigned char c)
{
	switch(vt->setup.state) {
		case STATE_TEXT:
			switch(c) {
				case CR:
					VT220SetupProcessEnter(vt);
					break;
				case LF:
					break;
				case ESC:
					vt->setup.state = STATE_ESC;
					break;
				case CSI:
					vt->setup.state = STATE_CSI;
					break;
				case SS3:
					vt->setup.state = STATE_SS3;
					break;
				default:
					VT220SetupShowHint(vt);
					break;
			}
			break;
		case STATE_ESC:
			switch(c) {
				case ESC:
					vt->setup.state = STATE_ESC;
					break;
				case CSI:
					vt->setup.state = STATE_CSI;
					break;
				case '[':
					vt->setup.state = STATE_CSI;
					break;
				case 'A': /* cursor up (VT52) */
					if(!(vt->mode & DECANM)) {
						vt->setup.state = STATE_TEXT;
						if(vt->setup.cursor_y > 0) {
							vt->setup.cursor_y--;
						}
						VT220SetupShowStatus(vt);
					} else {
						vt->setup.state = STATE_TEXT;
						VT220SetupShowHint(vt);
					}
					break;
				case 'B': /* cursor down (VT52) */
					if(!(vt->mode & DECANM)) {
						vt->setup.state = STATE_TEXT;
						if(vt->setup.cursor_y < 2) {
							vt->setup.cursor_y++;
						}
						VT220SetupShowStatus(vt);
					} else {
						vt->setup.state = STATE_TEXT;
						VT220SetupShowHint(vt);
					}
					break;
				case 'C': /* cursor right (VT52) */
					if(!(vt->mode & DECANM)) {
						vt->setup.state = STATE_TEXT;
						if(vt->setup.cursor_x < vt->columns) {
							vt->setup.cursor_x++;
						}
						VT220SetupShowStatus(vt);
					} else {
						vt->setup.state = STATE_TEXT;
						VT220SetupShowHint(vt);
					}
					break;
				case 'D': /* cursor left (VT52) */
					if(!(vt->mode & DECANM)) {
						vt->setup.state = STATE_TEXT;
						if(vt->setup.cursor_x > 0) {
							vt->setup.cursor_x--;
						}
						VT220SetupShowStatus(vt);
					} else {
						vt->setup.state = STATE_TEXT;
						VT220SetupShowHint(vt);
					}
					break;
				default:
					vt->setup.state = STATE_TEXT;
					VT220SetupShowHint(vt);
					break;
			}
			break;
		case STATE_CSI:
			switch(c) {
				case ESC:
					vt->setup.state = STATE_ESC;
					break;
				case CSI:
					vt->setup.state = STATE_CSI;
					break;
				case 'A': /* cursor up */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_y > 0) {
						vt->setup.cursor_y--;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'B': /* cursor down */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_y < 2) {
						vt->setup.cursor_y++;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'C': /* cursor right */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_x < vt->columns) {
						vt->setup.cursor_x++;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'D': /* cursor left */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_x > 0) {
						vt->setup.cursor_x--;
					}
					VT220SetupShowStatus(vt);
					break;
				default:
					vt->setup.state = STATE_TEXT;
					VT220SetupShowHint(vt);
					break;
			}
			break;
		case STATE_SS3:
			switch(c) {
				case ESC:
					vt->setup.state = STATE_ESC;
					break;
				case CSI:
					vt->setup.state = STATE_CSI;
					break;
				case 'A': /* cursor up */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_y > 0) {
						vt->setup.cursor_y--;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'B': /* cursor down */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_y < 2) {
						vt->setup.cursor_y++;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'C': /* cursor right */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_x < vt->columns) {
						vt->setup.cursor_x++;
					}
					VT220SetupShowStatus(vt);
					break;
				case 'D': /* cursor left */
					vt->setup.state = STATE_TEXT;
					if(vt->setup.cursor_x > 0) {
						vt->setup.cursor_x--;
					}
					VT220SetupShowStatus(vt);
					break;
				default:
					vt->setup.state = STATE_TEXT;
					VT220SetupShowHint(vt);
					break;
			}
			break;
	}

	VT220SetupShowScreen(vt);
}
