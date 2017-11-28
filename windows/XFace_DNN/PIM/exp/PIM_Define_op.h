#ifndef _DEFINE_H_
#define _DEFINE_H_
#include <math.h>


//#define _BN_MODIFY_

#ifndef _BN_MODIFY_
#include "libFaceAlignment_F/alignment/src/FMat.h"
#endif

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

	int Width() 	{ return right - left; }
	int Height() 	{ return bottom - top; }
	int HCenter() 	{ return left + (right - left) / 2; }
	int VCenter() 	{ return top + (bottom - top) / 2; }
};


#define MAX_FACE_NUM	5
#define FR_ENROLL_MAX_SIZE	200
#define FR_TL_LBP_FEATURE_SIZE	1000
#define FR_DNN_FEATURE_SIZE 1024
#define FR_NORM_FACE_WIDTH 190
#define FR_NORM_FACE_HEIGHT 190
#define FR_FACE_WIDTH	95
#define FR_FACE_HEIGHT	95


typedef struct
{
	int left;
	int top;
	int width;
	int height;
	unsigned char enable;
} FDRECT;

typedef struct
{
	FDRECT boundRect[MAX_FACE_NUM];  //¾ó±¼ÁÂÇ¥Á¤º¸
	FDRECT eyeR[MAX_FACE_NUM];       //¿À¸¥ÂÊ´«ÁÂÇ¥Á¤º¸
	FDRECT eyeL[MAX_FACE_NUM];       //¿ÞÂÊ´«ÁÂÇ¥Á¤º¸
	SMyPoint pntEyeL[MAX_FACE_NUM];
	SMyPoint pntEyeR[MAX_FACE_NUM];
	
	int rip_angle[MAX_FACE_NUM];        //¾ó±¼°¢µµ (0,-30,+30,-60,+60,-90,+90)
	int pose[MAX_FACE_NUM];
	int rop_angle[MAX_FACE_NUM];        //¾ó±¼°¢µµ (0,-30,+30,-60,+60,-90,+90)
	int smile[MAX_FACE_NUM];        //Smile ¿©ºÎ(score value:0~100)
	int blink[MAX_FACE_NUM];        //ºí¸µÅ© ¿©ºÎ(score value:0~100)
	int id[MAX_FACE_NUM];			 //¾ó±¼ÀÎÀÎID	
	int select;                     //Å«¾ó±¼ (0/1/2¡¦./9)
	int cnt;                        //¾ó±¼°³¼ö (1/2/3/¡¦/10)
	int prev_cnt;
	unsigned char enable[MAX_FACE_NUM];            //¾ó±¼À¯¹« (0/1)
	unsigned char hp_enabled;				 // ÇØµå Æ÷Áî °¡´É À¯¹«(0/1)	
} FD_INFO;



typedef struct img_info
{
	unsigned char *img;
	int img_width;
	int img_height;
	FMyRect fd;
	bool fd_enabled;
	bool gt_enabled;		// true if gt is enable, false otherwise
	char name[128];
	char path[256];
}img_info;


/*
* @brief structure of data
*/
//#pragma pack(1)
typedef struct rfffffdata_info
{
	CFMat current_pts;			// current points
	CFMat initial_pts;
	CFMat current_norm_pts;		// current normalized points

	float q[4];						// similarity transform from mean shape to pts
	float inv_q[4];					// similarity transform from pts to mean shape
	float fConfidence;
	img_info *pimg;
	int nROP;
	int nHitCnt;
	int nUnknownHitCnt;
	int nRecogID;
	int nRankOneID;
	
	int nGenderType;
	int nPermanentID;	
	int TRACKING_STATUS;

	int nRecogIDs[20];

}data_info, *pdata_info;
//#pragma pack()

typedef struct _fr_enroll_info
{
	int size, person_count;
	char name[FR_ENROLL_MAX_SIZE][32];
	int id[FR_ENROLL_MAX_SIZE];
	float feature[FR_ENROLL_MAX_SIZE][FR_DNN_FEATURE_SIZE];
	//unsigned char aligned_face[FR_ENROLL_MAX_SIZE][FR_NORM_FACE_WIDTH * FR_NORM_FACE_WIDTH*3];
	unsigned char aligned_face[FR_ENROLL_MAX_SIZE][FR_FACE_WIDTH * FR_FACE_HEIGHT * 3];
} fr_enroll_info;

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


#endif
