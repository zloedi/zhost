#include "common.h"
#include "zps2_private.h"
#include "zps2_public.h"

zps2Printf_t zps2_printf;
zps2Fatalf_t zps2_fatalf;

static void ZPS2_FatalError( const char *format, ... ) {
    va_list argptr;
    va_start( argptr, format );
    vfprintf( stderr, format, argptr );
    va_end( argptr );
    exit( 0 );
}

void ZPS2_Init( void ) {
    EE_Init();
    ISO_Init();

    zps2_printf( "ZPS2 Initialized.\n" );
}

void ZPS2_InitEx( zps2Fatalf_t Fatalf, zps2Printf_t Printf ) {
    zps2_printf = Printf != NULL ? Printf : printf;
    zps2_fatalf = Fatalf != NULL ? Fatalf : ZPS2_FatalError;
    ZPS2_Init();
}

void ZPS2_Done( void ) {
    ISO_Done();
    zps2_printf( "ZPS2 Done.\n" );
}

bool_t ZPS2_BootISO( const char *pathToISOImage ) {
    if ( ! ISO_Reset( pathToISOImage ) ) {
        return false;
    }
    elf_t *elf;
    if ( ! ISO_LoadGameELF( &elf ) ) {
        return false;
    }
    EE_Reset( elf );
    A_Free( elf );
    return true;
}
