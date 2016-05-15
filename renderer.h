typedef enum {
    RI_NONE            = 0,
    //RI_USE_MIPS      = 1 << 0,
    //RI_CLAMP         = 1 << 1,
    //RI_RAW_PATH      = 1 << 2, // the entire path is supplied in the form "/path/to/image.tga" instead of "image"
    //RI_NO_MIN_FILTER = 1 << 3,
    //RI_NO_MAG_FILTER = 1 << 4,
    //RI_NO_FILTER     = ( RI_NO_MIN_FILTER | RI_NO_MAG_FILTER ),
} riFlags_t;

typedef struct {
    float screenWidth;
    float screenHeight;
} rInfo_t;

extern rInfo_t r_info;

void R_Init( const char *windowTitle );
void R_FrameBegin( color_t clearColor );
void R_FrameEnd( void );
void R_Done( void );
void R_SaveScreenshot( void );

// FIXME: these used to be OpenGL routines
// FIXME: st coordinates larger than 1 dont work under SDL
// FIXME: use scale?
void R2D_Color( float red, float green, float blue, float alpha );
void R2D_SolidRect( float x, float y, float width, float height );
void R2D_DrawPic( float x, float y,
                  float width, float height,
                  float s0, float t0,
                  float s1, float t1,
                  int texture );
void R2D_DrawPicV2( v2_t position,
                  v2_t size,
                  v2_t stTopLeft,
                  v2_t stBottomRight,
                  int texture );

int RI_CreateStaticTexture( const byte *data, int width, int height, riFlags_t flags, int bytesPerPixel );
