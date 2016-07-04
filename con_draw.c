#include "zhost.h"
#include "con_private.h"

extern conSymbol_t con_font[];
extern int con_fontDef[];
extern byte con_fontBitmap[];

rImage_t *cond_fontTexture;

void COND_DrawChar( int x, int y, int c ) {
    conSymbol_t *symbol = &con_font[c & 255];
    if ( ! symbol->width )
        return;
    R_DrawPic( ( float )x + symbol->left, ( float )y + symbol->top,
                symbol->width, symbol->height,
                symbol->st0.x, symbol->st0.y,
                symbol->st1.x, symbol->st1.y,
                cond_fontTexture );
}

void COND_DrawConsoleFont( void ) {
    R_Color( 0.3f, 0, 0, 1 );
    R_SolidRect( 0, 0, 256, 256 );
    R_Color( 1, 1, 1, 1 );
    R_DrawPic( 0, 0, 256, 256, 0, 0, 1, 1, cond_fontTexture );
}

void COND_DrawLog( int numLines ) {
    if ( numLines < 1 ) {
        return;
    }
    if ( con.show != CON_SHOW_NONE ) {
        return;
    }
    numLines = Mini( numLines, ( int )R_GetWindowSize().y / CON_SYMBOL_ADVANCEY );
    int y = numLines - 1;
    for ( int j = con.numMessages - 1; ; j-- ) {
        const conMessage_t *msg = &con.messages[j & ( CON_MAX_MESSAGES - 1 )];
#define MAX_LIFETIME 6000
        int lifetime = SYS_RealTime() - msg->time;
        if ( lifetime > MAX_LIFETIME ) {
            // message died, no point in drawing older messages
            return;
        }
        R_Color( 1, 1, 1, 1 - ( float )lifetime / MAX_LIFETIME );
        for ( int i = msg->endChar - 1; i >= msg->startChar; i-- ) {
            int c = con.buf[i % con.bufSize];
            int x = i % con.bufWidth;
            COND_DrawChar( x * CON_SYMBOL_ADVANCEX, y * CON_SYMBOL_ADVANCEY, c );
            if ( ! x ) {
                y--;
            }
            if ( y < 0 ) {
                return;
            }
        }
    }
}

void COND_DrawConsole( void ) {
    v2_t ws = R_GetWindowSize();
    int frameHeight;
    if ( con.show == CON_SHOW_HALFSCREEN ) {
        frameHeight = ws.y / 3;
    } else if ( con.show == CON_SHOW_FULLSCREEN ) {
        frameHeight = ws.y;
    } else {
        return;
    }
    R_Color( 0.1f, 0, 0, 0.8f );
    R_SolidRect( 0, 0, ws.x, ( float )frameHeight + 4 );
    R_Color( 1, 1, 1, 1 );
    R_SolidRect( 0, ( float )frameHeight + 4, ws.x, 2 );
    int numLines = frameHeight / CON_SYMBOL_ADVANCEY;
    CONP_DrawPrompt( ( numLines - 1 ) * CON_SYMBOL_ADVANCEY );
    R_Color( 1, 1, 1, 0.75 );
    if ( con.bufPage < 0 ) {
        con.bufPage = 0;
    } else if ( con.bufPage > con.bufNumChars || con.bufNumChars / con.bufWidth < numLines ) {
        con.bufPage = con.bufNumChars;
    }
    int y;
    if ( con.bufPage == con.bufNumChars ) {
        y = numLines - 2;
    } else {
        for ( int i = 0; i < con.bufWidth; i++ ) {
            COND_DrawChar( i * CON_SYMBOL_ADVANCEX, ( numLines - 2 ) * CON_SYMBOL_ADVANCEY, '^' );
        }
        y = numLines - 3;
    }
    for ( int i = con.bufPage - 1; y >= 0 && i >= 0; i-- ) {
        int c = con.buf[i % con.bufSize];
        int x = i % con.bufWidth;
        COND_DrawChar( x * CON_SYMBOL_ADVANCEX, y * CON_SYMBOL_ADVANCEY, c );
        if ( ! x ) {
            y--;
        }
    }
}

void COND_RegisterVars( void ) {
}

void COND_Start( void ) {
    cond_fontTexture = R_CreateStaticTexture( con_fontBitmap, c2xy( 256, 256 ), 4 );
}

void COND_Stop( void ) {
}
