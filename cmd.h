typedef void (*cmdFunc_t)(void);

typedef struct cmd_s {
	struct cmd_s *next;
	char         *name;
	cmdFunc_t    func;
} cmd_t;

void CMD_Init( void );
void CMD_Done( void );
void CMD_Register( const char *name, cmdFunc_t func );
void CMD_RegisterHelp( const char *name, cmdFunc_t func, const char *help );
void CMD_ExecuteCfgString( const char *text );
void CMD_ExecuteString( const char *text );
void CMD_ExecuteStringParam( const char *text, const char *param );
int CMD_Argc( void );
const char* CMD_Argv( int index );
bool_t CMD_Autocomplete( char *prompt, int maxLen, bool_t matchCommands );
cmd_t* CMD_Find( const char *name );

#define CMD_ENGAGE '!'
#define CMD_RELEASE '^'

static inline bool_t CMD_ArgvEngaged( void ) { return CMD_Argv( 0 )[0] == CMD_ENGAGE; }
static inline bool_t CMD_ArgvReleased( void ) { return CMD_Argv( 0 )[0] == CMD_RELEASE; }

static inline int CMD_ArgvAxisSign( void ) { 
    const char *str = CMD_Argv( 0 );
    return ( str[0] == CMD_ENGAGE ) * ( 1 - 2 * ( str[3] == '-' ) );
}

static inline int CMD_ArgvAxisValue( void ) { 
    static const int pow10[] = {
        1,
        10,
        100,
        1000,
        10000,
    };
    const char *a = CMD_Argv( 0 );
    int sign;
    int result = 0;
    if ( a[0] == CMD_ENGAGE ) {
        sign = a[1] == '-' ? -1 : 1;
        for ( int i = 0; i < 5; i++ ) {
            int c = a[2 + i] - '0';
            result += c * pow10[4 - i];
        }
    }
    return result * sign;
}

static inline const char* CMD_CommandFromBind( const char *bindString, bool_t isJoystick, int joystickId, bool_t engage, int value ) {
	if ( ! bindString ) {
		return NULL;
	}
    int e = engage ? CMD_ENGAGE : CMD_RELEASE;
    if ( bindString[0] == CMD_ENGAGE || bindString[0] == CMD_RELEASE ) {
        if ( e != bindString[0] ) {
            return NULL;
        } 
        bindString++;
    }
    int sign = value >= 0 ? '+' : '-';
    if ( bindString[0] == '+' || bindString[0] == '-' ) {
        if ( value != 0 && sign != bindString[0] ) {
            return NULL;
        }
        bindString++;
    }
    int devFlag = isJoystick ? 'j' : 'k';
    int devId = '0' + joystickId;
    if ( engage ) {
        return va( "%c%c%c%c%05d%s", e, devFlag, devId, sign, abs( value ), bindString );
    }
    return va( "%c%c%c%s", e, devFlag, devId, bindString );
}

