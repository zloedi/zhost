#include "host.h"
#include "zps2_public.h"

int main( int argc, char *argv[] ) {
    if ( SDL_InitSubSystem( SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ) {
        SYS_ErrorBox( "main: SDL could not initialize! SDL Error: %s", SDL_GetError() );
    }
    SYS_SampleTime();
    atexit( X_AtExit );

    // allocator before all else
    A_InitEx( SYS_ErrorBox, CON_Printf );

    // some core utilites are initialized without vars
    SYS_Init();
    CON_Init();
    CMD_Init();
    VAR_Init();

    // RegisterVars block comes before Init
    CON_RegisterVars();

	// overwrite vars everywhere
	VAR_ReadCfg();

    // the Init block comes after the vars are read and overwritten
    R_Init();
    TTS_Init();

    // the Start block comes after the renderer is initialized
    CON_Start();

    bool_t quit = false;
    do {
        quit = X_ProcessEvents();
        R_FrameBegin( colorrgb( 0.1f, 0.1f, 0.1f ) );
		CON_Frame();
        R_FrameEnd();
        SYS_SampleTime();
    } while ( ! quit );
    return 0;
}
