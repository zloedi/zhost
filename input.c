#include "zhost.h"

v2_t i_mouseState;

void I_UpdateState( v2_t mousePosition ) {
    i_mouseState = mousePosition;
}

v2_t I_GetMousePosition( void ) {
    return i_mouseState;
}
