#include "zhost.h"

static void UT_Done_f( void ) {
    CON_Stop();
    I_Done();
    R_Done();
    VAR_Done();
    CMD_Done();
    CON_Done();
    SYS_Done();
    A_Done();
    SDL_Quit();
}

#define SAFE_CALL(f,...) (f?f(__VA_ARGS__):f)

void UT_Init( const char *appName, 
              size_t dynamicMem,
              size_t staticMem, 
              void (*registerVars)( void ),
              void (*init)( void ),
              void (*done)( void ) ) {
    // functions registered with atexit are called in reverse order
    atexit( UT_Done_f );
    if ( done ) {
        atexit( done );
    }

    // allocator before all else
    A_InitEx( SYS_Fatal, CON_Printf, 0, dynamicMem, staticMem );

    // some core utilites are initialized without vars
    SYS_InitEx( NULL, appName );
    CON_Init();
    CMD_Init();
    VAR_Init();

    // RegisterVars before init
    R_RegisterVars();
    I_RegisterVars();
    CON_RegisterVars();
    SAFE_CALL( registerVars );

    // overwrite vars everywhere
    VAR_ReadCfg();

    // Inits come after the vars are read and overwritten
    R_Init();
    R_SetClearColor( colorrgb( 0.1, 0.1, 0.1 ) );

    // some parts of the console need a working renderer
    CON_Start();

    // app specific init
    SAFE_CALL( init );
}

// sample loop, write your own if you need more control
void UT_Loop( void(*frame)( void ), int inputContext ) {
    bool_t quit = false;
    SYS_SampleTime();
    do {
        quit = E_DispatchEvents( inputContext );
        R_FrameBegin();
        SYS_SampleTime();
        // app Frame before frame end
        SAFE_CALL( frame );
        // console on top of everything
        CON_Frame();
        R_FrameEnd();
    } while ( ! quit );
}

// this is the fastest way to setup a zhost app
// just call this one in your main() and you are set
void UT_RunApp( const char *appName,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*frame)( void ),
                void (*done)( void ),
                int inputContext ) {
    UT_Init( appName, 0, 0, registerVars, init, done );
    R_SetWindowTitle( appName );
    UT_Loop( frame, inputContext );
}
