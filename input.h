// if needed more devices try hex and/or more chars in cmd
#define I_MAX_DEVICES 10
#define I_MAX_AXES 8
#define I_MAX_BUTTONS 16
#define I_AXIS_MIN_VALUE -32768
#define I_AXIS_MAX_VALUE 32767 

void I_UpdateMousePosition( c2_t mousePosition );
v2_t I_GetMousePositionV( void );
c2_t I_GetMousePositionC( void );
int I_JoystickButtonToButton( int button, int context );
bool_t I_OpenController( int controllerIndex );
void I_OpenJoystick( int joyIndex );
bool_t I_IsJoystickCode( int button );
void I_CloseDevice( int instanceId );
int I_GetDeviceIndex( int instanceId );
void I_SetJoystickDeadZone( int val );
void I_OnJoystickButton( int device, int code, bool_t down, int context );
int I_JoystickAxisToButton( int axis );
void I_OnJoystickAxis( int device, int code, int value, int context );
int I_JoystickHaxisToButton( int axis );
void I_OnJoystickHaxis( int device, int code, int value, int context );
void I_WriteBinds( FILE *f );
void I_BindContext( const char *button, const char *cmd, int context );
void I_Bind( const char *button, const char *cmd );
void I_OnMKButton( int code, int down, int context );
int I_MouseButtonToButton( int sdlButton );
int I_KeyToButton( int sdlKey );
void I_RegisterVars( void );
void I_Init( void );
void I_Done( void );
