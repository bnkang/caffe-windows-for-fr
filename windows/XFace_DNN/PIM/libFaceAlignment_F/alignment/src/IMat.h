// IMap.h: interface for the CIMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IMAT_H_
#define _IMAT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


class CIMat 
{
public:
	int clear( void );
	CIMat & IsProductAdd_fix( CIMat &a, CIMat &b, CIMat &c, int shift );
	CIMat & IsProductAdd(CIMat &a, CIMat &b, CIMat &c);
	CIMat & IsProduct_fix( CIMat &a, CIMat &b, int shift );
	CIMat & IsProduct( CIMat &a, CIMat &b );
	CIMat & IsSubst  ( CIMat &a, CIMat &b );

	CIMat() : m_row(0), m_col(0), m_wrap(0), m_data(0) {};
	CIMat( CIMat &mat ) : m_row(0), m_col(0),m_wrap(0),m_data(0) { Create(mat.Row(),mat.Col(),mat.D()); }
	CIMat( int row, int col, int val ) : m_row(0),m_col(0),m_wrap(0),m_data(0) { Create( row, col, val ); }
	CIMat( int row, int col ) : m_row(0),m_col(0),m_wrap(0),m_data(0) { Create( row, col ); }

	virtual ~CIMat() {Clear(); }
	void Clear( void );


	int Create( int row, int col, int *pDat=NULL );
	int Create( int row, int col, int val )
	{
		if( Create( row, col ) )  { Fill(val); return 1; }
		else return 0;
	}
	//int CreateFromCvMat( CvMat *pCvMat );

	void CopyFrom( CIMat &mat) { Clear(); Create(mat.Row(), mat.Col()); memcpy(m_data, mat.m_data, sizeof(int)*mat.Length()); }


	CIMat & Wrap( int row, int col, int *pDat );
	void UnWrap( void );


	int Row()		 { return m_row; }
	int Col()		 { return m_col; }
	int Height()	 { return m_row; }
	int Width()		 { return m_col; }
	int Length() 	 { return m_col*m_row; }
	int LengthByte() { return m_row*m_col*sizeof(int); }
	//int IsCol()     { return m_iscol; }

	int *D(void)	{ return m_data; }
	int *RowD( int iRow ) { return &(m_data[m_col*iRow]); }
	int Val( int iRow, int iCol ) { return m_data[iRow*m_col+iCol]; }

	CIMat& IsTransposed(CIMat &m);

	//CIMat &SetCol( void ) { m_iscol=1; return (*this); }
	//CIMat &UnsetCol( void ) { m_iscol=0; return (*this); }
	CIMat &SetVal( int iRow, int iCol, int val )
	{
		m_data[ iRow*m_col+iCol ] = val;
		return *this;
	}
	CIMat &Fill( int val );
	CIMat & Reshape(int row, int col)
	{
		if (row*col != m_row*m_col)
		{
			//errmsg("CFMat::Reshape(row,col) - incorrect size\n");
			return (*this);
		}
		m_row = row;
		m_col = col;
		return (*this);
	}
	void Sort();


	// I/O
	int ReadFromFile( int row, int col, FILE *fp );
	

	CIMat & operator=(int val);
	CIMat & operator=(CIMat &mat);
	CIMat & operator+=(int iScalar);
	CIMat & operator*=(double dScalar);
	CIMat & operator+=(CIMat &b);
	CIMat & operator>>=(int iScalar);
	int * operator[](int iRow) { return &(m_data[iRow*m_col]); }

	int *m_data;
	int m_row, m_col, m_wrap;
protected:
	int is_col;
};

#endif // !defined(AFX_IntMAP_H__F3DEB3EF_E51E_4E7A_B5D6_DE6E1D2B8337__INCLUDED_)
