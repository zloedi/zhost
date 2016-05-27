typedef enum {
    VF_NONE,
    VF_DONT_STORE = 1 << 0,
} varFlags_t;

typedef struct var_s *varPtr_t;

float VAR_Num( const varPtr_t var );
const char* VAR_String( const varPtr_t var );
const char* VAR_Name( const varPtr_t var );
const char* VAR_Help( const varPtr_t var );
varPtr_t VAR_Next( const varPtr_t var );

bool_t VAR_Changed( varPtr_t var );
varPtr_t VAR_Find( const char *name );
varPtr_t VAR_RegisterFlagsHelp( const char *name, const char *value, varFlags_t flags, const char *help );
varPtr_t VAR_RegisterHelp( const char *name, const char *value, const char *help );
varPtr_t VAR_RegisterFlags( const char *name, const char *value, varFlags_t flags );
varPtr_t VAR_Register( const char *name, const char *value );
void VAR_Set( varPtr_t var, const char *value, bool_t keepOldValues );
void VAR_Init( void );
void VAR_Done( void );
void VAR_ReadCfg( void );
void VAR_StoreCfg( void );
