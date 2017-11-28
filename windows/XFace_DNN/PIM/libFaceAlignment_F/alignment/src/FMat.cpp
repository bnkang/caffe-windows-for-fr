// DblMap.cpp: implementation of the CFMat class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <math.h>
#include "FMat.h"
#include "Util.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// debugging message
static char dmsg[1000];

//
// CFMat
//
void CFMat::Clear( void ) 
{ 
	if( m_wrap ) 
	{
		UnWrap(); 
		return;
	}

	if( m_data ) 
	{ 
		free(m_data); 
		m_data = NULL;
		//m_iscol = 0;
		m_row = m_col = 0; 
	}
}


void CFMat::Reset(void)
{
	if( m_data )
	{
		free(m_data);
		m_data = NULL;
		m_row = m_col = 0;
	}
}


// copy
int CFMat::Create(int row, int col, float *pDat )
{
	if( m_wrap || m_row != row || m_col != col || m_data==NULL )
	{
		Clear();
		m_data = (float *)malloc(sizeof(float)*row*col);
		if( m_data==NULL ) { return 0; }
	}

	if( pDat )	memcpy( m_data, pDat, sizeof(float)*(row*col) );
	m_row = row;
	m_col = col;
	m_wrap = 0;
	return 1;
}

CFMat & CFMat::Wrap(int row, int col, float *pDat)
{
	if( m_data != NULL ) Clear();

	m_row  = row;
	m_col  = col;

	m_data = pDat;
	m_wrap = 1;

	return (*this);
}

void CFMat::UnWrap()
{
	if( m_wrap ) 
	{
		//m_iscol = 0;
		m_row = m_col = m_wrap = 0; 
		m_data = NULL;
	}
}

CFMat &CFMat::Fill(float val)
{
	if (val == 0) {
		memset(m_data, 0, sizeof(float) * m_col * m_row);
	}
	else {
		float *ptr = m_data;
		int count = m_col*m_row;
		while( count-- >0 ) *ptr++ = val; 
	}
	return (*this);
}

void CFMat::Save(char *szname)
{
	if( m_data == NULL ) return;

	FILE *fp = fopen( szname, "wb" );
	if( fp == NULL ) 
	{
		//sprintf(dmsg,"CFMat::Save( %s ) ERROR - FILE OPEN ERROR\n", szname ); dout( dmsg );
		return;
	}
	Save( fp );
	fclose( fp );
}

void CFMat::Save( FILE *fp )
{
	if( m_data == NULL ) { m_row=m_col=0; }

	int size[3] = { m_row, m_col, 5 };
	fwrite( &size, sizeof(int), 3, fp );				

	if( m_row*m_col>0 ) fwrite( m_data, sizeof(float), Length(), fp );		
}

void CFMat::Load(char *szname)
{
	FILE *fp = fopen(szname, "rb");

	if (fp == NULL)
		return;

	int size[3];
	fread(size, sizeof(int), 3, fp);

	ReadFromFile(size[0], size[1], fp);
	fclose(fp);
}

int CFMat::ReadFromFile( int row, int col, FILE *fp )
{
	if( NULL == fp ) return 0;

	Clear();

	int count = row*col, rcount;
	m_data = (float *)malloc(sizeof(float)*row*col);
	if( (count == (rcount=fread( m_data, sizeof(float), count, fp )) ) ) 
	{
		m_row = row; 
		m_col = col;
		m_wrap = 0;
		return 1;
	}

	m_data = NULL;

	return 0;
}


CFMat & CFMat::operator=(float val)
{
	if( (val==0.0) && m_data )
	{
		memset( m_data, 0, Length()*sizeof(float) );
	}
	else
	{
		int count = Length();
		float *ptr = m_data;
		while( count-- >0 ) *ptr++ = val; 
	}

	return *this;
}

CFMat & CFMat::operator=(const CFMat &mat)
{
	Clear();
	Create( mat.m_row, mat.m_col, mat.m_data );
	return (*this);
}
	

CFMat & CFMat::operator+=(float dScalar)
{
	float *ptr = m_data;
	int count = Length();
	while( count-- ) *ptr++ += dScalar; 
	return (*this);		
}

CFMat & CFMat::operator-=(float dScalar)
{
	float *ptr = m_data;
	int count = Length();
	while( count-- ) *ptr++ -= dScalar; 
	return (*this);		
}

CFMat & CFMat::operator+=( CFMat &b )
{
	if( Length() != b.Length() )
	{
		return (*this);
	}

	float *pa = m_data;
	float *pb = b.m_data;
	int count = Length();
	while( count-- ) { *pa++ += *pb++; }
	return (*this);		
}

CFMat & CFMat::operator-=( CFMat &b )
{
	if( Length() != b.Length() )
	{
		return (*this);
	}
	float *pa = m_data;
	float *pb = b.m_data;
	int count = Length();
	while( count-- ) *pa++ -= *pb++; 
	return (*this);		
}

const CFMat CFMat::operator-(const CFMat &b) const
{
	CFMat new_mat;
	new_mat.Create(this->Row(), this->Col());

	float *pa = this->m_data;
	float *pb = b.m_data;
	float *pc = new_mat.m_data;
	int count = Length();
	while (count--) { *pc++ = *pa++ - *pb++; };
	return new_mat;
}

const CFMat CFMat::operator+(const CFMat &b) const
{
	CFMat new_mat;
	new_mat.Create(this->Row(), this->Col());

	float *pa = this->m_data;
	float *pb = b.m_data;
	float *pc = new_mat.m_data;
	int count = Length();
	while (count--) { *pc++ = *pa++ + *pb++; };
	return new_mat;
}

const CFMat CFMat::operator*(const int b) const
{
	CFMat new_mat;
	new_mat.Create(this->Row(), this->Col());

	float *pa = this->m_data;
	float *pc = new_mat.m_data;
	int count = Length();
	while (count--) { *pc++ = *pa++ * b; };
	return new_mat;
}

CFMat & CFMat::operator/=(float val) 
{ 
	if(val == 0) return (*this);

	int cnt=Length(); 
	float *data = m_data;
	while( cnt-- ) { *data++ /= val; } return (*this);
}



//	allocating new memory blocks according to the two input matrices 
//	can cause serious problem, when this matrix is wrapping some memory buffer !
//
//	speed 100x100 = 100x500 * 500x100
//	CVMATMUL	about 1000 msec
//  VER0		about 6200 msec
//	VER1		about  970 msec 
CFMat & CFMat::IsProduct(CFMat &a, CFMat &b)
{

	if( m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ) 
	{
		return (*this);

		//Create(a.Row(), b.Col());
	}

	float *data   = m_data, 
		   *adata  = a.m_data, 
		   *bdata  = b.m_data, 
		   *rowp, *colp, sum;

	register int row = m_row, 
		         col = m_col, 
				 ri, ci, i, 
		         loopcnt = a.Col(),
		         colstep = b.Col();


	for( ri=0 ; ri<row ; ri++ ) 
	{
		for( ci=0 ; ci<col ; ci++ ) 
		{
			rowp = adata;
			colp = bdata+ci;

			sum = 0;
			i   = loopcnt;
			while( i-- ) {
				sum  += *rowp++ * *colp; 
				colp += colstep;
			}
			*data++ = sum;
		}
		adata += loopcnt; 
	}
	return (*this);
}

// this = a*b + c
CFMat & CFMat::IsProductAdd(CFMat &a, CFMat &b, CFMat &c)
{
	if( m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ||
		m_row != c.Row() || m_col != c.Col() ) 
	{
		return (*this);
	}

	float *data  = m_data, 
		   *adata = a.m_data, *bdata = b.m_data, *cdata = c.D(), sum,
		   *rowp, *colp;
	register int ri, ci, i, 
		         row = m_row, col = m_col,
		         loopcnt = a.Col(),
		         colstep = b.Col();

	for( ri=0 ; ri<row ; ri++ ) {
		for( ci=0 ; ci<col ; ci++ ) {
			rowp = adata;
			colp = bdata+ci;

			sum = 0;
			i   = loopcnt;
			while( i-- ) {
				sum  += *rowp++ * *colp; 
				colp += colstep;
			}
			*data++ = sum + *cdata++;
		}
		adata += loopcnt;
	}
	return (*this);
}

int SolveLSE( CFMat &A, CFMat &b, CFMat &x )
{
	if( A.m_row != b.m_row )
	{
		return 0;
	}
	if( A.m_row < A.m_col )
	{
		return 0;
	}
	if (A.m_col != x.m_row || x.m_col != b.m_col)
	{
		return 0;
	}

	//x.Create( A.m_col, b.m_col );

	CFMat At; At.IsTransposed( A );
	CFMat H, invH;
	CFMat SD( At.m_row, b.m_col );

	H.IsMatMulAtA( At, A );
	invH.IsInverse( H );    // = inv( A'*A )
	SD.IsProduct( At, b );  // = A' * b

	x.IsProduct( invH, SD ); // = inv( A'*A ) * A' * b;

	return 1;
}

CFMat & CFMat::SetIdentity( float diagonal_value )
{
	Fill( 0 );
	for( int i=0 ; i<m_row ; i++ ) { *(m_data+i*m_col+i) = diagonal_value; }

	return (*this);
}

CFMat & CFMat::IsInverse(CFMat &m)
{
	float pivot, dtemp2;		//T pivot, dtemp2;
	float *dtemp;				// T *dtemp;
	float *p1, *p2, *p3;		// T *p1, *p2, *p3;
	int irow, i, j, k;

	// 임시 행렬 temp = 원본 m
	CFMat temp( m );			// IMatrix< T > temp( rows, rows, this->cells );

	// 출력 행렬 자신을 identity matrix로 생성.
	//float *temp = m.m_data;
	//this->Create( m.m_row, m.m_col );	// m1.unit( rows ); // output
	if (this->m_row == 0 && this->m_col ==0)
		this->Create( m.m_row, m.m_col );
	this->SetIdentity(1.0);

	// 행 버퍼.
	dtemp = (float *)malloc( sizeof(float) * m.m_row ); //dtemp = (T *)malloc( sizeof( T ) * rows );

	p2 = temp.m_data;	//temp.cells;
	p3 = this->m_data;		//m1.cells;
	for ( i = 0; i < m_row - 1; i++ )
	{
		irow = i;
		p1 = temp.m_data + ( i + 1 ) * m_row + i;
		for ( j = i + 1; j < m_row; j++ )
		{
			if ( fabs( *( temp.m_data + irow * m_row + i ) ) < fabs( *p1 ) )
			{
				irow = j;
			}
			p1 += m_row;
		}

		if ( irow != i )
		{
			memcpy( dtemp,                    p2,                       sizeof(float)*m_row );
			memcpy( p2,                       temp.m_data + irow*m_row, sizeof(float)*m_row );
			memcpy( temp.m_data + irow*m_row, dtemp,                    sizeof(float)*m_row );

			memcpy( dtemp,                    p3,                       sizeof(float)*m_row );
			memcpy( p3,                       m_data + irow*m_row,      sizeof(float)*m_row );
			memcpy( m_data + irow*m_row,      dtemp,                    sizeof(float)*m_row );
		}

		p2 += m_row;
		p3 += m_row;
	}

	for ( i = 0; i < m_row; i++ )
	{
		pivot = *( temp.m_data + i * m_row + i );
		for ( k = 0; k < m_row; k++ )
		{
			*( temp.m_data + i * m_row + k ) /= pivot;
			*( m_data      + i * m_row + k ) /= pivot;
		}
		for ( j = 0; j < m_row; j++ )
		{
			if ( i != j )
			{
				dtemp2 = *( temp.m_data + j * m_row + i );
				for ( k = i; k < m_row; k++ )
				{
					*( temp.m_data + j * m_row + k ) -= dtemp2 * *( temp.m_data + i * m_row + k );
				}

				for ( k = 0; k < m_row; k++ )
				{
					*( m_data      + j * m_row + k ) -= dtemp2 * *( m_data      + i * m_row + k );
				}
			}
		}
	}

	free( dtemp );
	return (*this);
}


//	allocating new memory blocks according to the two input matrices 
//	can cause serious problem, when this matrix is wrapping some memory buffer !
// this = a - b
void CFMat::IsSubst(CFMat &a, CFMat &b)
{
	if( m_row != a.Row() || m_col != a.Col() || m_row != b.Row() || m_col != b.Col() ) 
	{
		return;
	}

	int count = Length();
	float *pc = m_data, *pa = a.D(), *pb = b.D();
	CUtil::subst(pa, pb, pc, count);
	//while( count-- >0 ) { *pc++ = *pa++ - *pb++; }
}


// sum( mat, 1 ), sum each column. -> 1 x column vector
CFMat &CFMat::IsSumCol( CFMat &m )
{
//	ASSERT( Length() == m.Col() );
	//if( vecSumCol.Length() != m_col ) vecSumCol.Create( 1, m_col );

	int row = m.Row();
	for( int ci=0 ; ci<m_col ; ci++ )
	{
		float sum = 0;
		for( int ri=0 ; ri<row ; ri++ )
		{
			sum += m[ri][ci]; //m_data[ ri*m_col + ci ];
		}
		m_data[ci] = sum;
	}
	return (*this);
}

// this = m*m' or m'*m
CFMat &CFMat::IsMulTransposed(CFMat &A, int IsAtA )
{
	CFMat At; At.IsTransposed( A );

	if( IsAtA )
	{
		Create( A.m_col, A.m_col );
		IsMatMulAtA( At, A );
	}
	else
	{
		Create( A.m_row, A.m_row );
		IsMatMulAtA( A, At );
	}
	return (*this);
}
 
CFMat & CFMat::AccSubArea(CFMat &m, int left, int top, int width, int height)
{
	int bottom = top  + height;
	for( int ri=top ; ri<bottom ; ri++ ) 
	{
		CUtil::acc( &(m_data[ri*m_col+left]), m[ri], width );
	}
	return (*this);
}

CFMat& CFMat::IsTransposed(CFMat &m)
{
	Create( m.m_col, m.m_row );

	int ri, ci, step;
	float *dstptr;		// this matrix
	float *srcptr;		// m matrix

	step = m.m_col;
	for( ri=0 ; ri<m_row ; ri++ )
	{
		// copy each column of m matrix to corresponding row of this matrix
		dstptr = RowD(ri);
		srcptr = m.m_data+ri; // ri of this matrix is equal to ci of m matrix
		for( ci=0 ; ci<m_col ; ci++ )
		{
			*dstptr++ = *srcptr;
			srcptr += step;
		}
		
	}

	return (*this);
}


// result is symmetric metrix
CFMat & CFMat::IsMatMulAtA(CFMat &At, CFMat &A)
{
	if( At.m_row!=A.m_col || At.m_col!=A.m_row  ) 
	{
		return (*this);
	}


	if( m_row != At.Row() || m_col != A.Col() ) 
	{
		Create( At.m_row, A.m_col );
	}

	float *data   = m_data, 
		   *adata  = At.m_data, 
		   *bdata  = A.m_data, 
		   *rowp, *colp, sum;

	register int row = m_row, 
		         col = m_col, 
				 ri, ci, i, 
		         loopcnt = At.m_col,
		         colstep = A.m_col;

	for( ri=0 ; ri<row ; ri++ ) 
	{
		data = m_data+ri*col+ri; // begin at diagonal
		for( ci=ri ; ci<col ; ci++ ) 
		{
			rowp = adata;
			colp = bdata+ci;

			sum = 0;
			i   = loopcnt;
			while( i-- ) {
				sum  += *rowp++ * *colp; 
				colp += colstep;
			}
			*data++ = sum;
			*(m_data+ci*col+ri) = sum; //*(m_data+ri*col+ci);
			
		}
		adata += loopcnt; 
	}

	return (*this);
}

int CFMat::ReadFromTxt(char *szfile)
{
	FILE *fp = fopen( szfile, "rt" );
	if( fp==NULL ) return 0;

	int readcount = 0, maxcount = Length();

	char line[1024], *token, seps[] = " ,\t\r\n";
	while( fgets( line, 1023, fp ) ) 
	{
		if( strlen(line)<2 )  {
			//printf("- empty line\n");
			continue; // empty line
		}

		token = strtok( line, seps );
		while( token ) {
			if( token[0] == '/' ) // comments 
			{ 
				//printf("-comment line\n"); 
				break; 
			} 

			m_data[ readcount++ ] = atof( token );
			//printf("%d th data : %f (%s) \n", readcount, m_data[readcount-1], token );
			token = strtok( NULL, seps );

			if( readcount == maxcount ) break;
		}
		if( readcount == maxcount ) break;
	}
	fclose(fp);

	if( readcount < maxcount ) return 0;

	return 1;

}

void CFMat::sqrt()
{
	if( m_data==NULL ) return;
	int cnt = Length();
	float *d = m_data;
	while( cnt-- ) *d = ::sqrt( *d++ );
}


float CFMat::ssum()
{
	return CUtil::ssum( m_data, m_row*m_col );
}

int CFMat::clear()
{
	Clear();
	return 1;
}

CFMat & CFMat::AccSubBlk(int left, int top, int width, int height, CFMat &m, int mleft, int mtop)
{
	int bottom = top  + height;
	int roff;

	for( roff=0 ; roff<height ; roff++ ) 
	{
		CUtil::acc( m_data+(top+roff)*m_col+left, m.m_data+(mtop+roff)*m.m_col+mleft, width );
	}
	return (*this);

}
