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
static inline bool_t CMD_ArgvDeviceType( void ) { return CMD_Argv( 0 )[1]; }
static inline bool_t CMD_ArgvDeviceId( void ) { return CMD_Argv( 0 )[2]; }

int CMD_ArgvAxisSign( void );
int CMD_ArgvAxisValue( void );
const char* CMD_FromBind( const char *bindString, bool_t isJoystick, 
                            int device, bool_t engage, int value );
void CMD_FromBindBuf( const char *bindString, bool_t isJoystick, int device, 
                        bool_t engage, int value, char* buf, int bufSz );

