#include "sys.h"

static int  sys_realTime;
static char *sys_prefsDir;
static char *sys_baseDir;

void SYS_Fatal( const char *fmt, ... ) {
    char buf[VA_SIZE] = {0};
    va_list argptr;
    va_start (argptr, fmt);
    vsnprintf (buf, VA_SIZE, fmt, argptr);
    va_end (argptr);
    buf[VA_SIZE - 1] = '\0';
    fprintf( stderr, "%s\n", buf );
    fflush( stderr );
    SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error!", buf, NULL );
    exit( -1 );
}

void SYS_ErrorBox( const char *fmt, ... ) {
    char buf[VA_SIZE] = {0};
    va_list argptr;
    va_start (argptr, fmt);
    vsnprintf (buf, VA_SIZE, fmt, argptr);
    va_end (argptr);
    buf[VA_SIZE - 1] = '\0';
    fprintf( stderr, "%s\n", buf );
    fflush( stderr );
    SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_WARNING, "Error!", buf, NULL );
}

int SYS_SampleTime( void ) {
    sys_realTime = SDL_GetTicks();
    return sys_realTime;
}

int SYS_RealTime( void ) {
    return sys_realTime;
}

void SYS_WriteToClipboard( const char *str ) {
    fprintf( stderr, "NOT IMPLEMENTED SYS_WriteToClipboard" );
}

const char* SYS_ReadClipboard( void ) {
    fprintf( stderr, "NOT IMPLEMENTED SYS_ReadClipboard" );
    return "";
}

const char* SYS_BaseDir( void ) {
    return sys_baseDir;
}

const char* SYS_PrefsDir( void ) {
    return sys_prefsDir;
}

void SYS_InitEx( const char* organizationName, const char *appName ) {
    if ( SDL_InitSubSystem( SDL_INIT_TIMER ) < 0 ) {
        SYS_Fatal( "SYS_Init: SDL could not initialize! SDL Error: %s", SDL_GetError() );
    }
    if ( SDL_InitSubSystem( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0 ) {
        SYS_ErrorBox( "SYS_Init: SDL could not initialize! SDL Error: %s", SDL_GetError() );
    }
    SDL_GameControllerAddMappingsFromFile( "gamecontrollerdb.txt" );
    if ( SDL_InitSubSystem( SDL_INIT_AUDIO ) < 0 ) {
        SYS_ErrorBox( "SYS_Init: SDL could not initialize! SDL Error: %s", SDL_GetError() );
    }
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0 ) { 
        SYS_ErrorBox( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    Mix_AllocateChannels( 16 );
    SYS_SampleTime();
    sys_prefsDir = SDL_GetPrefPath( organizationName ? organizationName : "zhost", appName ? appName : "zhost" );
    sys_baseDir = SDL_GetBasePath();
    if ( ! sys_baseDir ) {
        sys_baseDir = A_StrDup( "./" );
    }
    unsigned seed = SDL_GetPerformanceCounter();
    COM_SRand( seed );
}

void SYS_Init( void ) {
    SYS_InitEx( NULL, NULL );
}

void SYS_Done( void ) {
    SDL_free( sys_prefsDir );
    SDL_free( sys_baseDir );
    Mix_CloseAudio();
    SDL_CloseAudio();
}
