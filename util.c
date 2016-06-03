#include "zhost.h"

static void UT_Done_f( void ) {
    CON_Stop();
    R_Done();
    VAR_Done();
    CMD_Done();
    CON_Done();
    SYS_Done();
    A_Done();
    SDL_Quit();
}

static void UT_ProcessKey( int code, bool_t down ) {
    // try the console
    if ( CON_OnKeyboard( code, down ) ) {
        return;
    }
    // TODO:
    // GUI widgets
    //if ( WG_OnKeyboard( code, ch, down ) )
    //  return;
    // application specific callback
    //if ( sys_onKeyCallback( code, ch, down) )
    //  return;
    // command key bindings
    //K_OnKeyboard( code, down );
}

static bool_t UT_ProcessEvents( utFrameParams_t *outParams ) {
    bool_t quit = false;
    SDL_Event event;
    while ( SDL_PollEvent( &event ) ) {
        int code = event.key.keysym.sym;
        static bool_t laltDown, raltDown, ctlDown;
        switch (event.type) {
            case SDL_TEXTINPUT:
                CON_OnText( event.text.text );
                break;

            case SDL_KEYDOWN:
                if ( code == SDLK_LCTRL || code == SDLK_RCTRL ) {
                    ctlDown = true;
                } else if ( code == SDLK_LALT ) {
                    laltDown = true;
                } else if ( code == SDLK_RALT ) {
                    raltDown = true;
                }
                // quit on alt + F4 is hardcoded
                if ( ( raltDown || laltDown ) && code == SDLK_F4 ) {
                    quit = true;
                }
                // console toggle is hardcoded
                else if ( code == SDLK_BACKQUOTE ) {
                    CON_Toggle( ctlDown );
                } 
                else {
                    UT_ProcessKey( code, true );
                }
                break;

            case SDL_KEYUP:
                if ( code == SDLK_LCTRL || code == SDLK_RCTRL ) {
                    ctlDown = false;
                } else if ( code == SDLK_LALT ) {
                    laltDown = false;
                } else if ( code == SDLK_RALT ) {
                    raltDown = false;
                } 
                // printscreen is hardcoded
                if ( code == SDLK_PRINTSCREEN || code == SDLK_SYSREQ ) {
                    R_SaveScreenshot();
                } 
                else {
                    UT_ProcessKey( code, false );
                }
                break;

            case SDL_QUIT:
                quit = true;
                break;

            default:;
        }
    }
    int x, y;
    SDL_GetMouseState( &x, &y );
    outParams->cursorPosition = v2xy( x, y );
    return quit;
}

#define SAFE_CALL(f,...) (f?f(__VA_ARGS__):f)

void UT_RunApp( const char *orgName, 
                const char *appName,
                const char *windowTitle,
                color_t clearColor,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*frame)( const utFrameParams_t* ),
                void (*done)( void ) ) {
    // functions registered with atexit are called in reverse order
    atexit( UT_Done_f );
    if ( done ) {
        atexit( done );
    }

    // allocator before all else
    A_InitEx( SYS_ErrorBox, CON_Printf );

    // some core utilites are initialized without vars
    SYS_InitEx( orgName, appName );
    CON_Init();
    CMD_Init();
    VAR_Init();

    // RegisterVars before init
    R_RegisterVars();
    CON_RegisterVars();
    SAFE_CALL( registerVars );

    // overwrite vars everywhere
    VAR_ReadCfg();

    // Inits come after the vars are read and overwritten
    R_InitEx( windowTitle );

    // some parts of the console need a working renderer
    CON_Start();

    // make sure the console can print on the log
    CON_Frame();

    // app specific init
    SAFE_CALL( init );

    // main loop
    bool_t quit = false;
    int milliseconds = SYS_RealTime();
    do {
        utFrameParams_t params;
        quit = UT_ProcessEvents( &params );
        R_FrameBegin( clearColor );
        CON_Frame();

        // store delta time
        int ms = SYS_SampleTime();
        params.timeDelta = ms - milliseconds;
        milliseconds = ms;

        // app Frame before frame end
        SAFE_CALL( frame, &params );

        R_FrameEnd();
    } while ( ! quit );
}
