#include "sys.h"

static int  sys_realTime;
static char *sys_prefsDir;
static char *sys_baseDir;

void SYS_ErrorBox( const char *fmt, ... ) {
    char buf[VA_SIZE] = {0};
    va_list argptr;
    va_start (argptr, fmt);
    vsnprintf (buf, VA_SIZE, fmt, argptr);
    va_end (argptr);
    buf[VA_SIZE - 1] = '\0';
    fprintf( stderr, "ERROR: %s\n", buf );
    fflush( stderr );
    SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_WARNING, "Error!", buf, NULL );
    exit( -1 );
}

static int SYS_Milliseconds( void ) {
    return SDL_GetTicks();
}

int SYS_SampleTime( void ) {
    sys_realTime = SYS_Milliseconds();
    return sys_realTime;
}

int SYS_RealTime( void ) {
    return sys_realTime;
}

void SYS_WriteToClipboard( const char *str ) {
    fprintf( stderr, "NOT IMPLEMENTED SYS_WriteToClipboard" );
}

const char* SYS_ReadClipboard( void ) {
    fprintf( stderr, "NOT IMPLEMENTED SYS_ReadClipboard" );
    return "";
}

const char* SYS_BaseDir( void ) {
    return sys_baseDir;
}

const char* SYS_PrefsDir( void ) {
    return sys_prefsDir;
}

void SYS_InitEx( const char* organizationName, const char *appName ) {
    if ( SDL_InitSubSystem( SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ) {
        SYS_ErrorBox( "SYS_Init: SDL could not initialize! SDL Error: %s", SDL_GetError() );
    }
    SYS_SampleTime();
    sys_prefsDir = SDL_GetPrefPath( organizationName ? organizationName : "zloedi", appName ? appName : "zhost" );
    sys_baseDir = SDL_GetBasePath();
    if ( ! sys_baseDir ) {
        sys_baseDir = A_StrDup( "./" );
    }
}

void SYS_Init( void ) {
    SYS_InitEx( NULL, NULL );
}

void SYS_Done( void ) {
    SDL_free( sys_prefsDir );
    SDL_free( sys_baseDir );
}

#ifdef __unix__

#include <dirent.h>

static int cmpr(const void *a, const void *b) { 
    return stricmp(*(char**)a, *(char**)b);
}

char** SYS_ListFiles( const char *dir, const char *pattern ) {
    struct dirent *de;
    DIR           *d;
    char          **list;
    char          buf[VA_SIZE];

    d = opendir( dir );
    if ( ! d ) {
        list = A_MallocZero( sizeof( *list ) );
        return list;
    }

    size_t count = 0;
    while( ( de = readdir( d ) ) ) {
        if ( de->d_name[0] == '.' )
            continue;
        
        if ( COM_Match( "*~", de->d_name, true ) )
            continue;
        
        if ( COM_Match( pattern, de->d_name, false ) ) {
            count++;
        }
    }

    rewinddir( d );
    
    list = A_Malloc( ( count + 1 ) * sizeof( *list ) );

    count = 0;
    while( ( de = readdir( d ) ) ) {
        if ( de->d_name[0] == '.' )
            continue;
        
        if ( COM_Match( "*~", de->d_name, true ) )
            continue;
        
        if ( COM_Match( pattern, de->d_name, false ) ) {
            if ( de->d_type == DT_DIR ) {
                COM_Sprintf( buf, VA_SIZE, "%s%s/", dir, de->d_name );
            } else {
                COM_Sprintf( buf, VA_SIZE, "%s%s", dir, de->d_name );
            }
            list[count] = A_StrDup( buf );
            count++;
        }
    }
    qsort( list, count, sizeof( *list ), cmpr );
    list[count] = NULL;
    closedir( d );
    return list;
}

#elif defined(__WIN32__)

#include <sys/types.h>
#include <windows.h>

char** SYS_ListFiles( const char *dir, const char *pattern ) {
    char **list = NULL;
    WIN32_FIND_DATA fd;
    HANDLE h;
    size_t count;

    h = FindFirstFile( va( "%s*", dir ), &fd );
    if ( h == INVALID_HANDLE_VALUE ) {
        list = A_MallocZero( sizeof( *list ) );
        return list;
    }

    count = 1;
    while ( FindNextFile( h, &fd ) != 0 ) {
        if ( fd.cFileName[0] == '.' )
            continue;
        
        if ( COM_Match( "*~", fd.cFileName, true ) )
            continue;

        if ( COM_Match( pattern, fd.cFileName, false ) ) {
            count++;
        }
    }

    list = A_MallocZero( ( count + 1 ) * sizeof( *list ) );
    list[count] = NULL;

    h = FindFirstFile( va( "%s/*", dir ), &fd );
    if ( h == INVALID_HANDLE_VALUE ) {
        return list;
    }

    count = 0;
    do {
        char *buf;

        if ( fd.cFileName[0] == '.' )
            continue;
        
        if ( COM_Match( "*~", fd.cFileName, true ) )
            continue;

        if ( ! COM_Match( pattern, fd.cFileName, false ) ) {
            continue;
        }

        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            buf = va( "%s%s/", dir, fd.cFileName );
        } else {
            buf = va( "%s%s", dir, fd.cFileName );
        }

        list[count++] = A_StrDup( buf );
    } while ( FindNextFile( h, &fd ) != 0 );

    return list;
}

#else
    #error "SYS_ListFiles not implemented"
#endif

