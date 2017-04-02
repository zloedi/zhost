#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>

#ifndef __cplusplus
typedef enum { false, true } bool_t;
#else
typedef int bool_t;
#endif

#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif 

typedef unsigned char byte;

typedef union {
    float a[4];
    struct { float r, g, b, alpha; };
} color_t;

static const color_t colBlack = { { 0, 0, 0, 1 } };
static const color_t colWhite = { { 1, 1, 1, 1 } };
static const color_t colRed = { { 1, 0, 0, 1 } };
static const color_t colGreen = { { 0, 1, 0, 1 } };
static const color_t colBlue = { { 0, 0, 1, 1 } };
static const color_t colYellow = { { 1, 1, 0, 1 } };
static const color_t colCyan = { { 0, 1, 1, 1 } };
static const color_t colMagenta = { { 1, 0, 1, 1 } };

static inline color_t colorrgb( float r, float g, float b ) {
    color_t c = { { r, g, b, 1 } };
    return c;
}

static inline color_t colorint( unsigned rgb ) { return colorrgb( ( float )( rgb & 255 ) / 255.0f, ( float )( ( rgb >> 8 ) & 255 ) / 255.0f, ( float )( ( rgb >> 16 ) & 255 ) / 255.0f ); }

static inline color_t colorrgba( float r, float g, float b, float a ) {
    color_t c = { { r, g, b, a } };
    return c;
}

static inline color_t colorScaleRGB( color_t c, float scale ) {
    return colorrgba( c.r * scale, c.g * scale, c.b * scale, c.alpha );
}

static inline color_t colorLerp( color_t from, color_t to, float t ) {
    float s = 1.0f - t;
    color_t result;

    result.a[0] = from.a[0] * s + to.a[0] * t;
    result.a[1] = from.a[1] * s + to.a[1] * t;
    result.a[2] = from.a[2] * s + to.a[2] * t;
    result.a[3] = from.a[3] * s + to.a[3] * t;

    return result;
}

typedef union {
    float a[2];

    struct {
        float x, y;
    };
} v2_t;

static v2_t v2zero = { { 0, 0 } };
static v2_t v2one = { { 1, 1 } };

static inline v2_t warnDisabler( void ) {
    return v2one;
}

static inline v2_t v2xy( float x, float y ) {
    v2_t v = { { x, y } };
    return v;
}

static inline v2_t v2v( const float v[2] ) {
    return v2xy( v[0], v[1] );
}

static inline v2_t v2Scale( v2_t v, float s ) {
    return v2xy( v.x * s, v.y * s );
}

static inline v2_t v2Neg( v2_t v ) {
    return v2xy( -v.x, -v.y );
}

static inline v2_t v2Sub( v2_t a, v2_t b ) {
    return v2xy( a.x - b.x, a.y - b.y );
}

static inline float v2CrossV( v2_t a, v2_t b ) {
    return a.x * b.y - a.y * b.x;
}

static inline v2_t v2Cross( float s, v2_t a ) {
    return v2xy( -s * a.y, s * a.x );
}

static inline v2_t v2Add( v2_t a, v2_t b ) {
    return v2xy( a.x + b.x, a.y + b.y );
}

static inline v2_t v2AddScale( v2_t a, float s, v2_t b ) {
    return v2xy( a.x + b.x * s, a.y + b.y * s );
}

static inline float v2Dot( v2_t a, v2_t b ) {
    return a.x * b.x + a.y * b.y;
}

static inline v2_t v2Lerp( v2_t from, v2_t to, float t ) {
    float s = 1.0f - t;
    v2_t result;
    result.a[0] = from.a[0] * s + to.a[0] * t;
    result.a[1] = from.a[1] * s + to.a[1] * t;
    return result;
}

static inline float v2Len( v2_t v ) {
    return sqrtf( v2Dot( v, v ) );
}

static inline v2_t v2Norm( v2_t v ) {
    float len = v2Len( v );

    if ( ! len ) {
        return v2zero;
    }

    float inv = 1 / len;
    return v2Scale( v, inv );
}

static inline float v2Angle( v2_t v ) {
    float angle;

#define ATAN_EPSILON 0.0001f

    if ( v.x > ATAN_EPSILON ) {
        angle = atanf( v.y / v.x );
    } else if ( v.x < -ATAN_EPSILON ) {
        angle = ( float )M_PI + atanf( v.y / v.x );
    } else if ( v.y > 0 ) {
        angle = ( float )M_PI / 2.0f;
    } else {
        angle = -( float )M_PI / 2.0f;
    }

    return angle;
}

static inline v2_t v2Rotate( v2_t v1, v2_t v2 ) {
    return v2xy( v1.x * v2.x - v1.y * v2.y, v1.x * v2.y + v1.y * v2.x);
}

static inline v2_t v2Perp( v2_t v ) {
    return v2xy( -v.y, v.x );
}

static inline float v2SqDistToPoint( v2_t a, v2_t b ) {
    v2_t d = v2Sub( a, b );
    return v2Dot( d, d );
}

static inline float v2Dist( v2_t a, v2_t b ) {
    return sqrtf( v2SqDistToPoint( a, b ) );
}

// Returns the squared distance between point c and segment ab
static inline float v2SqDistToSegment( v2_t a, v2_t b, v2_t c ) {
    v2_t ab = v2Sub( b, a ), ac = v2Sub( c, a ), bc = v2Sub( c, b );
    float e = v2Dot( ac, ab );

    // Handle cases where c projects outside ab
    if ( e <= 0.0f ) {
        return v2Dot( ac, ac );
    }

    float f = v2Dot( ab, ab );
    if ( e >= f ) {
        return v2Dot( bc, bc );
    }

    // Handle cases where c projects onto ab
    return v2Dot( ac, ac) - e * e / f;
}

static inline void v2OBBToPoly( v2_t center, v2_t axis1, v2_t axis2, float extent1, float extent2, v2_t out[4] ) {
    v2_t v1 = v2Scale( axis1, extent1 );
    v2_t v2 = v2Scale( axis2, extent2 );
    v2_t d1 = v2Add( v1, v2 );
    v2_t d2 = v2Sub( v2, v1 );

    out[0] = v2Add( center, d1 );
    out[1] = v2Add( center, d2 );
    out[2] = v2Sub( center, d1 );
    out[3] = v2Sub( center, d2 );
}

#define VA_SIZE 1024
#ifdef __linux__
#define stricmp strcasecmp
#endif

static inline char* va (const char *fmt, ...) {
    va_list argptr;
    static char buf[VA_SIZE];
    va_start (argptr, fmt);
    vsnprintf (buf, VA_SIZE - 1, fmt, argptr);
    va_end (argptr);
    buf[VA_SIZE - 1] = '\0';
    return buf;
}

static inline size_t Minsz( size_t a, size_t b ) {
    return a < b ? a : b;
}

static inline size_t Maxsz( size_t a, size_t b ) {
    return a > b ? a : b;
}

static inline size_t Clampsz( size_t v, size_t min, size_t max ) {
    return Maxsz( min, Minsz( v, max ) );
}

static inline unsigned Maxu( unsigned a, unsigned b ) {
    return a > b ? a : b;
}

static inline unsigned Max3u( unsigned a, unsigned b, unsigned c ) {
    return Maxu( Maxu( a, b ), c );
}

static inline unsigned Minu( unsigned a, unsigned b ) {
    return a < b ? a : b;
}

static inline unsigned Min3u( unsigned a, unsigned b, unsigned c ) {
    return Minu( Minu( a, b ), c );
}

static inline unsigned Clampu( unsigned v, unsigned min, unsigned max ) {
    return Maxu( min, Minu( v, max ) );
}

static inline int Maxi( int a, int b ) {
    return a > b ? a : b;
}

static inline int Max3i( int a, int b, int c ) {
    return Maxi( Maxi( a, b ), c );
}

static inline int Mini( int a, int b ) {
    return a < b ? a : b;
}

static inline int Min3i( int a, int b, int c ) {
    return Mini( Mini( a, b ), c );
}

static inline int Clampi( int v, int min, int max ) {
    return Maxi( min, Mini( v, max ) );
}

static inline float Maxf( float a, float b ) {
    return a > b ? a : b;
}

static inline float Minf( float a, float b ) {
    return a < b ? a : b;
}

static inline float Clampf( float v, float min, float max ) {
    return Maxf( min, Minf( v, max ) );
}

typedef union c2_s {
    struct {
        int x, y;
    };
    int a[2];
} c2_t;

static inline c2_t c2xy( int x, int y ) {
    c2_t c = { { x, y } };
    return c;
}

static const c2_t c2zero = { .a = { 0, 0 } };
static const c2_t c2one = { .a = { 1, 1 } };

static inline v2_t v2c2( c2_t c ) {
    return v2xy( ( float )c.x, ( float )c.y );
}

static inline c2_t c2v2( v2_t v ) {
    return c2xy( ( int )v.x, ( int )v.y );
}

static inline c2_t c2Perp( c2_t c ) {
    return c2xy( -c.y, c.x );
}

static inline c2_t c2Add( c2_t a, c2_t b ) {
    return c2xy( a.x + b.x, a.y + b.y );
}

static inline c2_t c2Sub( c2_t a, c2_t b ) {
    return c2xy( a.x - b.x, a.y - b.y );
}

static inline c2_t c2Abs( c2_t c ) {
    return c2xy( abs( c.x ), abs( c.y ) );
}

static inline int c2Dot( c2_t a, c2_t b ) {
    return a.x * b.x + a.y * b.y;
}

static inline int c2CrossC( c2_t a, c2_t b ) {
    return a.x * b.y - a.y * b.x;
}

static inline c2_t c2Mul( c2_t a, c2_t b ) {
    return c2xy( a.x * b.x, a.y * b.y );
}

static inline c2_t c2Div( c2_t a, c2_t b ) {
    return c2xy( a.x / b.x, a.y / b.y );
}

static inline c2_t c2Divs( c2_t a, int s ) {
    return c2xy( a.x / s, a.y / s );
}

//static inline c2_t c2Rand( void ) {
//    return c2xy( COM_Rand(), COM_Rand() );
//}

static inline c2_t c2Mod( c2_t a, c2_t m ) {
    return c2xy( a.x % m.x, a.y % m.y );
}

static inline c2_t c2Mods( c2_t a, int s ) {
    return c2xy( a.x % s, a.y % s );
}

static inline c2_t c2Scale( c2_t a, int s ) {
    return c2xy( a.x * s, a.y * s );
}

static inline c2_t c2Neg( c2_t c ) {
    return c2xy( -c.x, -c.y );
}

static inline bool_t c2Equal( c2_t a, c2_t b ) {
    return a.x == b.x && a.y == b.y;
}

static inline c2_t c2Adjacent4( c2_t c, int index ) {
    int ybit = index & 1;
    int xbit = ~ybit & 1;

    int x = ( index & ( xbit << 1 ) ) - xbit;
    int y = ( index & ( ybit << 1 ) ) - ybit;

    return c2Add( c, c2xy( x, y ) );
}

static inline c2_t c2Max( c2_t a, c2_t b ) {
    return c2xy( Maxi( a.x, b.x ), Maxi( a.y, b.y ) );
}

static inline c2_t c2Min( c2_t a, c2_t b ) {
    return c2xy( Mini( a.x, b.x ), Mini( a.y, b.y ) );
}

static inline c2_t c2Mins( c2_t c, int s ) {
    return c2xy( Mini( c.x, s ), Mini( c.y, s ) );
}

static inline c2_t c2Maxs( c2_t c, int s ) {
    return c2xy( Maxi( c.x, s ), Maxi( c.y, s ) );
}

static inline c2_t c2Clamps( c2_t c, int min, int max ) {
    return c2xy( Clampi( c.x, min, max ), Clampi( c.y, min, max ) );
}

static inline c2_t c2Clamp( c2_t c, c2_t min, c2_t max ) {
    return c2xy( Clampi( c.x, min.x, max.x ), Clampi( c.y, min.y, max.y ) );
}

static inline int c2SqrLen( c2_t c ) {
    return c2Dot( c, c );
}

static inline void DbgPrintToStdout( const char *fmt, ... ) {
    va_list argptr;
    va_start( argptr, fmt );
    vfprintf( stdout, fmt, argptr );
    va_end (argptr);
    fflush( stdout );
}

#ifndef DBG_PRINT
#define DBG_PRINT DbgPrintToStdout
#endif

static inline void DbgDump( const void *mem, size_t numBytes ) {
    for ( size_t i = 0; i < numBytes; i++ ) {
        int c = ( ( byte* )mem )[i];
        if ( c >= 32 && c <= 126 ) {
            putc( c, stdout );
        } else {
            putc( '.', stdout );
        }
    }
    printf( "\n" );
    fflush( stdout );
}

#define PrintInt(s)          DBG_PRINT("%s: %d\n",#s,(int)(s))
#define PrintIntHex(s)       DBG_PRINT("%s: 0x%x\n",#s,(int)(s))
#define PrintUint(s)         DBG_PRINT("%s: %u\n",#s,(unsigned)(s))
#define PrintScalarHex(s)    DBG_PRINT("%s: 0x%x\n",#s,(*(int*)(&(s))))
#define PrintScalar(s)       DBG_PRINT("%s: %f\n",#s,(float)(s))
#define PrintSizeT(s)        DBG_PRINT("%s: %zu\n",#s,(size_t)(s))
#define PrintPointer(p)      DBG_PRINT("%s: %p\n",#p,(void*)(p))
#define PrintString(s)       DBG_PRINT("%s: \"%s\"\n",#s,(s))
#define PrintV2(v)           DBG_PRINT("%s: %f,%f\n",#v,(v).x,(v).y)
#define PrintC2(c)           DBG_PRINT("%s: %d,%d\n",#c,(c).x,(c).y)
#define PrintColor(c)        DBG_PRINT("%s: %f %f %f %f\n",#c,(c).r,(c).g,(c).b,(c).a)
#define PrintMem(m,n)        DbgDump((m),(n))

#define STATIC_ASSERT(expr, msg)               \
{                                              \
    char STATIC_ASSERTION__##msg[(expr)?1:-1]; \
    (void)STATIC_ASSERTION__##msg[0];          \
}
