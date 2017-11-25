#include "zhost.h"

static bool_t E_Stub_f( int button, bool_t down ) {
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
    if ( e_buttonOverride( button, down ) ) {
        return;
    }
    // command button bindings
    I_OnButton( button, down, inputContext );
}

void E_SetButtonOverride( eButtonOverride_t func ) {
    e_buttonOverride = func ? func : E_Stub_f;
}

static void E_DispatchJoyHat( int device, int axis, int value, int inputContext ) {
    int button = I_JoystickHaxisToButton( device, axis );
    if ( ! e_buttonOverride( button, value ) ) {
        I_OnJoystickHaxis( button, value, inputContext );
    }
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
                I_AddController( event.cdevice.which );
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                I_RemoveController( event.cdevice.which );
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                break;

            case SDL_CONTROLLERAXISMOTION:
                //I_OnControllerAxis( event.caxis );
                break;

            case SDL_JOYAXISMOTION: {
                    int button = I_JoystickAxisToButton( event.jaxis.which, 
                                                            event.jaxis.axis );
                    if ( ! e_buttonOverride( button, event.jaxis.value ) ) {
                        I_OnJoystickAxis( button, event.jaxis.value, inputContext );
                    }
                }
                break;

            case SDL_JOYHATMOTION:
                if ( event.jhat.value == SDL_HAT_CENTERED ) {
                    E_DispatchJoyHat( event.jhat.which, 0, 0, inputContext );
                    E_DispatchJoyHat( event.jhat.which, 1, 0, inputContext );
                } else {
                    if ( event.jhat.value & SDL_HAT_LEFT )  E_DispatchJoyHat( event.jhat.which, 0, I_AXIS_MIN_VALUE, inputContext );
                    if ( event.jhat.value & SDL_HAT_RIGHT ) E_DispatchJoyHat( event.jhat.which, 0, I_AXIS_MAX_VALUE, inputContext );
                    if ( event.jhat.value & SDL_HAT_UP )    E_DispatchJoyHat( event.jhat.which, 1, I_AXIS_MIN_VALUE, inputContext );
                    if ( event.jhat.value & SDL_HAT_DOWN )  E_DispatchJoyHat( event.jhat.which, 1, I_AXIS_MAX_VALUE, inputContext );
                }
                break;

            case SDL_JOYBALLMOTION:
            case SDL_JOYDEVICEADDED:
                CON_Printf( "joy event\n" );
                break;
            
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP: {
                    int button = I_JoystickButtonToButton( event.jbutton.which, 
                                    event.jbutton.button,
                                    inputContext );
                    bool_t down = event.jbutton.state == SDL_PRESSED;
                    if ( ! e_buttonOverride( button, down ) ) {
                        I_OnJoystickButton( button, 
                                            down, 
                                            inputContext );
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
