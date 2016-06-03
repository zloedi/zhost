#include "zhost.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static varPtr_t r_windowWidth;
static varPtr_t r_windowHeight;

v2_t r_windowSize;

struct rImage_s {
    v2_t size;
    SDL_Texture *texture;
};

#define R_MAX_TEXTURES 256

static SDL_Window *r_window;
static SDL_Renderer *r_renderer;
static rImage_t *r_images;
static int r_numImages;
static color_t r_color;

void R_Color( float red, float green, float blue, float alpha ) {
    r_color = colorrgba( red, green, blue, alpha );
}

void R_SolidRect( float x, float y, float width, float height ) {
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

void R_DrawPic( float x, float y,
                  float width, float height,
                  float s0, float t0,
                  float s1, float t1,
                  rImage_t *img ) {
    R_DrawPicV2( v2xy( x, y ),
                   v2xy( width, height ),
                   v2xy( s0, t0 ),
                   v2xy( s1, t1 ),
                   img );
}

void R_DrawPicV2( v2_t position,
                    v2_t size,
                    v2_t stTopLeft,
                    v2_t stBottomRight,
                    rImage_t *img ) {
    SDL_SetTextureColorMod( img->texture,
            ( Uint8 )( r_color.r * 255 ), 
            ( Uint8 )( r_color.g * 255 ),
            ( Uint8 )( r_color.b * 255 ) );
    SDL_SetTextureAlphaMod( img->texture, ( Uint8 )( r_color.alpha * 255 ) );
    SDL_SetTextureBlendMode( img->texture, SDL_BLENDMODE_BLEND );
    v2_t st0 = v2xy( stTopLeft.x * img->size.x, stTopLeft.y * img->size.y );
    v2_t st1 = v2xy( stBottomRight.x * img->size.x, stBottomRight.y * img->size.y );
    v2_t stsz = v2Sub( st1, st0 );
    SDL_Rect src = {
        .x = ( int )( st0.x + 0.5f ),
        .y = ( int )( st0.y + 0.5f ),
        .w = ( int )( stsz.x + 0.5f ),
        .h = ( int )( stsz.y + 0.5f ),
    };
    SDL_Rect dest = {
        .x = ( int )position.x,
        .y = ( int )position.y,
        .w = ( int )size.x,
        .h = ( int )size.y,
    };
    SDL_RenderCopy( r_renderer, img->texture, &src, &dest );
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

void R_FrameBegin( color_t clearColor ) {
    int w, h;
    SDL_GetWindowSize( r_window, &w, &h );
    r_windowSize = v2xy( w, h );
    SDL_SetRenderDrawColor( r_renderer,
            ( Uint8 )( clearColor.r * 255 ), 
            ( Uint8 )( clearColor.g * 255 ),
            ( Uint8 )( clearColor.b * 255 ),
            ( Uint8 )( clearColor.alpha * 255 ) );
    SDL_RenderClear( r_renderer );
}

#define MIN_WINDOW_WIDTH 800
#define MAX_WINDOW_WIDTH (4*1024)
#define MIN_WINDOW_HEIGHT 600
#define MAX_WINDOW_HEIGHT (4*1024)

void R_FrameEnd() {
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

rImage_t* R_LoadTexture( const char *pathToImage ) {
    int x,y,n;
    unsigned char *data = stbi_load(pathToImage, &x, &y, &n, 0);
    // ... process data if not NULL ...
    // ... x = width, y = height, n = # 8-bit components per pixel ...
    // ... replace '0' with '1'..'4' to force that many components per pixel
    // ... but 'n' will always be the number that it would have been if you said 0
    rImage_t* result;
    if ( data ) {
        result = R_CreateStaticTexture( data, x, y, n );
        CON_Printf( "R_LoadTexture: loaded image \"%s\". stbi error: \"%s\"", pathToImage, stbi_failure_reason() );
    } else {
        result = &r_images[0];
        CON_Printf( "ERROR: R_LoadTexture: failed to load image \"%s\". stbi error: \"%s\"", pathToImage, stbi_failure_reason() );
    }
    stbi_image_free( data );
    return result;
}

rImage_t* R_CreateStaticTexture( const byte *data, int width, int height, int bytesPerPixel ) {
    if ( r_numImages == R_MAX_TEXTURES ) {
        CON_Printf( "ERROR: R_CreateStaticTexture: out of textures" );
        return &r_images[0];
    }
    SDL_Texture *texture = SDL_CreateTexture( r_renderer, 
            SDL_PIXELFORMAT_RGBA8888, 
            SDL_TEXTUREACCESS_STATIC, 
            width, 
            height );
    SDL_UpdateTexture( texture, NULL, data, width * bytesPerPixel );
    rImage_t *img= &r_images[r_numImages];
    img->size = v2xy( width, height );
    img->texture = texture;
    r_numImages++;
    CON_Printf( "R_CreateStaticTexture: created texture. size: %d,%d ; bpp: %d\n", width, height, bytesPerPixel );
    return img;
}

v2_t R_GetWindowSize( void ) {
    return r_windowSize;
}

void R_RegisterVars( void ) {
    r_windowWidth = VAR_RegisterHelp( "rWindowWidth", "1024", "Window width on app start" );
    r_windowHeight = VAR_RegisterHelp( "rWindowHeight", "768", "Window height on app start" );
}

void R_Init( void ) {
    R_InitEx( "zhost app" );
}

void R_InitEx( const char *windowTitle ) {
    if( SDL_InitSubSystem( SDL_INIT_VIDEO ) < 0 ) {
        return SYS_ErrorBox( "R_InitEx: SDL could not initialize video! SDL Error: %s", SDL_GetError() );
    }
    r_window = SDL_CreateWindow( windowTitle ? windowTitle : "missing window title",
                SDL_WINDOWPOS_UNDEFINED, 
                SDL_WINDOWPOS_UNDEFINED, 
                ( int )Clampf( VAR_Num( r_windowWidth ), MIN_WINDOW_WIDTH, MAX_WINDOW_WIDTH ),
                ( int )Clampf( VAR_Num( r_windowHeight ), MIN_WINDOW_HEIGHT, MAX_WINDOW_HEIGHT ),
                SDL_WINDOW_RESIZABLE );
    if( r_window == NULL ) {
        return SYS_ErrorBox( "Window could not be created! SDL Error: %s", SDL_GetError() );
    }
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengl" );
    r_renderer = SDL_CreateRenderer( r_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if ( r_renderer == NULL ) {
        return SYS_ErrorBox( "Renderer could not be created! SDL Error: %s", SDL_GetError() );
    }
    R_FrameBegin( colBlack );
    R_FrameEnd();
    r_images = A_Static( R_MAX_TEXTURES * sizeof( rImage_t ) );
    // white pixel placeholder at index 0
    byte whitePixel[] = { 0xff };
    R_CreateStaticTexture( whitePixel, 1, 1, 1 );
    CON_Printf( "Renderer initialized.\n" );
    R_PrintRendererInfo();
}

