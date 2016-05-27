#include "common.h"
#include <SDL.h>

void SYS_Init( void );
void SYS_InitEx( const char *appName );
void SYS_Done( void );
void SYS_ErrorBox( const char *fmt, ... );
timestamp_t SYS_RealTime( void );
void SYS_WriteToClipboard( const char *str );
const char* SYS_ReadClipboard( void );
const char* SYS_PrefsDir( void );
void SYS_SampleTime( void );
timestamp_t SYS_RealTime( void );
char** SYS_ListFiles( const char *dir, const char *pattern );
