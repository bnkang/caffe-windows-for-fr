#ifndef _DEFINE_H_
#define _DEFINE_H_
#include <math.h>

//struct SMyPoint {
//	int x, y;
//};
//
//struct SMyRect {
//	int left, top, right, bottom;
//};

class MyPoint {
public:	
	int x, y;

	MyPoint() { x = y = 0; }
	MyPoint(int x, int y) {
		this->x = x;	this->y = y;
	}
	MyPoint(const MyPoint& pt) {
		x = pt.x;	y = pt.y;
	}
	MyPoint& operator=(const MyPoint& pt) {
		x = pt.x;	y = pt.y;
		return *this;
	}	
};

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

	int Width() { return right-left; }
	int Height() { return bottom-top; }
};

#define FREE(x) { if(x) { free(x); x=NULL; } }

#endif