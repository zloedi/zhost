#ifdef _DEBUG
#define ALLOC_DEBUG
#endif

typedef int ( *aPrintf_t )( const char *format, ... );
typedef void ( *aFatalf_t )( const char *format, ... );
void A_InitEx( aFatalf_t fatalFn, aPrintf_t printFn, size_t smallBinSize, size_t normalBinSize, size_t staticBinSize );
void A_Init( void );
void A_Done( void );

#ifdef ALLOC_DEBUG

#define A_Malloc(size)       A_MallocDebug(size, #size, __FILE__, __LINE__)
#define A_MallocZero(size)   A_MallocZeroDebug(size, #size, __FILE__, __LINE__)
#define A_Realloc(ptr,size)  A_ReallocDebug(ptr, size, #size, __FILE__, __LINE__)
#define A_StrDup(str)        A_StrDupDebug(str, __FILE__, __LINE__)
#define A_Static(size)       A_StaticDebug(size, #size, __FILE__, __LINE__ )

void* A_MallocDebug( size_t size, const char *label, const char *file, int line );
void* A_MallocZeroDebug( size_t size, const char *label, const char *file, int line );
void* A_ReallocDebug( void *ptr, size_t size, const char *label, const char *file, int line );
char* A_StrDupDebug( const char *str, const char *file, int line );
void* A_StaticDebug( size_t size, const char *label, const char *file, int line );

#else

void* A_Malloc( size_t size );
void* A_MallocZero( size_t size );
void* A_Realloc( void *ptr, size_t size );
char* A_StrDup( const char *str );
void* A_Static( size_t size );

#endif

void  A_Free( void *ptr );

//=======================================================================================================

#define sb_free(a)         ((a) ? A_Free(stb__sbraw(a)),0 : 0)
#define sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((size_t *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      ((a) = stb__sbgrowf((a), (n), sizeof(*(a))))

static inline void * stb__sbgrowf(void *arr, size_t increment, size_t itemsize)
{
   size_t dbl_cur = arr ? 2*stb__sbm(arr) : 0;
   size_t min_needed = sb_count(arr) + increment;
   size_t m = dbl_cur > min_needed ? dbl_cur : min_needed;
   size_t reallocsz = itemsize * m + sizeof(size_t)*2;
   size_t *p = (size_t *) A_Realloc(arr ? stb__sbraw(arr) : NULL, reallocsz);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
      STRETCHY_BUFFER_OUT_OF_MEMORY ;
      #endif
      return (void *) (2*sizeof(size_t)); // try to force a NULL pointer exception later
   }
}
