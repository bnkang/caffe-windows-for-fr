#include "stdafx.h"

#include "Util.h"
#include "FMat.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define M_PI                    (3.14159265358979323846)

int comparef( const void *arg1, const void *arg2 )
{
	return (  *(float*)arg1 < *(float*)arg2 ) ? -1 : (  ( *(float*)arg1>*(float*)arg2 )? 1 : 0 );
}


void quick_sort( float *d, int cnt )
{
	qsort( (void*)d, cnt, sizeof(float), comparef );
}

float CUtil::mediand( float *dat, int cnt )
/*
@brief compute median value of input array
@param[in]	"float"		*dst	- input array
@param[in]	"int"		cnt		- number of input array
@return	median value
 */
{

	float *d = dat;
	quick_sort( d, cnt );

	if( cnt%2 == 0 ) { // even
		return ( d[(int)(cnt/2)-1]+d[(int)(cnt/2)] )/2;
	}
	else {
		return d[ (int)(cnt/2) ];
	}
	return 0;
}

void CUtil::compute_alphabeta(float *v1, float *v2, float *v3, float x, float y, float *alpha, float *beta)
{
	float d11, d12, d21, d22, dx, dy;

	// v2-v1
	d11 = v2[0]-v1[0];
	d21 = v2[1]-v1[1];
	// v3-v1
	d12 = v3[0]-v1[0];
	d22 = v3[1]-v1[1];

	dx = x - v1[0];
	dy = y - v1[1];

	*alpha = 1/(d11*d22-d12*d21) * (  d22 * dx - d12 * dy );
	*beta  = 1/(d11*d22-d12*d21) * ( -d21 * dx + d11 * dy );

}

double * CUtil::acc( double *a, double *b, int cnt )
{
	for( int i=0 ; i<cnt ; i++ ) a[i] += b[i];
	return a;
}

float * CUtil::acc( float *a, float *b, int cnt )
{
	for( int i=0 ; i<cnt ; i++ ) a[i] += b[i];
	return a;
}

double * CUtil::acc_scaled( double *a, double *b, double scale, int cnt )
{
	for( int i=0 ; i<cnt ; i++ ) a[i] += b[i] * scale;
	return a;
}

float * CUtil::acc_scaled( float *a, float *b, float scale, int cnt )
{
	//	double *a0 = a;
	//	while( cnt-- > 0 ) { *a++ += *b++ * scale; }
	//	return a0;
	for( int i=0 ; i<cnt ; i++ ) a[i] += b[i] * scale;
	return a;
}


// sums of squares
double CUtil::ssum( double *a, int cnt, int *mask )
{
	double sum = 0;
	register int i;
	if( mask ) {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			//	if( *mask ) { sum += *a; }
			//	mask++;			a++;
			if( mask[i] ) sum += a[i]*a[i];
		}
	}
	else {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			//sum += *a++; 
			sum += a[i]*a[i];
		}
	}
	return sum;
}

float CUtil::ssum( float *a, int cnt, int *mask )
{
	float sum = 0;
	register int i;
	if( mask ) {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			//	if( *mask ) { sum += *a; }
			//	mask++;			a++;
			if( mask[i] ) sum += a[i]*a[i];
		}
	}
	else {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			//sum += *a++; 
			sum += a[i]*a[i];
		}
	}
	return sum;
}

int CUtil::Euler2Rot(float *a, float *R33)
/*
 @brief change euler angles to 3x3 rotation matrix
 @param[in]	"float"		*a		- 1x3 array(angles in x,y,z-axes)
 @param[in]	"float"		*R33	- 3x3 matrix
 */
{
	float pitch = a[0];
	float yaw = a[1];
	float roll = a[2];

	float sina = sin(pitch), sinb = sin(yaw), sinc = sin(roll);
	float cosa = cos(pitch), cosb = cos(yaw), cosc = cos(roll);

	CFMat R;
	R.Wrap(3, 3, R33);

	R[0][0] = cosb * cosc;								R[0][1] = -cosb * sinc;								R[0][2] = sinb;
	R[1][0] = cosa * sinc + sina * sinb * cosc;			R[1][1] = cosa * cosc - sina * sinb * sinc;			R[1][2] = -sina * cosb;
	R[2][0] = R[0][1] * R[1][2] - R[0][2] * R[1][1];	R[2][1] = R[0][2] * R[1][0] - R[0][0] * R[1][2];	R[2][2] = R[0][0] * R[1][1] - R[0][1] * R[1][0];

	return 1;
}

int CUtil::Rot2Euler(float *R33, float *a)
/*
@brief change 3x3 rotation matrix to euler angles
@param[in]	"float"		*R33	- 3x3 matrix
@param[in]	"float"		*a		- 1x3 array(angles in x,y,z-axes)
 */
{
	float q[4], yaw, pitch, roll;
	CFMat R;
	R.Wrap(3, 3, R33);

	q[0] = sqrt(1 + R[0][0] + R[1][1] + R[2][2]) / (float)2.0;
	q[1] = (R[2][1] - R[1][2]) / (4 * q[0]);
	q[2] = (R[0][2] - R[2][0]) / (4 * q[0]);
	q[3] = (R[1][0] - R[0][1]) / (4 * q[0]);

	pitch	= atan2(2 * (q[0] * q[1] - q[2] * q[3]), q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3]);
	yaw		= asin (2 * (q[0] * q[2] + q[1] * q[3]));
	roll	= atan2(2 * (q[0] * q[3] - q[1] * q[2]), q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3]);

	*a = pitch;
	*(a+1) = yaw;
	*(a+2) = roll;
	return 1;
}

float CUtil::angle2radian(float ang)
{
	return (float)(ang * M_PI / 180.0);
}
float CUtil::radin2angle(float rad)
{
	return (float)(rad * 180.0 / M_PI);
}

float CUtil::inner( float *a, float *b, int cnt )
{
#ifdef USE_SSE
	float sum=0;
	/*__m256 xmm1, xmm2, xmm3, sum_xmm;
	sum_xmm = _mm256_setzero_ps();
	while (cnt - 8 > -1) {
		xmm1 = _mm256_loadu_ps(a);
		xmm2 = _mm256_loadu_ps(b);
		xmm3 = _mm256_mul_ps(xmm1, xmm2);
		sum_xmm = _mm256_add_ps(sum_xmm, xmm3);
		//_mm_store_ps(R, xmm3);
		a += 8;
		b += 8;
		cnt -= 8;
	}
	float R[8];
	_mm256_storeu_ps(R, sum_xmm);
	sum += R[0]; sum += R[1]; sum += R[2]; sum += R[3];
	sum += R[4]; sum += R[5]; sum += R[6]; sum += R[7];
	*/
	
	/*__declspec (align(16)) float R[4] = {0, 0, 0, 0};
	__m128 xmm1, xmm2, xmm3;

	while( cnt-4 > -1 ) {
		xmm1 = _mm_load_ps(a);
		xmm2 = _mm_load_ps(b);
		xmm3 = _mm_mul_ps(xmm1, xmm2);
		_mm_store_ps(R, xmm3);

		sum += R[0];
		sum += R[1];
		sum += R[2];
		sum += R[3];

		a += 4;
		b += 4;
		cnt -= 4;
	}*/

	while( cnt-- ) sum += *a++ * *b++;
#else
	float sum=0;
	while( cnt-- ) sum += *a++ * *b++;
#endif

	//printf("sum - %f\n", sum);
	return sum;
}

void CUtil::mul( double *a, double *b, double *c, int cnt )
{
	if( c==NULL ) {		c = a;	}

	while( cnt-- ) { *c++ = *a++ * *b++; }
}

void CUtil::mul( float *a, float *b, float *c, int cnt )
{
#ifdef USE_SSE
	/*__m256 xmm1, xmm2, xmm3;
	while( cnt-8 > -1 ) {
		xmm1 = _mm256_loadu_ps(a);
		xmm2 = _mm256_loadu_ps(b);
		xmm3 = _mm256_mul_ps(xmm1, xmm2);
		_mm256_storeu_ps(c, xmm3);
		a += 8;
		b += 8;
		c += 8;
		cnt -= 8;
	}*/

	__m128 xmm1, xmm2, xmm3;
	while( cnt-4 > -1 ) {
		xmm1 = _mm_loadu_ps(a);
		xmm2 = _mm_loadu_ps(b);
		xmm3 = _mm_mul_ps(xmm1, xmm2);
		_mm_storeu_ps(c, xmm3);
		a += 4;
		b += 4;
		c += 4;
		cnt -= 4;
	}
	while( cnt-- ) { *c++ = *a++ * *b++; }
#else
	if( c==NULL ) {		c = a;	}
	while( cnt-- ) { *c++ = *a++ * *b++; }
#endif
}

void CUtil::muladd( double *a, double *b, double *c, double *d, int cnt )
{

	while( cnt-- ) { *d++ = *a++ * *b++ + *c++; }
}

void CUtil::muladd( float *a, float *b, float *c, float *d, int cnt )
{
	while( cnt-- ) { *d++ = *a++ * *b++ + *c++; }
}

double *CUtil::subst( double *a, double *b, double *c, int cnt, int *mask )
{
	double *c0 = c;
	register int i;
	if( mask ) {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			if( mask[i] ) c[i] = a[i] - b[i];

		//	if( *mask ) { *c = *a - *b; }
		//	mask++; c++; a++; b++;
		}
	}
	else {


		for( i=0 ; i<cnt; i++ ) 
		{ 
			//*c++ = *a++ - *b++; 
			c[i] = a[i] - b[i];
		}
	}
	//return c0;
	return c;
}

float *CUtil::subst( float *a, float *b, float *c, int cnt, int *mask )
{
	float *c0 = c;
	register int i;
	if( mask ) {
		for( i=0 ; i<cnt; i++ ) 
		{ 
			if( mask[i] ) c[i] = a[i] - b[i];

		//	if( *mask ) { *c = *a - *b; }
		//	mask++; c++; a++; b++;
		}
	}
	else {


		for( i=0 ; i<cnt; i++ ) 
		{ 
			*c++ = *a++ - *b++; 
			//c[i] = a[i] - b[i];
		}
	}
	//return c0;
	return c;
}


