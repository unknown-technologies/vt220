#include <linux/input.h>

#include "types.h"
#include "vt.h"

#define	REPEAT_TIME	(1000 / 30)

void VT220InitKeyboard(VT220* vt)
{
	vt->modifiers = 0;
	vt->last_scancode = 0;
	vt->repeat_scancode = 0;
	vt->repeat_char = 0xFFFF;

	vt->repeat_time = 0;
	vt->repeat_state = 0;
}

u16 VT220TranslateKey(VT220* vt, int scancode)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);
	bool alt = (vt->modifiers & VT220_MODIFIER_ALT_L) || (vt->modifiers & VT220_MODIFIER_ALT_R);

	switch(scancode) {
		case KEY_PAUSE:
			/* The PAUSE key is also mapped to F1 = HOLD SCREEN for convenience */
			return VT220_KEY_HOLD_SCREEN;
		case KEY_F1:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F11_UDK : VT220_KEY_F11;
			} else {
				return VT220_KEY_HOLD_SCREEN;
			}
		case KEY_F2:
			if(alt) {
				return shift ? VT220_KEY_F12_UDK : VT220_KEY_F12;
			} else if(ctrl) {
				return VT220_KEY_AUTO_PRINT_MODE;
			} else {
				return VT220_KEY_PRINT_SCREEN;
			}
		case KEY_F3:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F13_UDK : VT220_KEY_F13;
			} else {
				return VT220_KEY_SET_UP;
			}
		case KEY_F4:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F14_UDK : VT220_KEY_F14;
			} else {
				return VT220_KEY_DATA_TALK;
			}
		case KEY_F5:
			if(alt) {
				return shift ? VT220_KEY_F15_UDK : VT220_KEY_F15;
			} else if(shift) {
				return VT220_KEY_DISCONNECT;
			} else if(ctrl) {
				return VT220_KEY_ANSWERBACK;
			} else {
				return VT220_KEY_BREAK;
			}
		case KEY_F6:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F16_UDK : VT220_KEY_F16;
			} else {
				return shift ? VT220_KEY_F6_UDK : VT220_KEY_F6;
			}
		case KEY_F7:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F17_UDK : VT220_KEY_F17;
			} else {
				return shift ? VT220_KEY_F7_UDK : VT220_KEY_F7;
			}
		case KEY_F8:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F18_UDK : VT220_KEY_F18;
			} else {
				return shift ? VT220_KEY_F8_UDK : VT220_KEY_F8;
			}
		case KEY_F9:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F19_UDK : VT220_KEY_F19;
			} else {
				return shift ? VT220_KEY_F9_UDK : VT220_KEY_F9;
			}
		case KEY_F10:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F20_UDK : VT220_KEY_F20;
			} else {
				return shift ? VT220_KEY_F10_UDK : VT220_KEY_F10;
			}
		case KEY_F11:
			return shift ? VT220_KEY_F11_UDK : VT220_KEY_F11;
		case KEY_F12:
			return shift ? VT220_KEY_F12_UDK : VT220_KEY_F12;
		case KEY_F13:
			return shift ? VT220_KEY_F13_UDK : VT220_KEY_F13;
		case KEY_F14:
			return shift ? VT220_KEY_F14_UDK : VT220_KEY_F14;
		case KEY_F15:
			return shift ? VT220_KEY_F15_UDK : VT220_KEY_F15;
		case KEY_F16:
			return shift ? VT220_KEY_F16_UDK : VT220_KEY_F16;
		case KEY_F17:
			return shift ? VT220_KEY_F17_UDK : VT220_KEY_F17;
		case KEY_F18:
			return shift ? VT220_KEY_F18_UDK : VT220_KEY_F18;
		case KEY_F19:
			return shift ? VT220_KEY_F19_UDK : VT220_KEY_F19;
		case KEY_F20:
			return shift ? VT220_KEY_F20_UDK : VT220_KEY_F20;
		case KEY_LEFT:
			return VT220_KEY_LEFT;
		case KEY_UP:
			return VT220_KEY_UP;
		case KEY_RIGHT:
			return VT220_KEY_RIGHT;
		case KEY_DOWN:
			return VT220_KEY_DOWN;
		case KEY_PAGEUP:
			return VT220_KEY_PREV_SCREEN;
		case KEY_PAGEDOWN:
			return VT220_KEY_NEXT_SCREEN;
		case KEY_HOME:
		case KEY_FIND:
			return VT220_KEY_FIND;
		case KEY_END:
		case KEY_SELECT:
			return VT220_KEY_SELECT;
		case KEY_INSERT:
			return VT220_KEY_INSERT;
		case KEY_DELETE:
			return VT220_KEY_REMOVE;
		case KEY_BACKSPACE:
			return ctrl ? CAN : DEL;
		case KEY_TAB:
			return HT;
		case KEY_ESC:
			return ESC;
		case KEY_SPACE:
			return 0x20;
		case KEY_LEFTSHIFT:
		case KEY_RIGHTSHIFT:
			return VT220_KEY_SHIFT;
		case KEY_ENTER:
			return CR;
			/* keypad */
		case KEY_KP0:
			return VT220_KEY_KP_0;
		case KEY_KP1:
			return VT220_KEY_KP_1;
		case KEY_KP2:
			return VT220_KEY_KP_2;
		case KEY_KP3:
			return VT220_KEY_KP_3;
		case KEY_KP4:
			return VT220_KEY_KP_4;
		case KEY_KP5:
			return VT220_KEY_KP_5;
		case KEY_KP6:
			return VT220_KEY_KP_6;
		case KEY_KP7:
			return VT220_KEY_KP_7;
		case KEY_KP8:
			return VT220_KEY_KP_8;
		case KEY_KP9:
			return VT220_KEY_KP_9;
		case KEY_KPDOT:
			return VT220_KEY_KP_PERIOD;
		case KEY_NUMLOCK:
			return VT220_KEY_KP_PF1;
		case KEY_KPSLASH:
			return VT220_KEY_KP_PF2;
		case KEY_KPASTERISK:
			return VT220_KEY_KP_PF3;
		case KEY_KPMINUS:
			return VT220_KEY_KP_PF4;
		case KEY_KPPLUS:
			return VT220_KEY_KP_MINUS;
		case KEY_KPENTER:
			return VT220_KEY_KP_ENTER;
		default:
			return 0xFFFF;
	}

	return 0xFFFF;
}

void VT220ProcessKeys(VT220* vt, unsigned long dt)
{
	vt->repeat_time += dt;

	if(vt->repeat_state == 0 && vt->repeat_time >= 500) {
		vt->repeat_state = 1;
		vt->repeat_time -= 500;
	}

	if(vt->repeat_state == 1 && vt->repeat_time >= REPEAT_TIME) {
		if(vt->repeat_char < 256) {
			if(vt->mode & DECARM) {
				VT220ProcessKey(vt, vt->repeat_char);
			}
		} else if(vt->repeat_scancode) {
			u16 code = VT220TranslateKey(vt, vt->repeat_scancode);

			/* The following keys never repeat: Hold Screen, Print
			 * Screen, Set-Up, Data/Talk, Break, Return, Compose
			 * Character, Lock, Shift, and Ctrl. */
			switch(code) {
				case VT220_KEY_HOLD_SCREEN:
				case VT220_KEY_PRINT_SCREEN:
				case VT220_KEY_SET_UP:
				case VT220_KEY_DATA_TALK:
				case VT220_KEY_BREAK:
				case VT220_KEY_SHIFT:
				case CR:
					break;
				default:
					if(vt->mode & DECARM) {
						VT220ProcessKey(vt, code);
					}
			}
		}

		if(vt->repeat_time > 2 * REPEAT_TIME) {
			vt->repeat_time = 0;
		} else {
			vt->repeat_time -= REPEAT_TIME;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// North American Keyboard
u16 VT220KeyboardTranslateKey1(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '`';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '@' : '2';
		case KEY_3:
			return shift ? '#' : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '^' : '6';
		case KEY_7:
			return shift ? '&' : '7';
		case KEY_8:
			return shift ? '*' : '8';
		case KEY_9:
			return shift ? '(' : '9';
		case KEY_0:
			return shift ? ')' : '0';
		case KEY_MINUS:
			return shift ? '_' : '-';
		case KEY_EQUAL:
			return shift ? '+' : '=';
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? '{' : '[';
		case KEY_RIGHTBRACE:
			return shift ? '}' : ']';
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? ':' : ';';
		case KEY_APOSTROPHE:
			return shift ? '"' : '\'';
		case KEY_BACKSLASH:
			return shift ? '|' : '\\';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Z' : 'z';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return ',';
		case KEY_DOT:
			return '.';
		case KEY_SLASH:
			return shift ? '?' : '/';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// British Keyboard
u16 VT220KeyboardTranslateKey2(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? 0xA3 : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '&' : '6';
		case KEY_7:
			return shift ? '\'' : '7';
		case KEY_8:
			return shift ? '(' : '8';
		case KEY_9:
			return shift ? ')' : '9';
		case KEY_0:
			return shift ? '=' : '0';
		case KEY_MINUS:
			return shift ? '_' : '-';
		case KEY_EQUAL:
			return shift ? 0xBD : 0xBC;
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xA7 : '@';
		case KEY_RIGHTBRACE:
			return shift ? ']' : '[';
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? '+' : ';';
		case KEY_APOSTROPHE:
			return shift ? '*' : ':';
		case KEY_BACKSLASH:
			return shift ? '`' : '^';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Z' : 'z';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return ',';
		case KEY_DOT:
			return '.';
		case KEY_SLASH:
			return shift ? '?' : '/';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Flemish Keyboard
// TODO: check for correctness of row 0
u16 VT220KeyboardTranslateKey3(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '`';
		case KEY_1:
			return shift ? '1' : '&';
		case KEY_2:
			return shift ? '2' : 0xE9;
		case KEY_3:
			return shift ? '3' : '"';
		case KEY_4:
			return shift ? '4' : '\'';
		case KEY_5:
			return shift ? '5' : '(';
		case KEY_6:
			return shift ? '6' : 0xA7;
		case KEY_7:
			return shift ? '7' : 0xE8;
		case KEY_8:
			return shift ? '8' : '!';
		case KEY_9:
			return shift ? '9' : 0xE7;
		case KEY_0:
			return shift ? '0' : 0xE0;
		case KEY_MINUS:
			return shift ? ')' : 0xB0;
		case KEY_EQUAL:
			return shift ? '_' : '-';
		/* Row 1 */
		case KEY_Q:
			return shift ? 'A' : 'a';
		case KEY_W:
			return shift ? 'Z' : 'z';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xFFFF : '^'; // TODO: dead key
		case KEY_RIGHTBRACE:
			return shift ? '*' : '$';
		/* Row 2 */
		case KEY_A:
			return shift ? 'Q' : 'q';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? 'M' : 'm';
		case KEY_APOSTROPHE:
			return shift ? '%' : 0xD9;
		case KEY_BACKSLASH:
			return shift ? '@' : '#';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'W' : 'w';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? '?' : ',';
		case KEY_COMMA:
			return shift ? '.' : ';';
		case KEY_DOT:
			return shift ? '/' : ':';
		case KEY_SLASH:
			return shift ? '+' : '=';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Canadian (French) Keyboard
u16 VT220KeyboardTranslateKey4(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? '/' : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '?' : '6';
		case KEY_7:
			return shift ? '&' : '7';
		case KEY_8:
			return shift ? '*' : '8';
		case KEY_9:
			return shift ? '(' : '9';
		case KEY_0:
			return shift ? ')' : '0';
		case KEY_MINUS:
			return shift ? '_' : '-';
		case KEY_EQUAL:
			return shift ? '+' : '=';
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xC7 : 0xE7;
		case KEY_RIGHTBRACE:
			return shift ? '@' : '#';
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? ':' : ';';
		case KEY_APOSTROPHE:
			return shift ? '^' : '`'; // TODO: dead key
		case KEY_BACKSLASH:
			return shift ? '|' : '\\';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Z' : 'z';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return ','; // TODO: dead key
		case KEY_DOT:
			return '.';
		case KEY_SLASH:
			return shift ? 0xC9 : 0xE9;
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Danish Keyboard
u16 VT220KeyboardTranslateKey5(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? 0xA7 : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '&' : '6';
		case KEY_7:
			return shift ? '/' : '7';
		case KEY_8:
			return shift ? '(' : '8';
		case KEY_9:
			return shift ? ')' : '9';
		case KEY_0:
			return shift ? '=' : '0';
		case KEY_MINUS:
			return shift ? '?' : '+';
		case KEY_EQUAL:
			return shift ? '`' : 0xFFFF; // TODO: dead key
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xC5 : 0xE5;
		case KEY_RIGHTBRACE:
			return shift ? '^' : 0xFFFF; // TODO: dead key
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? 0xC6 : 0xE6;
		case KEY_APOSTROPHE:
			return shift ? 0xD8 : 0xE8;
		case KEY_BACKSLASH:
			return shift ? '*' : 0xFFFF; // TODO: dead key
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Z' : 'z';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return shift ? ';' : ',';
		case KEY_DOT:
			return shift ? ':' : '.';
		case KEY_SLASH:
			return shift ? '_' : '-';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Finnish Keyboard
u16 VT220KeyboardTranslateKey6(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? 0xA7 : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '&' : '6';
		case KEY_7:
			return shift ? '/' : '7';
		case KEY_8:
			return shift ? '(' : '8';
		case KEY_9:
			return shift ? ')' : '9';
		case KEY_0:
			return shift ? '=' : '0';
		case KEY_MINUS:
			return shift ? '?' : '+';
		case KEY_EQUAL:
			return shift ? '`' : '^'; // TODO: dead key
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Y' : 'y';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xC5 : 0xE5;
		case KEY_RIGHTBRACE:
			return shift ? 0xDC : 0xFC;
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? 0xD6 : 0xF6;
		case KEY_APOSTROPHE:
			return shift ? 0xC4 : 0xE4;
		case KEY_BACKSLASH:
			return shift ? '*' : 0xFFFF; // TODO: dead key
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Z' : 'z';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return shift ? ';' : ',';
		case KEY_DOT:
			return shift ? ':' : '.';
		case KEY_SLASH:
			return shift ? '_' : '-';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// German Keyboard
u16 VT220KeyboardTranslateKey7(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '^' : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? 0xA7 : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '&' : '6';
		case KEY_7:
			return shift ? '/' : '7';
		case KEY_8:
			return shift ? '(' : '8';
		case KEY_9:
			return shift ? ')' : '9';
		case KEY_0:
			return shift ? '=' : '0';
		case KEY_MINUS:
			return shift ? '?' : 0xDF;
		case KEY_EQUAL:
			return shift ? '`' : 0xFFFF; // TODO: dead key
		/* Row 1 */
		case KEY_Q:
			return shift ? 'Q' : 'q';
		case KEY_W:
			return shift ? 'W' : 'w';
		case KEY_E:
			return shift ? 'E' : 'e';
		case KEY_R:
			return shift ? 'R' : 'r';
		case KEY_T:
			return shift ? 'T' : 't';
		case KEY_Y:
			return shift ? 'Z' : 'z';
		case KEY_U:
			return shift ? 'U' : 'u';
		case KEY_I:
			return shift ? 'I' : 'i';
		case KEY_O:
			return shift ? 'O' : 'o';
		case KEY_P:
			return shift ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? 0xDC : 0xFC;
		case KEY_RIGHTBRACE:
			return shift ? '*' : '+';
		/* Row 2 */
		case KEY_A:
			return shift ? 'A' : 'a';
		case KEY_S:
			return shift ? 'S' : 's';
		case KEY_D:
			return shift ? 'D' : 'd';
		case KEY_F:
			return shift ? 'F' : 'f';
		case KEY_G:
			return shift ? 'G' : 'g';
		case KEY_H:
			return shift ? 'H' : 'h';
		case KEY_J:
			return shift ? 'J' : 'j';
		case KEY_K:
			return shift ? 'K' : 'k';
		case KEY_L:
			return shift ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? 0xD6 : 0xF6;
		case KEY_APOSTROPHE:
			return shift ? 0xC4 : 0xE4;
		case KEY_BACKSLASH:
			return shift ? '\'' : '#';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return shift ? 'Y' : 'y';
		case KEY_X:
			return shift ? 'X' : 'x';
		case KEY_C:
			return shift ? 'C' : 'c';
		case KEY_V:
			return shift ? 'V' : 'v';
		case KEY_B:
			return shift ? 'B' : 'b';
		case KEY_N:
			return shift ? 'N' : 'n';
		case KEY_M:
			return shift ? 'M' : 'm';
		case KEY_COMMA:
			return shift ? ';' : ',';
		case KEY_DOT:
			return shift ? ':' : '.';
		case KEY_SLASH:
			return shift ? '_' : '-';
		default:
			return VT220TranslateKey(vt, key);
	}
}

void VT220KeyboardKeyDown(VT220* vt, int key)
{
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);

	switch(key) {
		case KEY_LEFTSHIFT:
			vt->modifiers |= VT220_MODIFIER_SHIFT_L;
			break;
		case KEY_RIGHTSHIFT:
			vt->modifiers |= VT220_MODIFIER_SHIFT_R;
			break;
		case KEY_LEFTCTRL:
			vt->modifiers |= VT220_MODIFIER_CTRL_L;
			break;
		case KEY_RIGHTCTRL:
			vt->modifiers |= VT220_MODIFIER_CTRL_R;
			break;
		case KEY_LEFTALT:
			vt->modifiers |= VT220_MODIFIER_ALT_L;
			break;
		case KEY_RIGHTALT:
			vt->modifiers |= VT220_MODIFIER_ALT_R;
			break;
		default:
			u16 (*translate_key)(VT220*, int);
			switch(vt->config.keyboard) {
				default:
				case VT220_KEYBOARD_NORTH_AMERICAN:
					translate_key = VT220KeyboardTranslateKey1;
					break;
				case VT220_KEYBOARD_BRITISH:
					translate_key = VT220KeyboardTranslateKey2;
					break;
				case VT220_KEYBOARD_FLEMISH:
					translate_key = VT220KeyboardTranslateKey3;
					break;
				case VT220_KEYBOARD_CANADIAN_FRENCH:
					translate_key = VT220KeyboardTranslateKey4;
					break;
				case VT220_KEYBOARD_DANISH:
					translate_key = VT220KeyboardTranslateKey5;
					break;
				case VT220_KEYBOARD_FINNISH:
					translate_key = VT220KeyboardTranslateKey6;
					break;
				case VT220_KEYBOARD_GERMAN:
					translate_key = VT220KeyboardTranslateKey7;
					break;
			}

			u16 value = translate_key(vt, key);
			if(ctrl) {
				if((value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z')) {
					value &= ~0x20; // upper case

					u16 code = value - 'A' + 1;

					vt->repeat_time = 0;
					vt->repeat_state = 0;

					vt->repeat_scancode = key;
					vt->repeat_char = code;

					VT220ProcessKey(vt, code);
				} else {
					u8 code = 0xFF;

					switch(value) {
						case '2':
						case ' ':
							code = NUL;
							break;
						case '3':
						case '[':
							code = ESC;
							break;
						case '4':
						case '\\':
							code = FS;
							break;
						case '5':
						case ']':
							code = GS;
							break;
						case '6':
						case '~':
							code = RS;
							break;
						case '7':
						case '?':
							code = US;
							break;
						case '8':
							code = DEL;
							break;
					}

					if(code != 0xFF) {
						vt->repeat_time = 0;
						vt->repeat_state = 0;

						vt->repeat_scancode = key;
						vt->repeat_char = code;

						VT220ProcessKey(vt, code);
					} else {
						/* no code defined, use default ones */
						u16 code = VT220TranslateKey(vt, key);
						if(code == VT220_KEY_SET_UP) {
							VT220ProcessKey(vt, code);
						} else if(code != 0xFFFF) {
							vt->repeat_time = 0;
							vt->repeat_state = 0;

							vt->repeat_scancode = key;
							vt->repeat_char = 0xFFFF;

							VT220ProcessKey(vt, code);
						} else {
							vt->last_scancode = key;
						}
					}
				}
			} else {
				if(value == VT220_KEY_SET_UP) {
					VT220ProcessKey(vt, value);
				} else if(value != 0xFFFF) {
					vt->repeat_time = 0;
					vt->repeat_state = 0;

					vt->repeat_scancode = key;
					vt->repeat_char = value;

					VT220ProcessKey(vt, value);
				} else {
					vt->last_scancode = key;
				}
			}
			break;
	}
}

void VT220KeyboardKeyUp(VT220* vt, int key)
{
	switch(key) {
		case KEY_LEFTSHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_L;
			break;
		case KEY_RIGHTSHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_R;
			break;
		case KEY_LEFTCTRL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_L;
			break;
		case KEY_RIGHTCTRL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_R;
			break;
		case KEY_LEFTALT:
			vt->modifiers &= ~VT220_MODIFIER_ALT_L;
			break;
		case KEY_RIGHTALT:
			vt->modifiers &= ~VT220_MODIFIER_ALT_R;
			break;
		default:
			if(vt->repeat_char < 256 && key == vt->repeat_scancode) {
				vt->repeat_char = 0xFFFF;
				vt->repeat_scancode = 0;
			} else if(vt->repeat_scancode == key) {
				vt->repeat_scancode = 0;
			}
			break;
	}
}
