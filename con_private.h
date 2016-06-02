#define CON_PROMPT_LEN 1024

#define CON_SYMBOL_ADVANCEX 8
#define CON_SYMBOL_ADVANCEY 16

typedef enum {
	CON_SHOW_NONE,
	CON_SHOW_HALFSCREEN,
	CON_SHOW_FULLSCREEN,
} show_t;

typedef struct {
	v2_t  st0, st1;
	float width, height, left, top;
} conSymbol_t;

#define CON_MAX_HISTORY   32
#define CON_MAX_MESSAGES  256

typedef struct {
	timestamp_t  time;
	int          startChar;
	int          endChar;
} conMessage_t;

typedef struct {
	int          bufPage;
	int          bufWidth;
	int          bufSize;
	int          bufNumChars;
	int          numMessages;
	conMessage_t messages[CON_MAX_MESSAGES];
	char         *buf;
	show_t       show;
	bool_t       ctlDown, shiftDown;
    timestamp_t  toggleTime;
} console_t;

extern console_t con;

/*
===================================
Draw
===================================
*/
void COND_DrawChar( int x, int y, int c );
void COND_DrawLog( int numLines );
void COND_DrawConsole( void );
void COND_DrawConsoleFont( void );
void COND_RegisterVars( void );
void COND_Start( void );
void COND_Stop( void );

/*
===================================
Prompt edit line
===================================
*/
typedef struct {
	char        buffer[CON_PROMPT_LEN];
	
	// private
	int         cursor;
} conPrompt_t;

extern conPrompt_t prompt;

void CONP_Insert( const char *chars, int numChars );
void CONP_DeleteChars( int numChars );
void CONP_DrawPrompt( int y );
void CONP_Home( void );
void CONP_End( void );
void CONP_Clear( void );
bool_t CONP_MoveCursor( int offset );
void CONP_Autocomplete( void );

/*
===================================
Typed commands logging
===================================
*/
void CONL_LogCommand( const char *cmd );
void CONL_OnUp( void );
void CONL_OnDown( void );
void CONL_Init( void );
void CONL_Done( void );
