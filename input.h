void I_UpdateState( v2_t mousePosition );
v2_t I_GetMousePosition( void );
void I_WriteBinds( FILE *f );
void I_BindContext( const char *button, const char *cmd, int context );
void I_RegisterVars( void );
void I_Bind( const char *button, const char *cmd );
void I_OnButton( int code, int down, int context );
int I_MapSDLButtonToButton( int sdlButton );
