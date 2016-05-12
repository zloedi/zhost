#include "common_types.h"
#include "allocator.h"

static inline int COM_LittleInt( const byte *mem ) {
    return ( mem[3] << 24 ) | ( mem[2] << 16 ) | ( mem[1] << 8 ) | ( mem[0] );
}

static inline unsigned COM_LittleUnsigned( const byte *mem ) {
    unsigned b0 = mem[0];
    unsigned b1 = mem[1];
    unsigned b2 = mem[2];
    unsigned b3 = mem[3];
    return ( b3 << 24 ) | ( b2 << 16 ) | ( b1 << 8 ) | b0;
}

bool_t COM_FileExists( const char *path );
size_t COM_FileSize( FILE *f );
// zero terminated, returns size EXCLUDING the terminating char
bool_t COM_ReadFile( const char *path, byte **buffer, size_t *outSz );
bool_t COM_ReadTextFile( const char *path, char **buffer, size_t *outSz );

void COM_StrCpy( char *out, const char *in, int size );
void COM_Sprintf( char *buf, int size, const char *fmt, ... );
bool_t COM_Match( const char *pattern, const char *text, bool_t matchCase );
void COM_SplitPath( const char *path, char *dir, char *fileName );
static inline int COM_StrLen( const char *str ) {
    return ( int )strlen( str );
}

// com_token is filled on each COM_Token call
extern char com_token[VA_SIZE];
const char* COM_Token( const char *data );
char** COM_Tokenize( const char *text );
const char* COM_GetLine( const char *string, char **outLine );
const char* COM_StrBefore( const char *text, const char *delimiter, const char **outBefore );
const char* COM_StrAfter( const char *text, const char *delimiter, const char **outAfter );
void COM_Split( const char *text, const char *delimiter, const char **outBefore, const char **outAfter );

//void COM_SRand( unsigned seed );
//int COM_Rand( void );
//static inline int COM_RandInRange( int min, int max ) {
//    if ( min == max ) {
//        return min;
//    }
//    return min + COM_Rand() % ( max - min );
//}

