#include "zhost.h"

#define I_MAX_CONTEXTS 8

enum {
    IB_NONE,

    IB_BACKSPACE,
    IB_TAB,
    IB_CLEAR,
    IB_RETURN,
    IB_PAUSE,
    IB_ESCAPE,
    IB_SPACE,
    IB_EXCLAIM,
    IB_QUOTEDBL,
    IB_HASH,
    IB_DOLLAR,
    IB_AMPERSAND,
    IB_QUOTE,
    IB_LEFTPAREN,
    IB_RIGHTPAREN,
    IB_ASTERISK,
    IB_PLUS,
    IB_COMMA,
    IB_MINUS,
    IB_PERIOD,
    IB_SLASH,

    IB_0,
    IB_1,
    IB_2,
    IB_3,
    IB_4,
    IB_5,
    IB_6,
    IB_7,
    IB_8,
    IB_9,
    IB_COLON,
    IB_SEMICOLON,
    IB_LESS,
    IB_EQUALS,
    IB_GREATER,
    IB_QUESTION,
    IB_AT,

    IB_LEFTBRACKET,
    IB_BACKSLASH,
    IB_RIGHTBRACKET,
    IB_CARET,
    IB_UNDERSCORE,
    IB_BACKQUOTE,
    IB_a,
    IB_b,
    IB_c,
    IB_d,
    IB_e,
    IB_f,
    IB_g,
    IB_h,
    IB_i,
    IB_j,
    IB_k,
    IB_l,
    IB_m,
    IB_n,
    IB_o,
    IB_p,
    IB_q,
    IB_r,
    IB_s,
    IB_t,
    IB_u,
    IB_v,
    IB_w,
    IB_x,
    IB_y,
    IB_z,
    IB_DELETE,

    IB_KP_0,
    IB_KP_1,
    IB_KP_2,
    IB_KP_3,
    IB_KP_4,
    IB_KP_5,
    IB_KP_6,
    IB_KP_7,
    IB_KP_8,
    IB_KP_9,
    IB_KP_PERIOD,
    IB_KP_DIVIDE,
    IB_KP_MULTIPLY,
    IB_KP_MINUS,
    IB_KP_PLUS,
    IB_KP_ENTER,
    IB_KP_EQUALS,

    IB_UP,
    IB_DOWN,
    IB_RIGHT,
    IB_LEFT,
    IB_INSERT,
    IB_HOME,
    IB_END,
    IB_PAGEUP,
    IB_PAGEDOWN,

    IB_F1,
    IB_F2,
    IB_F3,
    IB_F4,
    IB_F5,
    IB_F6,
    IB_F7,
    IB_F8,
    IB_F9,
    IB_F10,
    IB_F11,
    IB_F12,
    IB_F13,
    IB_F14,
    IB_F15,

    IB_NUMLOCKCLEAR,
    IB_CAPSLOCK,
    IB_SCROLLLOCK,
    IB_RSHIFT,
    IB_LSHIFT,
    IB_RCTRL,
    IB_LCTRL,
    IB_RALT,
    IB_LALT,

    IB_MODE,

    IB_HELP,
    IB_SYSREQ,
    IB_MENU,
    IB_POWER,
    IB_UNDO,

    // Keep these in this order
    IB_MOUSE_LEFT,
    IB_MOUSE_MIDDLE,
    IB_MOUSE_RIGHT,
    IB_MOUSE_BUTTON4,
    IB_MOUSE_BUTTON5,

    // Keep these in this order
    IB_JOY_AXES,
    IB_JOY_BUTTONS = IB_JOY_AXES + I_MAX_AXES * I_MAX_JOYSTICKS,
    IB_JOY_HAXES = IB_JOY_BUTTONS + I_MAX_BUTTONS * I_MAX_JOYSTICKS,

    IB_NUM_BUTTONS = IB_JOY_HAXES + 2 * I_MAX_JOYSTICKS,
};

static const char *i_buttonNames[IB_NUM_BUTTONS];
static char *i_binds[I_MAX_CONTEXTS][IB_NUM_BUTTONS];
static v2_t i_mousePositionV;
static c2_t i_mousePositionC;
static SDL_GameController *i_controllers[I_MAX_JOYSTICKS];
static SDL_Joystick *i_joysticks[I_MAX_JOYSTICKS];
static int i_joystickDeadZone;
static var_t *i_logCommands;

//static bool_t I_IsMouseButton( int button ) {
//    return button >= IB_MOUSE_LEFT && button < IB_JOY_AXES;
//}

//static bool_t I_IsKeyButton( int button ) {
//    return button < IB_MOUSE_LEFT;
//}

bool_t I_IsJoystickCode( int button ) {
    return button >= IB_JOY_AXES;
}

static bool_t I_IsJoyAxis( int button ) {
    return button >= IB_JOY_AXES && button < IB_JOY_BUTTONS;
}

static bool_t I_IsJoyButton( int button ) {
    return button >= IB_JOY_BUTTONS && button < IB_JOY_HAXES;
}

static bool_t I_IsJoyHaxis( int button ) {
    return button >= IB_JOY_HAXES;
}

int I_DeviceOfCode( int button ) {
    if ( I_IsJoyAxis( button ) ) {
        return ( button - IB_JOY_AXES ) / I_MAX_AXES;
    }
    if ( I_IsJoyHaxis( button ) ) {
        return ( button - IB_JOY_HAXES ) / 2;
    }
    if ( I_IsJoyButton( button ) ) {
        return ( button - IB_JOY_BUTTONS ) / I_MAX_BUTTONS;
    }
    return 0;
}

static void I_Bind_f( void ) {
    if ( CMD_Argc() < 3 ) {
        CON_Printf( "usage: i_bind <button_name> <command> [context]\n" );
        return;
    }

    int context = atoi( CMD_Argv( 3 ) );
    I_BindContext( CMD_Argv( 1 ), CMD_Argv( 2 ), context );
}

void I_UpdateMousePosition( c2_t mousePosition ) {
    i_mousePositionC = mousePosition;
    i_mousePositionV = v2c2( mousePosition );
}

c2_t I_GetMousePositionC( void ) {
    return i_mousePositionC;
}

v2_t I_GetMousePositionV( void ) {
    return i_mousePositionV;
}

static void I_CloseAllControllers( void ) {
    for ( int i = 0; i < I_MAX_JOYSTICKS; i++ ) {
        if ( i_controllers[i] ) {
            SDL_GameControllerClose( i_controllers[i] );
            i_controllers[i] = NULL;
        }
        if ( i_joysticks[i] ) {
            SDL_JoystickClose( i_joysticks[i] );
            i_joysticks[i] = NULL;
        }
    }
    CON_Printf( "Joysticks closed.\n" );
}

static void I_OpenAllControllers( void ) {
    int numJoys = SDL_NumJoysticks();
    CON_Printf( "Num joysticks found: %d\n", numJoys );
    for( int joy = 0, ctrl = 0; joy < numJoys; ++joy ) {
        if ( SDL_IsGameController( joy ) ) {
            CON_Printf( "   Found controller %s\n", SDL_JoystickNameForIndex( joy ) );
            if ( ctrl < I_MAX_JOYSTICKS ) {
                i_controllers[ctrl] = SDL_GameControllerOpen( joy );
                ctrl++;
            }
        } else {
            CON_Printf( "   Found joystick %s\n", SDL_JoystickNameForIndex( joy ) );
            if ( joy < I_MAX_JOYSTICKS ) {
                i_joysticks[joy] = SDL_JoystickOpen( joy );
            }
        }
    }
}

void I_AddController( int id ) {
    CON_Printf( "Added controller: %d\n", id );
}

void I_RemoveController( int id ) {
    CON_Printf( "Removed controller: %d\n", id );
}

static void I_TokenizeAndExecute( int code, int context, bool_t engage, bool_t value ) {
    for ( const char *data = COM_Token( i_binds[context][code] ); data; data = COM_Token( data ) ) {
        // FIXME: allow command arguments in binds
        if ( com_token[0] != ';' ) {
            const char *cmd = CMD_FromBind( com_token, 
                                             I_IsJoystickCode( code ), 
                                             I_DeviceOfCode( code ), 
                                             engage, value );
            if ( cmd ) {
                if ( VAR_Num( i_logCommands ) ) {
                    CON_Printf( "Execute command: %s\n", cmd );
                }
                CMD_ExecuteString( cmd );
            }
        }
        data = COM_Token( data );
    }
}

int I_JoystickButtonToButton( int device, int button, int context ) {
    if ( device >= I_MAX_JOYSTICKS ) {
        CON_Printf( "I_JoystickButtonToButton: device %d is out of range\n", device );
        return IB_NONE;
    }
    if ( button >= I_MAX_BUTTONS ) {
        CON_Printf( "I_JoystickButtonToButton: button %d is out of range\n", button );
        return IB_NONE;
    }
    return IB_JOY_BUTTONS + device * I_MAX_BUTTONS + button;
}

void I_OnJoystickButton( int code, bool_t down, int context ) {
    I_TokenizeAndExecute( code, context, down, I_AXIS_MAX_VALUE );
}

void I_OnJoystickHaxis( int device, int axis, int value, int context ) {
    int code = IB_JOY_HAXES + device * 2 + axis;
    I_TokenizeAndExecute( code, context, value != 0, value );
}

void I_SetJoystickDeadZone( int val ) {
    i_joystickDeadZone = val;
}

void I_OnJoystickAxis( int device, int axis, int value, int context ) {
    int code = IB_NONE;
    if ( device >= I_MAX_JOYSTICKS ) {
        CON_Printf( "I_OnJoystickAxis: device %d is out of range\n", device );
        return;
    }
    if ( axis >= I_MAX_AXES ) {
        CON_Printf( "I_OnJoystickAxis: axis %d is out of range\n", axis );
        return;
    }
    code = IB_JOY_AXES + device * I_MAX_AXES + axis;
    int deadZone = Clampi( i_joystickDeadZone, 0, I_AXIS_MAX_VALUE * 90 / 100 );
    bool_t engage = abs( value ) > deadZone;
    I_TokenizeAndExecute( code, context, engage, value );
}

void I_Bind( const char *button, const char *cmd ) {
    I_BindContext( button, cmd, 0 );
}

void I_BindContext( const char *button, const char *cmd, int context ) {
    if ( context < 0 || context >= I_MAX_CONTEXTS ) {
        CON_Printf( "I_BindContext: invalid context: %d\n", context );
        return;
    }
    for ( int i = 0; i < IB_NUM_BUTTONS; i++ ) {
        const char *bname = i_buttonNames[i];
        if ( bname && ! stricmp( bname, button ) ) {
            char *oldCmd = i_binds[context][i];
            A_Free( oldCmd );
            i_binds[context][i] = A_StrDup( cmd );
            CON_Printf( "button \"%s\" bound to command \"%s\"\n", button, cmd );
            return;
        }
    }
    
    CON_Printf( "I_BindContext: Cant find button named \"%s\"\n", button );
}

void I_WriteBinds( FILE *f ) {
    int i, j;
    for ( j = 0; j < I_MAX_CONTEXTS; j++ ) {
        for ( i = 0; i < IB_NUM_BUTTONS; i++ ) {
            const char *button = i_buttonNames[i];
            const char *cmd = i_binds[j][i];
            if ( button && cmd ) {
                fprintf( f, "i_Bind \"%s\" \"%s\" %d\n", button, cmd, j );
            }
        }
    }
}

void I_OnButton( int code, int down, int context ) {
    I_TokenizeAndExecute( code, context, down, I_AXIS_MAX_VALUE );
}

int I_MouseButtonToButton( int sdlButton ) {
    switch( sdlButton ) {
        case SDL_BUTTON_LEFT: return IB_MOUSE_LEFT;
        case SDL_BUTTON_MIDDLE: return IB_MOUSE_MIDDLE;
        case SDL_BUTTON_RIGHT: return IB_MOUSE_RIGHT;
        case SDL_BUTTON_X1: return IB_MOUSE_BUTTON4;
        case SDL_BUTTON_X2: return IB_MOUSE_BUTTON5;
    };
    return IB_NONE;
}

int I_KeyToButton( int sdlKey ) {
    switch ( sdlKey ) {
        case SDLK_BACKSPACE:                        return IB_BACKSPACE;
        case SDLK_TAB:                              return IB_TAB;
        case SDLK_CLEAR:                            return IB_CLEAR;
        case SDLK_RETURN:                           return IB_RETURN;
        case SDLK_PAUSE:                            return IB_PAUSE;
        case SDLK_ESCAPE:                           return IB_ESCAPE;
        case SDLK_SPACE:                            return IB_SPACE;
        case SDLK_EXCLAIM:                          return IB_EXCLAIM;
        case SDLK_QUOTEDBL:                         return IB_QUOTEDBL;
        case SDLK_HASH:                             return IB_HASH;
        case SDLK_DOLLAR:                           return IB_DOLLAR;
        case SDLK_AMPERSAND:                        return IB_AMPERSAND;
        case SDLK_QUOTE:                            return IB_QUOTE;
        case SDLK_LEFTPAREN:                        return IB_LEFTPAREN;
        case SDLK_RIGHTPAREN:                       return IB_RIGHTPAREN;
        case SDLK_ASTERISK:                         return IB_ASTERISK;
        case SDLK_PLUS:                             return IB_PLUS;
        case SDLK_COMMA:                            return IB_COMMA;
        case SDLK_MINUS:                            return IB_MINUS;
        case SDLK_PERIOD:                           return IB_PERIOD;
        case SDLK_SLASH:                            return IB_SLASH;

        case SDLK_0:                                return IB_0;
        case SDLK_1:                                return IB_1;
        case SDLK_2:                                return IB_2;
        case SDLK_3:                                return IB_3;
        case SDLK_4:                                return IB_4;
        case SDLK_5:                                return IB_5;
        case SDLK_6:                                return IB_6;
        case SDLK_7:                                return IB_7;
        case SDLK_8:                                return IB_8;
        case SDLK_9:                                return IB_9;
        case SDLK_COLON:                            return IB_COLON;
        case SDLK_SEMICOLON:                        return IB_SEMICOLON;
        case SDLK_LESS:                             return IB_LESS;
        case SDLK_EQUALS:                           return IB_EQUALS;
        case SDLK_GREATER:                          return IB_GREATER;
        case SDLK_QUESTION:                         return IB_QUESTION;
        case SDLK_AT:                               return IB_AT;

        case SDLK_LEFTBRACKET:                      return IB_LEFTBRACKET;
        case SDLK_BACKSLASH:                        return IB_BACKSLASH;
        case SDLK_RIGHTBRACKET:                     return IB_RIGHTBRACKET;
        case SDLK_CARET:                            return IB_CARET;
        case SDLK_UNDERSCORE:                       return IB_UNDERSCORE;
        case SDLK_BACKQUOTE:                        return IB_BACKQUOTE;
        case SDLK_a:                                return IB_a;
        case SDLK_b:                                return IB_b;
        case SDLK_c:                                return IB_c;
        case SDLK_d:                                return IB_d;
        case SDLK_e:                                return IB_e;
        case SDLK_f:                                return IB_f;
        case SDLK_g:                                return IB_g;
        case SDLK_h:                                return IB_h;
        case SDLK_i:                                return IB_i;
        case SDLK_j:                                return IB_j;
        case SDLK_k:                                return IB_k;
        case SDLK_l:                                return IB_l;
        case SDLK_m:                                return IB_m;
        case SDLK_n:                                return IB_n;
        case SDLK_o:                                return IB_o;
        case SDLK_p:                                return IB_p;
        case SDLK_q:                                return IB_q;
        case SDLK_r:                                return IB_r;
        case SDLK_s:                                return IB_s;
        case SDLK_t:                                return IB_t;
        case SDLK_u:                                return IB_u;
        case SDLK_v:                                return IB_v;
        case SDLK_w:                                return IB_w;
        case SDLK_x:                                return IB_x;
        case SDLK_y:                                return IB_y;
        case SDLK_z:                                return IB_z;
        case SDLK_DELETE:                           return IB_DELETE;

        case SDLK_KP_0:                             return IB_KP_0;
        case SDLK_KP_1:                             return IB_KP_1;
        case SDLK_KP_2:                             return IB_KP_2;
        case SDLK_KP_3:                             return IB_KP_3;
        case SDLK_KP_4:                             return IB_KP_4;
        case SDLK_KP_5:                             return IB_KP_5;
        case SDLK_KP_6:                             return IB_KP_6;
        case SDLK_KP_7:                             return IB_KP_7;
        case SDLK_KP_8:                             return IB_KP_8;
        case SDLK_KP_9:                             return IB_KP_9;
        case SDLK_KP_PERIOD:                        return IB_KP_PERIOD;
        case SDLK_KP_DIVIDE:                        return IB_KP_DIVIDE;
        case SDLK_KP_MULTIPLY:                      return IB_KP_MULTIPLY;
        case SDLK_KP_MINUS:                         return IB_KP_MINUS;
        case SDLK_KP_PLUS:                          return IB_KP_PLUS;
        case SDLK_KP_ENTER:                         return IB_KP_ENTER;
        case SDLK_KP_EQUALS:                        return IB_KP_EQUALS;

        case SDLK_UP:                               return IB_UP;
        case SDLK_DOWN:                             return IB_DOWN;
        case SDLK_RIGHT:                            return IB_RIGHT;
        case SDLK_LEFT:                             return IB_LEFT;
        case SDLK_INSERT:                           return IB_INSERT;
        case SDLK_HOME:                             return IB_HOME;
        case SDLK_END:                              return IB_END;
        case SDLK_PAGEUP:                           return IB_PAGEUP;
        case SDLK_PAGEDOWN:                         return IB_PAGEDOWN;

        case SDLK_F1:                               return IB_F1;
        case SDLK_F2:                               return IB_F2;
        case SDLK_F3:                               return IB_F3;
        case SDLK_F4:                               return IB_F4;
        case SDLK_F5:                               return IB_F5;
        case SDLK_F6:                               return IB_F6;
        case SDLK_F7:                               return IB_F7;
        case SDLK_F8:                               return IB_F8;
        case SDLK_F9:                               return IB_F9;
        case SDLK_F10:                              return IB_F10;
        case SDLK_F11:                              return IB_F11;
        case SDLK_F12:                              return IB_F12;
        case SDLK_F13:                              return IB_F13;
        case SDLK_F14:                              return IB_F14;
        case SDLK_F15:                              return IB_F15;

        case SDLK_NUMLOCKCLEAR:                     return IB_NUMLOCKCLEAR;
        case SDLK_CAPSLOCK:                         return IB_CAPSLOCK;
        case SDLK_SCROLLLOCK:                       return IB_SCROLLLOCK;
        case SDLK_RSHIFT:                           return IB_RSHIFT;
        case SDLK_LSHIFT:                           return IB_LSHIFT;
        case SDLK_RCTRL:                            return IB_RCTRL;
        case SDLK_LCTRL:                            return IB_LCTRL;
        case SDLK_RALT:                             return IB_RALT;
        case SDLK_LALT:                             return IB_LALT;

        case SDLK_MODE:                             return IB_MODE;

        case SDLK_HELP:                             return IB_HELP;
        case SDLK_SYSREQ:                           return IB_SYSREQ;
        case SDLK_MENU:                             return IB_MENU;
        case SDLK_POWER:                            return IB_POWER;
        case SDLK_UNDO:                             return IB_UNDO;
    }
    return IB_NONE;
}

static void I_InitButtons( void ) {

    // some buttons dont have names, thus are hidden for all mapping

    i_buttonNames[IB_BACKSPACE] = "Backspace";
    i_buttonNames[IB_TAB] = "Tab";
    i_buttonNames[IB_CLEAR] = "Clear";
    i_buttonNames[IB_RETURN] = "Return";
    i_buttonNames[IB_PAUSE] = "Pause";
    i_buttonNames[IB_ESCAPE] = "Escape";
    i_buttonNames[IB_SPACE] = "Space";
    i_buttonNames[IB_EXCLAIM] = "!";
    i_buttonNames[IB_QUOTEDBL] = "\"";
    i_buttonNames[IB_HASH] = "#";
    i_buttonNames[IB_DOLLAR] = "$";
    i_buttonNames[IB_AMPERSAND] = "&";
    i_buttonNames[IB_QUOTE] = "'";
    i_buttonNames[IB_LEFTPAREN] = "(";
    i_buttonNames[IB_RIGHTPAREN] = ")";
    i_buttonNames[IB_ASTERISK] = "*";
    i_buttonNames[IB_PLUS] = "+";
    i_buttonNames[IB_COMMA] = ",";
    i_buttonNames[IB_MINUS] = "-";
    i_buttonNames[IB_PERIOD] = ".";
    i_buttonNames[IB_SLASH] = "/";

    i_buttonNames[IB_0] = "0";
    i_buttonNames[IB_1] = "1";
    i_buttonNames[IB_2] = "2";
    i_buttonNames[IB_3] = "3";
    i_buttonNames[IB_4] = "4";
    i_buttonNames[IB_5] = "5";
    i_buttonNames[IB_6] = "6";
    i_buttonNames[IB_7] = "7";
    i_buttonNames[IB_8] = "8";
    i_buttonNames[IB_9] = "9";
    i_buttonNames[IB_COLON] = ":";
    i_buttonNames[IB_SEMICOLON] = ";";
    i_buttonNames[IB_LESS] = "<";
    i_buttonNames[IB_EQUALS] = "=";
    i_buttonNames[IB_GREATER] = ">";
    i_buttonNames[IB_QUESTION] = "?";
    i_buttonNames[IB_AT] = "@";

    i_buttonNames[IB_LEFTBRACKET] = "[";
    i_buttonNames[IB_BACKSLASH] = "\\";
    i_buttonNames[IB_RIGHTBRACKET] = "]";
    i_buttonNames[IB_CARET] = "^";
    i_buttonNames[IB_UNDERSCORE] = "_";
    i_buttonNames[IB_BACKQUOTE] = "`";
    i_buttonNames[IB_a] = "A";
    i_buttonNames[IB_b] = "B";
    i_buttonNames[IB_c] = "C";
    i_buttonNames[IB_d] = "D";
    i_buttonNames[IB_e] = "E";
    i_buttonNames[IB_f] = "F";
    i_buttonNames[IB_g] = "G";
    i_buttonNames[IB_h] = "H";
    i_buttonNames[IB_i] = "I";
    i_buttonNames[IB_j] = "J";
    i_buttonNames[IB_k] = "K";
    i_buttonNames[IB_l] = "L";
    i_buttonNames[IB_m] = "M";
    i_buttonNames[IB_n] = "N";
    i_buttonNames[IB_o] = "O";
    i_buttonNames[IB_p] = "P";
    i_buttonNames[IB_q] = "Q";
    i_buttonNames[IB_r] = "R";
    i_buttonNames[IB_s] = "S";
    i_buttonNames[IB_t] = "T";
    i_buttonNames[IB_u] = "U";
    i_buttonNames[IB_v] = "V";
    i_buttonNames[IB_w] = "W";
    i_buttonNames[IB_x] = "X";
    i_buttonNames[IB_y] = "Y";
    i_buttonNames[IB_z] = "Z";
    i_buttonNames[IB_DELETE] = "Delete";

    i_buttonNames[IB_KP_0] = "Keypad 0";
    i_buttonNames[IB_KP_1] = "Keypad 1";
    i_buttonNames[IB_KP_2] = "Keypad 2";
    i_buttonNames[IB_KP_3] = "Keypad 3";
    i_buttonNames[IB_KP_4] = "Keypad 4";
    i_buttonNames[IB_KP_5] = "Keypad 5";
    i_buttonNames[IB_KP_6] = "Keypad 6";
    i_buttonNames[IB_KP_7] = "Keypad 7";
    i_buttonNames[IB_KP_8] = "Keypad 8";
    i_buttonNames[IB_KP_9] = "Keypad 9";
    i_buttonNames[IB_KP_PERIOD] = "Keypad .";
    i_buttonNames[IB_KP_DIVIDE] = "Keypad /";
    i_buttonNames[IB_KP_MULTIPLY] = "Keypad *";
    i_buttonNames[IB_KP_MINUS] = "Keypad -";
    i_buttonNames[IB_KP_PLUS] = "Keypad +";
    i_buttonNames[IB_KP_ENTER] = "Keypad Enter";
    i_buttonNames[IB_KP_EQUALS] = "Keypad =";

    i_buttonNames[IB_UP] = "Up";
    i_buttonNames[IB_DOWN] = "Down";
    i_buttonNames[IB_RIGHT] = "Right";
    i_buttonNames[IB_LEFT] = "Left";
    i_buttonNames[IB_INSERT] = "Insert";
    i_buttonNames[IB_HOME] = "Home";
    i_buttonNames[IB_END] = "End";
    i_buttonNames[IB_PAGEUP] = "Page Up";
    i_buttonNames[IB_PAGEDOWN] = "Page Down";

    i_buttonNames[IB_F1] = "F1";
    i_buttonNames[IB_F2] = "F2";
    i_buttonNames[IB_F3] = "F3";
    i_buttonNames[IB_F4] = "F4";
    i_buttonNames[IB_F5] = "F5";
    i_buttonNames[IB_F6] = "F6";
    i_buttonNames[IB_F7] = "F7";
    i_buttonNames[IB_F8] = "F8";
    i_buttonNames[IB_F9] = "F9";
    i_buttonNames[IB_F10] = "F10";
    i_buttonNames[IB_F11] = "F11";
    i_buttonNames[IB_F12] = "F12";
    i_buttonNames[IB_F13] = "F13";
    i_buttonNames[IB_F14] = "F14";
    i_buttonNames[IB_F15] = "F15";

    i_buttonNames[IB_NUMLOCKCLEAR] = "Num Lock";
    i_buttonNames[IB_CAPSLOCK] = "Caps Lock";
    i_buttonNames[IB_SCROLLLOCK] = "Scroll Lock";
    i_buttonNames[IB_RSHIFT] = "Right Shift";
    i_buttonNames[IB_LSHIFT] = "Left Shift";
    i_buttonNames[IB_RCTRL] = "Right Ctl";
    i_buttonNames[IB_LCTRL] = "Left Ctl";
    i_buttonNames[IB_RALT] = "Right Alt";
    i_buttonNames[IB_LALT] = "Left Alt";
    
    i_buttonNames[IB_MODE] = "Mode";

    i_buttonNames[IB_HELP] = "Help";
    i_buttonNames[IB_SYSREQ] = "SysRq";
    i_buttonNames[IB_MENU] = "Menu";
    i_buttonNames[IB_POWER] = "Power";
    i_buttonNames[IB_UNDO] = "Undo";

    i_buttonNames[IB_MOUSE_LEFT] = "mouse left button";
    i_buttonNames[IB_MOUSE_MIDDLE] = "mouse middle button";
    i_buttonNames[IB_MOUSE_RIGHT] = "mouse right button";
    i_buttonNames[IB_MOUSE_BUTTON4] = "mouse roll up";
    i_buttonNames[IB_MOUSE_BUTTON5] = "mouse roll down";

#define I_JOY_NAME_SIZE 64

    static char joyAxisNames[I_MAX_JOYSTICKS * I_MAX_AXES][I_JOY_NAME_SIZE];
    static char joyButtNames[I_MAX_JOYSTICKS * I_MAX_BUTTONS][I_JOY_NAME_SIZE];
    static char joyHaxisNames[I_MAX_JOYSTICKS * 4][I_JOY_NAME_SIZE];
    for ( int joy = 0, i = 0, j = 0, k = 0; joy < I_MAX_JOYSTICKS; joy++ ) {
        for ( int axis = 0; axis < I_MAX_AXES; axis++, i++ ) {
            i_buttonNames[IB_JOY_AXES + i] = vab( joyAxisNames[i], I_JOY_NAME_SIZE, "joystick %d axis %d", joy, axis );
        }
        for ( int button = 0; button < I_MAX_BUTTONS; button++, j++ ) {
            i_buttonNames[IB_JOY_BUTTONS + j] = vab( joyButtNames[j], I_JOY_NAME_SIZE, "joystick %d button %d", joy, button );
        }
        i_buttonNames[IB_JOY_HAXES + k + 0] = vab( joyHaxisNames[k + 0], I_JOY_NAME_SIZE, "joystick %d hat horizontal", joy );
        i_buttonNames[IB_JOY_HAXES + k + 1] = vab( joyHaxisNames[k + 1], I_JOY_NAME_SIZE, "joystick %d hat vertical", joy );
        k += 2;
    }

    CON_Printf( "Button code to button name table filled.\n" );
}

void I_RegisterVars( void ) {
    I_InitButtons();
    i_logCommands = VAR_Register( "i_logCommands", "0" );
    CMD_Register( "i_Bind", I_Bind_f );
}

void I_Init( void ) {
    I_OpenAllControllers();
}

void I_Done( void ) {
    I_CloseAllControllers();
    int i, j;
    for ( j = 0; j < I_MAX_CONTEXTS; j++ ) {
        for ( i = 0; i < IB_NUM_BUTTONS; i++ ) {
            char *cmd = i_binds[j][i];
            A_Free( cmd );
            i_binds[j][i] = NULL;
        }
    }
    CON_Printf( "Done keybindings\n" );
}

