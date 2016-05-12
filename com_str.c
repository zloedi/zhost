#include "common.h"

void COM_Sprintf( char *buf, int size, const char *fmt, ... ) {
	va_list argptr;
	va_start (argptr, fmt);
	vsnprintf (buf, ( size_t )size, fmt, argptr);
	va_end (argptr);
	buf[size - 1] = '\0';
}

void COM_StrCpy( char *out, const char *in, int size ) {
	strncpy( out, in, ( size_t )size - 1 );
	out[size - 1] = '\0';
}
/*

    c    matches any literal character c
    .    matches any single character
    ^    matches the beginning of the input string
    $    matches the end of the input string
    *    matches zero or more occurrences of the previous character

// match: search for regexp anywhere in text 
int match(char *regexp, char *text)
{
	if (regexp[0] == '^')
		return matchhere(regexp+1, text);
	do {    // must look even if string is empty
		if (matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}

// matchhere: search for regexp at beginning of text 
int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp+2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';
	if (*text!='\0' && (regexp[0]=='.' || regexp[0]==*text))
		return matchhere(regexp+1, text+1);
	return 0;
}

// matchstar: search for c*regexp at beginning of text 
int matchstar(int c, char *regexp, char *text)
{
	do {    // a * matches zero or more instances
		if (matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));
	return 0;
}
*/    
static bool_t COM_MatchAfterStar( const char *pattern, const char *text, bool_t matchCase ) {
	const char *p = pattern;
	const char *t = text;

	while (*t) {
		switch (*p) {
			case '?' :
				p++; t++;
			case '*':
				return COM_Match (p,t,matchCase);
			default :
				if ( ( matchCase && *t != *p ) || toupper( *t ) != toupper( *p ) ) {
					p = pattern;
				} else {
					p++;
				}
				t++;
		}
	}
	return *p == '\0';
}

bool_t COM_Match( const char *pattern, const char *text, bool_t matchCase ) {
	const char *p = pattern;
	const char *t = text;

	while (*t) {
		switch (*p) {
			case '?' :
				t++;
			case '*' :
				p++;

				if (!*p)
					return true;

				return COM_MatchAfterStar( p, t, matchCase );

			default :
				if ( ( matchCase && *t != *p ) || toupper( *t ) != toupper( *p ) ) {
					return false;
				}
				t++; p++;
		}
	}

	while( *p == '*' )
		p++;
	
	return *p == '\0';
}

void COM_SplitPath( const char *path, char *dir, char *fileName ) {
	int  i;
	int  lastSlash = -1/*, lastDot = -1*/;
	char buf[VA_SIZE];

	*dir = '\0';
	*fileName = '\0';
	
	for( i = 0; i < VA_SIZE; i++ ) {
		char c = path[i];

		buf[i] = c;
		dir[i] = c;
		
		if ( ! c )
			break;

		if ( c == '/' ) {
			lastSlash = i;
		} else if ( c == '.' ) {
			//lastDot = i;
		}
	}
	dir[lastSlash + 1] = '\0';
	strcpy( fileName, &buf[lastSlash + 1] );
}
