// IMap.cpp: implementation of the CIMat class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "IMat.h"
#include <memory.h>




//#if !defined( _AAM_UTIL_ )
//#define dout(dmsg)	TRACE1(dmsg)
//#endif

static char dmsg[1000];


void CIMat::Clear()
{
	if( m_wrap ) 
	{
		UnWrap(); 
		return;
	}

	if( m_data ) 
	{ 
		delete [] m_data; 
		m_data = NULL;
		//m_iscol = 0;
		m_row = m_col = 0; 
	}
}

int CIMat::Create(int row, int col, int *pDat )
{
	if( m_wrap || m_row != row || m_col != col || m_data==NULL )
	{
		Clear();
		m_data = new int [ row*col ];
	}

	if( pDat ) memcpy( m_data, pDat, sizeof(int)*row*col );
	m_row = row;
	m_col = col;
	m_wrap = 0;

	return 1;
}

CIMat& CIMat::Fill(int val)
{
	*this = val;
	return *this;

//	int *p =m_data;
//	for( int i = 0 ; i < m_col*m_row ; i++ ) { m_data[i] = val; } //*p++ = val;	}
}



int CIMat::ReadFromFile(int row, int col, FILE *fp)
{
	if( NULL == fp ) return 0;

	Clear();

	int count = row*col, rcount;
	m_data = new int [ count ];
	if( (count == (rcount=fread( m_data, sizeof(int), count, fp )) ) ) 
	{
		m_row = row; 
		m_col = col;
		m_wrap = 0;
		return 1;
	}

	m_data = NULL;

	sprintf(dmsg,"CIMat::ReadFromFile(%d,%d) ERROR - read error(%d items < count(%d))",row,col,rcount,count);
	//dout( dmsg );
	return 0;
}

CIMat &CIMat::Wrap(int row, int col, int *pDat)
{
//	ASSERT( pDat != NULL );

	m_row = row;
	m_col = col;

	if( m_wrap ) m_data = pDat;
	else
	{
		Clear();
		m_data = pDat;
		m_wrap = 1;
	}
	return (*this);
}

void CIMat::UnWrap()
{
	if( m_wrap ) {
		//m_iscol = 0;
		m_row = m_col = m_wrap = 0;
		m_data = NULL;
	}
}

CIMat & CIMat::operator=(int val)
{
	if( (val==0) && m_data )
	{
		memset( m_data, 0, Length()*sizeof(int) );
	}
	else
	{
		int count = Length();
		int *ptr = m_data;
		while( count-- >0 ) *ptr++ = val; 
	}

	return *this;
}


CIMat & CIMat::operator=(CIMat &mat)
{
	if( m_wrap || m_row!=mat.Row() || m_col!=mat.Col() ) 
		Create( mat.Row(), mat.Col(), mat.D() );
	else 
		memcpy( m_data, mat.D(), sizeof(int)*Length() );

	return (*this);
}
	
CIMat & CIMat::operator+=(int iScalar)
{
	int *ptr = m_data;
	int count = Length();
	while( count-- >0 ) *ptr++ += iScalar; 
	return (*this);		
}

CIMat & CIMat::operator*=(double dScalar)
{
	int *ptr = m_data;
	int count = Length();
	while( count-- >0 ) 
	{
		*ptr++ = (int)( (double)(*ptr)*dScalar ); 
	}
	return (*this);		
}

CIMat & CIMat::operator+=( CIMat &b )
{
	if( Length() != b.Length() )
	{
		return (*this);
	}

	int *pa = m_data;
	int *pb = b.m_data;
	int count = Length();
	while( count-- ) { *pa++ += *pb++; }
	return (*this);		
}

CIMat & CIMat::operator>>=(int iShift )
{
	int *ptr = m_data;
	int count = Length();
	while( count-- >0 ) *ptr++ >>= iShift; 
	return (*this);		
}

CIMat & CIMat::IsProduct(CIMat &a, CIMat &b)
{

	if( m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ) 
	{
		return *this;

		//Create(a.Row(), b.Col());
	}

	register int *data = m_data, sum,
		         *rowvec, *colvec;
	register int ri, ci, i, row=m_row, col=m_col, 
		         loopcnt = a.Col(),
		         colstep = b.Col();
				 

	//rowvec = a.m_data;
	for( ri=0 ; ri<row ; ri++ ) {
		for( ci=0 ; ci<col ; ci++ ) {

			rowvec = a[ri]; 
			//rowvec += 
			colvec = &(b.m_data[ci]);

			sum = 0;
			i   = loopcnt;
			while( i-- ) {
				sum    += *rowvec++ * *colvec; 
				colvec += colstep;
			}
			*data++ = sum;
		}
	}
	return (*this);
}

CIMat & CIMat::IsProduct_fix(CIMat &a, CIMat &b, int shift)
{
	if( m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ) 
	{
		return *this;
	}

	register int *data = m_data, sum,
		         *rowvec, *colvec;
	register int ri, ci, i, row=m_row, col=m_col, 
		         loopcnt = a.Col(),
		         colstep = b.Col();
				 
	for( ri=0 ; ri<row ; ri++ ) {
		for( ci=0 ; ci<col ; ci++ ) {

			rowvec = a[ri]; 
			colvec = &(b.m_data[ci]);

			sum = 0;
			i   = loopcnt;
			while( i-- ) {
				sum    += *rowvec++ * *colvec; 
				colvec += colstep;
			}
			*data++ = sum>>shift;
		}
	}

	return (*this);

}

CIMat & CIMat::IsProductAdd_fix(CIMat &a, CIMat &b, CIMat &c, int shift)
{
	if( m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ||
		m_row != c.Row() || m_col != c.Col() ) 
	{
		return (*this);
	}

	int    *data  = m_data, 
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
			*data++ = (sum + *cdata++) >> shift;
		}
		adata += loopcnt;
	}
	return (*this);
}

CIMat & CIMat::IsProductAdd(CIMat &a, CIMat &b, CIMat &c)
{
	if (m_row != a.Row() || m_col != b.Col() || a.Col() != b.Row() ||
		m_row != c.Row() || m_col != c.Col())
	{
		return (*this);
	}

	int    *data = m_data,
		*adata = a.m_data, *bdata = b.m_data, *cdata = c.D(), sum,
		*rowp, *colp;
	register int ri, ci, i,
		row = m_row, col = m_col,
		loopcnt = a.Col(),
		colstep = b.Col();

	for (ri = 0; ri<row; ri++) {
		for (ci = 0; ci<col; ci++) {
			rowp = adata;
			colp = bdata + ci;

			sum = 0;
			i = loopcnt;
			while (i--) {
				sum += *rowp++ * *colp;
				colp += colstep;
			}
			*data++ = sum + *cdata++;
		}
		adata += loopcnt;
	}
	return (*this);
}


//	allocating new memory blocks according to the two input matrices 
//	can cause serious problem, when this matrix is wrapping some memory buffer !
// this = a - b
CIMat & CIMat::IsSubst(CIMat &a, CIMat &b)
{
	if( m_row != a.Row() || m_col != a.Col() || m_row != b.Row() || m_col != b.Col() ) 
	{
		return *this;
	}

	int count = Length();
	int *pc = m_data, *pa = a.D(), *pb = b.D();
	while( count-- >0 ) { *pc++ = *pa++ - *pb++; }

	return (*this);
}

int CIMat::clear()
{
	Clear();
	return 1;
}

static int comparei(const void *arg1, const void *arg2) {
	return (*(int*)arg1 < *(int*)arg2) ? -1 : ((*(int*)arg1>*(int*)arg2) ? 1 : 0);
}


void CIMat::Sort() {
	qsort((void*)m_data, Length(), sizeof(int), comparei);
}

CIMat& CIMat::IsTransposed(CIMat &m)
{
	Create(m.m_col, m.m_row);

	int ri, ci, step;
	int *dstptr;		// this matrix
	int *srcptr;		// m matrix

	step = m.m_col;
	for (ri = 0; ri<m_row; ri++)
	{
		// copy each column of m matrix to corresponding row of this matrix
		dstptr = RowD(ri);
		srcptr = m.m_data + ri; // ri of this matrix is equal to ci of m matrix
		for (ci = 0; ci<m_col; ci++)
		{
			*dstptr++ = *srcptr;
			srcptr += step;
		}

	}

	return (*this);
}