#include "sys.h"
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
