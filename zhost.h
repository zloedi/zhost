#include "sys.h"
#include "var.h"
#include "cmd.h"
#include "con_public.h"
#include "renderer.h"

void UT_RunApp( const char *orgName, 
                const char *appName,
                const char *windowTitle,
                color_t clearColor,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*start)( void ),
                void (*frame)( void ),
                void (*done)( void ) );
