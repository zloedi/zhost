typedef bool_t ( *eButtonOverride_t )( int button, bool_t down );

bool_t E_DispatchEvents( int inputContext );
// if the override returns true, all button input is consumed
void E_SetButtonOverride( eButtonOverride_t func );
