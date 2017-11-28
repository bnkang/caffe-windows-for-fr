
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/matrix.h"


static const PIM_Int16 PIM_LogTable[100] = {
	0, 0, 3010,	4771,	6021,	6990,	7782,	8451,	9031,	9542,	10000,	
	10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,	
	13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,	
	14914,	15051,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,	
	16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,	
	17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,	
	17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,	
	18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031,	
	19085,	19138,	19191,	19243,	19294,	19345,	19395,	19445,	19494,	19542,	
	19590,	19638,	19685,	19731,	19777,	19823,	19868,	19912,	19956
};

PIM_Int32 PIM_Math_Log10000(PIM_Int32 x)
{
	if (x <= 1) return 0;
	if (x < 100) return PIM_LogTable[x];
	return PIM_Math_Log10000(x / 10) + 10000;		
}



// for face align
PIM_Uint32 PIM_Math_Devide( PIM_Uint32 nom, PIM_Uint32 denom )
{
	return ( (nom%denom)*2 >= denom ? (nom/denom+1) : nom/denom );
}

PIM_Uint32 PIM_Math_Sqrt( PIM_Uint32 x )
{
	const PIM_Int32 e = 1;
	PIM_Uint32 r0 = 1;
	PIM_Uint32 r1 = x;
	PIM_Int32 diff = r1-r0;
	while( PIM_Math_Abs( diff ) > e )
	{
		r1 = r0;
		r0 = (r0 + 1 + x/r0)>>1;
		diff = r1-r0;
	}
	return r0;
}





// Defines simple version of sine and cosine
//
static const PIM_Int32 PIM_Math_CosSTable[91] = {
	 65536,  65526,  65496,  65446,  65376,  65286,  65176,  65047,  64898,  64729, 
	 64540,  64331,  64103,  63856,  63589,  63302,  62997,  62672,  62328,  61965, 
	 61583,  61183,  60763,  60326,  59870,  59395,  58903,  58393,  57864,  57319, 
	 56755,  56175,  55577,  54963,  54331,  53683,  53019,  52339,  51643,  50931, 
	 50203,  49460,  48702,  47929,  47142,  46340,  45525,  44695,  43852,  42995, 
	 42125,  41243,  40347,  39440,  38521,  37589,  36647,  35693,  34728,  33753, 
	 32767,  31772,  30767,  29752,  28729,  27696,  26655,  25606,  24550,  23486, 
	 22414,  21336,  20251,  19160,  18064,  16961,  15854,  14742,  13625,  12504, 
	 11380,  10252,   9120,   7986,   6850,   5711,   4571,   3429,   2287,   1143, 
	     0	
};



PIM_Int32 PIM_Math_SinS(PIM_Int32 degree)
{
	while (degree < 0) degree += 360;
	while (degree >= 360) degree -= 360;
	if (degree <= 90 ) return PIM_Math_CosSTable[90 - degree];
	else if (degree <= 180) return PIM_Math_CosSTable[degree - 90];
	else if (degree <= 270) return -PIM_Math_CosSTable[270 - degree];
	else return -PIM_Math_CosSTable[degree - 270];	
}

PIM_Int32 PIM_Math_CosS(PIM_Int32 degree)
{
	while (degree < 0) degree += 360;
	while (degree >= 360) degree -= 360;
	if (degree <= 90 ) return PIM_Math_CosSTable[degree];
	else if (degree <= 180) return -PIM_Math_CosSTable[180 - degree];
	else if (degree <= 270) return -PIM_Math_CosSTable[degree - 180];
	else return PIM_Math_CosSTable[360 - degree];	
}

#define SIGN(x) ( (x)>=0?1:-1 )



static const PIM_Int32 PIM_Math_TanSTable[] =
{
           0,          18,          36,          54,          72,          90,         108,         126,         144,
         162,         181,         199,         218,         236,         255,         274,         294,         313,
         333,         353,         373,         393,         414,         435,         456,         477,         499,
         522,         544,         568,         591,         615,         640,         665,         691,         717,
         744,         772,         800,         829,         859,         890,         922,         955,         989,
        1024,        1060,        1098,        1137,        1178,        1220,        1265,        1311,        1359,
        1409,        1462,        1518,        1577,        1639,        1704,        1774,        1847,        1926,
        2010,        2100,        2196,        2300,        2412,        2534,        2668,        2813,        2974,
        3152,        3349,        3571,        3822,        4107,        4435,        4818,        5268,        5807,
        6465,        7286,        8340,        9743,       11704,       14644,       19539,       29324,       58665,
  2147483647,
};


PIM_Int32 PIM_Math_ArcTan( PIM_Int32 y0, PIM_Int32 x0 )
{
	int low, mid, high;
	int value;
	int x = PIM_Math_Abs(x0);
	int y = PIM_Math_Abs(y0);
	int signx = SIGN(x0);
	int signy = SIGN(y0);

	//if( y == 0 ) return 0; // commented 20081027 cbHeo
	if( x == 0 ) return signy*90;
	if( 58*x < y ) return signy*90;

	low = 0;
	high = 90;
	value = (y<<PIM_Math_KTanSBaseShifter)/x;
	while( low <= high )
	{
		mid = (low+high)/2;
		if( PIM_Math_TanSTable[mid] > value ) high = mid-1;
		else if( PIM_Math_TanSTable[mid] < value ) low = mid+1;
		else break;
	}
	
	if( signx > 0 && signy > 0 )
		return mid;
	else if( signx > 0 && signy < 0 )
		return -mid;
	else if( signx < 0 && signy > 0 )
		return 180-mid;
	else
		return mid-180;
}

/*
PIM_Int32 PIM_Math_Sin(PIM_Int32 theta)
{
	int angle = theta * 100;
	while( angle < 0 ) angle += 36000;
	while( angle > 36000 ) angle -= 36000;
	if( angle <= 9000 ) return iCosTab[9000-angle];
	else if( angle <= 18000 ) return iCosTab[angle-9000];
	else if( angle <= 27000 ) return -iCosTab[27000-angle];
	else return -iCosTab[angle-27000];	
}

PIM_Int32 PIM_Math_Cos(PIM_Int32 theta)
{
	int angle = theta * 100;
	while( angle < 0 ) angle += 36000;
	while( angle > 36000 ) angle -= 36000;
	if( angle <= 9000 ) return iCosTab[angle];
	else if( angle <= 18000 ) return -iCosTab[18000-angle];
	else if( angle <= 27000 ) return -iCosTab[angle-18000];
	else return iCosTab[36000-angle];
}
*/

//
//PIM_Int32 PIM_Math_Rand( PIM_Int32* seed )
//{
//	int idx = (*seed)%10000;
//	*seed = (idx+1)%10000;
//	return (int)PIM_MATH_RANDTABLE[idx];
//}


const PIM_Int32 PIM_MATH_SQUARE_TABLE[] = {
     0,     1,     4,     9,    16,    25,    36,    49,    64,    81,
   100,   121,   144,   169,   196,   225,   256,   289,   324,   361,
   400,   441,   484,   529,   576,   625,   676,   729,   784,   841,
   900,   961,  1024,  1089,  1156,  1225,  1296,  1369,  1444,  1521,
  1600,  1681,  1764,  1849,  1936,  2025,  2116,  2209,  2304,  2401,
  2500,  2601,  2704,  2809,  2916,  3025,  3136,  3249,  3364,  3481,
  3600,  3721,  3844,  3969,  4096,  4225,  4356,  4489,  4624,  4761,
  4900,  5041,  5184,  5329,  5476,  5625,  5776,  5929,  6084,  6241,
  6400,  6561,  6724,  6889,  7056,  7225,  7396,  7569,  7744,  7921,
  8100,  8281,  8464,  8649,  8836,  9025,  9216,  9409,  9604,  9801,
 10000, 10201, 10404, 10609, 10816, 11025, 11236, 11449, 11664, 11881,
 12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161,
 14400, 14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641,
 16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321,
 19600, 19881, 20164, 20449, 20736, 21025, 21316, 21609, 21904, 22201,
 22500, 22801, 23104, 23409, 23716, 24025, 24336, 24649, 24964, 25281,
 25600, 25921, 26244, 26569, 26896, 27225, 27556, 27889, 28224, 28561,
 28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041,
 32400, 32761, 33124, 33489, 33856, 34225, 34596, 34969, 35344, 35721,
 36100, 36481, 36864, 37249, 37636, 38025, 38416, 38809, 39204, 39601,
 40000, 40401, 40804, 41209, 41616, 42025, 42436, 42849, 43264, 43681,
 44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961,
 48400, 48841, 49284, 49729, 50176, 50625, 51076, 51529, 51984, 52441,
 52900, 53361, 53824, 54289, 54756, 55225, 55696, 56169, 56644, 57121,
 57600, 58081, 58564, 59049, 59536, 60025, 60516, 61009, 61504, 62001,
 62500, 63001, 63504, 64009, 64516, 65025, 65536, 66049, 66564, 67081,
 67600, 68121, 68644, 69169, 69696, 70225, 70756, 71289, 71824, 72361,
 72900, 73441, 73984, 74529, 75076, 75625, 76176, 76729, 77284, 77841,
 78400, 78961, 79524, 80089, 80656, 81225, 81796, 82369, 82944, 83521,
 84100, 84681, 85264, 85849, 86436, 87025, 87616, 88209, 88804, 89401,
 90000, 90601, 91204, 91809, 92416, 93025, 93636, 94249, 94864, 95481,
 96100, 96721, 97344, 97969, 98596, 99225, 99856,100489,101124,101761,
102400,103041,103684,104329,104976,105625,106276,106929,107584,108241,
108900,109561,110224,110889,111556,112225,112896,113569,114244,114921,
115600,116281,116964,117649,118336,119025,119716,120409,121104,121801,
122500,123201,123904,124609,125316,126025,126736,127449,128164,128881,
129600,130321,131044,131769,132496,133225,133956,134689,135424,136161,
136900,137641,138384,139129,139876,140625,141376,142129,142884,143641,
144400,145161,145924,146689,147456,148225,148996,149769,150544,151321,
152100,152881,153664,154449,155236,156025,156816,157609,158404,159201,
160000,160801,161604,162409,163216,164025,164836,165649,166464,167281,
168100,168921,169744,170569,171396,172225,173056,173889,174724,175561,
176400,177241,178084,178929,179776,180625,181476,182329,183184,184041,
184900,185761,186624,187489,188356,189225,190096,190969,191844,192721,
193600,194481,195364,196249,197136,198025,198916,199809,200704,201601,
202500,203401,204304,205209,206116,207025,207936,208849,209764,210681,
211600,212521,213444,214369,215296,216225,217156,218089,219024,219961,
220900,221841,222784,223729,224676,225625,226576,227529,228484,229441,
230400,231361,232324,233289,234256,235225,236196,237169,238144,239121,
240100,241081,242064,243049,244036,245025,246016,247009,248004,249001,250000};



PIM_Int32 PIM_Math_SquareInt( PIM_Int32 x)
{
	if (x < 0) x = -x;
	if (x > 500)
		return x*x;
	return PIM_MATH_SQUARE_TABLE[x];
}


// originated from FaceFilter 
PIM_Int32 PIM_Math_SquareRoot(PIM_Int32 x) 
{
	const int max_index = 220;
	int l,r,m;

	if (x >= PIM_MATH_SQUARE_TABLE[max_index]) return 10 * PIM_Math_SquareRoot(x / 100); //max_index;

	l = 0;
	r = max_index;

	while(l<r) {
		m = (l+r+1) >> 1;
		if (x == PIM_MATH_SQUARE_TABLE[m]) return m;
		if (x < PIM_MATH_SQUARE_TABLE[m]) {
			r = m - 1;
		} else {
			l = m + 1;
		}
	}

	return l;

}

PIM_Int32 mySqrtInt( PIM_Int32 x )
{
	const PIM_Int32 e = 1;
	PIM_Uint32 X = PIM_Math_Abs( x );
	PIM_Int32 r0 = 1;
	PIM_Int32 r1 = X;
	PIM_Int32 diff = r1 - r0;

	if( X == 0 ) return 0;
	if( X == 1 ) return 1;

	while( PIM_Math_Abs( diff ) > e )
	{
		r1 = r0;
		r0 = ( r0 + X/r0 ) >> 1;
		diff = r1 - r0;
	}

	return r0;
}



PIM_Int32 myDevideInt( PIM_Int32 nom, PIM_Int32 denom )
{
	PIM_Int32 sign = (nom*denom)>=0 ? 1 : -1;
	PIM_Int32 u_nom = nom>=0 ? nom : -nom;
	PIM_Int32 u_denom = denom>=0 ? denom : -denom;

	return sign * ( (u_nom%u_denom)*2 >= u_denom ? (u_nom/u_denom+1) : u_nom/u_denom );
}


/********************************************************************/
/* Make the integral image with zero padding						*/
/********************************************************************/
void make_integralimage_0padding(PIM_Int32 **img, PIM_Int32 **img_out, PIM_Int32 n_row, PIM_Int32 n_col)
{
	PIM_Int32 i, j, n_row_p1, n_col_p1;
	PIM_Int32 **s;

	n_row_p1 = n_row + 1;
	n_col_p1 = n_col + 1;
	s = int_matrix(n_row_p1, n_col_p1);

	/* first row */
	for (j=1; j<=n_col_p1; j++){
		s[1][j] = 0;
		img_out[1][j] = 0;
	}

	/* the other rows */
	for (i=2; i<=n_row_p1; i++){
		s[i][1] = 0;
		img_out[i][1] = 0;
		for (j=2; j<=n_col_p1; j++){
			s[i][j] = s[i-1][j] + img[i-1][j-1];
			img_out[i][j] = img_out[i][j-1] + s[i][j];
		}
	}

	free_int_matrix(s);
}

void make_integralimage_0padding_Blink(int **img, int **img_out, int n_row, int n_col)
{
	int i, j, n_row_p1, n_col_p1;
	int **s;

	n_row_p1 = n_row + 1;
	n_col_p1 = n_col + 1;
	s = int_matrix_start_index(n_row_p1, n_col_p1);

	//first row
	for (j=0; j<n_col_p1; j++) {
		s[0][j] = 0;
		img_out[0][j] = 0;
	}

	//the other rows
	for (i=1; i<n_row_p1; i++) {
		s[i][0] = 0;
		img_out[i][0] = 0;
		for (j=1; j<n_col_p1; j++){
			s[i][j] = s[i-1][j] + img[i-1][j-1];
			img_out[i][j] = img_out[i][j-1] + s[i][j];
		}
	}

	free_int_matrix_start_index(s);
}

/********************************************************************/
/* Calculate a matrix whose elements are squared					*/
/********************************************************************/
void cal_sqmat(PIM_Int32 **img, PIM_Int32 **img_out, PIM_Int32 n_row, PIM_Int32 n_col)
{
	PIM_Int32 i, j;

	for (i=1; i<=n_row; i++){
		for (j=1; j<=n_col; j++){
			img_out[i][j] = img[i][j] * img[i][j];
		}
	}
}


/********************************************************************/
/* Copy a part of mat_in to mat_out									*/
/********************************************************************/
void copy_mat_int(PIM_Int32 **mat_in, PIM_Int32 srow_in, PIM_Int32 scol_in, PIM_Int32 erow_in, PIM_Int32 ecol_in, PIM_Int32 **mat_out, PIM_Int32 srow_out, PIM_Int32 scol_out)
{
	PIM_Int32 i, j;
	PIM_Int32 r_idx = srow_out - 1;
	PIM_Int32 c_idx = scol_out - 1;

	for (i=srow_in; i<=erow_in; i++){
		r_idx = r_idx + 1;
		for (j=scol_in; j<=ecol_in; j++){
			c_idx = c_idx + 1;
			mat_out[r_idx][c_idx] = mat_in[i][j];
		}
		c_idx = scol_out - 1;
	}
}

/********************************************************************/
/* Vertically inversion, and then copy								*/
/********************************************************************/
void vinv_copy_mat_int(PIM_Int32 **mat_in, PIM_Int32 **mat_out, PIM_Int32 n_row, PIM_Int32 n_col_in, PIM_Int32 n_col_out)
{
	PIM_Int32 i, j, ncol_p1;

	ncol_p1 = n_col_in + 1;
	for (i=1; i<=n_row; i++){
		for (j=1; j<=n_col_out; j++){
			mat_out[i][j] = mat_in[i][ncol_p1-j];
		}
	}
}

