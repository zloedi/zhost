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

static inline bool_t CMD_ButDown( void ) { return CMD_Argv( 0 )[0] == '+'; }
