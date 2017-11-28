//
//		Utility functions 
//

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <math.h>


#define FIXEDSHIFT 5
#define ROUND(x)	floor((x+0.5))

typedef unsigned char BYTE;

class CUtil 
{
public:


	CUtil() {};
	~CUtil() {};

	static int compared( const void *arg1, const void *arg2 );
	static float  mediand( float *dat, int cnt );

	//
	//		VECTOR OPERATION
	//

	//	inner product
	static float inner( float *a, float *b, int cnt );

	// a += b
	static double* acc( double *a, double *b, int cnt );
	static float * acc( float *a, float *b, int cnt );
	// a += b*scale
	static double * acc_scaled( double *a, double *b, double scale, int cnt );
	static float * acc_scaled( float *a, float *b, float scale, int cnt );
	// c = a + b*s
	static void add( double *a, double *b, double scale, double *c, int cnt );
	static void add( float  *a, float  *b, float  scale, float  *c, int cnt );
	// b_i = a_i + scalar
	static double *add_scalar( double *a, double scalar, double *b, int cnt, int *mask=NULL );
	static float  *add_scalar( float  *a, float  scalar, float  *b, int cnt, int *mask=NULL );

	// c = a - b
	static double *subst( double *a, double *b, double *c, int cnt, int *mask=NULL );
	static float *subst( float *a, float *b, float *c, int cnt, int *mask=NULL );

	// c = a * b
	static void mul( double *a, double *b, double *c, int cnt );
	static void mul( double *a, double *b, double *c, int cnt, int *mask );
	static void mul( float *a, float *b, float *c, int cnt );
	// d = a * b + c
	static void muladd( double *a, double *b, double *c, double *d, int cnt );
	static void muladd( double *a, double *b, double *c, double *d, int cnt, int *mask );
	static void muladd( float *a, float *b, float *c, float *d, int cnt );
	// b = a * scalar
	static void scale( double *a, int cnt, double scalar, double *b=NULL );
	static void scale( double *a, int cnt, double scalar, double *b, int *mask=NULL );

	// norm
	static double norm ( float *v, int cnt );

	// sum { a_i }
	static double sum( double *a, int cnt, int *mask=NULL );
	static int    sum( int    *a, int cnt, int *mask=NULL );
	// ssum { a_i*a_i }
	static double ssum( double *a, int cnt, int *mask=NULL );
	static float ssum( float *a, int cnt, int *mask=NULL );

	// sum { a_i } / N
	static double mean( double *a, int cnt, int *mask=NULL );
	static float  mean( float *a,  int cnt, int *mask=NULL );

	// warping function related
	static void compute_alphabeta(float  *v1, float  *v2, float  *v3, float  x, float  y, float  *alpha, float  *beta);

	static int Euler2Rot(float *a, float *R33);
	static int Rot2Euler(float *R33, float *a);

	static float angle2radian(float);
	static float radin2angle(float);


};
//

#endif
