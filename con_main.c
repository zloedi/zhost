#include "host.h"
#include "con_private.h"

console_t con;

static var_t *con_showLog;
static var_t *con_showFontTexture;

void CON_Toggle( bool_t fullscreen ) {
    if ( con.show == CON_SHOW_NONE ) {
        if ( fullscreen ) {
            con.show = CON_SHOW_FULLSCREEN;
        } else {
            con.show = CON_SHOW_HALFSCREEN;
        }
    } else if ( con.show == CON_SHOW_HALFSCREEN && fullscreen ) {
        con.show = CON_SHOW_FULLSCREEN;
    } else {
        con.show = CON_SHOW_NONE;
    }
    con.toggleTime = SYS_RealTime();
}

void CON_OnText( const char *text ) {
    if ( con.toggleTime != SYS_RealTime() && con.show != CON_SHOW_NONE ) {
        CONP_Insert( text, COM_StrLen( text ) );
    }
}

bool_t CON_OnKeyboard( int code, bool_t down ) {
    if ( code == SDLK_LCTRL || code == SDLK_RCTRL ) {
        con.ctlDown = down;
    } else if ( code == SDLK_LSHIFT || code == SDLK_RSHIFT ) {
        con.shiftDown = down;
    }
    
    if ( con.show == CON_SHOW_NONE ) {
        return false;
    }
    
    if ( ! down ) {
        return true;
    }

    // on escape clear buffer
    if ( code == SDLK_ESCAPE ) {
        CONP_Clear();
        return true;
    }

    // copy
    if ( con.ctlDown && ( code == SDLK_INSERT || code == SDLK_c ) ) {
        SYS_WriteToClipboard( prompt.buffer );
        return true;
    }

    // paste
    if ( ( con.ctlDown && code == SDLK_v ) || ( con.shiftDown && code == SDLK_INSERT ) ) {
        const char *buf = SYS_ReadClipboard();
        if ( *buf ) {
            CONP_Insert( buf, COM_StrLen( buf ) );
        }
        return true;
    }

    if ( code == SDLK_PAGEUP ) {
        con.bufPage -= con.bufWidth * ( ( int )r_info.screenHeight / CON_SYMBOL_ADVANCEY ) / 4;
        return true;
    }

    if ( code == SDLK_PAGEDOWN ) {
        con.bufPage += con.bufWidth * ( ( int )r_info.screenHeight / CON_SYMBOL_ADVANCEY ) / 4;
        return true;
    }

    if ( code == SDLK_UP ) {
        CONL_OnUp();
        return true;
    }
    
    if ( code == SDLK_DOWN ) {
        CONL_OnDown();
        return true;
    }
    
    if ( code == SDLK_LEFT ) {
        CONP_MoveCursor( -1 );
        return true;
    }
    
    if ( code == SDLK_RIGHT ) {
        CONP_MoveCursor( 1 );
        return true;
    }

    if ( code == SDLK_HOME ) {
        CONP_Home();
        return true;
    }

    if ( code == SDLK_END ) {
        CONP_End();
        return true;
    }

    if ( code == SDLK_DELETE ) {
        CONP_DeleteChars( 1 );
        return true;
    }

    // tab
    if ( code == '\t' ) {
        CONP_Autocomplete();
        return true;
    }
    
    // new line
    if ( code == '\r' ) {
        // store the prompt for later UP key
        if ( *prompt.buffer ) {
            CONL_LogCommand( prompt.buffer );
        }

        // execute the prompt string
        CON_Printf( ">%s\n", prompt.buffer );
        CMD_ExecuteString( prompt.buffer );
        CONP_Clear();
        return true;
    }
    
    // backspace
    if ( code == '\b' ) {
        if ( CONP_MoveCursor( -1 ) ) {
            CONP_DeleteChars( 1 );
        }
        return true;
    }

    return true;
}

static void CON_CheckResize( void ) {
    int w = ( int )r_info.screenWidth / CON_SYMBOL_ADVANCEX;
    int h = ( int )r_info.screenHeight / CON_SYMBOL_ADVANCEY;
    int  i, j;
    char *tmp;
    
    if ( w == con.bufWidth ) {
        return;
    }
    int sz = Clampi( w * h * 2, 512, 512 * 1024 );
    tmp = A_MallocZero( ( size_t )sz );
    if ( con.bufNumChars > con.bufSize ) {
        i = con.bufNumChars - con.bufSize;
    } else {
        i = 0;
    }
    for ( j = 0; i < con.bufNumChars; ) {
        char c = con.buf[i % con.bufSize];
        tmp[j % sz] = c;
        if ( c == '\n' ) {
            i += con.bufWidth - i % con.bufWidth;
            j += w - j % w;
        } else {
            i++;
            j++;
        }
    }
    A_Free( con.buf );
    con.buf = tmp;
    con.bufWidth = w;
    con.bufSize = sz;
    con.bufNumChars = j;
    con.bufPage = j;
    for ( int i = 0; i < CON_MAX_MESSAGES; i++ ) {
        conMessage_t *msg = &con.messages[i];
        memset( msg, 0, sizeof( *msg ) );
        msg->time = -999999999;
    }
    printf( "CON_CheckResize: console resized. Allocated %d bytes.\n", sz );
}

void CON_Frame( void ) {
    CON_CheckResize();
    COND_DrawConsole();
    COND_DrawLog( ( int )con_showLog->number );
    if ( con_showFontTexture->number ) {
        COND_DrawConsoleFont(); 
    }
}

static void CON_CopyToBuffer( const char *str ) {
    bool_t resizePage = false;
    if ( con.bufPage == con.bufNumChars ) {
        resizePage = true;
    }
    conMessage_t *msg = &con.messages[con.numMessages & ( CON_MAX_MESSAGES - 1 )];
    msg->time = SYS_RealTime();
    msg->startChar = con.bufNumChars; 
    while( *str ) {
        con.buf[con.bufNumChars % con.bufSize] = *str;
        con.bufNumChars++;
        if ( *str == '\n' ) {
            int i, base = con.bufNumChars % con.bufSize, remaining = con.bufWidth - con.bufNumChars % con.bufWidth;

            for ( i = 0; i < remaining; i++ ) {
                con.buf[base + i] = '\0';
            }
            con.bufNumChars += remaining;
        }
        str++;
    }
    msg->endChar = con.bufNumChars; 
    con.numMessages++;
    // keep the index if we have pageup/pagedown-ed
    if ( resizePage ) {
        con.bufPage = con.bufNumChars;
    }
}

int CON_Printf( const char *fmt, ... ) {
    char buf[VA_SIZE];

    va_list argptr;
    va_start( argptr, fmt );
    int result = vsnprintf( buf, VA_SIZE, fmt, argptr );
    va_end( argptr );
    buf[VA_SIZE - 1] = '\0';

    // the con_printf routine works even if console is not yet initialized
    if ( con.buf ) {
        CON_CopyToBuffer( buf );
    }
    
    printf( "%s", buf );
    fflush( stdout );
    return result;
}

static void Quit_f() {
    exit( 0 );
}

void CON_RegisterVars( void ) {
    con_showLog = VAR_RegisterHelp( "con_showLog", "0", "Print console messages on screen. The value is used as the number of lines shown." );
    con_showFontTexture = VAR_Register( "con_showFontTexture", "0" );
    COND_RegisterVars();
	CMD_Register( "quit", Quit_f );
}

void CON_Init( void ) {
    // put some arbitrary values here so the console is logged
    // until resize
    con.bufSize = 10 * 1024;
    con.buf = A_MallocZero( ( size_t )con.bufSize );
    con.bufWidth = 256;
    CONL_Init();
    CON_Printf( "Console initialized\n" );
}

void CON_Start( void ) {
    COND_Start();
}

void CON_End( void ) {
    COND_End();
}

void CON_Done( void ) {
    CONL_Done();
    A_Free( con.buf );
    con.buf = NULL;
    fflush( stdout );
}
