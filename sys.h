#include "common.h"
#include <SDL.h>
#include <SDL_mixer.h>

void SYS_Init( void );
void SYS_InitEx( const char* organizationName, const char *appName );
void SYS_Done( void );
void SYS_ErrorBox( const char *fmt, ... );
void SYS_WriteToClipboard( const char *str );
const char* SYS_ReadClipboard( void );
const char* SYS_PrefsDir( void );
const char* SYS_BaseDir( void );
int SYS_SampleTime( void );
int SYS_RealTime( void );
char** SYS_ListFiles( const char *dir, const char *pattern );
