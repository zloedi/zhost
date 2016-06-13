#include "sys.h"
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
