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
		case KEY_LEFTMETA:
		case KEY_RIGHTMETA:
		case KEY_COMPOSE:
			return VT220_KEY_COMPOSE;
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

static inline bool VT220KeyboardIsShift(VT220* vt)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);
	if(vt->config.lock == VT220_LOCK_CAPS_LOCK) {
		return shift;
	} else {
		return shift || (vt->modifiers & VT220_MODIFIER_LOCK);
	}
}

static inline bool VT220KeyboardIsCaps(VT220* vt)
{
	/* for letters it doesn't matter if it's caps or shift lock */
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);
	return shift != !!(vt->modifiers & VT220_MODIFIER_LOCK);
}

////////////////////////////////////////////////////////////////////////////////
// North American Keyboard
u16 VT220KeyboardTranslateKey1(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);

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
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? '{' : '[';
		case KEY_RIGHTBRACE:
			return shift ? '}' : ']';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
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
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? (typewriter ? 0xA3 : '#') : '3';
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
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? (typewriter ? 0xA7 : '\\') : '@';
		case KEY_RIGHTBRACE:
			return shift ? ']' : '[';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
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
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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
u16 VT220KeyboardTranslateKey3(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '`';
		case KEY_1:
			return shift ? '&' : '1';
		case KEY_2:
			return shift ? 0xE9 : '2';
		case KEY_3:
			return shift ? '"' : '3';
		case KEY_4:
			return shift ? '\'' : '4';
		case KEY_5:
			return shift ? '(' : '5';
		case KEY_6:
			return shift ? (typewriter ? 0xA7 : '[') : '6';
		case KEY_7:
			return shift ? (typewriter ? 0xE8 : ']') : '7';
		case KEY_8:
			return shift ? '!' : '8';
		case KEY_9:
			return shift ? '9' : '9';
		case KEY_0:
			return shift ? 0xE0 : '0';
		case KEY_MINUS:
			return shift ? ')' : 0xB0;
		case KEY_EQUAL:
			return shift ? '_' : '-';
		/* Row 1 */
		case KEY_Q:
			return caps ? 'A' : 'a';
		case KEY_W:
			return caps ? 'Z' : 'z';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? VT220_KEY_UMLAUT : '^';
		case KEY_RIGHTBRACE:
			return shift ? '*' : '$';
		/* Row 2 */
		case KEY_A:
			return caps ? 'Q' : 'q';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? 'M' : 'm';
		case KEY_APOSTROPHE:
			return shift ? '%' : (typewriter ? 0xD9 : '\\');
		case KEY_BACKSLASH:
			return shift ? '@' : '#';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'W' : 'w';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
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
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

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
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? 0xC7 : 0xE7) : (shift ? '[' : ']');
		case KEY_RIGHTBRACE:
			return shift ? '@' : '#';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
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
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
		case KEY_COMMA:
			return shift ? '\'' : ',';
		case KEY_DOT:
			return '.';
		case KEY_SLASH:
			return caps ? 0xC9 : 0xE9;
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Danish Keyboard
u16 VT220KeyboardTranslateKey5(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? (typewriter ? 0xA7 : '@') : '3';
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
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? '[' : ']');
		case KEY_RIGHTBRACE:
			return shift ? '^' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? 0xC6 : 0xE6;
		case KEY_APOSTROPHE:
			return typewriter ? (caps ? 0xD8 : 0xE8) : (shift ? '\\' : '#');
		case KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? 0xB0 : '~';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? (typewriter ? 0xA7 : '@') : '3';
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
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? '[' : ']');
		case KEY_RIGHTBRACE:
			return typewriter ? (caps ? 0xDC : 0xFC) : (shift ? '\\' : '#');
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? 0xD6 : 0xF6;
		case KEY_APOSTROPHE:
			return caps ? 0xC4 : 0xE4;
		case KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? VT220_KEY_CIRCUMFLEX : '~';
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
			return shift ? VT220_KEY_GRAVE : VT220_KEY_ACUTE;
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Z' : 'z';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? 0xDC : 0xFC) : (shift ? '\\' : '@');
		case KEY_RIGHTBRACE:
			return shift ? '*' : '+';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? 0xD6 : 0xF6;
		case KEY_APOSTROPHE:
			return typewriter ? (caps ? 0xC4 : 0xE4) : (shift ? '[' : ']');
		case KEY_BACKSLASH:
			return shift ? '\'' : '#';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Y' : 'y';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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

// Dutch Keyboard
u16 VT220KeyboardTranslateKey8(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '\xB0' : '~';
		case KEY_1:
			return shift ? '\xBC' : '1';
		case KEY_2:
			return shift ? (typewriter ? '[' : '\xBD') : '2';
		case KEY_3:
			return shift ? (typewriter ? ']' : '\xA3') : '3';
		case KEY_4:
			return shift ? '$' : '4';
		case KEY_5:
			return shift ? '%' : '5';
		case KEY_6:
			return shift ? '#' : '6';
		case KEY_7:
			return shift ? '&' : '7';
		case KEY_8:
			return shift ? '(' : '8';
		case KEY_9:
			return shift ? ')' : '9';
		case KEY_0:
			return shift ? (typewriter ? '\xA7' : '\\') : '0';
		case KEY_MINUS:
			return shift ? '"' : '\'';
		case KEY_EQUAL:
			return shift ? ':' : '/';
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? VT220_KEY_UMLAUT : '^'; // TODO: dead key
		case KEY_RIGHTBRACE:
			return shift ? '!' : '*';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? '_' : '-';
		case KEY_APOSTROPHE:
			return shift ? '\'' : '`'; // TODO: dead key
		case KEY_BACKSLASH:
			return shift ? '|' : '@';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
		case KEY_COMMA:
			return shift ? '?' : ',';
		case KEY_DOT:
			return shift ? ';' : '.';
		case KEY_SLASH:
			return shift ? '=' : '+';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Italian Keyboard
u16 VT220KeyboardTranslateKey9(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '`'; // TODO: dead key?
		case KEY_1:
			return shift ? '1' : (typewriter ? '\xA3' : '@');
		case KEY_2:
			return shift ? '2' : (typewriter ? '\xE9' : '#');
		case KEY_3:
			return shift ? '3' : '"';
		case KEY_4:
			return shift ? '4' : '\''; // TODO: dead key?
		case KEY_5:
			return shift ? '5' : '(';
		case KEY_6:
			return shift ? '6' : '_';
		case KEY_7:
			return shift ? '7' : '\xE8';
		case KEY_8:
			return shift ? '8' : '^'; // TODO: dead key?
		case KEY_9:
			return shift ? '9' : (typewriter ? '\xE7' : '[');
		case KEY_0:
			return shift ? '0' : (typewriter ? '\xE0' : ']');
		case KEY_MINUS:
			return shift ? '\xB0' : ')';
		case KEY_EQUAL:
			return shift ? '+' : '-';
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'Z' : 'z';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? '=' : '\xEC';
		case KEY_RIGHTBRACE:
			return shift ? '&' : '$';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? 'M' : 'm';
		case KEY_APOSTROPHE:
			return shift ? '%' : '\xF9';
		case KEY_BACKSLASH:
			return shift ? (typewriter ? '\xA7' : '\\') : '*';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'W' : 'w';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return shift ? '?' : '\'';
		case KEY_COMMA:
			return shift ? ',' : ';';
		case KEY_DOT:
			return shift ? '/' : ':';
		case KEY_SLASH:
			return shift ? '!' : '\xF2';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Swiss (French) Keyboard
u16 VT220KeyboardTranslateKey10(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '\xB0' : '!';
		case KEY_1:
			return shift ? '+' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? '*' : '3';
		case KEY_4:
			return shift ? (typewriter ? '\xE7' : '@') : '4';
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
			return shift ? '?' : '\''; // TODO: dead key?
		case KEY_EQUAL:
			return shift ? '`' : '^'; // TODO: dead key?
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Z' : 'z';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? (typewriter ? '\xFC' : '\\') : '\xE8';
		case KEY_RIGHTBRACE:
			return shift ? '~' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? (typewriter ? '\xF6' : '[') : '\xE9';
		case KEY_APOSTROPHE:
			return shift ? (typewriter ? '\xE4' : ']') : '\xE0';
		case KEY_BACKSLASH:
			return shift ? (typewriter ? '\xA3' : '#') : '$';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Y' : 'y';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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

// Swiss (German) Keyboard
u16 VT220KeyboardTranslateKey11(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '\xB0' : '!';
		case KEY_1:
			return shift ? '+' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? '*' : '3';
		case KEY_4:
			return shift ? (typewriter ? '\xE7' : '@') : '4';
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
			return shift ? '?' : '\''; // TODO: dead key?
		case KEY_EQUAL:
			return shift ? '`' : '^'; // TODO: dead key?
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Z' : 'z';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? (typewriter ? '\xE8' : '\\') : '\xFC';
		case KEY_RIGHTBRACE:
			return shift ? '~' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? (typewriter ? '\xE9' : '[') : '\xF6';
		case KEY_APOSTROPHE:
			return shift ? (typewriter ? '\xE0' : ']') : '\xE4';
		case KEY_BACKSLASH:
			return shift ? (typewriter ? '\xA3' : '#') : '$';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Y' : 'y';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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

// Swedish Keyboard
u16 VT220KeyboardTranslateKey12(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '\xB0' : '~'; // TODO: dead key?
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? (typewriter ? '\xA7' : '@') : '3';
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
			return shift ? '`' : '^'; // TODO: dead key?
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? '\xC5' : '\xE5') : (shift ? '[' : ']');
		case KEY_RIGHTBRACE:
			return typewriter ? (caps ? '\xDC' : '\xFC') : (shift ? '\\' : '#');
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? '\xD6' : '\xF6';
		case KEY_APOSTROPHE:
			return caps ? '\xC4' : '\xE4';
		case KEY_BACKSLASH:
			return shift ? '*' : '\''; // TODO: dead key?
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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

// Norwegian Keyboard
u16 VT220KeyboardTranslateKey13(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '\xB0';
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? (typewriter ? '\xA7' : '@') : '3';
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
			return shift ? '`' : '\''; // TODO: dead key
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return typewriter ? (caps ? '\xC5' : '\xE5') : (shift ? '[' : ']');
		case KEY_RIGHTBRACE:
			return shift ? VT220_KEY_CIRCUMFLEX : VT220_KEY_UMLAUT;
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? '\xD8' : '\xF8';
		case KEY_APOSTROPHE:
			return typewriter ? (caps ? '\xC6' : '\xE6') : (shift ? '\\' : '#');
		case KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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

// French Keyboard
u16 VT220KeyboardTranslateKey14(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return shift ? '~' : '`'; // TODO: dead key?
		case KEY_1:
			return shift ? '1' : '&';
		case KEY_2:
			return shift ? '2' : '\xE9';
		case KEY_3:
			return shift ? '3' : '"';
		case KEY_4:
			return shift ? '4' : '\''; // TODO: dead key?
		case KEY_5:
			return shift ? '5' : '(';
		case KEY_6:
			return shift ? '6' : (typewriter ? '\xA7' : '[');
		case KEY_7:
			return shift ? '7' : (typewriter ? '\xE8' : ']');
		case KEY_8:
			return shift ? '8' : '!';
		case KEY_9:
			return shift ? '9' : '\xE7';
		case KEY_0:
			return shift ? '0' : '\xE0';
		case KEY_MINUS:
			return shift ? '\xB0' : ')';
		case KEY_EQUAL:
			return shift ? '_' : '-';
		/* Row 1 */
		case KEY_Q:
			return caps ? 'A' : 'a';
		case KEY_W:
			return caps ? 'Z' : 'z';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_CIRCUMFLEX;
		case KEY_RIGHTBRACE:
			return shift ? '*' : '$';
		/* Row 2 */
		case KEY_A:
			return caps ? 'Q' : 'q';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return shift ? 'M' : 'm';
		case KEY_APOSTROPHE:
			return shift ? '%' : (typewriter ? '\xF9' : '\\');
		case KEY_BACKSLASH:
			return shift ? '@' : '#';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'W' : 'w';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
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

// Spanish Keyboard
u16 VT220KeyboardTranslateKey15(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case KEY_GRAVE:
			return typewriter ? (shift ? '\xA1' : '\xBF') : (shift ? '\\' : '@');
		case KEY_1:
			return shift ? '!' : '1';
		case KEY_2:
			return shift ? '"' : '2';
		case KEY_3:
			return shift ? '#' : '3';
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
			return shift ? '?' : '\'';
		case KEY_EQUAL:
			return typewriter ? (shift ? '\xAA' : '\xBA') : (shift ? '[' : ']');
		/* Row 1 */
		case KEY_Q:
			return caps ? 'Q' : 'q';
		case KEY_W:
			return caps ? 'W' : 'w';
		case KEY_E:
			return caps ? 'E' : 'e';
		case KEY_R:
			return caps ? 'R' : 'r';
		case KEY_T:
			return caps ? 'T' : 't';
		case KEY_Y:
			return caps ? 'Y' : 'y';
		case KEY_U:
			return caps ? 'U' : 'u';
		case KEY_I:
			return caps ? 'I' : 'i';
		case KEY_O:
			return caps ? 'O' : 'o';
		case KEY_P:
			return caps ? 'P' : 'p';
		case KEY_LEFTBRACE:
			return shift ? VT220_KEY_CIRCUMFLEX : VT220_KEY_GRAVE;
		case KEY_RIGHTBRACE:
			return shift ? '*' : '+';
		/* Row 2 */
		case KEY_A:
			return caps ? 'A' : 'a';
		case KEY_S:
			return caps ? 'S' : 's';
		case KEY_D:
			return caps ? 'D' : 'd';
		case KEY_F:
			return caps ? 'F' : 'f';
		case KEY_G:
			return caps ? 'G' : 'g';
		case KEY_H:
			return caps ? 'H' : 'h';
		case KEY_J:
			return caps ? 'J' : 'j';
		case KEY_K:
			return caps ? 'K' : 'k';
		case KEY_L:
			return caps ? 'L' : 'l';
		case KEY_SEMICOLON:
			return caps ? '\xD1' : '\xF1';
		case KEY_APOSTROPHE:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_ACUTE;
		case KEY_BACKSLASH:
			return shift ? VT220_KEY_TILDE : '\xE7';
		/* Row 3 */
		case KEY_102ND:
			return shift ? '>' : '<';
		case KEY_Z:
			return caps ? 'Z' : 'z';
		case KEY_X:
			return caps ? 'X' : 'x';
		case KEY_C:
			return caps ? 'C' : 'c';
		case KEY_V:
			return caps ? 'V' : 'v';
		case KEY_B:
			return caps ? 'B' : 'b';
		case KEY_N:
			return caps ? 'N' : 'n';
		case KEY_M:
			return caps ? 'M' : 'm';
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
	u32 last_modifiers = vt->modifiers;
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);

	switch(key) {
		case KEY_LEFTSHIFT:
			if(vt->config.lock == VT220_LOCK_SHIFT_LOCK) {
				vt->modifiers &= ~VT220_MODIFIER_LOCK;
			}
			vt->modifiers |= VT220_MODIFIER_SHIFT_L;
			break;
		case KEY_RIGHTSHIFT:
			if(vt->config.lock == VT220_LOCK_SHIFT_LOCK) {
				vt->modifiers &= ~VT220_MODIFIER_LOCK;
			}
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
		case KEY_CAPSLOCK:
			vt->modifiers ^= VT220_MODIFIER_LOCK;
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
				case VT220_KEYBOARD_DUTCH:
					translate_key = VT220KeyboardTranslateKey8;
					break;
				case VT220_KEYBOARD_ITALIAN:
					translate_key = VT220KeyboardTranslateKey9;
					break;
				case VT220_KEYBOARD_SWISS_FRENCH:
					translate_key = VT220KeyboardTranslateKey10;
					break;
				case VT220_KEYBOARD_SWISS_GERMAN:
					translate_key = VT220KeyboardTranslateKey11;
					break;
				case VT220_KEYBOARD_SWEDISH:
					translate_key = VT220KeyboardTranslateKey12;
					break;
				case VT220_KEYBOARD_NORWEGIAN:
					translate_key = VT220KeyboardTranslateKey13;
					break;
				case VT220_KEYBOARD_FRENCH:
					translate_key = VT220KeyboardTranslateKey14;
					break;
				case VT220_KEYBOARD_SPANISH:
					translate_key = VT220KeyboardTranslateKey15;
					break;
			}

			u16 value = translate_key(vt, key);
			// TODO: properly implement dead keys
			switch(value) {
				case VT220_KEY_UMLAUT:
					value = '"';
					break;
				case VT220_KEY_ACUTE:
					value = '\'';
					break;
				case VT220_KEY_GRAVE:
					value = '`';
					break;
				case VT220_KEY_CIRCUMFLEX:
					value = '^';
					break;
				case VT220_KEY_TILDE:
					value = '~';
					break;
			}
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

	if(vt->modifiers != last_modifiers) {
		VT220KeyboardUpdateLEDs(vt);
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
