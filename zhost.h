#include "sys.h"
#include "var.h"
#include "cmd.h"
#include "con_public.h"
#include "renderer.h"
#include "events.h"
#include "input.h"

void UT_RunApp( const char *appName,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*frame)( void ),
                void (*done)( void ) );
