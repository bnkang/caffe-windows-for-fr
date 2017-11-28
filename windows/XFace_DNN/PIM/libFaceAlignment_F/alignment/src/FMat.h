// FMat.h: interface for the CFMat class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FMAT_H_
#define _FMAT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

class CFMat 
{
public:
	CFMat & AccSubBlk(int left, int top, int width, int height, CFMat &m, int mleft, int mtop);
	int clear( void );
	float ssum( void );
	void sqrt();
	int ReadFromTxt( char *szfile );  


	// this = a*b + c
	CFMat & IsProductAdd( CFMat &a, CFMat &b, CFMat &c );


	// constructor
	CFMat() : m_row(0), m_col(0), m_wrap(0), m_data(0) {};
	CFMat( const CFMat &mat ) : m_row(0),m_col(0),m_wrap(0),m_data(0) { 
		Create( mat.Row(), mat.Col(), mat.D() ); 
	}
	CFMat(int row, int col) : m_row(0), m_col(0), m_wrap(0), m_data(0) { Create(row, col); Fill(0); }
	CFMat( int row, int col, float val ) : m_row(0),m_col(0),m_wrap(0),m_data(0) { Create( row, col, val ); }
	// destructor
	virtual ~CFMat() { Clear(); }

	// creation
	int Create( int row, int col ) 
	{ 
		Clear(); 
		m_data = (float *)malloc(sizeof(float)*row*col); 
		//m_data = (float *)_aligned_malloc(sizeof(float)*row*col, 16); 
		if( m_data == NULL ) return 0;
		m_row=row; m_col=col; 
		return 1;
	}
	int CreateFromCFMat(CFMat *cdM) {
		return this->Create(cdM->Row(),cdM->Col());
	}

	int Create( int row, int col, float val )
	{
		if( Create( row, col ) ) 
		{ 
			Fill(val); 
			return 1; 
		}
		else return 0;
	}

	int Create( int row, int col, float *pDat );

	// clear function
	void Clear( void );
	void Reset(void);

	// wrap existing data
	CFMat & Wrap( int row, int col, float *pDat );
	void UnWrap( void );



	// DATA ACCESS 
	int Row() const		 { return m_row; }
	int Col() const		 { return m_col; }
	int Height()	 { return m_row; }
	int Width()		 { return m_col; }
	int Length() const	 { return m_row*m_col; }
	int LengthByte() { return m_row*m_col*sizeof(float); }
	int IsCol()      { return m_iscol; }
	CFMat &SetCol( void ) { m_iscol=1; return (*this); }
	CFMat &UnsetCol( void ) { m_iscol=0; return (*this); }

	float At( int idx ) { return m_data[idx]; }
	float *D(void) const { return m_data; }
	float *RowD( int iRow ) { return &(m_data[m_col*iRow]); }
	float Val( int iRow, int iCol ) { 
		if( iRow<0 || iRow>=m_row-1 || iCol<0 || iCol>=m_col-1 ) return 0;
		return m_data[iRow*m_col+iCol]; 
	}
	void Set( int idx, float val ) { m_data[idx] = val; }

	CFMat &SetVal( int iRow, int iCol, float val ) 
	{ 
		if( iRow<0 || iRow>=m_row-1 || iCol<0 || iCol>=m_col-1 ) return (*this);
		m_data[ iRow*m_col+iCol ] = val; 
		return (*this); 
	}
	CFMat &Fill( float val );
	void CopyTo( float *buf ) { memcpy( buf, m_data, LengthByte() ); }
	void CopyFrom( CFMat &mat) { Clear(); Create(mat.Row(), mat.Col()); memcpy(m_data, mat.m_data, sizeof(float)*mat.Length()); }

	friend int SolveLSE( CFMat &A, CFMat &b, CFMat &x );

	// I/O
	int ReadFromFile( int row, int col, FILE *fp );


	// OPERATOR
	CFMat &operator/=(float val) ;
	CFMat &operator= (const CFMat &mat);
	CFMat &operator= (float val);
	CFMat &operator+=(float val);
	CFMat &operator-=(float val);
	CFMat &operator+=(CFMat  &b );
	CFMat &operator-=(CFMat  &b );
	const CFMat operator-(const CFMat  &b) const;
	const CFMat operator+(const CFMat  &b) const;
	const CFMat operator*(const int  b) const;
	CFMat &operator*=(float val) 
	{ 
		int cnt=Length(); 
		float *data = m_data;
		while( cnt-- ) { *data++ *= val; } return (*this);
	}
	CFMat &operator*=(int *idx) 
	{ 
		int cnt=Length(); 
		float *data = m_data;
		while( cnt-- ) { *data++ *= *idx++; } return (*this);
	}
	float *operator[](int iRow) { return  m_data+iRow*m_col; }
	CFMat & IsProduct( CFMat &a, CFMat &b );  // this = a * b
	CFMat & IsSquareDistance( CFMat &dest, const CFMat &a, const CFMat &b );
	void IsSubst  ( CFMat &a, CFMat &b );
	CFMat & AccSubArea( CFMat &m, int left, int top, int width, int height );
	CFMat & IsMulTransposed( CFMat &m, int IsAtA=0 );
	CFMat & IsMatMulAtA(CFMat &At, CFMat &A);
	CFMat & IsTransposed(CFMat &m);
	CFMat & IsInverse(CFMat &m);
	CFMat & SetIdentity( float diagonal_value );
	CFMat & IsSumCol( CFMat &m );
	CFMat & IntoRowVector() { m_col = Length(); m_row = 1; return (*this); }
	CFMat & IntoColVector() { m_row = Length(); m_col = 1; return (*this); }
	CFMat & Reshape( int row, int col )
	{
		if ( row*col != m_row*m_col )
		{
			//errmsg("CFMat::Reshape(row,col) - incorrect size\n");
			return (*this);
		}
		m_row = row; 
		m_col = col;
		return (*this);
	}

	void Save( FILE *fp );
	void Save(char *szname);
	void Load(char *szname);




	float *m_data;
	int m_col, m_row, m_wrap;
protected:
	int m_iscol;	// if 1, will be treated as column major matrix
};

#endif // !defined(AFX_DblMAP_H__B9B4CEDB_093F_435A_9EC2_1B1CF588D720__INCLUDED_)
