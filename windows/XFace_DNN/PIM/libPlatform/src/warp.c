
#include "PIMTypes.h"
#include "libPlatform/inc/warp.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/PIMBitmap.h"



static int warpImageBy2Points_uc(	unsigned char * src, int w1, int h1, int lx1, int ly1, int rx1, int ry1,
						  unsigned char ** dst, int w2, int h2, int lx2, int ly2, int rx2, int ry2 );


void rotation_degree(int * point_in, int* point_out, int degree)
{
	//double sinT, cosT;
	int sinT, cosT;
	int xTmp;

	sinT = PIM_Math_SinS(degree);
	cosT = PIM_Math_CosS(degree);
	
	xTmp		 = ( ( cosT * point_in[0] ) - ( sinT * point_in[1] ) )>>PIM_Math_KCosSBaseShifter;
	point_out[1] = ( ( sinT * point_in[0] ) + ( cosT * point_in[1] ) )>>PIM_Math_KCosSBaseShifter;;
	point_out[0] = xTmp;
}


#ifndef __CODE_SIZE_OPTIMIZE__
void vinv_copy_mat_uc(unsigned char *mat_in, unsigned char *mat_out, int n_row, int n_col)
{
	int i;
	int size;
	int idx_in, idx_out;

	idx_in =0;
	idx_out = -1;
	for (i=0; i<n_row; i++){
		size = n_col;
		idx_out += n_col;
		while(size--)
		{
			mat_out[idx_in++] = mat_in[idx_out--];
		}
		idx_out += n_col;
	}
}
#endif //__CODE_SIZE_OPTIMIZE__

#ifndef __CODE_SIZE_OPTIMIZE__
void vinv_copy_mat_uc_vertical(unsigned char *mat_in, unsigned char *mat_out, int n_row, int n_col)	// must "mat_in != mat_out"
{
	int i, j;
	int idx_from , idx_to;

	idx_from = (n_row-1)*n_col;
	idx_to = 0;
	for(i=0; i<n_row; i++)
	{
		for(j=0; j<n_col; j++)
			mat_out[idx_to++] = mat_in[idx_from++];
		idx_from -= (n_col<<1);
	}
}
#endif //__CODE_SIZE_OPTIMIZE__


// FD에서 Face Rect 가 커지면   strength를 늘리시오.
// FD에서 Face Rect 가 작아지면 strength를 줄이시오.
// 두 점을 입력으로 가지며, strength값에 따라서 두 점을 늘리기도(<1) 줄이기도(>1)
// 입력 출력 같아도 무방.
void strength2Pnts_i(int *pnt1, int *pnt2, int *out_pnt1, int *out_pnt2, int strengthShift)
{
	int len = ((1<<10)-strengthShift)>>1;
	int	d;
	int out_pnt1_[2], out_pnt2_[2];

	d = pnt1[0]-pnt2[0];
	out_pnt1_[0] = pnt1[0]+ ((d*len)>>10);
	out_pnt2_[0] = pnt2[0]- ((d*len)>>10);

	d = pnt1[1]-pnt2[1];
	out_pnt1_[1] = pnt1[1]+ ((d*len)>>10);
	out_pnt2_[1] = pnt2[1]- ((d*len)>>10);

	PIM_Memcpy(out_pnt1, out_pnt1_, sizeof(int)* 2);
	PIM_Memcpy(out_pnt2, out_pnt2_, sizeof(int)* 2);
}

//== static function definition,
// in, out 같아도 상관 없도록 수정함.
int warpPointBy2Points( int in_x, int in_y, int leyex1, int leyey1, int reyex1, int reyey1,
					   int *out_x, int *out_y, int leyex2, int leyey2, int reyex2, int reyey2 )
{
	int meyex1, meyey1, meyex2, meyey2;
	int dist2sq, dist2sqsq;
	int cosT, sinT;
	int in_x_copy = in_x;
	int in_y_copy = in_y;

	if( !out_x || !out_y ) return -1;

	meyex1 = (leyex1+reyex1+1)>>1;
	meyey1 = (leyey1+reyey1+1)>>1;
	meyex2 = (leyex2+reyex2+1)>>1;
	meyey2 = (leyey2+reyey2+1)>>1;
	dist2sq = (leyex2-reyex2)*(leyex2-reyex2) + (leyey2-reyey2)*(leyey2-reyey2);
	dist2sqsq = dist2sq*dist2sq;
	cosT = (reyex1-leyex1)*(reyex2-leyex2) + (reyey1-leyey1)*(reyey2-leyey2);
	sinT = (reyex1-leyex1)*(reyey2-leyey2) - (reyex2-leyex2)*(reyey1-leyey1);

	if( dist2sq <= 0 ) return 0;

	*out_x = ( cosT*(in_x_copy-meyex2) + sinT*(in_y_copy-meyey2) + dist2sq*meyex1 + (dist2sq>>1) ) / dist2sq;
	*out_y = ( cosT*(in_y_copy-meyey2) - sinT*(in_x_copy-meyex2) + dist2sq*meyey1 + (dist2sq>>1) ) / dist2sq;

	return 1;
}

//.
int warpImageBy2Points_uc_scale( unsigned char * src, int w1, int h1, int lx1, int ly1, int rx1, int ry1,
							 unsigned char ** dst, int w2, int h2, int lx2, int ly2, int rx2, int ry2 )
{
	int mx1, my1, mx2, my2;
	int dist2sq, dist2sqsq;
//	int cosT, sinT;

	mx1 = (lx1+rx1+1)>>1;
	my1 = (ly1+ry1+1)>>1;
	mx2 = (lx2+rx2+1)>>1;
	my2 = (ly2+ry2+1)>>1;
	dist2sq = (lx2-rx2)*(lx2-rx2) + (ly2-ry2)*(ly2-ry2);
	dist2sqsq = dist2sq*dist2sq;

	if(dist2sqsq>6000000)
		return warpImageBy2Points_uc_scale(	src, w1, h1, (lx1+mx1+1)>>1, (ly1+my1+1)>>1, (rx1+mx1+1)>>1, (ry1+my1+1)>>1, 
		dst, w2, h2, (lx2+mx2+1)>>1, (ly2+my2+1)>>1, (rx2+mx2+1)>>1, (ry2+my2+1)>>1);
	else
		return warpImageBy2Points_uc(  src, w1, h1, lx1, ly1, rx1, ry1,
		dst, w2, h2, lx2, ly2, rx2, ry2 );
		//return warpImageBy2Points_uc_NN_yj(  src, w1, h1, lx1, ly1, rx1, ry1,
		//dst, w2, h2, lx2, ly2, rx2, ry2 );


}

///== static function definition, 수정 없음
static int warpImageBy2Points_uc(	unsigned char * src, int w1, int h1, int lx1, int ly1, int rx1, int ry1,
						  unsigned char ** dst, int w2, int h2, int lx2, int ly2, int rx2, int ry2 )
{
	int y, x;
	int mx1, my1, mx2, my2;
	int dist2sq, dist2sqsq;
	int cosT, sinT;

	mx1 = (lx1+rx1+1)>>1;
	my1 = (ly1+ry1+1)>>1;
	mx2 = (lx2+rx2+1)>>1;
	my2 = (ly2+ry2+1)>>1;
	dist2sq = (lx2-rx2)*(lx2-rx2) + (ly2-ry2)*(ly2-ry2);
	dist2sqsq = dist2sq*dist2sq;
	cosT = (rx1-lx1)*(rx2-lx2) + (ry1-ly1)*(ry2-ly2);
	sinT = (rx1-lx1)*(ry2-ly2) - (rx2-lx2)*(ry1-ly1);

	if( dist2sq <= 0 ) return 0;

	for( y = 0; y < h2; y++ )
	{
		for( x = 0; x < w2; x++ )
		{
			int X1 = cosT*(x-mx2) + sinT*(y-my2) + dist2sq*mx1;
			int Y1 = cosT*(y-my2) - sinT*(x-mx2) + dist2sq*my1;
			int x1 = X1/dist2sq;
			int x1r = X1%dist2sq;
			int y1 = Y1/dist2sq;
			int y1r = Y1%dist2sq;

			if( x1 < 0 || x1 >= w1 || y1 < 0 || y1 >= h1 )
			{
				dst[y][x] = 0;
			}
			else
			{
				int ix0, ix1, iy0, iy1;
				int v00, v01, v10, v11;
				int value;
				ix0 = x1 < w1-1 ? (int)x1 : (int)x1-1;
				ix1 = ix0 + 1;
				iy0 = y1 < h1-1 ? (int)y1 : (int)y1-1;
				iy1 = iy0 + 1;

				v00 = src[(w1*iy0) + ix0]; //src[iy0][ix0];
				v01 = src[(w1*iy0) + ix1]; //src[iy0][ix1];
				v10 = src[(w1*iy1) + ix0]; //src[iy1][ix0];
				v11 = src[(w1*iy1) + ix1]; //src[iy1][ix1];
				//value = ( v00*dist2sqsq + x1r*(v01-v00)*dist2sq + y1r*(v10-v00)*dist2sq + x1r*y1r*(v00+v11-v01-v10) ) / dist2sqsq;
				value = v00 + (x1r*(v01-v00) + y1r*(v10-v00))/dist2sq + ((x1r*y1r)>>4)*(v00+v11-v01-v10)/(dist2sqsq>>4);
				dst[y][x] = (unsigned char)value;
			}
		}
	}

	return 1;
}


#ifndef __CODE_SIZE_OPTIMIZE__
//. 실험중 
static int warpImageBy2Points_uc_NN_yj( unsigned char ** src, int w1, int h1, int lx1, int ly1, int rx1, int ry1,
								   unsigned char ** dst, int w2, int h2, int lx2, int ly2, int rx2, int ry2 )
{
	int y, x;

	int dx1 = rx1 - lx1,				dx2 = rx2 - lx2;
	int dy1 = ry1 - ly1,				dy2 = ry2 - ly2;

	int cx1 = (lx1+rx1+1)>>1;
	int cy1 = (ly1+ry1+1)>>1;
	int cx2 = (lx2+rx2+1)>>1;
	int cy2 = (ly2+ry2+1)>>1;

	int dz1 = PIM_Math_Sqrt(dx1*dx1 + dy1*dy1),		dz2 = PIM_Math_Sqrt(dx2*dx2 + dy2*dy2);

	int xs, ys;

	double scale;
	double sinT, cosT, sinT1_minus, cosT1_minus, sinT2, cosT2;


	// same warping points
	if( (lx1 == lx2) && (ly1 == ly2) && (rx1 == rx2) && (ry1 == ry2) )
	{
		int w2_ = PIM_Math_Min( w1, w2 );
		int h2_ = PIM_Math_Min( h1, h2 );
		for(y=0; y<h2_; y++)
			PIM_Memcpy(dst[y], src[y], w2_);
		return 1;
	}

	// two points is same
	if( (dz2 == 0) || (dz1 == 0) )
		return -1;

	// here... 하나로 합쳐야.  --> fixxed coding 하여야.
	scale = (double)dz1 / dz2;	
	sinT1_minus = (double)(-dy1)/dz1,	cosT1_minus = (double)dx1/dz1;
	sinT2 = (double)dy2/dz2,			cosT2 = (double)dx2/dz2;
	sinT = sinT2 * cosT1_minus + cosT2 * sinT1_minus;
	cosT = cosT2 + cosT1_minus * sinT2 * sinT1_minus;
	sinT *= scale;
	cosT *= scale;

	for( y = 0; y < h2; y++ )
		for( x = 0; x < w2; x++ )
		{
			xs = (int) ( ( cosT * (x - cx2) - sinT * (y - cy2) )  +   cx1 );
			ys = (int) ( ( sinT * (x - cx2) + cosT * (y - cy2) )  +   cy1 );

			if( xs < 0 || xs >= w1 || ys < 0 || ys >= h1 )
				dst[y][x] = 0;
			else
				dst[y][x] = src[ys][xs];
		}


	return 1;
}

///== static function definition, 수정 없음
int warpImageBy2Points_uc_new(	unsigned char ** src, int w1, int h1, int lx1, int ly1, int rx1, int ry1,
						  unsigned char ** dst, int w2, int h2, int lx2, int ly2, int rx2, int ry2 )
{
	int y, x;
	int mx1, my1, mx2, my2;
	int dist2sq, dist2sqsq;
	int cosT, sinT;
	int w1_dist2sq;
	int h1_dist2sq;

	mx1 = (lx1+rx1+1)>>1;
	my1 = (ly1+ry1+1)>>1;
	mx2 = (lx2+rx2+1)>>1;
	my2 = (ly2+ry2+1)>>1;
	dist2sq = (lx2-rx2)*(lx2-rx2) + (ly2-ry2)*(ly2-ry2);
	dist2sqsq = dist2sq*dist2sq;
	cosT = (rx1-lx1)*(rx2-lx2) + (ry1-ly1)*(ry2-ly2);
	sinT = (rx1-lx1)*(ry2-ly2) - (rx2-lx2)*(ry1-ly1);

	if( dist2sq <= 0 ) return 0;

	w1_dist2sq = w1 * dist2sq;
	h1_dist2sq = h1 * dist2sq;

	for( y = 0; y < h2; y++ )
	{
		for( x = 0; x < w2; x++ )
		{			

			int X1 = cosT*(x-mx2) + sinT*(y-my2) + dist2sq*mx1;
			int Y1 = cosT*(y-my2) - sinT*(x-mx2) + dist2sq*my1;

			//if( x1 < 0 || x1 >= w1 || y1 < 0 || y1 >= h1 )
			if( X1 < 0 || X1 >= w1_dist2sq || Y1 < 0 || Y1 >= h1_dist2sq )
			{
				dst[y][x] = 0;
			}
			else
			{
				int x1r = X1%dist2sq;
				int y1r = Y1%dist2sq;
				int x1 = X1/dist2sq;
				int y1 = Y1/dist2sq;

				int ix0, ix1, iy0, iy1;
				int v00, v01, v10, v11;
				int value;
				ix0 = x1 < w1-1 ? (int)x1 : (int)x1-1;
				ix1 = ix0 + 1;
				iy0 = y1 < h1-1 ? (int)y1 : (int)y1-1;
				iy1 = iy0 + 1;

				v00 = src[iy0][ix0];
				v01 = src[iy0][ix1];
				v10 = src[iy1][ix0];
				v11 = src[iy1][ix1];
				//value = ( v00*dist2sqsq + x1r*(v01-v00)*dist2sq + y1r*(v10-v00)*dist2sq + x1r*y1r*(v00+v11-v01-v10) ) / dist2sqsq;
				value = v00 + (x1r*(v01-v00) + y1r*(v10-v00))/dist2sq + ((x1r*y1r)>>4)*(v00+v11-v01-v10)/(dist2sqsq>>4);
				dst[y][x] = (unsigned char)value;
			}
		}
	}

	return 1;
}
#endif //__CODE_SIZE_OPTIMIZE__

PIM_Int32 warpImageBy2Points( const PIM_Bitmap *imgGray, PIM_Int32 lx1, PIM_Int32 ly1, PIM_Int32 rx1, PIM_Int32 ry1,
                PIM_Bitmap *imgAligned, PIM_Int32 lx2, PIM_Int32 ly2, PIM_Int32 rx2, PIM_Int32 ry2 )
{
    PIM_Int32 y, x;
    PIM_Int32 w1, h1, w2, h2;
//    PIM_Uint8 **src = NULL;
//    PIM_Uint8 **dst = NULL;
    PIM_Int32 mx1, my1, mx2, my2;
    PIM_Int32 dist2sq, dist2sqsq;
    PIM_Int32 cosT, sinT;

    if( !imgGray || !imgAligned ) return -1;
    
    w1 = imgGray->width;
    h1 = imgGray->height;
    w2 = imgAligned->width;
    h2 = imgAligned->height;
    mx1 = (lx1+rx1+1)>>1;
    my1 = (ly1+ry1+1)>>1;
    mx2 = (lx2+rx2+1)>>1;
    my2 = (ly2+ry2+1)>>1;
    dist2sq = (lx2-rx2)*(lx2-rx2) + (ly2-ry2)*(ly2-ry2);
    dist2sqsq = dist2sq*dist2sq;
    cosT = (rx1-lx1)*(rx2-lx2) + (ry1-ly1)*(ry2-ly2);
    sinT = (rx1-lx1)*(ry2-ly2) - (rx2-lx2)*(ry1-ly1);
    
    if( dist2sq <= 0 ) return 0;
        
//    src = imgGray->data;
//    dst = imgAligned->data;
    
    for( y = 0; y < h2; y++ )
    {
        for( x = 0; x < w2; x++ )
        {
            PIM_Int32 X1 = cosT*(x-mx2) + sinT*(y-my2) + dist2sq*mx1;
            PIM_Int32 Y1 = cosT*(y-my2) - sinT*(x-mx2) + dist2sq*my1;
            PIM_Int32 x1 = X1/dist2sq;
            PIM_Int32 x1r = X1%dist2sq;
            PIM_Int32 y1 = Y1/dist2sq;
            PIM_Int32 y1r = Y1%dist2sq;
            
            if( x1 < 0 || x1 >= w1 || y1 < 0 || y1 >= h1 )
            {
//                dst[y][x] = 0;
                SET_GRAY(imgAligned, x, y, 0);
            }
            else
            {
                PIM_Int32 ix0, ix1, iy0, iy1;
                PIM_Int32 v00, v01, v10, v11;
                PIM_Int32 value;
                ix0 = x1 < w1-1 ? (PIM_Int32)x1 : (PIM_Int32)x1-1;
                ix1 = ix0 + 1;
                iy0 = y1 < h1-1 ? (PIM_Int32)y1 : (PIM_Int32)y1-1;
                iy1 = iy0 + 1;
                
                v00 = GET_GRAY(imgGray, ix0, iy0); // src[iy0][ix0];
                v01 = GET_GRAY(imgGray, ix1, iy0); //src[iy0][ix1];
                v10 = GET_GRAY(imgGray, ix0, iy1); //src[iy1][ix0];
                v11 = GET_GRAY(imgGray, ix1, iy1); //src[iy1][ix1];
                value = v00 + (x1r*(v01-v00) + y1r*(v10-v00))/dist2sq + ((x1r*y1r)>>4)*(v00+v11-v01-v10)/(dist2sqsq>>4);
//                dst[y][x] = (PIM_Uint8)value;
                SET_GRAY(imgAligned, x, y, (PIM_Uint8)value);
            }
        }
    }

    return 1;
}
