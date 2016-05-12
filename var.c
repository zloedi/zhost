#include "host.h"

var_t *vars;

bool_t VAR_Changed( var_t *var ) {
	bool_t res = var->number != var->oldNumber;
	var->oldNumber = var->number;
	return res;
}

var_t* VAR_Find( const char *name ) {
	var_t *var;
 
	for ( var = vars; var; var = var->next ) {
		int res = stricmp( var->name, name );
		
		if ( res == 0 ) {
			return var;
		} else if ( res > 0 ) {
			break;
		}
	}
	return NULL;
}

var_t* VAR_RegisterFlagsHelp( const char *name, const char *value, varFlags_t flags, const char *help ) {
	var_t *var, **prev = &vars, *newVar;

	for ( var = vars; var; var = var->next ) {
		int res = stricmp( var->name, name );
		
		if ( res == 0 ) {
			return var;
		} else if ( res > 0 ) {
			break;
		}
		prev = &var->next;
	}

	newVar = A_MallocZero( sizeof( *newVar ) );
	newVar->next = var;
	*prev = newVar;
	
	newVar->name = A_StrDup( name );
	newVar->flags = flags;
	newVar->help = A_StrDup( help );
	
	VAR_Set( newVar, value, false );

	return newVar;
}

var_t* VAR_RegisterFlags( const char *name, const char *value, varFlags_t flags ) {
	return VAR_RegisterFlagsHelp( name, value, flags, "no help available" );
}

var_t* VAR_RegisterHelp( const char *name, const char *value, const char *help ) {
	return VAR_RegisterFlagsHelp( name, value, 0, help );
}

var_t* VAR_Register( const char *name, const char *value ) {
	return VAR_RegisterFlags( name, value, 0 );
}

void VAR_StoreCfg( void ) {
	var_t  *var;
	FILE   *f;
	char   *path = va( "%sdefault.cfg", SYS_PrefsDir() );

	f = fopen( path, "wb" );
	if ( ! f ) {
		CON_Printf( "VAR_StoreCfg: failed to store \"%s\"\n", path );
		return;
	}
	
	for ( var = vars; var; var = var->next ) {
		if ( ! ( var->flags & VF_DONT_STORE ) ) {
			fprintf( f, "%s = \"%s\"\n", var->name, var->string );
		}
	}
	
	//K_WriteBinds( f );
	
	fclose( f );
	
	CON_Printf( "Stored config file \"%s\"\n", path );
}

void VAR_Set( var_t *var, const char *value, bool_t keepOldValues ) {
	A_Free( var->string );
	var->string = A_StrDup( value );
	var->number = ( float )atof( var->string );
	
	// sometimes we dont need the old != new conditionals to trigger
	// i.e. when loading vars from a cfg file
	if ( ! keepOldValues ) {
		var->oldNumber = var->number;
	}
}

static const char* VAR_GetLine( const char *data, char *line ) {
	int c, i = 0;
	
	while( ( c = data[i] ) != '\n' ) {
		if ( c == '\0' || i == VA_SIZE - 1 ) {
			line[i] = '\0';
			return NULL;
		}
		
		line[i] = ( char )c;
		
		i++;
	}
	line[i] = '\0';
	return data + i + 1;
}

void VAR_ReadCfg( void ) {
	char *path = va( "%sdefault.cfg", SYS_PrefsDir() );
	char *buffer;
	const char *data;
	char line[VA_SIZE];

	size_t sz;
	if ( ! COM_ReadTextFile( path, &buffer, &sz ) ) {
		CON_Printf( "VAR_ReadCfg: failed to open \"%s\" for reading\n", path );
		return;
	}

	CON_Printf( "Reading config file \"%s\"\n", path );
	
	// read the config file line by line and execute each one
	data = buffer;
	
	do {
		data = VAR_GetLine( data, line );
		if ( line[0] ) {
			CMD_ExecuteCfgString( line );
		}
	} while( data );
	
	A_Free( buffer );
}

void VAR_Init( void ) {
}

void VAR_Done( void ) {
	var_t *var, *next;

	VAR_StoreCfg();
	
	for ( var = vars; var; var = next ) {
		next = var->next;
		
		A_Free( var->name );
		A_Free( var->string );
		A_Free( var->help );
		A_Free( var );
	}
	CON_Printf( "Vars done\n" );
}
