#include "common.h"
#include "zps2_types.h"
#include "iso.h"

typedef struct {
    unsigned lba;
    unsigned size;
    char name[32];
} isoFile_t;

static FILE *iso_image;
static char *iso_pathToImage;
//static byte *mem_main;

static void ISO_Close( void ) {
    if ( iso_image != NULL ) {
        fclose( iso_image );
        zps2_printf( "ISO_Close: Closed image \"%s\".\n", iso_pathToImage );
        A_Free( iso_pathToImage );
        iso_pathToImage = NULL;
    }
}

static bool_t ISO_Open( const char *pathToISOImage ) {
    // is "-D_FILE_OFFSET_BITS=64" in CFLAGS?
    STATIC_ASSERT(sizeof(off_t)==8,64bit_off_t)
    iso_image = fopen( pathToISOImage, "rb" );
    if ( iso_image == NULL ) {
        zps2_printf( "ERROR: ISO_Open: Couldn't open \"%s\" for reading.\n", pathToISOImage );
        return false;
    }
    iso_pathToImage = A_StrDup( pathToISOImage );
    zps2_printf( "ISO_Open: Opened \"%s\" for reading.\n", pathToISOImage );
    return true;
}

static bool_t ISO_Read( unsigned lba, unsigned numBytes, void *outBytes ) {
    if ( fseeko( iso_image, ( off_t )lba * 2048, SEEK_SET ) == -1 ) {
        zps2_fatalf( "ISO_ReadSector: fseeko failed. errno: %d\n", errno );
    }
    size_t result = fread( outBytes, 1, numBytes, iso_image ); 
    return result == numBytes;
}

static bool_t ISO_ReadSector( unsigned lba, void *outSector ) {
    return ISO_Read( lba, 2048, outSector );
}

static void ISO_FileFromSecData( byte *data, isoFile_t *outFile ) {
    outFile->lba = COM_LittleUnsigned( &data[2] );
    outFile->size = COM_LittleUnsigned( &data[10] );
    int fileNameLength = data[32];
    if ( fileNameLength == 1 ) {
        int c = data[33];
        if ( c == 0 ) {
            COM_StrCpy( outFile->name, ".", 32 );
        } else if ( c == 1 ) {
            COM_StrCpy( outFile->name, "..", 32 );
        } else {
            outFile->name[0] = ( char )c;
        }
    } else {
        COM_StrCpy( outFile->name, ( char* )&data[33], 32 );
    }
    //PrintUint( outFile->lba ); 
    //PrintString( outFile->name ); 
    //PrintUint( outFile->size ); 
}

static bool_t ISO_FindFile( isoFile_t *directory, const char *fileName, isoFile_t *outFile ) {
    byte dirSector[2048];
    ISO_ReadSector( directory->lba, dirSector );
    for ( int i = 0; i < directory->size; ) {
        int sz = dirSector[i];
        if ( sz == 0 ) {
            return false;
        }
        // FIXME: create file descriptor just to get the name
        ISO_FileFromSecData( &dirSector[i], outFile );
        if ( stricmp( outFile->name, fileName ) == 0 ) {
            return true;
        }
        i += sz;
    }
    return false;
}

bool_t ISO_LoadGameELF( elf_t **outElf ) {
    byte sector[2048];
    ISO_ReadSector( 16, sector );
    if ( sector[0] != 1 || memcmp( &sector[1], "CD001", 5 ) != 0 ) {
        zps2_printf( "ERROR: ISO_LoadGameELF: Invalid volume descriptor on image\n" );
        return false;
    }
    zps2_printf( "ISO_LoadGameELF: Found primary partition info on ISO image with 2K block size.\n" );
    isoFile_t rootDir, syscnf;
    ISO_FileFromSecData( &sector[156], &rootDir );
    if ( ! ISO_FindFile( &rootDir, "SYSTEM.CNF;1", &syscnf ) ) {
        zps2_printf( "ERROR: ISO_LoadGameELF: SYSTEM.CNF not found on image\n" );
        return false;
    }
    zps2_printf( "ISO_LoadGameELF: Found SYSTEM.CNF on ISO image.\n" );
    // assumes syscnf fits in a sector
    char syscnfBuf[2048];
    ISO_ReadSector( syscnf.lba, syscnfBuf );
    syscnfBuf[Minu( 2047, syscnf.size )] = '\0';
    const char *first = NULL;
    const char *second = NULL;
    char *line;
    const char *data = syscnfBuf;
    // parse the configuration
    // example system.cnf entry "BOOT2 = cdrom0:\SLUS_209.63;1"
    while( ( data = COM_GetLine( data, &line ) ) != NULL ) {
        COM_Split( line, "=", &first, &second );
        if ( stricmp( first, "BOOT2" ) == 0 ) {
            break;
        }
    }
    if ( ! ( first && second ) ) {
        zps2_printf( "ERROR: ISO_LoadGameELF: No BOOT2 statement in SYSTEM.CNF.\n" );
        return false;
    }
    const char *elfName;
    if ( ! COM_StrAfter( line, "\\", &elfName ) ) {
        COM_StrAfter( line, "/", &elfName );
    }
    if ( ! elfName[0] ) {
        zps2_printf( "ERROR: ISO_LoadGameELF: Invalid BOOT2 statement in SYSTEM.CNF.\n" );
        return false;
    }
    zps2_printf( "ISO_LoadGameELF: Parsed out ELF name: \"%s\".\n", elfName );
    isoFile_t elfFile;
    if ( ! ISO_FindFile( &rootDir, elfName, &elfFile ) ) {
        zps2_printf( "ISO_LoadGameELF: No game ELF found on ISO image.\n" );
        return false;
    }
    zps2_printf( "ISO_LoadGameELF: Found game ELF file descriptor.\n" );
    elf_t *elf = A_Malloc( elfFile.size );
    if ( ! ISO_Read( elfFile.lba, elfFile.size, elf ) ) {
        zps2_printf( "ISO_LoadGameELF: Failed to read %u bytes from \"%s\".\n", elfFile.size, elfFile.name );
        A_Free( elf );
        return false;
    }
    *outElf = elf;
    //progHeader_t *phs = ( progHeader_t* )&elf->data[elf->header.e_phoff];
    //for ( int i = 0; i < elf->header.e_phnum; i++ ) {
    //    progHeader_t *ph = &phs[i];
    //    // PT_LOAD segment type is the only one we care about
    //    if ( ph->p_type == 1 ) {
    //        //memcpy( &mem_main[ph->p_paddr], &elf->data[ph->p_offset], ph->p_filesz );
    //    }
    //}
    //sectionHeader_t *shs = ( sectionHeader_t* )&elf->data[elf->header.e_shoff];
    //sectionHeader_t *namessh = &shs[elf->header.e_shstrndx];
    //const char *sectionNames = ( char *)&elf->data[namessh->sh_offset];
    //for ( int i = 0; i < elf->header.e_shnum; i++ ) {
    //    sectionHeader_t *sh = &shs[i];
    //    // SHF_ALLOC flag is the only one we care about
    //    if ( sh->sh_flags & 2 ) {
    //        PrintString( &sectionNames[sh->sh_name] );
    //        PrintIntHex( sh->sh_addr );
    //        PrintInt( sh->sh_size );
    //        PrintInt( sh->sh_offset );
    //    }
    //}
    //A_Free( elf );
    return true;
}

bool_t ISO_Reset( const char *pathToISOImage ) {
    ISO_Close();
    return ISO_Open( pathToISOImage );
}

void ISO_Init( void ) {
    //mem_main = A_Static( 32 * 1024 * 1024 );
}

void ISO_Done( void ) {
    ISO_Close();
}

