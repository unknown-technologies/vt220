#include <GLFW/glfw3.h>

#include "types.h"
#include "vt.h"

#define	COMPOSE_NONE	0
#define	COMPOSE_FIRST	1
#define	COMPOSE_SECOND	2

/* TODO: keep track of all pressed keys and handle key up + autorepeat
 * appropriately */

void VT220InitKeyboard(VT220* vt)
{
	vt->modifiers = 0;
	vt->compose_state = COMPOSE_NONE;
	vt->last_scancode = 0;
	vt->repeat_scancode = 0;
	vt->repeat_char = 0xFFFF;

	vt->repeat_time = 0;
	vt->repeat_state = 0;
}

u16 VT220TranslateKey(VT220* vt, int key)
{
	bool shift = (vt->modifiers & VT220_MODIFIER_SHIFT_L) || (vt->modifiers & VT220_MODIFIER_SHIFT_R);
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);
	bool alt = (vt->modifiers & VT220_MODIFIER_ALT_L) || (vt->modifiers & VT220_MODIFIER_ALT_R);

	switch(key) {
		case GLFW_KEY_PAUSE:
			/* The PAUSE key is also mapped to F1 = HOLD SCREEN for convenience */
			return VT220_KEY_HOLD_SCREEN;
		case GLFW_KEY_F1:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F11_UDK : VT220_KEY_F11;
			} else {
				return VT220_KEY_HOLD_SCREEN;
			}
		case GLFW_KEY_F2:
			if(alt) {
				return shift ? VT220_KEY_F12_UDK : VT220_KEY_F12;
			} else if(ctrl) {
				return VT220_KEY_AUTO_PRINT_MODE;
			} else {
				return VT220_KEY_PRINT_SCREEN;
			}
		case GLFW_KEY_F3:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F13_UDK : VT220_KEY_F13;
			} else {
				return VT220_KEY_SET_UP;
			}
		case GLFW_KEY_F4:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F14_UDK : VT220_KEY_F14;
			} else {
				return VT220_KEY_DATA_TALK;
			}
		case GLFW_KEY_F5:
			if(alt) {
				return shift ? VT220_KEY_F15_UDK : VT220_KEY_F15;
			} else if(shift) {
				return VT220_KEY_DISCONNECT;
			} else if(ctrl) {
				return VT220_KEY_ANSWERBACK;
			} else {
				return VT220_KEY_BREAK;
			}
		case GLFW_KEY_F6:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F16_UDK : VT220_KEY_F16;
			} else {
				return shift ? VT220_KEY_F6_UDK : VT220_KEY_F6;
			}
		case GLFW_KEY_F7:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F17_UDK : VT220_KEY_F17;
			} else {
				return shift ? VT220_KEY_F7_UDK : VT220_KEY_F7;
			}
		case GLFW_KEY_F8:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F18_UDK : VT220_KEY_F18;
			} else {
				return shift ? VT220_KEY_F8_UDK : VT220_KEY_F8;
			}
		case GLFW_KEY_F9:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F19_UDK : VT220_KEY_F19;
			} else {
				return shift ? VT220_KEY_F9_UDK : VT220_KEY_F9;
			}
		case GLFW_KEY_F10:
			if(ctrl || alt) {
				return shift ? VT220_KEY_F20_UDK : VT220_KEY_F20;
			} else {
				return shift ? VT220_KEY_F10_UDK : VT220_KEY_F10;
			}
		case GLFW_KEY_F11:
			return shift ? VT220_KEY_F11_UDK : VT220_KEY_F11;
		case GLFW_KEY_F12:
			return shift ? VT220_KEY_F12_UDK : VT220_KEY_F12;
		case GLFW_KEY_F13:
			return shift ? VT220_KEY_F13_UDK : VT220_KEY_F13;
		case GLFW_KEY_F14:
			return shift ? VT220_KEY_F14_UDK : VT220_KEY_F14;
		case GLFW_KEY_F15:
			return shift ? VT220_KEY_F15_UDK : VT220_KEY_F15;
		case GLFW_KEY_F16:
			return shift ? VT220_KEY_F16_UDK : VT220_KEY_F16;
		case GLFW_KEY_F17:
			return shift ? VT220_KEY_F17_UDK : VT220_KEY_F17;
		case GLFW_KEY_F18:
			return shift ? VT220_KEY_F18_UDK : VT220_KEY_F18;
		case GLFW_KEY_F19:
			return shift ? VT220_KEY_F19_UDK : VT220_KEY_F19;
		case GLFW_KEY_F20:
			return shift ? VT220_KEY_F20_UDK : VT220_KEY_F20;
		case GLFW_KEY_LEFT:
			return VT220_KEY_LEFT;
		case GLFW_KEY_UP:
			return VT220_KEY_UP;
		case GLFW_KEY_RIGHT:
			return VT220_KEY_RIGHT;
		case GLFW_KEY_DOWN:
			return VT220_KEY_DOWN;
		case GLFW_KEY_PAGE_UP:
			return VT220_KEY_PREV_SCREEN;
		case GLFW_KEY_PAGE_DOWN:
			return VT220_KEY_NEXT_SCREEN;
		case GLFW_KEY_HOME:
			return VT220_KEY_FIND;
		case GLFW_KEY_END:
			return VT220_KEY_SELECT;
		case GLFW_KEY_INSERT:
			return VT220_KEY_INSERT;
		case GLFW_KEY_DELETE:
			return VT220_KEY_REMOVE;
		case GLFW_KEY_BACKSPACE:
			return ctrl ? CAN : DEL;
		case GLFW_KEY_TAB:
			return HT;
		case GLFW_KEY_ESCAPE:
			return ESC;
		case GLFW_KEY_SPACE:
			return 0x20;
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:
			return VT220_KEY_SHIFT;
		case GLFW_KEY_ENTER:
			return CR;
		/* keypad */
		case GLFW_KEY_KP_0:
			return VT220_KEY_KP_0;
		case GLFW_KEY_KP_1:
			return VT220_KEY_KP_1;
		case GLFW_KEY_KP_2:
			return VT220_KEY_KP_2;
		case GLFW_KEY_KP_3:
			return VT220_KEY_KP_3;
		case GLFW_KEY_KP_4:
			return VT220_KEY_KP_4;
		case GLFW_KEY_KP_5:
			return VT220_KEY_KP_5;
		case GLFW_KEY_KP_6:
			return VT220_KEY_KP_6;
		case GLFW_KEY_KP_7:
			return VT220_KEY_KP_7;
		case GLFW_KEY_KP_8:
			return VT220_KEY_KP_8;
		case GLFW_KEY_KP_9:
			return VT220_KEY_KP_9;
		case GLFW_KEY_KP_DECIMAL:
			return VT220_KEY_KP_PERIOD;
		case GLFW_KEY_NUM_LOCK:
			return VT220_KEY_KP_PF1;
		case GLFW_KEY_KP_DIVIDE:
			return VT220_KEY_KP_PF2;
		case GLFW_KEY_KP_MULTIPLY:
			return VT220_KEY_KP_PF3;
		case GLFW_KEY_KP_SUBTRACT:
			return VT220_KEY_KP_PF4;
		case GLFW_KEY_KP_ADD:
			return VT220_KEY_KP_MINUS;
		case GLFW_KEY_KP_ENTER:
			return VT220_KEY_KP_ENTER;
		case GLFW_KEY_KP_EQUAL:
		default:
			return 0xFFFF;
	}
}

void VT220KeyboardKeyDownHost(VT220* vt, int key)
{
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);

	switch(key) {
		case GLFW_KEY_LEFT_SHIFT:
			vt->modifiers |= VT220_MODIFIER_SHIFT_L;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			vt->modifiers |= VT220_MODIFIER_SHIFT_R;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			vt->modifiers |= VT220_MODIFIER_CTRL_L;
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			vt->modifiers |= VT220_MODIFIER_CTRL_R;
			break;
		case GLFW_KEY_LEFT_ALT:
			vt->modifiers |= VT220_MODIFIER_ALT_L;
			break;
		case GLFW_KEY_RIGHT_ALT:
			vt->modifiers |= VT220_MODIFIER_ALT_R;
			break;
		default:
			if(ctrl) {
				const char* name = glfwGetKeyName(key, 0);
				if(name && !(*name & 0x80) && !name[1]) {
					u16 value = *name;

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
				} else if(key == GLFW_KEY_SPACE) {
					vt->repeat_time = 0;
					vt->repeat_state = 0;

					vt->repeat_scancode = key;
					vt->repeat_char = 0;

					VT220ProcessKey(vt, 0);
				} else if(key == GLFW_KEY_BACKSPACE) {
					vt->repeat_time = 0;
					vt->repeat_state = 0;

					vt->repeat_scancode = key;
					vt->repeat_char = CAN;

					VT220ProcessKey(vt, CAN);
				} else {
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
			} else {
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
			break;
	}
}

void VT220KeyboardKeyUpHost(VT220* vt, int key)
{
	switch(key) {
		case GLFW_KEY_LEFT_SHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_L;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_R;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_L;
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_R;
			break;
		case GLFW_KEY_LEFT_ALT:
			vt->modifiers &= ~VT220_MODIFIER_ALT_L;
			break;
		case GLFW_KEY_RIGHT_ALT:
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

void VT220KeyboardChar(VT220* vt, unsigned int code)
{
	if(vt->config.keyboard != VT220_KEYBOARD_UNKNOWN) {
		return;
	}

	if(vt->last_scancode) {
		vt->repeat_scancode = vt->last_scancode;
		vt->repeat_char = code;
		vt->last_scancode = 0;

		vt->repeat_time = 0;
		vt->repeat_state = 0;

		if(code < 256) {
			VT220ProcessKey(vt, code);
		}
	}
}

#define	REPEAT_TIME	(1000 / 30)
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? '~' : '`';
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '@' : '2';
		case GLFW_KEY_3:
			return shift ? '#' : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '^' : '6';
		case GLFW_KEY_7:
			return shift ? '&' : '7';
		case GLFW_KEY_8:
			return shift ? '*' : '8';
		case GLFW_KEY_9:
			return shift ? '(' : '9';
		case GLFW_KEY_0:
			return shift ? ')' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '_' : '-';
		case GLFW_KEY_EQUAL:
			return shift ? '+' : '=';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? '{' : '[';
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '}' : ']';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? ':' : ';';
		case GLFW_KEY_APOSTROPHE:
			return shift ? '"' : '\'';
		case GLFW_KEY_BACKSLASH:
			return shift ? '|' : '\\';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return ',';
		case GLFW_KEY_PERIOD:
			return '.';
		case GLFW_KEY_SLASH:
			return shift ? '?' : '/';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// British Keyboard
// NOTE: this layout differs significantly from what is show in the Owner's Manual
u16 VT220KeyboardTranslateKey2(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? '~' : '`';
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '@' : '2';
		case GLFW_KEY_3:
			return shift ? (typewriter ? 0xA3 : '#') : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '^' : '6';
		case GLFW_KEY_7:
			return shift ? '&' : '7';
		case GLFW_KEY_8:
			return shift ? '*' : '8';
		case GLFW_KEY_9:
			return shift ? '(' : '9';
		case GLFW_KEY_0:
			return shift ? ')' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '_' : '-';
		case GLFW_KEY_EQUAL:
			return shift ? '+' : '=';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? '{' : '[';
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '}' : ']';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? ':' : ';';
		case GLFW_KEY_APOSTROPHE:
			return shift ? '"' : '\'';
		case GLFW_KEY_BACKSLASH:
			return shift ? '|' : '\\';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return ',';
		case GLFW_KEY_PERIOD:
			return '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? VT220_KEY_TILDE : VT220_KEY_GRAVE;
		case GLFW_KEY_1:
			return shift ? '&' : '1';
		case GLFW_KEY_2:
			return shift ? 0xE9 : '2';
		case GLFW_KEY_3:
			return shift ? '"' : '3';
		case GLFW_KEY_4:
			return shift ? '\'' : '4';
		case GLFW_KEY_5:
			return shift ? '(' : '5';
		case GLFW_KEY_6:
			return shift ? (typewriter ? 0xA7 : '[') : '6';
		case GLFW_KEY_7:
			return shift ? (typewriter ? 0xE8 : ']') : '7';
		case GLFW_KEY_8:
			return shift ? '!' : '8';
		case GLFW_KEY_9:
			return shift ? 0xE7 : '9';
		case GLFW_KEY_0:
			return shift ? 0xE0 : '0';
		case GLFW_KEY_MINUS:
			return shift ? ')' : 0xB0;
		case GLFW_KEY_EQUAL:
			return shift ? '_' : '-';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'A' : 'a';
		case GLFW_KEY_W:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_CIRCUMFLEX;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '*' : '$';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 'M' : 'm';
		case GLFW_KEY_APOSTROPHE:
			return shift ? '%' : (typewriter ? 0xD9 : '\\');
		case GLFW_KEY_BACKSLASH:
			return shift ? '@' : '#';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'W' : 'w';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return shift ? '?' : ',';
		case GLFW_KEY_COMMA:
			return shift ? '.' : ';';
		case GLFW_KEY_PERIOD:
			return shift ? '/' : ':';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : VT220_KEY_TILDE;
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? '/' : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '?' : '6';
		case GLFW_KEY_7:
			return shift ? '&' : '7';
		case GLFW_KEY_8:
			return shift ? '*' : '8';
		case GLFW_KEY_9:
			return shift ? '(' : '9';
		case GLFW_KEY_0:
			return shift ? ')' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '_' : '-';
		case GLFW_KEY_EQUAL:
			return shift ? '+' : '=';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xC7 : 0xE7) : (shift ? '[' : ']');
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '@' : '#';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? ':' : ';';
		case GLFW_KEY_APOSTROPHE:
			return shift ? VT220_KEY_CIRCUMFLEX : VT220_KEY_GRAVE;
		case GLFW_KEY_BACKSLASH:
			return shift ? '|' : '\\';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? '\'' : ',';
		case GLFW_KEY_PERIOD:
			return '.';
		case GLFW_KEY_SLASH:
			return caps ? 0xC9 : 0xE9;
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Danish Keyboard
// NOTE: this keyboard layout differs from the layout show in the Owner's Manual
u16 VT220KeyboardTranslateKey5(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xA7 : '~';
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? (typewriter ? 0xA3 : '#') : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '+';
		case GLFW_KEY_EQUAL:
			return shift ? '`' : VT220_KEY_ACUTE;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? ']' : '}');
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '^' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return typewriter ? (shift ? '[' : '{') : (caps ? 0xC6 : 0xE6);
		case GLFW_KEY_APOSTROPHE:
			return typewriter ? (caps ? 0xD8 : 0xE8) : (shift ? '\\' : '|');
		case GLFW_KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : VT220_KEY_TILDE;
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? (typewriter ? 0xA7 : '@') : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '+';
		case GLFW_KEY_EQUAL:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_CIRCUMFLEX;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? '[' : ']');
		case GLFW_KEY_RIGHT_BRACKET:
			return typewriter ? (caps ? 0xDC : 0xFC) : (shift ? '\\' : '#');
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 0xD6 : 0xF6;
		case GLFW_KEY_APOSTROPHE:
			return caps ? 0xC4 : 0xE4;
		case GLFW_KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? VT220_KEY_CIRCUMFLEX : VT220_KEY_TILDE;
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? 0xA7 : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : 0xDF;
		case GLFW_KEY_EQUAL:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_ACUTE;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xDC : 0xFC) : (shift ? '\\' : '@');
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '*' : '+';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 0xD6 : 0xF6;
		case GLFW_KEY_APOSTROPHE:
			return typewriter ? (caps ? 0xC4 : 0xE4) : (shift ? '[' : ']');
		case GLFW_KEY_BACKSLASH:
			return shift ? '\'' : '#';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : VT220_KEY_TILDE;
		case GLFW_KEY_1:
			return shift ? 0xBC : '1';
		case GLFW_KEY_2:
			return shift ? (typewriter ? '[' : 0xBD) : '2';
		case GLFW_KEY_3:
			return shift ? (typewriter ? ']' : 0xA3) : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '#' : '6';
		case GLFW_KEY_7:
			return shift ? '&' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? (typewriter ? 0xA7 : '\\') : '0';
		case GLFW_KEY_MINUS:
			return shift ? '"' : '\'';
		case GLFW_KEY_EQUAL:
			return shift ? ':' : '/';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_CIRCUMFLEX;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '!' : '*';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? '_' : '-';
		case GLFW_KEY_APOSTROPHE:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_ACUTE;
		case GLFW_KEY_BACKSLASH:
			return shift ? '|' : '@';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? '?' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ';' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? VT220_KEY_TILDE : VT220_KEY_GRAVE;
		case GLFW_KEY_1:
			return shift ? '1' : (typewriter ? 0xA3 : '@');
		case GLFW_KEY_2:
			return shift ? '2' : (typewriter ? 0xE9 : '#');
		case GLFW_KEY_3:
			return shift ? '3' : '"';
		case GLFW_KEY_4:
			return shift ? '4' : '\'';
		case GLFW_KEY_5:
			return shift ? '5' : '(';
		case GLFW_KEY_6:
			return shift ? '6' : '_';
		case GLFW_KEY_7:
			return shift ? '7' : 0xE8;
		case GLFW_KEY_8:
			return shift ? '8' : VT220_KEY_CIRCUMFLEX;
		case GLFW_KEY_9:
			return shift ? '9' : (typewriter ? 0xE7 : '[');
		case GLFW_KEY_0:
			return shift ? '0' : (typewriter ? 0xE0 : ']');
		case GLFW_KEY_MINUS:
			return shift ? 0xB0 : ')';
		case GLFW_KEY_EQUAL:
			return shift ? '+' : '-';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? '=' : 0xEC;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '&' : '$';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 'M' : 'm';
		case GLFW_KEY_APOSTROPHE:
			return shift ? '%' : 0xF9;
		case GLFW_KEY_BACKSLASH:
			return shift ? (typewriter ? 0xA7 : '\\') : '*';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'W' : 'w';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return shift ? '?' : ',';
		case GLFW_KEY_COMMA:
			return shift ? '.' : ';';
		case GLFW_KEY_PERIOD:
			return shift ? '/' : ':';
		case GLFW_KEY_SLASH:
			return shift ? '!' : 0xF2;
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : '!';
		case GLFW_KEY_1:
			return shift ? '+' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? '*' : '3';
		case GLFW_KEY_4:
			return shift ? (typewriter ? 0xE7 : '@') : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '\'';
		case GLFW_KEY_EQUAL:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_CIRCUMFLEX;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? (typewriter ? 0xFC : '\\') : 0xE8;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? VT220_KEY_TILDE : VT220_KEY_UMLAUT;
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? (typewriter ? 0xF6 : '[') : 0xE9;
		case GLFW_KEY_APOSTROPHE:
			return shift ? (typewriter ? 0xE4 : ']') : 0xE0;
		case GLFW_KEY_BACKSLASH:
			return shift ? (typewriter ? 0xA3 : '#') : '$';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : '!';
		case GLFW_KEY_1:
			return shift ? '+' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? '*' : '3';
		case GLFW_KEY_4:
			return shift ? (typewriter ? 0xE7 : '@') : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '\'';
		case GLFW_KEY_EQUAL:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_CIRCUMFLEX;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? (typewriter ? 0xE8 : '\\') : 0xFC;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '~' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return shift ? (typewriter ? 0xE9 : '[') : 0xF6;
		case GLFW_KEY_APOSTROPHE:
			return shift ? (typewriter ? 0xE0 : ']') : 0xE4;
		case GLFW_KEY_BACKSLASH:
			return shift ? (typewriter ? 0xA3 : '#') : '$';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? 0xB0 : VT220_KEY_TILDE;
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? (typewriter ? 0xA7 : '@') : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '+';
		case GLFW_KEY_EQUAL:
			return shift ? VT220_KEY_GRAVE : VT220_KEY_CIRCUMFLEX;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? '[' : ']');
		case GLFW_KEY_RIGHT_BRACKET:
			return typewriter ? (caps ? 0xDC : 0xFC) : (shift ? '\\' : '#');
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 0xD6 : 0xF6;
		case GLFW_KEY_APOSTROPHE:
			return caps ? 0xC4 : 0xE4;
		case GLFW_KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
			return shift ? '_' : '-';
		default:
			return VT220TranslateKey(vt, key);
	}
}

// Norwegian Keyboard
// NOTE: this keyboard layout differs from the layout show in the Owner's Manual
u16 VT220KeyboardTranslateKey13(VT220* vt, int key)
{
	bool shift = VT220KeyboardIsShift(vt);
	bool caps = VT220KeyboardIsCaps(vt);
	bool typewriter = vt->config.keys == VT220_KEYS_TYPEWRITER;

	switch(key) {
		/* Row 0 */
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? '@' : '~';
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? '#' : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '+';
		case GLFW_KEY_EQUAL:
			return shift ? '`' : VT220_KEY_ACUTE;
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return typewriter ? (caps ? 0xC5 : 0xE5) : (shift ? ']' : '}');
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '^' : VT220_KEY_UMLAUT;
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return typewriter ? (caps ? 0xD8 : 0xF8) : (shift ? '\\' : '|');
		case GLFW_KEY_APOSTROPHE:
			return typewriter ? (caps ? 0xC6 : 0xE6) : (shift ? '[' : '{');
		case GLFW_KEY_BACKSLASH:
			return shift ? '*' : '\'';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return shift ? VT220_KEY_TILDE : VT220_KEY_GRAVE;
		case GLFW_KEY_1:
			return shift ? '1' : '&';
		case GLFW_KEY_2:
			return shift ? '2' : 0xE9;
		case GLFW_KEY_3:
			return shift ? '3' : '"';
		case GLFW_KEY_4:
			return shift ? '4' : '\'';
		case GLFW_KEY_5:
			return shift ? '5' : '(';
		case GLFW_KEY_6:
			return shift ? '6' : (typewriter ? 0xA7 : '[');
		case GLFW_KEY_7:
			return shift ? '7' : (typewriter ? 0xE8 : ']');
		case GLFW_KEY_8:
			return shift ? '8' : '!';
		case GLFW_KEY_9:
			return shift ? '9' : 0xE7;
		case GLFW_KEY_0:
			return shift ? '0' : 0xE0;
		case GLFW_KEY_MINUS:
			return shift ? 0xB0 : ')';
		case GLFW_KEY_EQUAL:
			return shift ? '_' : '-';
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'A' : 'a';
		case GLFW_KEY_W:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_CIRCUMFLEX;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '*' : '$';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 'M' : 'm';
		case GLFW_KEY_APOSTROPHE:
			return shift ? '%' : (typewriter ? 0xF9 : '\\');
		case GLFW_KEY_BACKSLASH:
			return shift ? '@' : '#';
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'W' : 'w';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return shift ? '?' : ',';
		case GLFW_KEY_COMMA:
			return shift ? '.' : ';';
		case GLFW_KEY_PERIOD:
			return shift ? '/' : ':';
		case GLFW_KEY_SLASH:
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
		case GLFW_KEY_GRAVE_ACCENT:
			return typewriter ? (shift ? 0xA1 : 0xBF) : (shift ? '\\' : '@');
		case GLFW_KEY_1:
			return shift ? '!' : '1';
		case GLFW_KEY_2:
			return shift ? '"' : '2';
		case GLFW_KEY_3:
			return shift ? '#' : '3';
		case GLFW_KEY_4:
			return shift ? '$' : '4';
		case GLFW_KEY_5:
			return shift ? '%' : '5';
		case GLFW_KEY_6:
			return shift ? '&' : '6';
		case GLFW_KEY_7:
			return shift ? '/' : '7';
		case GLFW_KEY_8:
			return shift ? '(' : '8';
		case GLFW_KEY_9:
			return shift ? ')' : '9';
		case GLFW_KEY_0:
			return shift ? '=' : '0';
		case GLFW_KEY_MINUS:
			return shift ? '?' : '\'';
		case GLFW_KEY_EQUAL:
			return typewriter ? (shift ? 0xAA : 0xBA) : (shift ? '[' : ']');
		/* Row 1 */
		case GLFW_KEY_Q:
			return caps ? 'Q' : 'q';
		case GLFW_KEY_W:
			return caps ? 'W' : 'w';
		case GLFW_KEY_E:
			return caps ? 'E' : 'e';
		case GLFW_KEY_R:
			return caps ? 'R' : 'r';
		case GLFW_KEY_T:
			return caps ? 'T' : 't';
		case GLFW_KEY_Y:
			return caps ? 'Y' : 'y';
		case GLFW_KEY_U:
			return caps ? 'U' : 'u';
		case GLFW_KEY_I:
			return caps ? 'I' : 'i';
		case GLFW_KEY_O:
			return caps ? 'O' : 'o';
		case GLFW_KEY_P:
			return caps ? 'P' : 'p';
		case GLFW_KEY_LEFT_BRACKET:
			return shift ? VT220_KEY_CIRCUMFLEX : VT220_KEY_GRAVE;
		case GLFW_KEY_RIGHT_BRACKET:
			return shift ? '*' : '+';
		/* Row 2 */
		case GLFW_KEY_A:
			return caps ? 'A' : 'a';
		case GLFW_KEY_S:
			return caps ? 'S' : 's';
		case GLFW_KEY_D:
			return caps ? 'D' : 'd';
		case GLFW_KEY_F:
			return caps ? 'F' : 'f';
		case GLFW_KEY_G:
			return caps ? 'G' : 'g';
		case GLFW_KEY_H:
			return caps ? 'H' : 'h';
		case GLFW_KEY_J:
			return caps ? 'J' : 'j';
		case GLFW_KEY_K:
			return caps ? 'K' : 'k';
		case GLFW_KEY_L:
			return caps ? 'L' : 'l';
		case GLFW_KEY_SEMICOLON:
			return caps ? 0xD1 : 0xF1;
		case GLFW_KEY_APOSTROPHE:
			return shift ? VT220_KEY_UMLAUT : VT220_KEY_ACUTE;
		case GLFW_KEY_BACKSLASH:
			return shift ? VT220_KEY_TILDE : 0xE7;
		/* Row 3 */
		case GLFW_KEY_WORLD_1:
			return shift ? '>' : '<';
		case GLFW_KEY_Z:
			return caps ? 'Z' : 'z';
		case GLFW_KEY_X:
			return caps ? 'X' : 'x';
		case GLFW_KEY_C:
			return caps ? 'C' : 'c';
		case GLFW_KEY_V:
			return caps ? 'V' : 'v';
		case GLFW_KEY_B:
			return caps ? 'B' : 'b';
		case GLFW_KEY_N:
			return caps ? 'N' : 'n';
		case GLFW_KEY_M:
			return caps ? 'M' : 'm';
		case GLFW_KEY_COMMA:
			return shift ? ';' : ',';
		case GLFW_KEY_PERIOD:
			return shift ? ':' : '.';
		case GLFW_KEY_SLASH:
			return shift ? '_' : '-';
		default:
			return VT220TranslateKey(vt, key);
	}
}

////////////////////////////////////////////////////////////////////////////////
u16 VT220KeyboardResolveCompose(VT220* vt)
{
	/* order sensitive stuff first */
	switch(vt->compose_keys[0]) {
		case '/':
			switch(vt->compose_keys[1]) {
				case 'U':
				case 'u':
					return 0xB5;
			}
			break;
		case '1':
			switch(vt->compose_keys[1]) {
				case '2':
					return 0xBD;
				case '4':
					return 0xBC;
			}
			break;
		case 'A':
			if(vt->compose_keys[1] == 'E') {
				return 0xC6;
			}
			break;
		case 'a':
			if(vt->compose_keys[1] == 'e') {
				return 0xE6;
			}
			break;
		case 'O':
			if(vt->compose_keys[1] == 'E') {
				return 0xD7;
			}
			break;
		case 'o':
			if(vt->compose_keys[1] == 'e') {
				return 0xF7;
			}
			break;
	}

	bool swap = vt->compose_keys[0] == SP;
	switch(vt->compose_keys[1]) {
		case VT220_KEY_UMLAUT:
		case VT220_KEY_ACUTE:
		case VT220_KEY_GRAVE:
		case VT220_KEY_CIRCUMFLEX:
		case VT220_KEY_TILDE:
		case '"':
		case '\'':
		case '`':
		case '^':
		case '~':
		case '/':
		case '|':
		case '-':
		case '=':
		case '!':
		case ',':
		case '*':
		case 0xB0:
			swap = true;
			break;
	}

	if(swap) {
		/* swap keys[0] and keys[1] */
		u16 tmp = vt->compose_keys[0];
		vt->compose_keys[0] = vt->compose_keys[1];
		vt->compose_keys[1] = tmp;
	}

	switch(vt->compose_keys[0]) {
		case '"':
		case VT220_KEY_UMLAUT:
			switch(vt->compose_keys[1]) {
				case ' ':
					return '"';
				case 'A':
					return 0xC4;
				case 'E':
					return 0xCB;
				case 'I':
					return 0xCF;
				case 'O':
					return 0xD6;
				case 'U':
					return 0xDC;
				case 'Y':
					return 0xDD;
				case 'a':
					return 0xE4;
				case 'e':
					return 0xEB;
				case 'i':
					return 0xEF;
				case 'o':
					return 0xF6;
				case 'u':
					return 0xFC;
				case 'y':
					return 0xFD;
			}
			break;
		case '\'':
		case VT220_KEY_ACUTE:
			switch(vt->compose_keys[1]) {
				case ' ':
					return '\'';
				case 'A':
					return 0xC1;
				case 'E':
					return 0xC9;
				case 'I':
					return 0xCD;
				case 'O':
					return 0xD3;
				case 'U':
					return 0xDA;
				case 'a':
					return 0xE1;
				case 'e':
					return 0xE9;
				case 'i':
					return 0xED;
				case 'o':
					return 0xF3;
				case 'u':
					return 0xFA;
			}
			break;
		case '`':
		case VT220_KEY_GRAVE:
			switch(vt->compose_keys[1]) {
				case ' ':
					return '`';
				case 'A':
					return 0xC0;
				case 'E':
					return 0xC8;
				case 'I':
					return 0xCC;
				case 'O':
					return 0xD2;
				case 'U':
					return 0xD9;
				case 'a':
					return 0xE0;
				case 'e':
					return 0xE8;
				case 'i':
					return 0xEC;
				case 'o':
					return 0xF2;
				case 'u':
					return 0xF9;
			}
			break;
		case '^':
			switch(vt->compose_keys[1]) {
				case '/':
					return '|';
				case '.':
					return 0xB7;
				case '0':
					return 0xB0;
				case '1':
					return 0xB9;
				case '2':
					return 0xB2;
				case '3':
					return 0xB3;
			}
			/* fallthrough */
		case VT220_KEY_CIRCUMFLEX:
			switch(vt->compose_keys[1]) {
				case ' ':
					return '^';
				case 'A':
					return 0xC2;
				case 'E':
					return 0xCA;
				case 'I':
					return 0xCE;
				case 'O':
					return 0xD4;
				case 'U':
					return 0xDB;
				case 'a':
					return 0xE2;
				case 'e':
					return 0xEA;
				case 'i':
					return 0xEE;
				case 'o':
					return 0xF4;
				case 'u':
					return 0xFB;
			}
			break;
		case '~':
		case VT220_KEY_TILDE:
			switch(vt->compose_keys[1]) {
				case ' ':
					return '~';
				case 'A':
					return 0xC3;
				case 'N':
					return 0xD1;
				case 'a':
					return 0xE3;
				case 'n':
					return 0xF1;
			}
			break;
		case '*':
		case 0xB0: /* ring */
			switch(vt->compose_keys[1]) {
				case 'A':
					return 0xC5;
				case 'a':
					return 0xE5;
			}
			break;
		case ' ':
			switch(vt->compose_keys[1]) {
				case '#':
				case 0xB0:
					return 0xB0;
			}
			break;
		case ',':
			switch(vt->compose_keys[1]) {
				case 'C':
					return 0xC7;
				case 'c':
					return 0xE7;
			}
			break;
		case '+':
			switch(vt->compose_keys[1]) {
				case '+':
					return '#';
			}
			break;
		case 'A':
			switch(vt->compose_keys[1]) {
				case 'A':
					return '@';
			}
			break;
		case 'a':
			switch(vt->compose_keys[1]) {
				case 'a':
					return '@';
			}
			break;
		case '/':
			switch(vt->compose_keys[1]) {
				case '/':
				case '<':
					return '\\';
				case '^':
					return '|';
				case 'c':
				case 'C':
					return 0xA2;
				case 'O':
					return 0xD8;
				case 'o':
					return 0xF8;
			}
			break;
		case '|':
			switch(vt->compose_keys[1]) {
				case 'c':
				case 'C':
					return 0xA2;
			}
			break;
		case '-':
			switch(vt->compose_keys[1]) {
				case '(':
					return '{';
				case ')':
					return '}';
				case 'l':
				case 'L':
					return 0xA3;
				case 'y':
				case 'Y':
					return 0xA5;
				case 'a':
				case 'A':
					return 0xAA;
				case 'o':
				case 'O':
					return 0xBA;
				case '+':
					return 0xB1;
			}
			break;
		case '=':
			switch(vt->compose_keys[1]) {
				case 'l':
				case 'L':
					return 0xA3;
				case 'y':
				case 'Y':
					return 0xA5;
			}
			break;
		case '(':
			switch(vt->compose_keys[1]) {
				case '(':
					return '[';
				case '-':
					return '{';
			}
			break;
		case ')':
			switch(vt->compose_keys[1]) {
				case ')':
					return ']';
				case '-':
					return '}';
			}
			break;
		case '!':
			switch(vt->compose_keys[1]) {
				case '!':
					return 0xA1;
				case 'S':
				case 's':
					return 0xA7;
				case 'P':
				case 'p':
					return 0xB6;
			}
			break;
		case '<':
			switch(vt->compose_keys[1]) {
				case '<':
					return 0xAB;
			}
			break;
		case '>':
			switch(vt->compose_keys[1]) {
				case '>':
					return 0xBB;
			}
			break;
		case '?':
			switch(vt->compose_keys[1]) {
				case '?':
					return 0xBF;
			}
			break;
		case 'X':
		case 'x':
			switch(vt->compose_keys[1]) {
				case 'O':
				case 'o':
					return 0xA8;
			}
			break;
		case 'C':
		case 'c':
			switch(vt->compose_keys[1]) {
				case 'O':
				case 'o':
					return 0xA9;
			}
			break;
		case 'S':
			switch(vt->compose_keys[1]) {
				case 'O':
				case 'o':
					return 0xA7;
			}
			break;
		case 's':
			switch(vt->compose_keys[1]) {
				case 'O':
				case 'o':
					return 0xA7;
				case 's':
					return 0xDF;
			}
			break;
		case 'O':
		case 'o':
		case '0':
			switch(vt->compose_keys[1]) {
				case 'C':
				case 'c':
					return 0xA9;
				case 'S':
				case 's':
					return 0xA7;
			}
			break;
	}

	VT220Bell(vt);
	return 0xFFFF;
}

void VT220KeyboardKeyDownLocal(VT220* vt, int key)
{
	u32 last_modifiers = vt->modifiers;
	bool ctrl = (vt->modifiers & VT220_MODIFIER_CTRL_L) || (vt->modifiers & VT220_MODIFIER_CTRL_R);

	switch(key) {
		case GLFW_KEY_LEFT_SHIFT:
			if(vt->config.lock == VT220_LOCK_SHIFT_LOCK) {
				vt->modifiers &= ~VT220_MODIFIER_LOCK;
			}
			vt->modifiers |= VT220_MODIFIER_SHIFT_L;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			if(vt->config.lock == VT220_LOCK_SHIFT_LOCK) {
				vt->modifiers &= ~VT220_MODIFIER_LOCK;
			}
			vt->modifiers |= VT220_MODIFIER_SHIFT_R;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			vt->modifiers |= VT220_MODIFIER_CTRL_L;
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			vt->modifiers |= VT220_MODIFIER_CTRL_R;
			break;
		case GLFW_KEY_LEFT_ALT:
			vt->modifiers |= VT220_MODIFIER_ALT_L;
			break;
		case GLFW_KEY_RIGHT_ALT:
			vt->modifiers |= VT220_MODIFIER_ALT_R;
			break;
		case GLFW_KEY_CAPS_LOCK:
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

			switch(value) {
				case VT220_KEY_COMPOSE:
					vt->compose_state = COMPOSE_FIRST;
					VT220KeyboardUpdateLEDs(vt);
					break;
				case CR:
				case VT220_KEY_F6:
				case VT220_KEY_F7:
				case VT220_KEY_F8:
				case VT220_KEY_F9:
				case VT220_KEY_F10:
				case VT220_KEY_F11:
				case VT220_KEY_F12:
				case VT220_KEY_F13:
				case VT220_KEY_F14:
				case VT220_KEY_F15:
				case VT220_KEY_F16:
				case VT220_KEY_F17:
				case VT220_KEY_F18:
				case VT220_KEY_F19:
				case VT220_KEY_F20:
				case VT220_KEY_F6_UDK:
				case VT220_KEY_F7_UDK:
				case VT220_KEY_F8_UDK:
				case VT220_KEY_F9_UDK:
				case VT220_KEY_F10_UDK:
				case VT220_KEY_F11_UDK:
				case VT220_KEY_F12_UDK:
				case VT220_KEY_F13_UDK:
				case VT220_KEY_F14_UDK:
				case VT220_KEY_F15_UDK:
				case VT220_KEY_F16_UDK:
				case VT220_KEY_F17_UDK:
				case VT220_KEY_F18_UDK:
				case VT220_KEY_F19_UDK:
				case VT220_KEY_F20_UDK:
				case VT220_KEY_KP_0:
				case VT220_KEY_KP_1:
				case VT220_KEY_KP_2:
				case VT220_KEY_KP_3:
				case VT220_KEY_KP_4:
				case VT220_KEY_KP_5:
				case VT220_KEY_KP_6:
				case VT220_KEY_KP_7:
				case VT220_KEY_KP_8:
				case VT220_KEY_KP_9:
				case VT220_KEY_KP_PF1:
				case VT220_KEY_KP_PF2:
				case VT220_KEY_KP_PF3:
				case VT220_KEY_KP_PF4:
				case VT220_KEY_KP_MINUS:
				case VT220_KEY_KP_PERIOD:
				case VT220_KEY_KP_COMMA:
				case VT220_KEY_KP_ENTER:
					vt->compose_state = COMPOSE_NONE;
					VT220KeyboardUpdateLEDs(vt);
					break;
				default:
					/* DEL cancels any compose sequence without sending anything to the host */
					if(vt->compose_state != COMPOSE_NONE && value == DEL) {
						vt->compose_state = COMPOSE_NONE;
						VT220KeyboardUpdateLEDs(vt);
						return;
					} else if(vt->compose_state == COMPOSE_NONE) {
						/* a dead key starts a two-stroke compose sequence (only if there is no
						 * active sequence) */
						switch(value) {
							case VT220_KEY_ACUTE:
							case VT220_KEY_GRAVE:
							case VT220_KEY_CIRCUMFLEX:
							case VT220_KEY_UMLAUT:
							case VT220_KEY_TILDE:
								vt->compose_state = COMPOSE_SECOND;
								vt->compose_keys[0] = value;
								VT220KeyboardUpdateLEDs(vt);
								break;
						}
					} else if(vt->compose_state == COMPOSE_FIRST) {
						/* the first character in the sequence */
						vt->compose_keys[0] = value;
						vt->compose_state = COMPOSE_SECOND;
					} else if(vt->compose_state == COMPOSE_SECOND) {
						/* the second and final character in the sequence */
						vt->compose_keys[1] = value;
						vt->compose_state = COMPOSE_NONE;
						VT220KeyboardUpdateLEDs(vt);
						value = VT220KeyboardResolveCompose(vt);
						if(value == 0xFFFF) {
							return;
						}
					}
					break;
			}

			if(vt->compose_state != COMPOSE_NONE) {
				return;
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

void VT220KeyboardKeyUpLocal(VT220* vt, int key)
{
	switch(key) {
		case GLFW_KEY_LEFT_SHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_L;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			vt->modifiers &= ~VT220_MODIFIER_SHIFT_R;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_L;
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			vt->modifiers &= ~VT220_MODIFIER_CTRL_R;
			break;
		case GLFW_KEY_LEFT_ALT:
			vt->modifiers &= ~VT220_MODIFIER_ALT_L;
			break;
		case GLFW_KEY_RIGHT_ALT:
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

void VT220KeyboardKeyDown(VT220* vt, int key)
{
	if(vt->config.keyboard == VT220_KEYBOARD_UNKNOWN) {
		VT220KeyboardKeyDownHost(vt, key);
	} else {
		VT220KeyboardKeyDownLocal(vt, key);
	}
}

void VT220KeyboardKeyUp(VT220* vt, int key)
{
	if(vt->config.keyboard == VT220_KEYBOARD_UNKNOWN) {
		VT220KeyboardKeyUpHost(vt, key);
	} else {
		VT220KeyboardKeyUpLocal(vt, key);
	}
}
