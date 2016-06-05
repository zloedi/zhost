#include "sys.h"
#include "var.h"
#include "cmd.h"
#include "con_public.h"
#include "renderer.h"

typedef struct {
    //int    numFrames;
    int  timeDelta;
    v2_t cursorPosition;
    //bool_t keyPresses[256];
} utFrameParams_t;

void UT_RunApp( const char *orgName, 
                const char *appName,
                const char *windowTitle,
                bool_t showCursor,
                color_t clearColor,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*frame)( const utFrameParams_t* ),
                void (*done)( void ) );
