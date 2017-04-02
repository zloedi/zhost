#include "zhost.h"

static const char* i_buttonNames[256];

v2_t i_mouseState;

void I_UpdateState( v2_t mousePosition ) {
    i_mouseState = mousePosition;
}

v2_t I_GetMousePosition( void ) {
    return i_mouseState;
}

void I_Init( void ) {

	// some keys dont have names, thus are hidden for all mapping

	//i_buttonNames[SDLK_MOUSE_BUTTON1] = "mouse left button";
	//i_buttonNames[SDLK_MOUSE_BUTTON2] = "mouse middle button";
	//i_buttonNames[SDLK_MOUSE_BUTTON3] = "mouse right button";
	//i_buttonNames[SDLK_MOUSE_BUTTON4] = "mouse roll up";
	//i_buttonNames[SDLK_MOUSE_BUTTON5] = "mouse roll down";

	i_buttonNames[SDLK_BACKSPACE] = "Backspace";
	i_buttonNames[SDLK_TAB] = "Tab";
	i_buttonNames[SDLK_CLEAR] = "Clear";
	i_buttonNames[SDLK_RETURN] = "Return";
	i_buttonNames[SDLK_PAUSE] = "Pause";
	i_buttonNames[SDLK_ESCAPE] = "Escape";
	i_buttonNames[SDLK_SPACE] = "Space";
	i_buttonNames[SDLK_EXCLAIM] = "!";
	i_buttonNames[SDLK_QUOTEDBL] = "\"";
	i_buttonNames[SDLK_HASH] = "#";
	i_buttonNames[SDLK_DOLLAR] = "$";
	i_buttonNames[SDLK_AMPERSAND] = "&";
	i_buttonNames[SDLK_QUOTE] = "'";
	i_buttonNames[SDLK_LEFTPAREN] = "(";
	i_buttonNames[SDLK_RIGHTPAREN] = ")";
	i_buttonNames[SDLK_ASTERISK] = "*";
	i_buttonNames[SDLK_PLUS] = "+";
	i_buttonNames[SDLK_COMMA] = ",";
	i_buttonNames[SDLK_MINUS] = "-";
	i_buttonNames[SDLK_PERIOD] = ".";
	i_buttonNames[SDLK_SLASH] = "/";

	i_buttonNames[SDLK_0] = "0";
	i_buttonNames[SDLK_1] = "1";
	i_buttonNames[SDLK_2] = "2";
	i_buttonNames[SDLK_3] = "3";
	i_buttonNames[SDLK_4] = "4";
	i_buttonNames[SDLK_5] = "5";
	i_buttonNames[SDLK_6] = "6";
	i_buttonNames[SDLK_7] = "7";
	i_buttonNames[SDLK_8] = "8";
	i_buttonNames[SDLK_9] = "9";
	i_buttonNames[SDLK_COLON] = ":";
	i_buttonNames[SDLK_SEMICOLON] = ";";
	i_buttonNames[SDLK_LESS] = "<";
	i_buttonNames[SDLK_EQUALS] = "=";
	i_buttonNames[SDLK_GREATER] = ">";
	i_buttonNames[SDLK_QUESTION] = "?";
	i_buttonNames[SDLK_AT] = "@";

	i_buttonNames[SDLK_LEFTBRACKET] = "[";
	i_buttonNames[SDLK_BACKSLASH] = "\\";
	i_buttonNames[SDLK_RIGHTBRACKET] = "]";
	i_buttonNames[SDLK_CARET] = "^";
	i_buttonNames[SDLK_UNDERSCORE] = "_";
	i_buttonNames[SDLK_BACKQUOTE] = "`";
	i_buttonNames[SDLK_a] = "A";
	i_buttonNames[SDLK_b] = "B";
	i_buttonNames[SDLK_c] = "C";
	i_buttonNames[SDLK_d] = "D";
	i_buttonNames[SDLK_e] = "E";
	i_buttonNames[SDLK_f] = "F";
	i_buttonNames[SDLK_g] = "G";
	i_buttonNames[SDLK_h] = "H";
	i_buttonNames[SDLK_i] = "I";
	i_buttonNames[SDLK_j] = "J";
	i_buttonNames[SDLK_k] = "K";
	i_buttonNames[SDLK_l] = "L";
	i_buttonNames[SDLK_m] = "M";
	i_buttonNames[SDLK_n] = "N";
	i_buttonNames[SDLK_o] = "O";
	i_buttonNames[SDLK_p] = "P";
	i_buttonNames[SDLK_q] = "Q";
	i_buttonNames[SDLK_r] = "R";
	i_buttonNames[SDLK_s] = "S";
	i_buttonNames[SDLK_t] = "T";
	i_buttonNames[SDLK_u] = "U";
	i_buttonNames[SDLK_v] = "V";
	i_buttonNames[SDLK_w] = "W";
	i_buttonNames[SDLK_x] = "X";
	i_buttonNames[SDLK_y] = "Y";
	i_buttonNames[SDLK_z] = "Z";
	i_buttonNames[SDLK_DELETE] = "Delete";

	i_buttonNames[SDLK_KP_0] = "Keypad 0";
	i_buttonNames[SDLK_KP_1] = "Keypad 1";
	i_buttonNames[SDLK_KP_2] = "Keypad 2";
	i_buttonNames[SDLK_KP_3] = "Keypad 3";
	i_buttonNames[SDLK_KP_4] = "Keypad 4";
	i_buttonNames[SDLK_KP_5] = "Keypad 5";
	i_buttonNames[SDLK_KP_6] = "Keypad 6";
	i_buttonNames[SDLK_KP_7] = "Keypad 7";
	i_buttonNames[SDLK_KP_8] = "Keypad 8";
	i_buttonNames[SDLK_KP_9] = "Keypad 9";
	i_buttonNames[SDLK_KP_PERIOD] = "Keypad .";
	i_buttonNames[SDLK_KP_DIVIDE] = "Keypad /";
	i_buttonNames[SDLK_KP_MULTIPLY] = "Keypad *";
	i_buttonNames[SDLK_KP_MINUS] = "Keypad -";
	i_buttonNames[SDLK_KP_PLUS] = "Keypad +";
	i_buttonNames[SDLK_KP_ENTER] = "Keypad Enter";
	i_buttonNames[SDLK_KP_EQUALS] = "Keypad =";

	i_buttonNames[SDLK_UP] = "Up";
	i_buttonNames[SDLK_DOWN] = "Down";
	i_buttonNames[SDLK_RIGHT] = "Right";
	i_buttonNames[SDLK_LEFT] = "Left";
	i_buttonNames[SDLK_INSERT] = "Insert";
	i_buttonNames[SDLK_HOME] = "Home";
	i_buttonNames[SDLK_END] = "End";
	i_buttonNames[SDLK_PAGEUP] = "Page Up";
	i_buttonNames[SDLK_PAGEDOWN] = "Page Down";

	i_buttonNames[SDLK_F1] = "F1";
	i_buttonNames[SDLK_F2] = "F2";
	i_buttonNames[SDLK_F3] = "F3";
	i_buttonNames[SDLK_F4] = "F4";
	i_buttonNames[SDLK_F5] = "F5";
	i_buttonNames[SDLK_F6] = "F6";
	i_buttonNames[SDLK_F7] = "F7";
	i_buttonNames[SDLK_F8] = "F8";
	i_buttonNames[SDLK_F9] = "F9";
	i_buttonNames[SDLK_F10] = "F10";
	i_buttonNames[SDLK_F11] = "F11";
	i_buttonNames[SDLK_F12] = "F12";
	i_buttonNames[SDLK_F13] = "F13";
	i_buttonNames[SDLK_F14] = "F14";
	i_buttonNames[SDLK_F15] = "F15";

	i_buttonNames[SDLK_NUMLOCKCLEAR] = "Num Lock";
	i_buttonNames[SDLK_CAPSLOCK] = "Caps Lock";
	i_buttonNames[SDLK_SCROLLLOCK] = "Scroll Lock";
	i_buttonNames[SDLK_RSHIFT] = "Right Shift";
	i_buttonNames[SDLK_LSHIFT] = "Left Shift";
	i_buttonNames[SDLK_RCTRL] = "Right Ctl";
	i_buttonNames[SDLK_LCTRL] = "Left Ctl";
	i_buttonNames[SDLK_RALT] = "Right Alt";
	i_buttonNames[SDLK_LALT] = "Left Alt";
	
	i_buttonNames[SDLK_MODE] = "Mode";

	i_buttonNames[SDLK_HELP] = "Help";
	i_buttonNames[SDLK_SYSREQ] = "SysRq";
	i_buttonNames[SDLK_MENU] = "Menu";
	i_buttonNames[SDLK_POWER] = "Power";
	i_buttonNames[SDLK_UNDO] = "Undo";

	CON_Printf( "Key code to key name table filled.\n" );
}
