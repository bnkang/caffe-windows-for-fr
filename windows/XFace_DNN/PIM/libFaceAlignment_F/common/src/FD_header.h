#ifndef __FD_HEADER_H__
#define __FD_HEADER_H__

#define MAX_FACE_NUM 	10


#define FD_ANGLE1               30
#define FD_ANGLE2               60

typedef struct {
	int x, y;
}SMyPoint;

typedef struct {
	int left, top, right, bottom;
}SMyRect;

typedef struct
{
	long left;
	long top;
	long width;
	long height;
	unsigned char enable;
}FDRECT;

typedef struct
{
	FDRECT boundRect[MAX_FACE_NUM];  //¾ó±¼ÁÂÇ¥Á¤º¸
	long angle[MAX_FACE_NUM];        //¾ó±¼°¢µµ (0,-30,+30,-60,+60,-90,+90)
	long expression[MAX_FACE_NUM][4];        //facial expression ¿©ºÎ(score value:0~100)
	long id[MAX_FACE_NUM];			 //¾ó±¼ÀÎÀÎID
	long select;                     //Å«¾ó±¼ (0/1/2¡¦./9)
	long cnt;                        //¾ó±¼°³¼ö (1/2/3/¡¦/10)
	unsigned char enable;            //¾ó±¼À¯¹« (0/1)
	int img_direction[MAX_FACE_NUM];
	float RIP[MAX_FACE_NUM];
	float ROP[MAX_FACE_NUM];
} FD_INFO;

typedef struct
{
	unsigned char enable;
	unsigned char status;
	unsigned char success;
} FD_STATUS_INFO;

typedef struct
{
    long left;
    long top;
    long width;
    long height;
} BSRECT;
#endif

