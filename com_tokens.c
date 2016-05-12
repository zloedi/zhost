#include "common.h"

char com_token[VA_SIZE];

static inline bool_t IsSpecial( int c ) {
	switch( c ) {
		case '=':
		case ';':
		case '{':
		case '}':
		case '(':
		case ')':
        case ':':
        case '\\':
        case '/':
			return true;
	}
	return false;
}

static inline bool_t IsSpace( int c ) {
	switch( c ) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return true;
	}
	return false;
}

const char* COM_Token( const char *data ) {
	int  c;
	int  i = 0;

	if ( ! data || ! *data ) {
		com_token[0] = '\0';
		return NULL;
	}

eat_spaces:
	// eat spaces
	while( IsSpace( *data ) ) {
		data++;
	}

	// eat comments
	if ( data[0] == '/' && data[1] == '/' ) {
		data += 2;
		while( *data && *data != '\n' ) {
			data++;
		}
		goto eat_spaces;
	}

	// some symbols are tokens
	if ( IsSpecial( c = *data ) ) {
		if ( i < VA_SIZE - 1 ) {
			com_token[i] = ( char )c;
			i++;
		}
		data++;
		goto out;
	}

	// strings
	if ( *data == '"' ) {
		data++;
		
		while ( ( c = *data ) ) {
			data++;
			
			if ( c == '"' ) {
				break;
			}
			
			if ( i < VA_SIZE - 1 ) {
				com_token[i] = ( char )c;
				i++;
			}
		}
		goto out;
	}
	
	while( true ) {
		c = *data;
		
		if ( ! c ) {
			break;
		}
		
		if( IsSpace( c ) ) {
			break;
		}
		
		if ( c == '/' && data[1] == '/' ) {
			break;
		}
		
		if ( IsSpecial( c ) ) {
			break;
		}

		if ( c == '"' ) {
			break;
		}
		
		if ( i < VA_SIZE - 1 ) {
			com_token[i] = ( char )c;
			i++;
		}

		data++;
	}
	
out:
	com_token[i] = '\0';
	return c ? data : NULL;
}

char** COM_Tokenize( const char *text ) {
    size_t numTokens;
	const char *data = COM_Token( text );
	for( numTokens = 0; *com_token; numTokens++ ) {
		data = COM_Token( data );
	}
    char **result = A_Malloc( sizeof( *result ) * ( numTokens + 1 ) );
	data = COM_Token( text );
	for( int i = 0; *com_token; i++ ) {
        result[i] = A_StrDup( com_token );
		data = COM_Token( data );
	}
    result[numTokens] = NULL;
	return result;
}

void COM_TokenizeFree( char **tokens ) {
    char *token;
    for ( int i = 0; ( token = tokens[i] ) != NULL; i++ ) {
        A_Free( token );
    }
}

const char* COM_StrBefore( const char *text, const char *delimiter, const char **outBefore ) {
    static char before[VA_SIZE];
    before[0] = '\0';
	const char *data = text;
	while( ( data = COM_Token( data ) ) != NULL ) {
        if ( strcmp( com_token, delimiter ) == 0 ) {
            while ( *data == ' ' ) {
                data--;
            }
            int sz = ( int )( data - text );
            COM_StrCpy( before, text, sz );
            break;
        }
	}
    *outBefore = before;
    return data;
}

const char* COM_StrAfter( const char *text, const char *delimiter, const char **outAfter ) {
    static char after[VA_SIZE];
    after[0] = '\0';
	const char *data = text;
	while( ( data = COM_Token( data ) ) != NULL ) {
        if ( strcmp( com_token, delimiter ) == 0 ) {
            while ( *data == ' ' ) {
                data++;
            }
            COM_StrCpy( after, data, VA_SIZE );
            break;
        }
	}
    *outAfter = after;
    return data;
}

void COM_Split( const char *text, const char *delimiter, const char **outBefore, const char **outAfter ) {
    const char *data = COM_StrBefore( text, "=", outBefore );
    COM_StrAfter( data, "=", outAfter );
}

const char* COM_GetLine( const char *text, char **outLine ) {
    static char line[VA_SIZE];
    int c, numChars = 0;
    for ( numChars = 0; ( c = text[numChars] ) != '\0' && c != '\n'; numChars++ ) {
    }
    if ( numChars == 0 ) {
        return NULL;
    }
    COM_StrCpy( line, text, Mini( VA_SIZE, numChars ) );
    *outLine = line;
    return &text[numChars + 1];
}
