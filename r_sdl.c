#include "zhost.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_MALLOC A_Malloc
#define STBI_REALLOC A_Realloc
#define STBI_FREE A_Free
#include "stb_image.h"

#define R_MAX_TEXTURES 256
#define MIN_WINDOW_WIDTH 800
#define MAX_WINDOW_WIDTH (4*1024)
#define MIN_WINDOW_HEIGHT 600
#define MAX_WINDOW_HEIGHT (4*1024)

static var_t *r_windowWidth;
static var_t *r_windowHeight;
static v2_t r_windowSize;
static color_t r_clearColor;

struct rImage_s {
    c2_t size;
    SDL_Texture *texture;
};

rImage_t *r_fallbackTexture;

static SDL_Window *r_window;
static rImage_t *r_images;
static int r_numImages;
static color_t r_color;

typedef struct {
    v2_t position;
    color_t color;
    char *text;
} rDBGPoint_t;

static rDBGPoint_t *r_DBGPoints;

SDL_Renderer *r_renderer;

void R_ColorC( color_t color ) {
    r_color = color;
}

void R_Color( float red, float green, float blue, float alpha ) {
    r_color = colorrgba( red, green, blue, alpha );
}

void R_BlendRect( float x, float y, float width, float height ) {
    SDL_SetRenderDrawColor( r_renderer,
            ( Uint8 )( r_color.r * 255 ), 
            ( Uint8 )( r_color.g * 255 ),
            ( Uint8 )( r_color.b * 255 ),
            ( Uint8 )( r_color.alpha * 255 ) );
    SDL_SetRenderDrawBlendMode( r_renderer, SDL_BLENDMODE_BLEND );
    SDL_Rect rect = {
        .x = ( int )x,
        .y = ( int )y,
        .w = ( int )width,
        .h = ( int )height,
    };
    SDL_RenderFillRect( r_renderer, &rect );
}

void R_BlendPic( float x, float y,
                  float width, float height,
                  float s0, float t0,
                  float s1, float t1,
                  rImage_t *img ) {
    R_BlendPicV2( v2xy( x, y ),
                   v2xy( width, height ),
                   v2xy( s0, t0 ),
                   v2xy( s1, t1 ),
                   img );
}

void R_BlendPicV2( v2_t position,
                    v2_t size,
                    v2_t st0,
                    v2_t st1,
                    rImage_t *img ) {
    SDL_SetTextureColorMod( img->texture,
            ( Uint8 )( r_color.r * 255 ), 
            ( Uint8 )( r_color.g * 255 ),
            ( Uint8 )( r_color.b * 255 ) );
    SDL_SetTextureAlphaMod( img->texture, ( Uint8 )( r_color.alpha * 255 ) );
    SDL_SetTextureBlendMode( img->texture, SDL_BLENDMODE_BLEND );
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    v2_t stmin = st0, stmax = st1;
    if ( st0.x > st1.x ) {
        flip |= SDL_FLIP_HORIZONTAL;
        stmin.x = st1.x;
        stmax.x = st0.x;
    }
    if ( st0.y > st1.y ) {
        flip |= SDL_FLIP_VERTICAL;
        stmin.y = st1.y; 
        stmax.y = st0.y;
    }
    stmin = v2xy( stmin.x * img->size.x, stmin.y * img->size.y );
    stmax = v2xy( stmax.x * img->size.x, stmax.y * img->size.y );
    v2_t stsz = v2Sub( stmax, stmin );
    SDL_Rect src = {
        .x = ( int )( stmin.x + 0.5f ),
        .y = ( int )( stmin.y + 0.5f ),
        .w = ( int )( stsz.x + 0.5f ),
        .h = ( int )( stsz.y + 0.5f ),
    };
    SDL_Rect dest = {
        .x = ( int )position.x,
        .y = ( int )position.y,
        .w = ( int )size.x,
        .h = ( int )size.y,
    };
    //SDL_RenderCopy( r_renderer, img->texture, &src, &dest );
    const SDL_Point zero = { 0, 0 };
    SDL_RenderCopyEx( r_renderer,
                      img->texture,
                      &src,
                      &dest,
                      0,
                      &zero,
                      flip );
}

rImage_t* R_BlankStaticTexture( void ) {
    byte pixel = { 0xff };
    return R_CreateStaticTexture( &pixel, c2xy( 1, 1 ), 1 );
}

static byte* R_ConvertTextureToRGBA( const byte* src, c2_t srcSz, int srcBytesPerPixel, int *outRGBAPitch ) {
    int rgbaPitch = srcSz.x * 4;
    byte *result = A_Malloc( rgbaPitch * srcSz.y );
    int srcPitch = srcSz.x * srcBytesPerPixel;
    if ( srcBytesPerPixel == 3 ) {
        for ( int y = 0; y < srcSz.y; y++ ) {
            for ( int xd = 0, xs = 0; xd < rgbaPitch; xd += 4, xs += srcBytesPerPixel ) {
                result[xd + 0 + y * rgbaPitch] = src[xs + 0 + y * srcPitch];
                result[xd + 1 + y * rgbaPitch] = src[xs + 1 + y * srcPitch];
                result[xd + 2 + y * rgbaPitch] = src[xs + 2 + y * srcPitch];
                result[xd + 3 + y * rgbaPitch] = 0xff;
            }
        }
    } else if ( srcBytesPerPixel == 1 ) {
        for ( int y = 0; y < srcSz.y; y++ ) {
            for ( int xd = 0, xs = 0; xd < rgbaPitch; xd += 4, xs += srcBytesPerPixel ) {
                result[xd + 0 + y * rgbaPitch] = 0xff;
                result[xd + 1 + y * rgbaPitch] = 0xff;
                result[xd + 2 + y * rgbaPitch] = 0xff;
                result[xd + 3 + y * rgbaPitch] = src[xs + 0 + y * srcPitch];
            }
        }
    }
    *outRGBAPitch = rgbaPitch;
    return result;
}

SDL_Texture* R_CreateStaticTexFromBitmap( const byte *bitmap, c2_t bitmapSize, int bytesPerPixel ) {
    SDL_Texture* result;
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" );
    result = SDL_CreateTexture( r_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 
                                                  bitmapSize.x, bitmapSize.y );
    if ( bytesPerPixel == 4 ) {
        SDL_UpdateTexture( result, NULL, bitmap, bitmapSize.x * 4 );
    } else {
        int rgbaPitch;
        byte *rgbaData = R_ConvertTextureToRGBA( bitmap, bitmapSize, bytesPerPixel, &rgbaPitch );
        SDL_UpdateTexture( result, NULL, rgbaData, rgbaPitch );
        A_Free( rgbaData );
    }
    return result;
}

void R_BlitToTexture( rImage_t *image, const byte *data, c2_t size, int bytesPerPixel ) {
    if ( ! c2Equal( size, image->size ) ) {
        if ( image->texture ) {
            SDL_DestroyTexture( image->texture );
        }
        image->texture = SDL_CreateTexture( r_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 
                                                  size.x, size.y );
        image->size = size;
    }
    if ( bytesPerPixel == 4 ) {
        SDL_UpdateTexture( image->texture, NULL, data, size.x * 4 );
    } else {
        int rgbaPitch;
        byte *rgbaData = R_ConvertTextureToRGBA( data, size, bytesPerPixel, &rgbaPitch );
        SDL_UpdateTexture( image->texture, NULL, rgbaData, rgbaPitch );
        A_Free( rgbaData );
    }
}

void R_SaveScreenshot() {
    CON_Printf( "R_SaveScreenshot not implemented" );
}

static void R_PrintRendererInfo() {
    SDL_RendererInfo info;
    SDL_GetRendererInfo( r_renderer, &info);
    CON_Printf( " name: %s\n", info.name );
    CON_Printf( " max texture width: %d\n", info.max_texture_width );
    CON_Printf( " max texture height: %d\n", info.max_texture_height );
}

void R_FrameBegin( void ) {
    int w, h;
    SDL_GetWindowSize( r_window, &w, &h );
    r_windowSize = v2xy( w, h );
    SDL_SetRenderDrawColor( r_renderer,
            ( Uint8 )( r_clearColor.r * 255 ), 
            ( Uint8 )( r_clearColor.g * 255 ),
            ( Uint8 )( r_clearColor.b * 255 ),
            ( Uint8 )( r_clearColor.alpha * 255 ) );
    SDL_RenderClear( r_renderer );
}

void R_FrameEnd() {
    int n = sb_count( r_DBGPoints );
    int prev = 0;
    for ( int i = 0; i < n; i++ ) {
        rDBGPoint_t pt = r_DBGPoints[i];
        if ( pt.text ) {
            CON_DrawOnScreen( pt.position, pt.color, pt.text );
            A_Free( pt.text );
        } else {
            if (pt.color.alpha > 0.000001f) {
                SDL_SetRenderDrawColor( r_renderer, 
                                        pt.color.r * 255, 
                                        pt.color.g * 255, 
                                        pt.color.b * 255, 
                                        pt.color.alpha * 255 );
            } else {
                rDBGPoint_t prevPt = r_DBGPoints[prev];
                SDL_RenderDrawLine( r_renderer, prevPt.position.x, prevPt.position.y,
                                                pt.position.x, pt.position.y );
            }
        }
        prev = i;
    }
    sb_free( r_DBGPoints );
    r_DBGPoints = NULL;
    SDL_RenderPresent( r_renderer );
    if ( VAR_Changed( r_windowWidth ) || VAR_Changed( r_windowHeight ) ) {
        SDL_SetWindowSize( r_window,
                ( int )Clampf( VAR_Num( r_windowWidth ), MIN_WINDOW_WIDTH, MAX_WINDOW_WIDTH ),
                ( int )Clampf( VAR_Num( r_windowHeight ), MIN_WINDOW_HEIGHT, MAX_WINDOW_HEIGHT ) );
    }
}

void R_Done() {
    SDL_DestroyRenderer( r_renderer );
    SDL_DestroyWindow( r_window );
    CON_Printf( "Renderer done.\n" );
}

rImage_t* R_LoadStaticTexture( const char *pathToImage ) {
    return R_LoadStaticTextureEx( pathToImage, NULL );
}

const char* R_ImagePath( const char *pathToImage ) {
    return va( "%sdata/%s", SYS_BaseDir(), pathToImage );
}

byte* R_LoadImageRaw( const char *pathToImage, c2_t *sz, int *n, int forcedBPP ) {
    byte *data = stbi_load( R_ImagePath( pathToImage ), &sz->x, &sz->y, n, forcedBPP );
    // ... process data if not NULL ...
    // ... x = width, y = height, n = # 8-bit components per pixel ...
    // ... replace '0' with '1'..'4' to force that many components per pixel
    // ... but 'n' will always be the number that it would have been if you said 0
    if ( data ) {
        CON_Printf( "R_LoadImageRaw: loaded image \"%s\"\n", pathToImage );
        CON_Printf( " width:  %d\n", sz->x );
        CON_Printf( " height: %d\n", sz->y );
        CON_Printf( " bpp:    %d\n", *n * 8 );
    } else {
        CON_Printf( "ERROR: R_LoadImageRaw: failed to load image \"%s\". stbi error: \"%s\"\n", pathToImage, stbi_failure_reason() );
    }
    return data;
}

void R_DestroyImageRaw( byte *data ) {
    stbi_image_free( data );
}

rImage_t* R_LoadStaticTextureEx( const char *pathToImage, v2_t *outSize ) {
    // the fallback (white) texture is one on one pixel
    c2_t sz = c2xy( 1, 1 );
    int n;
    rImage_t* result = r_fallbackTexture;
    byte *data = R_LoadImageRaw( pathToImage, &sz, &n, 0 );
    if ( data ) {
        result = R_CreateStaticTexture( data, sz, n );
        stbi_image_free( data );
    }
    if ( outSize ) {
        *outSize = v2c2( sz );
    }
    return result;
}

rImage_t* R_CreateStaticTexture( const byte *data, c2_t size, int bytesPerPixel ) {
    if ( r_numImages == R_MAX_TEXTURES ) {
        CON_Printf( "ERROR: R_CreateStaticTexture: out of textures" );
        return r_fallbackTexture;
    }
    rImage_t *img = &r_images[r_numImages];
    memset( img, 0, sizeof( *img ) );
    R_BlitToTexture( img, data, size, bytesPerPixel );
    r_numImages++;
    CON_Printf( "R_CreateStaticTexture: created texture. size: %d,%d\n", size.x, size.y );
    return img;
}

void R_ShowCursor( bool_t show ) {
    SDL_ShowCursor( show );
}

void R_SetClearColor( color_t color ) {
    r_clearColor = color;
}

v2_t R_GetWindowSize( void ) {
    return r_windowSize;
}

void R_RegisterVars( void ) {
    r_windowWidth = VAR_RegisterHelp( "r_windowWidth", "1024", "Window width on app start" );
    r_windowHeight = VAR_RegisterHelp( "r_windowHeight", "768", "Window height on app start" );
}

void R_SetWindowTitle( const char *windowTitle ) {
    SDL_SetWindowTitle( r_window, windowTitle );
}

void R_Init( void ) {
    if( SDL_InitSubSystem( SDL_INIT_VIDEO ) < 0 ) {
        return SYS_Fatal( "R_InitEx: SDL could not initialize video! SDL Error: %s", SDL_GetError() );
    }
    r_window = SDL_CreateWindow( NULL,
                SDL_WINDOWPOS_UNDEFINED, 
                SDL_WINDOWPOS_UNDEFINED, 
                ( int )Clampf( VAR_Num( r_windowWidth ), MIN_WINDOW_WIDTH, MAX_WINDOW_WIDTH ),
                ( int )Clampf( VAR_Num( r_windowHeight ), MIN_WINDOW_HEIGHT, MAX_WINDOW_HEIGHT ),
                SDL_WINDOW_RESIZABLE );
    if( r_window == NULL ) {
        return SYS_Fatal( "Window could not be created! SDL Error: %s", SDL_GetError() );
    }
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengl" );
    r_renderer = SDL_CreateRenderer( r_window, -1, SDL_RENDERER_ACCELERATED );
    if ( r_renderer == NULL ) {
        return SYS_Fatal( "Renderer could not be created! SDL Error: %s", SDL_GetError() );
    }
    r_images = A_Static( R_MAX_TEXTURES * sizeof( rImage_t ) );
    // white pixel placeholder at index 0
    r_fallbackTexture = R_BlankStaticTexture();
    CON_Printf( "Renderer initialized.\n" );
    R_PrintRendererInfo();
}

//=============================================================================================

void R_DBGLine( v2_t start, v2_t end ) {
    R_DBGLineBegin( start );
    R_DBGLineTo( end );
}

void R_DBGLineBegin( v2_t start ) {
    R_DBGLineBeginColor( start, colWhite );
}

void R_DBGLineBeginColor( v2_t start, color_t color ) {
    rDBGPoint_t pt = {
        .color = color,
        .position = start,
    };
    sb_push( r_DBGPoints, pt );
}

void R_DBGLineTo( v2_t pos ) {
    rDBGPoint_t pt = {
        .position = pos,
    };
    sb_push( r_DBGPoints, pt );
}

void R_DBGAABB( v2_t min, v2_t max ) {
    R_DBGLineBegin( min );
    R_DBGLineTo( v2xy( max.x, min.y ) );
    R_DBGLineTo( max );
    R_DBGLineTo( v2xy( min.x, max.y ) );
    R_DBGLineTo( min );
}

void R_DBGPoint( v2_t pt ) {
    v2_t off = v2Scale( v2one, 2 );
    R_DBGAABB( v2Sub( pt, off ), v2Add( pt, off ) );
}

void R_DBGVector( v2_t origin, v2_t vector ) {
    R_DBGVectorColor( origin, vector, colWhite );
}

void R_DBGVectorColor( v2_t origin, v2_t vector, color_t color ) {
    float sqSize = v2SqLen( vector );
    if ( sqSize > 0.0001f ) {
        float len = sqrtf( sqSize );
        R_DBGLineBeginColor( origin, color );
        v2_t tip = v2Add( origin, vector );
        R_DBGLineTo( tip );
        float tipSize = len / 5;
        v2_t n = v2Scale( vector, 1 / len );
        v2_t pn = v2Perp( n );
        v2_t v = v2Sub( tip, v2Scale( n, tipSize ) );
        v2_t spn = v2Scale( pn, tipSize * 0.5f );
        R_DBGLineBegin( v2Add( v, spn ) );
        R_DBGLineTo( tip );
        R_DBGLineTo( v2Sub( v, spn ) );
    }
}

void R_DBGTextColor( v2_t origin, const char *str, color_t color ) {
    v2_t sz = CON_Measure( str );
    rDBGPoint_t pt = {
        .color = color,
        .position = v2Sub( origin, v2xy(sz.x * 0.5f, sz.y + 5 ) ),
        .text = A_StrDup( str ),
    };
    sb_push( r_DBGPoints, pt );
}

void R_DBGText( v2_t origin, const char *str ) {
    R_DBGTextColor( origin, str, colWhite );
}
