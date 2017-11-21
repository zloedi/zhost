#define I_MAX_JOYSTICKS 4
#define I_MAX_AXES 8
#define I_MAX_BUTTONS 16
#define I_AXIS_MIN_VALUE -32768
#define I_AXIS_MAX_VALUE 32767 

void I_UpdateMousePosition( c2_t mousePosition );
v2_t I_GetMousePositionV( void );
c2_t I_GetMousePositionC( void );
void I_AddController( int id );
void I_RemoveController( int id );
int I_JoystickButtonToButton( int device, int button, int context );
bool_t I_IsJoystickCode( int button );
int I_DeviceOfCode( int button );
void I_OnJoystickButton( int code, bool_t down, int context );
void I_OnJoystickAxis( int device, int axis, int value, int context );
void I_OnJoystickHaxis( int device, int axis, int value, int context );
void I_WriteBinds( FILE *f );
void I_BindContext( const char *button, const char *cmd, int context );
void I_Bind( const char *button, const char *cmd );
void I_OnButton( int code, int down, int context );
int I_MouseButtonToButton( int sdlButton );
int I_KeyToButton( int sdlKey );
void I_RegisterVars( void );
void I_Init( void );
void I_Done( void );
