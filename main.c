#include "host.h"
#include "zps2_public.h"

static var_t *emuPathToISO;

static const struct {
  int 	         width;
  int 	         height;
  int 	         bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[22 * 8 * 4 + 1];
} gimp_image = {
  22, 8, 4,
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0"
  "\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\377\377\377\377"
  "\377\377\377\377\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0"
  "\377\377\377\377\377\377\377\377\0\0\0\0\377\377\377\377\377\377\377\377"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\377\377"
  "\377\377\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0"
  "\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\377\377\377\377\377\377"
  "\377\377\0\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0"
  "\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\377\377\377\377"
  "\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\377\377\377\377"
  "\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\377\377\377"
  "\377\377\377\377\377\377\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\377\377\377\377\377\377\377\377\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0",
};

static int X_LogoGenTexture( int *width, int *height ) {
    *width = gimp_image.width;
    *height = gimp_image.height;
    return RI_CreateStaticTexture( gimp_image.pixel_data, 
            gimp_image.width, 
            gimp_image.height, 
            0, 
            gimp_image.bytes_per_pixel );
}

void X_AtExit( void ) {
    // End block before the renderer is killed
    CON_End();

    ZPS2_Done();
    R_Done();
    VAR_Done();
    CMD_Done();
    CON_Done();
    SYS_Done();
    A_Done();
    SDL_Quit();
}

static void X_DrawLogo( void ) {
    static int w, h;
    static int logoTex;
    if ( ! logoTex ) {
        logoTex = X_LogoGenTexture( &w, &h );
    }
    R2D_Color( 1, 1, 1, 0.05f );
    float aspect = ( float )w / ( float )h;
    float margin = r_info.screenWidth * 0.2f;
    float dstW = r_info.screenWidth - margin * 2;
    float dstH = dstW / aspect;
    R2D_DrawPic( margin, ( r_info.screenHeight - dstH ) / 2,
            dstW, dstH,
            0, 0,
            1, 1,
            logoTex );
}

static void X_ProcessKey( int code, bool_t down ) {
	// try the console
	if ( CON_OnKeyboard( code, down ) ) {
		return;
    }
	// GUI widgets
	//if ( WG_OnKeyboard( code, ch, down ) )
	//	return;
	// application specific callback
	//if ( sys_onKeyCallback( code, ch, down) )
	//	return;
	// command key bindings
	//K_OnKeyboard( code, down );
}

static bool_t X_ProcessEvents( void ) {
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
                    X_ProcessKey( code, true );
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
                    X_ProcessKey( code, false );
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
    emuPathToISO = VAR_RegisterHelp( "emuPathToISO", "NONE", "!!!HELP NOT SET!!!" );
    CON_RegisterVars();

	// overwrite vars everywhere
	VAR_ReadCfg();

    // the Init block comes after the vars are read and overwritten
    R_Init();
    //ZPS2_InitEx( SYS_ErrorBox, CON_Printf );
    TTS_Init();

    // the Start block comes after the renderer is initialized
    CON_Start();

    //ZPS2_BootISO( emuPathToISO->string );

    bool_t quit = false;
    do {
        quit = X_ProcessEvents();
        R_FrameBegin( colorrgb( 0.1f, 0.1f, 0.1f ) );
        X_DrawLogo();
        TTS_Frame();
		CON_Frame();
        R_FrameEnd();
        SYS_SampleTime();
    } while ( ! quit );
    return 0;
}
