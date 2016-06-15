#include "common_types.h"
#include "allocator.h"

#define CHUNK_GUARD ((int)0xdeadf00d)

// FIXME: make sure this is larger than sizeof( chunk_t );
#define MIN_CHUNK_SIZE 128

#define STATIC_BIN_SIZE (8*1024*1024)
#define SMALL_BIN_SIZE  (128 * 1024)
#define NORMAL_BIN_SIZE (16*1024*1024)

typedef struct {
    const char *label;
    const char *file;
    int        line;
    size_t     allocSize;
} chunkDebug_t;

typedef enum {
    CT_NONE = 0,
    CT_CAP,

    CT_SMALL,
    CT_NORMAL,
    CT_STATIC,
} chunkType_t;

typedef struct chunk_s {
    struct chunk_s *prev, *next;

#ifdef ALLOC_DEBUG
    chunkDebug_t   debug;
#endif

    chunkType_t    type;  
    size_t         size;  // including header
    int            guard; // must be CHUNK_GUARD
} chunk_t;

typedef struct {
    size_t     size;            // total bytes malloced, including header
    size_t     used;            // total bytes used
    const char *name;
    chunk_t    cap;
    chunk_t    *rover;
} bin_t;

static bin_t *a_smallBin;
static bin_t *a_normalBin;
static bin_t *a_staticBin;

static void A_FatalError( const char *format, ... ) {
    va_list argptr;
    va_start( argptr, format );
    vfprintf( stderr, format, argptr );
    va_end( argptr );
    exit( 0 );
}

static int A_Print( const char *format, ... ) {
    va_list argptr;
    va_start( argptr, format );
    int result = vfprintf( stdout, format, argptr );
    va_end( argptr );
    return result;
}

static aFatalf_t a_fatalFn = A_FatalError;
static aPrintf_t a_printFn = A_Print;

static int* A_GetTrailGuard( chunk_t *chunk ) {
    return ( int* )( ( byte* )chunk + chunk->size - sizeof( chunk->guard ) );
}

static bin_t* A_NewBin( size_t size, const char *name ) {
    bin_t   *bin;
    chunk_t *chunk;

    bin = malloc( size );

    chunk = ( chunk_t* )( bin + 1 );
    
    // the cap doesnt allow merge of the first and last blocks
    bin->cap.type = CT_CAP;
    bin->cap.next = bin->cap.prev = chunk;
    bin->cap.size = 0;

    bin->name = name;
    bin->rover = chunk;
    bin->size = size;
    bin->used = 0;

    // make all available memory one chunk
    chunk->next = chunk->prev = &bin->cap;
    chunk->size = size - sizeof( *bin );
    chunk->type = CT_NONE;

    return bin;
}

void A_InitEx( aFatalf_t fatalFn, aPrintf_t printFn, size_t smallBinSize, size_t normalBinSize, size_t staticBinSize ) {
    STATIC_ASSERT( MIN_CHUNK_SIZE > sizeof(chunk_t), min_chunk_size_should_be_larger_than_chunk_t );
    if ( fatalFn != NULL ) {
        a_fatalFn = fatalFn;
    }
    if ( printFn != NULL ) {
        a_printFn = printFn;
    }
    smallBinSize = smallBinSize == 0 ? SMALL_BIN_SIZE : smallBinSize;
    normalBinSize = normalBinSize == 0 ? NORMAL_BIN_SIZE : normalBinSize;
    staticBinSize = staticBinSize == 0 ? STATIC_BIN_SIZE : staticBinSize;
    a_smallBin = A_NewBin( smallBinSize, "SMALL" );
    a_normalBin = A_NewBin( normalBinSize, "NORMAL" );
    a_staticBin = A_NewBin( staticBinSize, "STATIC" );
    float mb = 1024. * 1024.;
    a_printFn( "Memory allocator initialized %dKb small bin, %.1fMb normal bin, %.1fMb static bin\n", 
                smallBinSize / 1024, 
                normalBinSize / mb, 
                staticBinSize / mb );
}

void A_Init( void ) {
    A_InitEx( NULL, NULL, 0, 0, 0 );
}

static bin_t* A_GetBin( chunkType_t type ) {
    bin_t *bin = NULL;

    if ( type == CT_SMALL ) {
        bin = a_smallBin;
    } else if ( type == CT_NORMAL ) {
        bin = a_normalBin;
    } else if ( type == CT_STATIC ) {
        bin = a_staticBin;
    }

    return bin; 
}

#ifdef ALLOC_DEBUG
static const char* A_DebugString( const chunk_t *ch ) {
    const char *binName = "NONE";

    bin_t *bin = A_GetBin( ch->type );
    if ( bin ) {
        binName = bin->name;
    }

    return va( "type: %s\nnum bytes: %s\nfile: %s\nline: %d\n", binName, ch->debug.label, ch->debug.file, ch->debug.line );
}

static void A_CheckValid( chunk_t *chunk ) {
    int     guard;
    
    if ( chunk->guard != CHUNK_GUARD ) {
        a_fatalFn( "A_CheckValid: chunk without start guard\n%s\n", A_DebugString( chunk ) );
    }

    guard = *A_GetTrailGuard( chunk );
    if ( guard != CHUNK_GUARD ) {
        a_fatalFn( "A_CheckValid: chunk without trail guard\n%s\n", A_DebugString( chunk ) );
    }
}
#endif

void A_Done( void ) {
#ifdef ALLOC_DEBUG
    chunk_t *ch;

    for ( ch = a_smallBin->cap.next; ch != &a_smallBin->cap; ch = ch->next ) {
        if ( ch->type != CT_NONE ) {
            A_CheckValid( ch );
            a_fatalFn( "ERROR: F_Done: Leaks found %s\n", A_DebugString( ch ) );
        }
    }
    free( a_smallBin );

    for ( ch = a_normalBin->cap.next; ch != &a_normalBin->cap; ch = ch->next ) {
        if ( ch->type != CT_NONE ) {
            A_CheckValid( ch );
            a_fatalFn( "ERROR: F_Done: Leaks found %s\n", A_DebugString( ch ) );
        }
    }
    free( a_normalBin );

    for ( ch = a_staticBin->cap.next; ch != &a_staticBin->cap; ch = ch->next ) {
        if ( ch->type != CT_NONE ) {
            A_CheckValid( ch );
        }
    }
    free( a_staticBin );
#endif
    a_printFn( "Allocator done\n" );
}

static size_t A_GetChunkOverhead( void ) {
    size_t size = 0;
    
    size += sizeof( chunk_t );
    size += sizeof( int );      // for trail guard
    
    return size;
}

static chunk_t* A_GetChunk( void *ptr ) {
    return ( chunk_t* )( ( byte* )ptr - sizeof( chunk_t ) );
}

static void* A_GetPtr( chunk_t *chunk ) {
    return ( byte* )chunk + sizeof( *chunk );
}

static void A_TrySplitChunk( chunk_t *chunk, size_t suggestedSize ) {
    size_t size;

    // make sure there is at least MIN_CHUNK_SIZE bytes on both sides
    if ( suggestedSize < MIN_CHUNK_SIZE ) {
        size = MIN_CHUNK_SIZE;
    } else {
        size = suggestedSize;
    }

    size_t newSize = chunk->size - size;

    if ( newSize >= MIN_CHUNK_SIZE ) {
        // split this chunk in two
        chunk_t *newFree = ( chunk_t* )( ( byte* )chunk + size );

        newFree->type = CT_NONE;
        newFree->size = newSize;
        
        newFree->prev = chunk;
        newFree->next = chunk->next;
        newFree->next->prev = newFree;
        newFree->prev->next = newFree;

        chunk->size = size;
    }
}

#ifdef ALLOC_DEBUG
void* A_MallocTypeDebug( int type, size_t size, const char *label, const char *file, int line ) {
    size_t     allocSize = size;
#else
void* A_MallocType( int type, size_t size ) {
#endif
    chunk_t *chunk;
    int     *trailGuard;

    bin_t   *bin = A_GetBin( type );

    size += A_GetChunkOverhead();

    for ( chunk = bin->rover; chunk->type != CT_NONE || chunk->size < size; chunk = chunk->next ) {
        if ( chunk == bin->rover->prev ) {
            /*
            FILE *f;

            f = fopen( "out.txt", "wb" );
            for ( chunk = bin->rover; chunk != start; chunk = chunk->next ) {
                fprintf( f, "size: %d %s \"%s\"\n", chunk->size, chunk->type == CT_NONE ? "free" : "used", chunk->debug.file  );
            }
            fclose( f );
            */


#ifdef ALLOC_DEBUG
            a_fatalFn( "A_MallocTypeDebug: out of memory in bin %s, label: %s, file: %s, line: %d \n", bin->name, label, file, line );
#else
            a_fatalFn( "A_MallocType: out of memory in bin %s\n", bin->name );
#endif
        }
    }

    // mark as allocated
    chunk->type = type;

    // split this chunk if big enough
    A_TrySplitChunk( chunk, size );
    
    // mark the buffer at both ends so overruns are handled
    chunk->guard = CHUNK_GUARD;
    trailGuard = A_GetTrailGuard( chunk );
    *trailGuard = CHUNK_GUARD;

    bin->used += chunk->size;
    bin->rover = chunk->next;

#ifdef ALLOC_DEBUG
    chunk->debug.label = label;
    chunk->debug.file = file;
    chunk->debug.line = line;
    chunk->debug.allocSize = allocSize;
#endif

    return A_GetPtr( chunk );
}

#ifdef ALLOC_DEBUG
void* A_MallocDebug( size_t size, const char *label, const char *file, const int line ) {
#else
void* A_Malloc( size_t size ) {
#endif

#ifdef ALLOC_DEBUG
    return A_MallocTypeDebug( CT_NORMAL, size, label, file, line );
#else
    return A_MallocType( CT_NORMAL, size );
#endif
}

#ifdef ALLOC_DEBUG
void* A_MallocZeroDebug( size_t size, const char *label, const char *file, int line ) {
#else
void* A_MallocZero( size_t size ) {
#endif
    byte *buf;

#ifdef ALLOC_DEBUG
    buf = A_MallocDebug( size, label, file, line );
#else
    buf = A_Malloc( size );
#endif

    memset( buf, 0, size );
    return buf;
}

#ifdef ALLOC_DEBUG
void* A_StaticDebug( size_t size, const char *label, const char *file, int line ) {
#else
void* A_Static( size_t size ) {
#endif

    byte *buf;
    
    a_printFn( "static buffer allocation: %dk\n", size / 1024 );

#ifdef ALLOC_DEBUG
    a_printFn( "%s\n", label );
    buf = A_MallocTypeDebug( CT_STATIC, size, label, file, line );
#else
    buf = A_MallocType( CT_STATIC, size );
#endif

    memset( buf, 0, size );
    return buf;
}

void A_Free( void *ptr ) {
    bin_t   *bin;
    chunk_t *chunk, *other;

    if ( ! ptr ) {
        return;
    }

    chunk = A_GetChunk( ptr );

#ifdef ALLOC_DEBUG
    if ( chunk->type == CT_NONE ) {
        a_fatalFn( "A_Free: chunk already freed\n" );
    }
    A_CheckValid( chunk );
#endif

    bin = A_GetBin( chunk->type );
    bin->used -= chunk->size;

    // merge with previous
    other = chunk->prev;
    if ( other->type == CT_NONE ) {
        other->size += chunk->size;
        other->next = chunk->next;
        other->next->prev = other;
        chunk = other;
    }

    // should be after merge, because rover may point to self
    chunk->type = CT_NONE;
    
    // next alloc will start searching here
    bin->rover = chunk;

    // merge with next
    other = chunk->next;
    if ( other->type == CT_NONE ) {
        chunk->size += other->size;
        chunk->next = other->next;
        chunk->next->prev = chunk;
    }
    
#ifdef ALLOC_DEBUG
    //memset( ( byte* )chunk + sizeof( *chunk ), 0xfe, chunk->size - sizeof( *chunk ) );
#endif
}

#ifdef ALLOC_DEBUG
void* A_ReallocDebug( void *ptr, size_t size, const char *label, const char *file, int line ) {
#else
void* A_Realloc( void *ptr, size_t size ) {
#endif
    chunk_t *chunk, *other;
    byte    *newPtr;

    if ( ! ptr ) {
#ifdef ALLOC_DEBUG
        return A_MallocDebug( size, label, file, line );
#else 
        return A_Malloc( size );
#endif
    }
    
    chunk = A_GetChunk( ptr );

#ifdef ALLOC_DEBUG
    if ( chunk->type == CT_NONE ) {
        a_fatalFn( "A_Realloc: chunk already freed\n%s\n", A_DebugString( chunk ) );
    }
    A_CheckValid( chunk );
#endif
    
    size_t totalSizeNeeded = size + A_GetChunkOverhead();
    
    // we never shrink chunks
    if ( totalSizeNeeded <= chunk->size ) {
        return ptr;
    }
    
    other = chunk->next;
    if ( other->type == CT_NONE && chunk->size + other->size >= totalSizeNeeded ) {
        A_TrySplitChunk( other, totalSizeNeeded - chunk->size );
        
        bin_t *bin = A_GetBin( chunk->type );

        // merge both adjacent chunks
        bin->used += other->size;
        chunk->size += other->size;
        chunk->next = other->next;
        chunk->next->prev = chunk;
        
        int *trailGuard = A_GetTrailGuard( chunk );
        *trailGuard = CHUNK_GUARD;
        
        // we might break the rover, so reset it here
        bin->rover = chunk->next;

        // unchanged block pointer
        newPtr = ptr;
    } else {
        size_t oldSize = chunk->size - A_GetChunkOverhead();
#ifdef ALLOC_DEBUG
        newPtr = A_MallocDebug( size, label, file, line );
#else 
        newPtr = A_Malloc( size );
#endif
        memcpy( newPtr, ptr, oldSize );
        A_Free( ptr );
    }
    
    return newPtr;
}

#ifdef ALLOC_DEBUG
char* A_StrDupDebug( const char *str, const char *file, int line ) {
#else
char* A_StrDup( const char *str ) {
#endif
    size_t size = strlen( str ) + 1;
    char *dup;
    
#ifdef ALLOC_DEBUG
    dup = A_MallocTypeDebug( CT_SMALL, size, "A_StrDup", file, line );
#else
    dup = A_MallocType( CT_SMALL, size );
#endif
    memcpy( dup, str, size );

    return dup;
}
