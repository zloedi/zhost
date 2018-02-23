#include "zhost.h"

static bool_t E_Stub_f( int device, int button, bool_t down ) {
    return false;
}

static eButtonOverride_t e_buttonOverride = E_Stub_f;

static void E_DispatchKey( int sdlCode, int button, bool_t down, int inputContext ) {
    // try the console
    if ( CON_OnKeyboard( sdlCode, down ) ) {
        return;
    }
    // TODO:
    // GUI widgets
    //if ( WG_OnKeyboard( sdlCode, ch, down ) )
    //  return;
    // application specific callback
    // mouse and keyboard are always device 0
    if ( e_buttonOverride( 0, button, down ) ) {
        return;
    }
    // command button bindings
    I_OnMKButton( button, down, inputContext );
}

void E_SetButtonOverride( eButtonOverride_t func ) {
    e_buttonOverride = func ? func : E_Stub_f;
}

bool_t E_DispatchEvents( int inputContext ) {
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
                    E_DispatchKey( code, I_KeyToButton( code ), true, inputContext );
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
                    E_DispatchKey( code, I_KeyToButton( code ), false, inputContext );
                }
                break;

			case SDL_MOUSEMOTION:
				I_UpdateMousePosition( c2xy( event.motion.x, event.motion.y ) );
				break;

			case SDL_MOUSEBUTTONDOWN:
                E_DispatchKey( SDLK_UNKNOWN, I_MouseButtonToButton( event.button.button ), 
                        true, inputContext );
				break;

			case SDL_MOUSEBUTTONUP:
                E_DispatchKey( SDLK_UNKNOWN, I_MouseButtonToButton( event.button.button ), 
                        false, inputContext );
				break;
				
           case SDL_CONTROLLERDEVICEADDED:
                I_OpenController( event.cdevice.which );
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                I_CloseDevice( event.cdevice.which );
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP: {
                    int device = I_GetDeviceIndex( event.cbutton.which );
                    int button = I_JoystickButtonToButton( event.cbutton.button,
                                                            inputContext );
                    bool_t down = event.cbutton.state == SDL_PRESSED;
                    if ( ! e_buttonOverride( device, button, down ) ) {
                        I_OnJoystickButton( device, button, down, inputContext );
                    }
                }
                break;

            case SDL_CONTROLLERAXISMOTION: {
                    int device = I_GetDeviceIndex( event.caxis.which );
                    int button = I_JoystickAxisToButton( event.caxis.axis );
                    if ( ! e_buttonOverride( device, button, event.caxis.value ) ) {
                        I_OnJoystickAxis( device, button, event.caxis.value, inputContext );
                    }
                }
                break;

            case SDL_JOYAXISMOTION: {
                    int device = I_GetDeviceIndex( event.jaxis.which );
                    int button = I_JoystickAxisToButton( event.jaxis.axis );
                    if ( ! e_buttonOverride( device, button, event.jaxis.value ) ) {
                        I_OnJoystickAxis( device, button, event.jaxis.value, inputContext );
                    }
                }
                break;

            case SDL_JOYHATMOTION:
                CON_Printf( "joy hat\n" );
                break;

            case SDL_JOYBALLMOTION:
                CON_Printf( "joy ball\n" );
                break;

            case SDL_JOYDEVICEREMOVED:
                I_CloseDevice( event.jdevice.which );
                break;
            
            case SDL_JOYDEVICEADDED:
                I_OpenJoystick( event.jdevice.which );
                break;
            
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP: {
                    int device = I_GetDeviceIndex( event.jbutton.which );
                    int button = I_JoystickButtonToButton( event.jbutton.button,
                                                            inputContext );
                    bool_t down = event.jbutton.state == SDL_PRESSED;
                    if ( ! e_buttonOverride( device, button, down ) ) {
                        I_OnJoystickButton( device, button, down, inputContext );
                    }
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
