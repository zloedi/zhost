#include "zhost.h"

static cmd_t *commands;

#define CMD_MAX_ARGS 64

static int  cmd_argc;
static char *cmd_argv[CMD_MAX_ARGS];

extern varPtr_t vars;

int CMD_Argc( void ) {
    return cmd_argc;
}

const char* CMD_Argv( int index ) {
    if ( index < 0 || index >= cmd_argc ) {
        return "";
    }

    return cmd_argv[index];
}

static char** CMD_List( const char *pattern ) {
    size_t     count = 1; // terminating NULL
    varPtr_t   var;
    cmd_t      *cmd;
    char       **list;
 
    for ( var = vars; var; var = VAR_Next( var ) ) {
        if ( COM_Match( pattern, VAR_Name( var ), false ) ) {
            count++;
        }
    }
    
    for ( cmd = commands; cmd; cmd = cmd->next ) {
        if ( COM_Match( pattern, cmd->name, false ) ) {
            count++;
        }
    }

    list = A_MallocZero( count * sizeof( *list ) );
    if ( count == 1 ) {
        return list;
    }
    
    count = 0;
    for ( var = vars; var; var = VAR_Next( var ) ) {
        const char *varName = VAR_Name( var );
        if ( COM_Match( pattern, varName, false ) ) {
            list[count] = A_StrDup( varName );
            count++;
        }
    }
    
    for ( cmd = commands; cmd; cmd = cmd->next ) {
        if ( COM_Match( pattern, cmd->name, false ) ) {
            list[count] = A_StrDup( cmd->name );
            count++;
        }
    }
    
    return list;
}

void CMD_DestroyList( char **list ) {
    int i;

    if ( ! list ) {
        return;
    }   
    
    for ( i = 0; list[i]; i++ ) {
        A_Free( list[i] );
    }
    A_Free( list );
}

cmd_t* CMD_Find( const char *name ) {
    cmd_t *cmd;
 
    for ( cmd = commands; cmd; cmd = cmd->next ) {
        int res = stricmp( cmd->name, name );
        
        if ( res == 0 ) {
            return cmd;
        } else if ( res > 0 ) {
            break;
        }
    }
    return NULL;
}

static void CMD_AppendArg( const char *a ) {
    char **arg = &cmd_argv[cmd_argc];
    
    A_Free( *arg );
    *arg = A_StrDup( a );
}

static const char* CMD_Tokenize( const char *text ) {
    const char *data = COM_Token( text );
    
    for( cmd_argc = 0; *com_token && cmd_argc < CMD_MAX_ARGS; cmd_argc++ ) {

        if ( com_token[0] == ';' ) {
            data++;
            break;
        }
        
        CMD_AppendArg( com_token );
        data = COM_Token( data );
    }

    return data;
}

static void CMD_ExecuteStringOV( const char *text, const char *param, bool_t keepOldValues ) {
    varPtr_t   var;
    cmd_t      *cmd;
    const char *data;
    const char *cmdName;

    data = text;

    do {
        data = CMD_Tokenize( data );
        
        if ( ! cmd_argc ) {
            continue;
        }
        
        // look for a var
        var = VAR_Find( cmd_argv[0] );
        if ( var ) {
            if ( cmd_argc > 2 ) {
                if ( *cmd_argv[1] == '=' ) {
                    VAR_Set( var, cmd_argv[2], keepOldValues );
                }
            } else if ( cmd_argc > 1 ) {
                if ( *cmd_argv[1] == '=' ) {
                    VAR_Set( var, "", keepOldValues );
                } else {
                    VAR_Set( var, cmd_argv[1], keepOldValues );
                }
            } else {
                CON_Printf( "%s = \"%s\" (%s)\n", VAR_Name( var ), VAR_String( var ), VAR_Help( var ) );
            }
            continue;
        }

        cmdName = cmd_argv[0];

        // look for a cmd
        if ( param ) {
            CMD_AppendArg( param );
        }

        // clear key sign
        if ( cmdName[0] == '+' || cmdName[0] == '-' ) {
            cmdName++;
        }

        if ( cmdName ) {
            cmd = CMD_Find( cmdName );
            if ( cmd ) {
                cmd->func();
            } else {
                CON_Printf( "Unknown command \"%s\"\n", cmd_argv[0] );
            }
        }
    } while ( data );
}

void CMD_ExecuteCfgString( const char *text ) {
    CMD_ExecuteStringOV( text, NULL, false );
}

void CMD_ExecuteString( const char *text ) {
    CMD_ExecuteStringOV( text, NULL, true );
}

// param will be appended to each "line" separated by ';'
void CMD_ExecuteStringParam( const char *text, const char *param ) {
    CMD_ExecuteStringOV( text, param, true );
}

void CMD_Init( void ) {
    int i;

    commands = NULL;

    for ( i = 0; i < CMD_MAX_ARGS; i++ ) {
        cmd_argv[i] = A_StrDup( "" );
    }

    CON_Printf( "Commands initialized\n" );
}

void CMD_Done( void ) {
    int   i;
    cmd_t *cmd, *next;

    for ( i = 0; i < CMD_MAX_ARGS; i++ ) {
        A_Free( cmd_argv[i] );
    }

    for ( cmd = commands; cmd; cmd = next ) {
        next = cmd->next;
        A_Free( cmd->name );
        A_Free( cmd );
    }
}

static int CMD_MinMatch( char **list ) {
    int  i, j, matchLen;
    char *match;

    if ( ! *list )
        return 0;
    
    match = list[0];
    matchLen = COM_StrLen( match );
    
    for ( i = 1; list[i]; i++ ) {
        char *str = list[i];

        for ( j = 0; ; j++ ) {
            if ( toupper( str[j] ) != toupper( match[j] ) )
                break;
            else if ( ! match[j] )
                break;
        }

        if ( j < matchLen )
            matchLen = j;
    }
    return matchLen;
}

static char** CMD_ListFiles( const char *path ) {
    char **list;
    char dir[VA_SIZE];
    char fileName[VA_SIZE];
    int  c;

    COM_SplitPath( path, dir, fileName );

    c = toupper( dir[0] );
    if ( c != '.' && c != '/' && ! ( c >= 'A' && c <= 'Z' && dir[1] ==':' ) ) {
        COM_StrCpy( dir, va( "./data/%s", dir ), VA_SIZE );
    }
    strncat( fileName, "*", VA_SIZE );
    list = SYS_ListFiles( dir, fileName );
    
    return list;
}

// returns true if prompt is changed
bool_t CMD_Autocomplete( char *prompt, int maxLen, bool_t matchCommands ) {
    char   **list = NULL;
    int    minMatch;
    
    if ( ! *prompt )
        return false;
    
/*
    FIXME: must be in console code
    cmd_t *cmd = CMD_Find( cmd_argv[0] );
    
    if ( cmd && cmd->completeArg ) {
        cmd->completeArg( prompt );
    }
*/

    // try to match commands
    if ( matchCommands ) {
        list = CMD_List( va( "%s*", prompt ) );
    }

    // no matching commands, try to match paths
    if ( ! list || ! list[0] ) {
        CMD_DestroyList( list );
        list = CMD_ListFiles( prompt );
    }

    // get the shortest common match
    minMatch = CMD_MinMatch( list );

    if ( minMatch ) {
        int  i;
        char *res = A_StrDup( list[0] );

        res[minMatch] = '\0';

        if ( ! stricmp( prompt, res ) ) {
            // we have typed the min match, print a list of matches
            CON_Printf( ">%s\n", prompt );
            for ( i = 0; list[i]; i++ ) {
                varPtr_t v = VAR_Find( list[i] );
                
                if ( v ) {
                    CON_Printf( "%s = \"%s\" (%s)\n", VAR_Name( v ), VAR_String( v ), VAR_Help( v ) );
                } else {
                    CON_Printf( "%s\n", list[i] );
                }
            }
        } else {
            COM_StrCpy( prompt, res, maxLen );
        }

        A_Free( res );
    }
    
    CMD_DestroyList( list );
    return minMatch;
}

void CMD_Register( const char *name, cmdFunc_t func ) {
    cmd_t *cmd, **prev = &commands, *newCmd;

    for ( cmd = commands; cmd; cmd = cmd->next ) {
        int res = stricmp( cmd->name, name );
        
        if ( res == 0 ) {
            CON_Printf( "Command \"%s\" already registered\n", name );
            return;
        } else if ( res > 0 ) {
            break;
        }
        prev = &cmd->next;
    }

    newCmd = A_Malloc( sizeof( *newCmd ) );
    newCmd->next = cmd;
    *prev = newCmd;
    
    newCmd->name = A_StrDup( name );
    newCmd->func = func;
}
