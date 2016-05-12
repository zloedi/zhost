typedef int ( *zps2Printf_t )( const char *format, ... );
typedef void ( *zps2Fatalf_t )( const char *format, ... );

void ZPS2_InitEx( zps2Fatalf_t Fatalf, zps2Printf_t Printf );
void ZPS2_Init( void );
void ZPS2_Done( void );
bool_t ZPS2_BootISO( const char *pathToISOImage );
