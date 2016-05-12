#include "host.h"
#include "con_private.h"

extern conSymbol_t con_font[];
extern int con_fontDef[];
extern byte con_fontBitmap[];

int cond_fontTexture;

void COND_DrawChar( int x, int y, int c ) {
	conSymbol_t *symbol = &con_font[c & 255];
	if ( ! symbol->width )
		return;
	R2D_DrawPic( ( float )x + symbol->left, ( float )y + symbol->top,
			    symbol->width, symbol->height,
				symbol->st0.x, symbol->st0.y,
				symbol->st1.x, symbol->st1.y,
	  			cond_fontTexture );
}

void COND_DrawConsoleFont( void ) {
	R2D_Color( 0.3f, 0, 0, 1 );
	R2D_SolidRect( 0, 0, 256, 256 );
	R2D_Color( 1, 1, 1, 1 );
	R2D_DrawPic( 0, 0, 256, 256, 0, 0, 1, 1, cond_fontTexture );
}

void COND_DrawLog( int numLines ) {
	if ( numLines < 1 ) {
		return;
	}
	if ( con.show != CON_SHOW_NONE ) {
		return;
	}
    numLines = Mini( numLines, ( int )r_info.screenHeight / CON_SYMBOL_ADVANCEY );
	int y = numLines - 1;
	for ( int j = con.numMessages - 1; ; j-- ) {
		const conMessage_t *msg = &con.messages[j & ( CON_MAX_MESSAGES - 1 )];
#define MAX_LIFETIME 6000
		timestamp_t lifetime = SYS_RealTime() - msg->time;
		if ( lifetime > MAX_LIFETIME ) {
            // message died, no point in drawing older messages
			return;
		}
		R2D_Color( 1, 1, 1, 1 - ( float )lifetime / MAX_LIFETIME );
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
	int frameHeight;
	if ( con.show == CON_SHOW_HALFSCREEN ) {
		frameHeight = ( int )( r_info.screenHeight / 3 );
	} else if ( con.show == CON_SHOW_FULLSCREEN ) {
		frameHeight = ( int )r_info.screenHeight;
	} else {
		return;
	}
	R2D_Color( 0.1f, 0, 0, 0.8f );
	R2D_SolidRect( 0, 0, r_info.screenWidth, ( float )frameHeight + 4 );
	R2D_Color( 1, 1, 1, 1 );
	R2D_SolidRect( 0, ( float )frameHeight + 4, ( float )r_info.screenWidth, 2 );
	int numLines = frameHeight / CON_SYMBOL_ADVANCEY;
	CONP_DrawPrompt( ( numLines - 1 ) * CON_SYMBOL_ADVANCEY );
	R2D_Color( 1, 1, 1, 0.75 );
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
    cond_fontTexture = RI_CreateStaticTexture( con_fontBitmap, 256, 256, RI_NONE, 4 );
}

void COND_End( void ) {
}
