#ifndef _DEFINE_H_
#define _DEFINE_H_
#include <math.h>

struct MyPoint {
	int x, y;
};

// struct SMyPoint {
// 	int x, y;
// };
// 
// struct SMyPoint2 {
// 	int x, y;
// };

struct DMyPoint {
	double x, y;
};

struct DMyPoint2 {
	double x, y;
};

// typedef struct SMyRect {
// 	int left, top, right, bottom;
// }SMyRect;




class MyRect {
public:
	int left, top, right, bottom;

	MyRect() { left = top = right = bottom = 0; }
	MyRect(int left, int top, int right, int bottom) {
		this->left = left;		this->top = top;
		this->right = right;	this->bottom = bottom;
	}
	MyRect(const MyRect& rc) {
		left = rc.left;		top = rc.top;
		right = rc.right;	bottom = rc.bottom;
	}
	MyRect& operator=(const MyRect& rc) {
		left = rc.left;		top = rc.top;
		right = rc.right;	bottom = rc.bottom;
		return *this;
	}

	int Width() 	{ return right-left; }
	int Height() 	{ return bottom-top; }
	int HCenter() 	{ return left + (right  - left)/ 2; }
	int VCenter() 	{ return top  + (bottom - top) / 2; }
};

#define FREE(x) { if(x) { free(x); x=NULL; } }

// #if !defined (true ) ||  !defined (false) 
// #define true 1
// #define false 0
// #endif

//#ifndef bool 
//	typedef 	int 	bool;
//#endif

#ifndef max
#define max( a, b )            ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#ifndef min
#define min( a, b )            ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif
//#define VCNT	16
//#define VCNT2	32


//#define ARGB_8888	1
#endif