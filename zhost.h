#include "sys.h"
#include "var.h"
#include "cmd.h"
#include "con_public.h"
#include "renderer.h"
#include "sound.h"
#include "events.h"
#include "input.h"

void UT_Init( const char *appName, 
              size_t dynamicMem,
              size_t staticMem, 
              void (*registerVars)( void ),
              void (*init)( void ),
              void (*done)( void ) );

void UT_Loop( void(*frame)( void ), int inputContext );

void UT_RunApp( const char *appName,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*frame)( void ),
                void (*done)( void ),
                int inputContext );
