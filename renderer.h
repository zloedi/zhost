typedef struct rImage_s rImage_t;

extern rImage_t *r_fallbackTexture;

void R_SetClearColor( color_t color );
void R_SetWindowTitle( const char *windowTitle );

void R_RegisterVars( void );
void R_Init( void );
void R_FrameBegin( void );
void R_FrameEnd( void );
void R_Done( void );
void R_SaveScreenshot( void );

// FIXME: these used to be OpenGL routines
// FIXME: st coordinates larger than 1 dont work under SDL
// FIXME: use scale?
void R_ColorC( color_t color );
void R_Color( float red, float green, float blue, float alpha );
void R_BlendRect( float x, float y, float width, float height );
void R_BlendPic( float x, float y,
                float width, float height,
                float s0, float t0,
                float s1, float t1,
                rImage_t *img );
void R_BlendPicV2( v2_t position,
                  v2_t size,
                  v2_t stTopLeft,
                  v2_t stBottomRight,
                  rImage_t *img );

void R_BlitToTexture( rImage_t *image, const byte *data, c2_t size, int bytesPerPixel );

rImage_t* R_BlankStaticTexture( void );
const char* R_ImagePath( const char *pathToImage );
byte* R_LoadImageRaw( const char *pathToImage, c2_t *sz, int *n, int forcedBPP );
void R_DestroyImageRaw( byte *data );
rImage_t* R_LoadStaticTexture( const char *pathToImage );
rImage_t* R_LoadStaticTextureEx( const char *pathToImage, v2_t *outSize );
rImage_t* R_CreateStaticTexture( const byte *data, c2_t size, int bytesPerPixel );
v2_t R_GetWindowSize( void );
void R_ShowCursor( bool_t show );

void R_DBGLine( v2_t start, v2_t end );
void R_DBGLineBegin( v2_t start );
void R_DBGLineBeginColor( v2_t start, color_t color );
void R_DBGLineTo( v2_t pt );
void R_DBGAABB( v2_t min, v2_t max );
void R_DBGVector( v2_t origin, v2_t vector );
void R_DBGVectorColor( v2_t origin, v2_t vector, color_t color );

SDL_Texture* R_CreateStaticTexFromBitmap( const byte *bitmap, c2_t bitmapSize, int bytesPerPixel );
SDL_Texture* R_FallbackTexture( void );
SDL_Renderer *r_renderer;
