typedef enum {
    VF_NONE,
    VF_DONT_STORE = 1 << 0,
} varFlags_t;

typedef struct var_s var_t;

void VAR_SetCFGVersion( int version );

float VAR_Num( const var_t *var );
const char* VAR_String( const var_t *var );
const char* VAR_Name( const var_t *var );
const char* VAR_Help( const var_t *var );
var_t* VAR_Next( const var_t *var );

bool_t VAR_Changed( var_t *var );
var_t* VAR_Find( const char *name );
var_t* VAR_RegisterFlagsHelp( const char *name, const char *value, varFlags_t flags, const char *help );
var_t* VAR_RegisterHelp( const char *name, const char *value, const char *help );
var_t* VAR_RegisterFlags( const char *name, const char *value, varFlags_t flags );
var_t* VAR_Register( const char *name, const char *value );
void VAR_Set( var_t *var, const char *value, bool_t keepOldValues );
void VAR_Init( void );
void VAR_Done( void );
void VAR_ReadCfg( void );
void VAR_StoreCfg( void );
