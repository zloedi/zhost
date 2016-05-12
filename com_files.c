#include "common.h"

bool_t COM_FileExists( const char *path ) {
	FILE *f = fopen( path, "rb" );
	
	if ( f ) {
		fclose( f );
		return true;
	}

	return false;
}

size_t COM_FileSize( FILE *f ) {
	fseek( f, 0 , SEEK_END );
	size_t sz = ( size_t )ftell( f );
	rewind( f );
	return sz;
}

// FIXME: evil evil terminating zero
// adding a terminal zero
bool_t COM_ReadFile( const char *path, byte **outBuf, size_t *outSz ) {
	size_t sz;
	FILE *f = fopen( path, "rb" );
	byte *b;
	
	if ( ! f ) {
		return false;
	}
	
	sz = COM_FileSize( f );

	b = A_Malloc( sz + 1 ); // make space for trailing zero
	sz = Minsz( sz, fread( b, 1, sz, f ) );
	fclose( f );

	b[sz] = '\0';

	*outBuf = b;
    *outSz = sz;
	return true;
}

bool_t COM_ReadTextFile( const char *path, char **buffer, size_t *outSz ) {
	return COM_ReadFile( path, ( byte** )buffer, outSz );
}
