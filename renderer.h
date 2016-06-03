typedef struct rImage_s rImage_t;

void R_RegisterVars( void );
void R_InitEx( const char *windowTitle );
void R_Init( void );
void R_FrameBegin( color_t clearColor );
void R_FrameEnd( void );
void R_Done( void );
void R_SaveScreenshot( void );

// FIXME: these used to be OpenGL routines
// FIXME: st coordinates larger than 1 dont work under SDL
// FIXME: use scale?
void R_Color( float red, float green, float blue, float alpha );
void R_SolidRect( float x, float y, float width, float height );
void R_DrawPic( float x, float y,
                float width, float height,
                float s0, float t0,
                float s1, float t1,
                rImage_t *img );
void R_DrawPicV2( v2_t position,
                  v2_t size,
                  v2_t stTopLeft,
                  v2_t stBottomRight,
                  rImage_t *img );

rImage_t* R_LoadTexture( const char *pathToImage );
rImage_t* R_CreateStaticTexture( const byte *data, int width, int height, int bytesPerPixel );
v2_t R_GetWindowSize( void );
