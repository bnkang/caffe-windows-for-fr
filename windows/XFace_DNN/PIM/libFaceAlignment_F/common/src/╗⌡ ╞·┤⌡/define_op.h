#ifndef _DEFINE_H_
#define _DEFINE_H_
#include <math.h>

struct MyPoint {
	int x, y;
};

struct SMyPoint {
	int x, y;
};

struct SMyPoint2 {
	int x, y;
};

struct DMyPoint {
	double x, y;
};

struct FMyPoint {
	float x, y;
};

struct DMyPoint2 {
	double x, y;
};

typedef struct SMyRect {
	int left, top, right, bottom;
}SMyRect;

typedef struct FMyRect {
	float left, top, right, bottom;
}FMyRect;

#define RECT_WIDTH(rect)	(rect.right - rect.left)
#define RECT_HEIGHT(rect)	(rect.bottom - rect.top)

typedef struct SSize {
	int width, height;
};

typedef struct UMat {
	int cols, rows;
	unsigned char *m_data;
};

typedef struct FMat {
	int rows, cols;
	float *m_data;
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

	int Width() 	{ return right-left; }
	int Height() 	{ return bottom-top; }
	int HCenter() 	{ return left + (right  - left)/ 2; }
	int VCenter() 	{ return top  + (bottom - top) / 2; }
};


#define MAX_FACE_NUM	200

typedef struct
{
	long left;
	long top;
	long width;
	long height;
	unsigned char enable;
} FDRECT;

typedef struct
{
	FDRECT boundRect[MAX_FACE_NUM];  //����ǥ����
	FDRECT eyeR[MAX_FACE_NUM];       //�����ʴ���ǥ����
	FDRECT eyeL[MAX_FACE_NUM];       //���ʴ���ǥ����
	long rip_angle[MAX_FACE_NUM];        //�󱼰��� (0,-30,+30,-60,+60,-90,+90)
	long rop_angle[MAX_FACE_NUM];        //�󱼰��� (0,-30,+30,-60,+60,-90,+90)
	long smile[MAX_FACE_NUM];        //Smile ����(score value:0~100)
	long blink[MAX_FACE_NUM];        //��ũ ����(score value:0~100)
	long id[MAX_FACE_NUM];			 //������ID	
	long select;                     //ū�� (0/1/2��./9)
	long cnt;                        //�󱼰��� (1/2/3/��/10)
	bool enable[MAX_FACE_NUM];            // alignment ����
} FD_INFO;

#define FREE(x) { if(x) { free(x); x=NULL; } }

#if !defined (true ) ||  !defined (false) 
#define true 1
#define false 0
#endif

//#ifndef bool 
//	typedef 	int 	bool;
//#endif

#ifndef max
#define max( a, b )            ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#ifndef min
#define min( a, b )            ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

#define fa_max( a, b )            ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define fa_min( a, b )            ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )



//#define ARGB_8888	1
#endif