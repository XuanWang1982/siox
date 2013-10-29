#include <mpi.h>
#include "siox-datatypes-helper-mpi.h"

#include <sys/types.h>
#include <unistd.h>
#include <glib.h>


static inline unsigned translateMPIOpenFlagsToSIOX( unsigned flags )
{
        return  ( ( flags & MPI_MODE_RDONLY ) > 0 ? SIOX_MPI_MODE_RDONLY : 0 ) |  ( ( flags & MPI_MODE_RDWR ) > 0 ? SIOX_MPI_MODE_RDWR : 0 ) |  ( ( flags & MPI_MODE_WRONLY ) > 0 ? SIOX_MPI_MODE_WRONLY : 0 ) |  ( ( flags & MPI_MODE_CREATE ) > 0 ? SIOX_MPI_MODE_CREATE : 0 ) |  ( ( flags & MPI_MODE_EXCL ) > 0 ? SIOX_MPI_MODE_EXCL : 0 ) |  ( ( flags & MPI_MODE_DELETE_ON_CLOSE ) > 0 ? SIOX_MPI_MODE_DELETE_ON_CLOSE : 0 ) |  ( ( flags & MPI_MODE_UNIQUE_OPEN ) > 0 ? SIOX_MPI_MODE_UNIQUE_OPEN : 0 ) |  ( ( flags & MPI_MODE_SEQUENTIAL ) > 0 ? SIOX_MPI_MODE_SEQUENTIAL : 0 ) |  ( ( flags & MPI_MODE_APPEND ) > 0 ? SIOX_MPI_MODE_APPEND : 0 );
}

static inline int translateMPIThreadLevelToSIOX( int flags )
{
        return  ( ( flags & MPI_THREAD_SINGLE ) > 0 ? SIOX_MPI_THREAD_SINGLE : 0 ) |  ( ( flags & MPI_THREAD_FUNNELED ) > 0 ? SIOX_MPI_THREAD_FUNNELED : 0 ) |  ( ( flags & MPI_THREAD_SERIALIZED ) > 0 ? SIOX_MPI_THREAD_SERIALIZED : 0 ) |  ( ( flags & MPI_THREAD_MULTIPLE ) > 0 ? SIOX_MPI_THREAD_MULTIPLE : 0 );
}

// this routine is taken from PIOSIM, mpi-names.c, author Paul Mueller
static const char * getCombinerName(int combiner)
{
	if(combiner == MPI_COMBINER_NAMED)
		return "NAMED";
	else if(combiner == MPI_COMBINER_DUP)
		return "DUP";
	else if(combiner == MPI_COMBINER_CONTIGUOUS)
		return "CONTIGUOUS";
	else if(combiner == MPI_COMBINER_VECTOR)
		return "VECTOR";
	else if(combiner == MPI_COMBINER_HVECTOR_INTEGER)
		return "HVECTOR_INTEGER";
	else if(combiner == MPI_COMBINER_HVECTOR)
		return "HVECTOR";
	else if(combiner == MPI_COMBINER_INDEXED)
		return "INDEXED";
	else if(combiner == MPI_COMBINER_HINDEXED_INTEGER)
		return "HINDEXED_INTEGER";
	else if(combiner == MPI_COMBINER_HINDEXED)
		return "HINDEXED";
	else if(combiner == MPI_COMBINER_INDEXED_BLOCK)
		return "INDEXED_BLOCK";
	else if(combiner == MPI_COMBINER_STRUCT_INTEGER)
		return "STRUCT_INTEGER";
	else if(combiner == MPI_COMBINER_STRUCT)
		return "STRUCT";
	else if(combiner == MPI_COMBINER_SUBARRAY)
		return "SUBARRAY";
	else if(combiner == MPI_COMBINER_DARRAY)
		return "DARRAY";
	else if(combiner == MPI_COMBINER_F90_REAL)
		return "F90_REAL";
	else if(combiner == MPI_COMBINER_F90_COMPLEX)
		return "F90_COMPLEX";
	else if(combiner == MPI_COMBINER_F90_INTEGER)
		return "F90_INTEGER";
	else if(combiner == MPI_COMBINER_RESIZED)
		return "RESIZED";
	else
	{
		printf("[MPI] %s: unknown combiner constant requested: %d\n", __FUNCTION__, combiner);
		return "UNKNOWN";
	}
}

static const char * getOrderConstantName(int constant)
{
	if(constant == MPI_ORDER_FORTRAN)
		return "FORTRAN";
	else if(constant == MPI_ORDER_C)
		return "C";
	else
	{
		printf("[MPI] %s: unknown constant constant requested: %d\n", __FUNCTION__, constant);
		return "UNKNOWN";
	}
}

static const char * getDistributeConstantName(int constant)
{
	if(constant == MPI_DISTRIBUTE_BLOCK)
		return "BLOCK";
	else if(constant == MPI_DISTRIBUTE_CYCLIC)
		return "CYCLIC";
	else if(constant == MPI_DISTRIBUTE_NONE)
		return "NONE";
	else if(constant == MPI_DISTRIBUTE_DFLT_DARG)
		return "DFLT_DARG";
	else
	{
		printf("[MPI] %s: unknown constant constant requested: %d\n", __FUNCTION__, constant);
		return "UNKNOWN";
	}
}

#define APPEND_STR(myStr, count) if(*length < *pos + count ){ if (! *malloced){ *str = (char*)  malloc(2 * *length);}  else{ *str = (char*) realloc(*str, 2 * *length); } *length = 2 * *length; } *pos += sprintf(*str + *pos, "%s", myStr);

#define APPEND_INT(myInt) if(*length < *pos + 10 ){ if (! *malloced){ *str = (char*)  malloc(2 * *length);}  else{ *str = (char*)  realloc(*str, 2 * *length); } *length = 2 * *length; } *pos += sprintf(*str + *pos, "%lld,", (long long int) myInt);

#define APPEND_COMMA *pos += sprintf(*str + *pos, ",");


static inline void datatypeToString(char ** str, int * pos, int * length, int * malloced, MPI_Datatype type){
	// this routine is partly taken from PIOSIM, write_info.c, author Paul Mueller
	int max_integers,	max_addresses, max_datatypes,	combiner;

	PMPI_Type_get_envelope(type, &max_integers, &max_addresses, &max_datatypes, &combiner);


	APPEND_STR(getCombinerName(combiner), 20);
	APPEND_STR("(", 1);
	if(combiner == MPI_COMBINER_NAMED) 	// cannot call get_contents on this
	{
		int resultlen;
		char typename[MPI_MAX_OBJECT_NAME];
		PMPI_Type_get_name(type, typename, &resultlen);
		APPEND_STR(typename, 20);
		APPEND_STR(")", 1);

	}	else	{
		int *integers = malloc(sizeof(int) * max_integers);
		MPI_Aint *addresses = malloc(sizeof(MPI_Aint) * max_addresses);
		MPI_Datatype *datatypes = malloc(sizeof(MPI_Datatype) * max_datatypes);
		PMPI_Type_get_contents(type, max_integers, max_addresses, max_datatypes, integers, addresses, datatypes);


		if(combiner == MPI_COMBINER_DARRAY){
			// this is not nice, but we need to log the names of the constantes
			// used by mpi_create_darray, not the numerical values
			assert(max_integers > 2);

			APPEND_STR("SIZE=", 5);
			APPEND_INT(integers[0]);
			APPEND_STR("RANK=", 5);
			APPEND_INT(integers[1]);
			APPEND_STR("DIM=", 5);
			APPEND_INT(integers[2]);

			int dims = integers[2];

			APPEND_STR("GS=", 5);
			int i = 3;
			for(; i < dims + 3; ++i){
				APPEND_INT(integers[i]);
			}

			APPEND_STR("D=", 5);
			for(; i < dims * 2 + 3; ++i){
				APPEND_STR( getDistributeConstantName(integers[i]), 20 );
				APPEND_COMMA				
			}
			APPEND_STR("A=", 5);
			for(; i < dims * 3 + 3; ++i){
				APPEND_INT(integers[i]);
			}
			APPEND_STR("P=", 5);
			for(; i < dims * 3 + 3; ++i){
				APPEND_INT(integers[i]);
			}

			APPEND_STR(getOrderConstantName(integers[max_integers - 1]), 20);
		}else if(combiner == MPI_COMBINER_SUBARRAY){
			for(int i = 0; i < max_integers - 1; ++i)
			{
				APPEND_INT(integers[i]);
			}

			APPEND_STR(getOrderConstantName(integers[max_integers - 1]), 20);
		}else if(combiner == MPI_COMBINER_STRUCT){
			APPEND_STR("BLOCKS=", 10);
			APPEND_INT(integers[0]);

			APPEND_STR("BL=", 4);
			for(int i = 1; i < max_integers; ++i)
			{
				APPEND_INT(integers[i]);
			}
			*pos = *pos -1;

		}else{
			for(int i = 0; i < max_integers; ++i)
			{
				APPEND_INT(integers[i]);
			}
			*pos = *pos -1;
		}

		APPEND_STR(",ADDR=", 8);

		for(int i = 0; i < max_addresses; ++i)
		{
			APPEND_INT(addresses[i]);
		}
		*pos = *pos -1;

		APPEND_STR(",TYPES=", 10);			
		for(int i = 0; i < max_datatypes; ++i)
		{
			datatypeToString(str, pos, length, malloced, datatypes[i]);
			APPEND_COMMA
		}
		*pos = *pos -1;

		APPEND_STR(")", 1);

		free(integers);
		free(addresses);
		free(datatypes);
	}
}


// This function detects how a mpi datatype is composed and records this information into the given (string) attribute.
static inline void recordDatatype(siox_activity * sioxActivity, siox_attribute * attribute, MPI_Datatype type){
	// we assume this size suffices
	int max_length = 2048;
	int pos = 0;
	int mallocedBuffer = FALSE;
	char buff[max_length];
	char * type_str = buff;

	datatypeToString(& type_str, & pos, & max_length, & mallocedBuffer, type);

	siox_activity_set_attribute( sioxActivity, attribute, type_str );

	if (mallocedBuffer){
		free(type_str);
	}
}

/* 
 The list of known MPI Hints and the corresponding SIOX attribute.
 */
struct known_hint_t{
	char * name;
	siox_attribute ** attribute;
};

// Three different types are supported, int32, int63 and strings.
static struct known_hint_t knownHintValueInt32 [3] = {{"mpiio_concurrency", & infoConcurrency}, {"mpiio_coll_contiguous", & infoCollContiguous}, {NULL, NULL}};
static struct known_hint_t knownHintValueInt64 [] = {  {"noncoll_read_bufsize" , & infoReadBuffSize},  {"noncoll_write_bufsize", &infoWriteBuffSize},  {"coll_read_bufsize", & infoCollReadBuffSize},  {"coll_write_bufsize", & infoCollWriteBuffSize}, {NULL, NULL}};
static struct known_hint_t knownHintValueStr [] = {{NULL, NULL}};

/*
	Record the recognized default info key/value pairs.
	Additionally, record all hints in a big string separated by "||" (hopefully no string contains ||). 
	Note that this string may contain not recognized hints.
 */
static inline void recordFileInfo(siox_activity * sioxActivity, MPI_File fh){
	MPI_Info info = MPI_INFO_NULL;
	PMPI_File_get_info( fh, & info );
	if (info == MPI_INFO_NULL){
		return;
	}

	int number_of_keys = 0;
	PMPI_Info_get_nkeys(info, & number_of_keys);

	printf("[MPI] hint count: %d\n", number_of_keys);

	// iterate over all hints, determine their size.
	unsigned stringLength = -1;
	for(int i=0; i < number_of_keys; i++){
		char key[MPI_MAX_INFO_KEY];
		char value[MPI_MAX_INFO_VAL];
		int isDefined;
		int ret;

		PMPI_Info_get_nthkey(info, i, key);
		ret = PMPI_Info_get(info, key, MPI_MAX_INFO_VAL, value, & isDefined);

		if ( ! isDefined || ret != MPI_SUCCESS ){
			continue;
		}
		stringLength += strlen(key) + strlen(value) + 4;

		printf("[MPI] Hint: %s, %s\n", key, value);
	}

	if	(stringLength > 0){
		// store all hints into one string
		char * allHints = malloc(stringLength);
		char * curPos = allHints;

		for(int i=0; i < number_of_keys; i++){
			char key[MPI_MAX_INFO_KEY];
			char value[MPI_MAX_INFO_VAL];
			int isDefined;
			int ret;

			PMPI_Info_get_nthkey(info, i, key);
			ret = PMPI_Info_get(info, key, MPI_MAX_INFO_VAL, value, & isDefined);

			if ( ! isDefined || ret != MPI_SUCCESS ){
				continue;
			}		

			curPos += sprintf(curPos, "%s||%s||", key, value);
		}

		// 0 termination, strip the last two |
		curPos[-2] = 0;

		siox_activity_set_attribute( sioxActivity , infoString,  allHints);

		free(allHints);
	}

	// check for known hints and record them:
	struct known_hint_t * cur;
	cur = knownHintValueInt32;
	while(cur->name != NULL){
		int isDefined, ret;
		char value[MPI_MAX_INFO_VAL];
		ret = PMPI_Info_get(info, cur->name, MPI_MAX_INFO_VAL, value, & isDefined);

		cur++;
		if ( ! isDefined || ret != MPI_SUCCESS ){
			continue;
		}
		int32_t val = (int32_t) atoi(value);
		siox_activity_set_attribute( sioxActivity, *cur->attribute , & val );
	}

	cur = knownHintValueInt64;
	while(cur->name != NULL){
		int isDefined, ret;
		char value[MPI_MAX_INFO_VAL];
		ret = PMPI_Info_get(info, cur->name, MPI_MAX_INFO_VAL, value, & isDefined);

		cur++;
		if ( ! isDefined || ret != MPI_SUCCESS ){
			continue;
		}
		int64_t val = (int64_t) atol(value);
		siox_activity_set_attribute( sioxActivity, *cur->attribute , & val );
	}

	cur = knownHintValueStr;
	while(cur->name != NULL){
		int isDefined, ret;
		char value[MPI_MAX_INFO_VAL];
		ret = PMPI_Info_get(info, cur->name, MPI_MAX_INFO_VAL, value, & isDefined);

		cur++;
		if ( ! isDefined || ret != MPI_SUCCESS ){
			continue;
		}
		siox_activity_set_attribute( sioxActivity, *cur->attribute , value );
	}
}

/* generated using the shell script
	for I in   ; do echo -n " ( ( flags & $I ) > 0 ? SIOX_$I : 0 ) | " ; done ; echo
*/

