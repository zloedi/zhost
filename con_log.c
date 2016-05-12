#include "host.h"
#include "con_private.h"

// history of text typed in the prompt
typedef struct {
	int         rover, head;
	char        *lines[CON_MAX_HISTORY];
} conLog_t;

static conLog_t clog;

void CONL_LogCommand( const char *cmd ) {
	int idx;

	if ( ! stricmp( cmd, "quit" ) ) {
		return;
	}
	
	if ( stricmp( cmd, clog.lines[clog.head & ( CON_MAX_HISTORY - 1 )] ) ) {
		clog.head++;
		idx = clog.head & ( CON_MAX_HISTORY - 1 );
		A_Free( clog.lines[idx] );
		clog.lines[idx] = A_StrDup( cmd );
	}
	
	clog.rover = clog.head + 1;
}

static void CONL_SetPrompt( void ) {
	int idx = clog.rover & ( CON_MAX_HISTORY - 1 );
	const char *historyLine = clog.lines[idx];

	CONP_Clear();
	CONP_Insert( historyLine, COM_StrLen( historyLine ) );
}

void CONL_OnUp( void ) {
	int idx = clog.rover - 1;

	if ( clog.head - idx == CON_MAX_HISTORY )
		return;

	if ( ! *clog.lines[idx & ( CON_MAX_HISTORY - 1 )] )
		return;
	
	clog.rover--;
	
	CONL_SetPrompt();
}

void CONL_OnDown( void ) {
	if ( clog.rover >= clog.head )
		return;

	clog.rover++;
	CONL_SetPrompt();
}

static void CONL_StoreLogToFile( void ) {
    const char *dataDir = SYS_PrefsDir();
    if ( dataDir ) {
        int    i;
        FILE   *f;
        const char *path = va( "%sconsole.log", dataDir );

        f = fopen( path, "wb" );
        if ( ! f ) {
            CON_Printf( "CON_StoreHistoryToFile: failed to store \"%s\"\n", path );
            return;
        }

        for ( i = 0; i < CON_MAX_HISTORY; i++ ) {
            int idx = clog.head + 1;
            const char *line = clog.lines[(idx + i ) & ( CON_MAX_HISTORY - 1 )];

            if ( *line ) {
                fprintf( f, "%s\n", line );
            }
        }

        fclose( f );
    }
}

static const char* CONL_GetLine( const char *buffer, char *out ) {
	int i;
	
	for ( i = 0; ; i++ ) {
		int c = buffer[i];

		if ( ! c ) {
			buffer = NULL;
			break;
		}
		
		// make sure we quit properly even if there is a too large line
		if ( i >= VA_SIZE - 1 || c == '\n' ) {
			buffer += i + 1;
			break;
		}
		
		out[i] = ( char )c;
	}

	out[i] = '\0';
	return buffer;
}

static void CONL_ReadLogFromFile( void ) {
	int  i;
	char *buffer;
    const char *dataDir = SYS_PrefsDir();
    if ( dataDir ) {
        const char *data;
        const char *path = va( "%sconsole.log", dataDir );
        size_t sz;
        if ( ! COM_ReadTextFile( path, &buffer, &sz ) ) {
            CON_Printf( "CON_ReadHistoryFromFile: failed to open \"%s\" for reading\n", path );
            return;
        }
        for ( i = CON_MAX_HISTORY, data = buffer; data;  ) {
            char line[VA_SIZE];

            data = CONL_GetLine( data, line );
            if ( *line ) {
                char **p = &clog.lines[i & ( CON_MAX_HISTORY - 1 )];
                A_Free( *p );
                *p = A_StrDup( line );
                i++;
            }
        }
        clog.rover = i;
        clog.head = clog.rover - 1;
        A_Free( buffer );
        CON_Printf( "Read console history from \"%s\"\n", path );
    }
}

void CONL_Init( void ) {
	int i;
	
	for ( i = 0; i < CON_MAX_HISTORY; i++ ) {
		clog.lines[i] = A_StrDup( "" );
	}
	clog.head = CON_MAX_HISTORY;
	clog.rover = clog.head + 1;
	CONL_ReadLogFromFile();
	
	CON_Printf( "Console log initialized\n" );
}

void CONL_Done( void ) {
	int i;
	
	CONL_StoreLogToFile();
	for ( i = 0; i < CON_MAX_HISTORY; i++ ) {
		A_Free( clog.lines[i] );
	}
}
