/********************************************************************/
/* Matrix-related Functions											*/
/********************************************************************/
/* Editors:															*/
/*  1. Chunghoon Kim, April 17, 2007								*/
/*  2. modified by CB	May 16, 2008								*/
/********************************************************************/


#include "PIMTypes.h"
#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/matrix.h"

#define ARRAY_START_INDEX	0// 1 for matlab compatible

/********************************************************************/
/* Error handler													*/
/********************************************************************/
static void erhand(PIM_Int8 * err_msg)
{
}


/********************************************************************/
/* Allocate the integer vector storage with range [1..n]			*/
/********************************************************************/
PIM_Int32 *int_vector(PIM_Int32 n)
{
	PIM_Int32 *v;

	v = (PIM_Int32 *)PIM_Malloc(n*sizeof(PIM_Int32));
	if (!v) erhand("Allocation failure in vector().");
	return v-1;
}


/********************************************************************/
/* Allocate the integer matrix storage with range [1..n][1..m]		*/
/********************************************************************/
PIM_Int32 **int_matrix(PIM_Int32 n,PIM_Int32 m)
//int n, m;
{
	PIM_Int32 i;
	PIM_Int32 **mat;
	PIM_Int32 *vec;

	/* Allocate pointers to rows */
	mat = (PIM_Int32 **) PIM_Calloc(n, sizeof(PIM_Int32*));
	if (!mat) erhand("Allocation failure 1 in matrix().");
	
	/* Allocate rows and set pointers to them */
	vec = (PIM_Int32*) PIM_Calloc(n*m, sizeof(PIM_Int32));
	if( !vec ) erhand( "Allocation failure 2 in matrix()." );
	for( i = 0; i < n; i++ )
	{
		mat[i] = &vec[m*i-1];
	}

	/* Return pointer to array of pointers to rows */
	return mat - 1;
}


/********************************************************************/
/* Deallocate the integer vector storage							*/
/********************************************************************/
void free_int_vector(PIM_Int32 *v)//,int n)
{
	if( v ) PIM_Free( (PIM_Int8*) (v+1));
}


/********************************************************************/
/* Deallocate the integer matrix storage							*/
/********************************************************************/
void free_int_matrix(PIM_Int32 **mat)
{
	if( !mat ) return;
	if( mat[1]+1 ) PIM_Free(&mat[1][1] );
	if( mat+1 ) PIM_Free( &mat[1] );

}


#ifndef __CODE_SIZE_OPTIMIZE__
/********************************************************************/
/* Allocate the double vector storage with range [1..n]				*/
/********************************************************************/
double *vector_start_index( PIM_Int32 n )
{
	double *v;

	v = (double *) PIM_Malloc(n*sizeof(double));
	if (!v) erhand("Allocation failure in vector().");
	return v-ARRAY_START_INDEX;
}
#endif /* __CODE_SIZE_OPTIMIZE__ */



/********************************************************************/
/* Allocate the integer vector storage with range [1..n]			*/
/********************************************************************/
PIM_Int32 *int_vector_start_index( PIM_Int32 n )
{
	PIM_Int32 *v;

	v = (PIM_Int32 *) PIM_Malloc(n*sizeof(PIM_Int32));
	if (!v) erhand("Allocation failure in vector().");
	return v-ARRAY_START_INDEX;
}



#ifndef __CODE_SIZE_OPTIMIZE__
/********************************************************************/
/* Allocate the double matrix storage with range [1..n][1..m]		*/
/********************************************************************/
double **matrix_start_index( PIM_Int32 n, PIM_Int32 m )
{
	PIM_Int32 i;
	double **mat;
	double *vec;

	/* Allocate pointers to rows */
	mat = (double **) PIM_Malloc(n*sizeof(double*) );
	if (!mat) erhand("Allocation failure 1 in matrix().");
	
	/* Allocate rows and set pointers to them */
	vec = (double*) PIM_Malloc(n*m*sizeof(double) );
	if( !vec ) erhand( "Allocation failure 2 in matrix()." );
	
	for( i = 0; i < n; i++ )
	{
		mat[i] = &vec[m*i-ARRAY_START_INDEX];
	}
	
	/* Return pointer to array of pointers to rows */
	return mat - ARRAY_START_INDEX;
}
#endif /* __CODE_SIZE_OPTIMIZE__ */



/********************************************************************/
/* Allocate the integer matrix storage with range [1..n][1..m]		*/
/********************************************************************/
PIM_Int32 **int_matrix_start_index( PIM_Int32 n, PIM_Int32 m )
{
	PIM_Int32 i;
	PIM_Int32 **mat;
	PIM_Int32 *vec;

	/* Allocate pointers to rows */
	mat = (PIM_Int32**) PIM_Malloc(n*sizeof(PIM_Int32*) );
	if (!mat) erhand("Allocation failure 1 in matrix().");
	
	/* Allocate rows and set pointers to them */
	vec = (PIM_Int32*) PIM_Malloc(n*m*sizeof(PIM_Int32) );
	if( !vec ) erhand( "Allocation failure 2 in matrix()." );
	for( i = 0; i < n; i++ )
	{
		mat[i] = &vec[m*i-ARRAY_START_INDEX];
	}

	/* Return pointer to array of pointers to rows */
	return mat - ARRAY_START_INDEX;
}



#ifndef __CODE_SIZE_OPTIMIZE__
/********************************************************************/
/* Deallocate the double vector storage								*/
/********************************************************************/
void free_vector_start_index( double *v )
{
	if( v ) PIM_Free((PIM_Int8*) (v+ARRAY_START_INDEX));
}
#endif /* __CODE_SIZE_OPTIMIZE__ */



/********************************************************************/
/* Deallocate the integer vector storage							*/
/********************************************************************/
void free_int_vector_start_index( PIM_Int32 *v )
{
   if( v ) PIM_Free((PIM_Int8*) (v+ARRAY_START_INDEX));
}



#ifndef __CODE_SIZE_OPTIMIZE__
/********************************************************************/
/* Deallocate the double matrix storage								*/
/********************************************************************/
void free_matrix_start_index( double **mat )
{
	if( !mat ) return;
	if( mat[ARRAY_START_INDEX]+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX][ARRAY_START_INDEX] );
	if( mat+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX] );
}
#endif /* __CODE_SIZE_OPTIMIZE__ */



/********************************************************************/
/* Deallocate the integer matrix storage							*/
/********************************************************************/
void free_int_matrix_start_index( PIM_Int32 **mat )
{
	if( !mat ) return;
	if( mat[ARRAY_START_INDEX]+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX][ARRAY_START_INDEX] );
	if( mat+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX] );
}



PIM_Uint8  **uc_matrix( PIM_Int32 n, PIM_Int32 m )
{
	PIM_Int32 i;
	PIM_Uint8 **mat;
	PIM_Uint8 *vec;

	/* Allocate pointers to rows */
	mat = (PIM_Uint8 **) PIM_Malloc( n*sizeof(PIM_Uint8*) );
	if (!mat) erhand("Allocation failure 1 in matrix().");

	/* Allocate rows and set pointers to them */
	vec = (PIM_Uint8*)PIM_Malloc( n*m*sizeof(PIM_Uint8) );
	if( !vec ) erhand( "Allocation failure 2 in matrix()." );
	for( i = 0; i < n; i++ )
	{
		mat[i] = &vec[m*i-ARRAY_START_INDEX];
	}

	/* Return pointer to array of pointers to rows */
	return mat - ARRAY_START_INDEX;
}

//
PIM_Uint8 ***uc_matrix_D3( PIM_Int32 l, PIM_Int32 n, PIM_Int32 m )		// by cheon
{
	PIM_Int32 j,i;
	PIM_Uint8 ***ten;
	PIM_Uint8 **mat;
	PIM_Uint8 *vec;

	vec = (PIM_Uint8*)PIM_Malloc( l*n*m*sizeof(PIM_Uint8));
	if (!vec) erhand("Allocation failure 1 in matrix().");

	mat = (PIM_Uint8**) PIM_Malloc( l*n*sizeof(PIM_Uint8*));
	if (!mat) erhand("Allocation failure 1 in matrix().");

	ten = (PIM_Uint8***) PIM_Malloc( l*sizeof(PIM_Uint8**));
	if (!ten) erhand("Allocation failure 1 in matrix().");

	for( j = 0; j<l; j++)
	{
		ten[j] = &mat[n*j-ARRAY_START_INDEX];
		for( i=0; i<n; i++)
			ten[j][i] = &vec[n*m*j + m*i - ARRAY_START_INDEX];
	}

	/* Return pointer to array of pointers to rows */
	return ten - ARRAY_START_INDEX;
}

void free_uc_matrix( PIM_Uint8 **mat )
{
	if( !mat ) return;
	if( mat[ARRAY_START_INDEX]+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX][ARRAY_START_INDEX] );
	if( mat+ARRAY_START_INDEX ) PIM_Free(&mat[ARRAY_START_INDEX] );
}

void free_uc_matrix_D3( PIM_Uint8 ***ten )
{
	if( !ten ) return;
	if( ten[ARRAY_START_INDEX][ARRAY_START_INDEX]+ARRAY_START_INDEX ) PIM_Free(&ten[ARRAY_START_INDEX][ARRAY_START_INDEX][ARRAY_START_INDEX] );
	if( ten[ARRAY_START_INDEX]+ARRAY_START_INDEX ) PIM_Free(&ten[ARRAY_START_INDEX][ARRAY_START_INDEX] );
	if( ten+ARRAY_START_INDEX ) PIM_Free(&ten[ARRAY_START_INDEX] );
}
