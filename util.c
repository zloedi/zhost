#include "zhost.h"

static void UT_AtExit( void ) {
    // End block before the renderer is killed
    CON_End();

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

// TODO: move much of this to SYS
static bool_t UT_ProcessEvents( void ) {
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
    return quit;
}

#define SAFE_CALL(f,...) (f?f(__VA_ARGS__):f)

// TODO: split this to give users more control
void UT_RunApp( const char *orgName, 
                const char *appName,
                const char *windowTitle,
                color_t clearColor,
                void (*registerVars)( void ),
                void (*init)( void ),
                void (*start)( void ),
                void (*frame)( void ),
                void (*atExit)( void ) ) {
    atexit( UT_AtExit );

    // allocator before all else
    A_InitEx( SYS_ErrorBox, CON_Printf );

    // some core utilites are initialized without vars
    SYS_InitEx( orgName, appName );
    CON_Init();
    CMD_Init();
    VAR_Init();

    // RegisterVars before app Init
    R_RegisterVars();
    CON_RegisterVars();
    SAFE_CALL( registerVars );

    // overwrite vars everywhere
    VAR_ReadCfg();

    // the app Init comes after the vars are read and overwritten
    R_InitEx( windowTitle );
    SAFE_CALL( init );

    // the app Start comes after the renderer is initialized
    CON_Start();
    SAFE_CALL( start );

    bool_t quit = false;
    do {
        quit = UT_ProcessEvents();
        R_FrameBegin( clearColor );
        CON_Frame();
        // app Frame before frame end
        SAFE_CALL( frame );
        R_FrameEnd();
        SYS_SampleTime();
    } while ( ! quit );
}
