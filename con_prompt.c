#include "zhost.h"
#include "con_private.h"

conPrompt_t prompt;

static void CONP_TokenBeforeCursor( const char *buf, int cursor, int *outStart, int *outEnd ) {
    const char *cursorPtr = &buf[cursor];
    // track first preceding non space character
    while ( cursorPtr > buf && ( *cursorPtr == ' ' || *cursorPtr == '\0' ) ) {
        cursorPtr--;
    }
    // skip tokens until cursor is passed
    const char *start = buf;
    const char *end = buf;
    while ( ( end = COM_Token( end ) ) != NULL ) {
        if ( end > cursorPtr ) {
            break;
        }
        start = end;
    }
    // eat any leading spaces
    while ( *start == ' ' ) {
        start++;
    }
    *outStart = ( int )( start - buf );
    if ( end ) {
        *outEnd = ( int )( end - buf ) - 1;
    } else {
        *outEnd = COM_StrLen( buf ) - 1;
    }
}

void CONP_Autocomplete( void ) {
    char   buf[CON_PROMPT_LEN];
    int    start, end, count;

    CONP_TokenBeforeCursor( prompt.buffer, prompt.cursor, &start, &end );
    
    // get the interesting token to autocomplete

    // track back to first slash or windows drive so entire paths are autocompleted
    for ( int i = start; i >= 0; i-- ) {
        int c = prompt.buffer[i];
        if ( c == '"' || c == ';' ) {
            break;
        }
        if ( c == ':' ) {
            i--;
            if ( i >= 0 ) {
                start = i;
            }
        } else if ( c == '/' ) {
            start = i;
        }
    }

    COM_StrCpy( buf, &prompt.buffer[start], CON_PROMPT_LEN );
    count = end - start + 1;
    buf[count] = '\0';

    // check if there is a matching command or path
    if ( CMD_Autocomplete( buf, CON_PROMPT_LEN, true ) ) {
        const char *str;

        // handle quoted strings
        if ( prompt.buffer[start] == '"' ) {
            str = va( "\"%s\"", buf );
        } else {
            str = buf;
        }

        // remove old token and place the match there
        prompt.cursor = start;
        CONP_DeleteChars( count );
        CONP_Insert( str, COM_StrLen( str ) );
    }
}

bool_t CONP_MoveCursor( int offset ) {
    int newCursor;
    
    // if at the end of string, dont move forward
    if ( offset > 0 && ! prompt.buffer[prompt.cursor] ) {
        return false;
    }
    
    // clamp to bounds
    newCursor = prompt.cursor + offset;
    if ( newCursor < 0 || newCursor >= CON_PROMPT_LEN ) {
        return false;
    }
    
    prompt.cursor = newCursor;
    
    if ( con.ctlDown ) {
        int start, end;
        if ( offset > 0 ) {
            const char *next = COM_Token( &prompt.buffer[prompt.cursor] );
            CONP_TokenBeforeCursor( prompt.buffer, ( int )( next - prompt.buffer ), &start, &end );
            prompt.cursor = end + 1;
        } else {
            CONP_TokenBeforeCursor( prompt.buffer, prompt.cursor, &start, &end );
            prompt.cursor = start;
        }
    }

    return true;
}

void CONP_Home( void ) {
    prompt.cursor = 0;
}

void CONP_End( void ) {
    prompt.cursor = COM_StrLen( prompt.buffer );
}

void CONP_Clear( void ) {
    *prompt.buffer = '\0';
    CONP_Home();
}

void CONP_DeleteChars( int numChars ) {
    int len = COM_StrLen( prompt.buffer );
    // clamp to buffer size
    if ( prompt.cursor + numChars >= len ) {
        numChars = len - prompt.cursor;
    }
    // shift left
    for ( int i = prompt.cursor; i < len; i++ ) {
        prompt.buffer[i] = prompt.buffer[i + numChars];
    }
}

void CONP_Insert( const char *chars, int numChars ) {
    int len = COM_StrLen( prompt.buffer );

    // quit if out of space
    if ( len + numChars > CON_PROMPT_LEN - 1 ) {
        return;
    }
    
    // shift prompt to the right
    for ( int i = len + numChars; i >= prompt.cursor; i-- ) {
        prompt.buffer[i] = prompt.buffer[i - numChars];
    }
    
    // copy to prompt
    for ( int i = 0; i < numChars; i++ ) {
        prompt.buffer[prompt.cursor + i] = chars[i];
    }
    
    prompt.cursor += numChars;
}

void CONP_DrawPrompt( int y ) {
    static int start;

    int len, hiStart, hiEnd;
    int max = con.bufWidth - 2;

    // calling COM_StrLen each frame, oh well...
    len = COM_StrLen( prompt.buffer );
    
    if ( len < max ) {
        // show the entire text if fits
        start = 0;
    } else {
        // make sure that cursor is always onscreen
        if ( start > len ) {
            start = 0;
        }
        
        if ( prompt.cursor - start > max ) {
            start = prompt.cursor - max;
        } else if ( prompt.cursor < start ) {
            start = prompt.cursor;
        }
    }
    
    // prompt character
    COND_DrawChar( 0, y, '>' );

    // print prompt
    CONP_TokenBeforeCursor( prompt.buffer, prompt.cursor, &hiStart, &hiEnd );
    for ( int i = 0; ; i++ ) {
        int idx = i + start;
        int c = prompt.buffer[idx];
        
        if ( ! c )
            break;
        
        // hilight the current word
        if ( idx >= hiStart && idx <= hiEnd ) {
            R_Color( 1, 1, 1, 1 );
        } else {
            R_Color( 1, 1, 1, 0.6f );
        }
        
        int x = ( 1 + i ) * CON_SYMBOL_ADVANCEX;
        COND_DrawChar( x, y, c );
    }
    
    // blinking cursor
    R_Color( 1, 1, 1, 0.75 );
    if ( SYS_RealTime() & 128 ) {
        COND_DrawChar( ( 1 + prompt.cursor - start ) * CON_SYMBOL_ADVANCEX, y, '_' );
    }
}
