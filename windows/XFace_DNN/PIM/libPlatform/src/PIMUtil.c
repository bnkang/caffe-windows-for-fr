
#include "PIMTypes.h"
//#include "PIMFaceDetector.h"
#include "PIMDeviceApi.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMUtil.h"
#include "libPlatform/inc/PIMLog.h"

#if defined(WP7)
#include "oem/ConfigEngine.wp7.h"
#else
#include <time.h>
#endif

#define TAG "PIMUtil"

#ifdef WIN32
//#define USE_TIME_LIMIT
#ifdef USE_TIME_LIMIT
#define TimeSecurity_LimitYear		2011
#define TimeSecurity_LimitMonth		12
#define TimeSecurity_LimitDay		1
#endif /*USE_TIME_LIMIT*/
#endif /*WIN32*/


PIM_Int32 PIM_Util_ConvertOrientationToAngle(PIM_ImageOrientation ori)
{
	switch(ori) {
        case PIM_IMAGE_ORIENTATION_NORMAL:
            return 0;

        case PIM_IMAGE_ORIENTATION_CW90:
            return 90;

        case PIM_IMAGE_ORIENTATION_CW180:
            return 180;

        case PIM_IMAGE_ORIENTATION_CW270:
            return 270;    // return -90;

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
}

void PIM_Util_BzeroMp(void *p, PIM_Int32 size)
{
	PIM_Int32 i;
	PIM_Uint8 *pc = (PIM_Uint8 *)p;
	
	if ((PIM_Int32)p & 0x3) {
		// Unaligned 
		for (i=0; i<size; i++) pc[i] = 0;
	} else {
		// Aligned	
		union { void *pv; PIM_Int32 *pi; PIM_Uint8 *pc; } u;

		PIM_Int32 i;
		u.pv = p;
		
		for (i=size>>5; i>0; i--) {
			*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;
			*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;
		}
		
		if (size & (1 << 4)) {
			*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;	*u.pi++ = 0;
		}
			
		if (size & (1 << 3)) {
			*u.pi++ = 0;	*u.pi++ = 0;
		}

		if (size & (1 << 2)) {
			*u.pi++ = 0;
		}
		
		if (size & (1 << 1)) {
			*u.pc++ = 0;	*u.pc++ = 0;
		}
		
		if (size & 1) *u.pc++ = 0;
		
	}
}

void PIM_Util_SwapInt(PIM_Int32 *a, PIM_Int32 *b)
{
	PIM_Int32 t;
	
	t = *a;
	*a = *b;
	*b = t;
}

void *PIM_Util_MemsetMp(void *p, PIM_Int32 v, PIM_Int32 size)
{
	PIM_Int32 i;
	PIM_Uint8 *pc = (PIM_Uint8 *)p;
	
	if (v == 0) {
		PIM_Util_BzeroMp(p, size);
		return p;
	}
	
	for (i=0; i<size; i++) pc[i] = v;
	
	return p;
}

void *PIM_Util_MemcpyMp(void *dst, const void *src, PIM_Int32 size)
{
	PIM_Int32 i;
	PIM_Uint8 *pd = (PIM_Uint8 *)dst;
	PIM_Uint8 *ps = (PIM_Uint8 *)src;
	
	
	if (((PIM_Int32)pd & 0x03) || ((PIM_Int32)ps & 0x03)) {
		// Unaligned memory : copy byte-by-byte
		for (i=0; i<size; i++) pd[i] = ps[i];		
	} else {
		// Aligned memory : copy 4-byte at a time
		PIM_Int32 *pid = (PIM_Int32 *)dst;
		PIM_Int32 *pis = (PIM_Int32 *)src;
		PIM_Int32 int_size = size >> 2;
		for (i=0; i<int_size; i++) pid[i] = pis[i];
		for (i<<=2; i<size; i++) pd[i] = ps[i];		// copy remaining byte when size is not a multiple of 4
	}
	
	return dst;
}

PIM_Int32 PIM_Util_MemcmpMp(const void * cs, const void *ct, PIM_Uint32 count)
{
	const PIM_Uint8 *su1, *su2;
	PIM_Int32 res = 0;
	
	for( su1 =(const PIM_Uint8 *) cs, su2 =(const PIM_Uint8 *)ct; 0< count; ++su1, ++su2, count--)
		if(( res = *su1- *su2) != 0)
			break;
	return res;
}



PIM_Uint32 PIM_Random()
{
	return PIM_Device_Random();
}


// PIM_Int32 PIM_Draw_MouthRect(PIM_FaceDetector *fd, PIM_Int32 index, PIM_Bitmap *out_img)
// {
// #if 0
//     PIM_Int32 dx_v, dy_v;
//     PIM_Int32 dx_h, dy_h;
//     PIM_Point from, to;
//     PIM_Point from_d, to_d;
// 	PIM_Int32 dist_eye;
// 	PIM_Int32 numFaces = fd->numDetectedFaces;
// 	PIM_Int32 PIMRet = PIM_ERROR_GENERAL;
// 
// 	if(index >= numFaces) return PIM_ERROR_INVALID_ARGUMENT;
// 
//   	dist_eye = PIM_Point_Dist(&fd->eyeLeft[index], &fd->eyeRight[index]) * 3/ 4;
//    	dx_v = ((dist_eye* PIM_Math_CosS(fd->ripAngle[index]+90))>>PIM_Math_KCosSBaseShifter);
//    	dy_v = ((dist_eye* PIM_Math_SinS(fd->ripAngle[index]+90))>>PIM_Math_KCosSBaseShifter);
//    	dx_h = ((dist_eye* PIM_Math_CosS(fd->ripAngle[index]))>>PIM_Math_KCosSBaseShifter);
//    	dy_h = ((dist_eye* PIM_Math_SinS(fd->ripAngle[index]))>>PIM_Math_KCosSBaseShifter);
// 
//    	from.x = fd->eyeLeft[index].x + dx_v - dx_h;  from.y = fd->eyeLeft[index].y + dy_v - dy_h;
//    	to.x = fd->eyeRight[index].x + dx_v + dx_h;  to.y = fd->eyeRight[index].y + dy_v + dy_h;
//    	from_d.x = fd->eyeLeft[index].x + 2*dx_v - dx_h;  from_d.y = fd->eyeLeft[index].y + 2*dy_v - dy_h;
//    	to_d.x = fd->eyeRight[index].x + 2*dx_v + dx_h;  to_d.y = fd->eyeRight[index].y + 2*dy_v + dy_h;
// 
//    	PIMRet = PIM_Bitmap_DrawLine(out_img, from.x, from.y, to.x, to.y, 0x00FF00);
// 	if(PIMRet < PIM_SUCCESS) return PIMRet;
// 
//    	PIM_Bitmap_DrawLine(out_img, from_d.x, from_d.y, to_d.x, to_d.y, 0x00FF00);
//    	PIM_Bitmap_DrawLine(out_img, from_d.x, from_d.y, from.x, from.y, 0x00FF00);
//    	PIM_Bitmap_DrawLine(out_img, to.x, to.y, to_d.x, to_d.y, 0x00FF00);
// 
// #endif
// 	return PIM_SUCCESS;
// }



static void __quickSort(PIM_Index_Value* data, PIM_Int32 index_low, PIM_Int32 index_high, PIM_Int32 pivot);
static PIM_Int32  __partition(PIM_Index_Value* data, PIM_Int32 index_low, PIM_Int32 index_high, PIM_Int32 pivot);
static void __swap(PIM_Index_Value *a, PIM_Index_Value *b);

void PIM_Util_Quicksort_Int(PIM_Index_Value* data, PIM_Int32 data_size)
{
    __quickSort(data, 0, data_size, data[data_size/2].index);
}

void __quickSort(PIM_Index_Value* data, PIM_Int32 index_low, PIM_Int32 index_high, PIM_Int32 pivot)
{
    if( index_high > index_low ) {
        pivot = __partition(data, index_low, index_high, pivot);
        __quickSort(data, index_low, pivot-1, pivot);
        __quickSort(data, pivot+1, index_high, pivot);
    }
}

PIM_Int32 __partition(PIM_Index_Value* data, PIM_Int32 index_low, PIM_Int32 index_high, PIM_Int32 pivot)
{
    PIM_Int32 i, j;
    PIM_Int32 item;

    item = data[index_low].value;
    j = index_low;

    for(i=index_low+1; i<=index_high; i++) {
        if(data[i].value < item){
            j++;
            __swap(&data[i], &data[j]);
        }
    }

    pivot = j;
    __swap(&data[index_low], &data[pivot]);

    return pivot;
}

void __swap(PIM_Index_Value *a, PIM_Index_Value *b)
{
    PIM_Int32 temp;
    temp = a->index;
    a->index = b->index;
    b->index = temp;

    temp = a->value;
    a->value = b->value;
    b->value = temp;
}

static const PIM_Int8 DAYS_PER_MONTH[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static PIM_Bool isLeapYear(PIM_Int32 year)
{
	if ((year % 400) == 0) return PIM_TRUE;
	if ((year % 100) == 0) return PIM_FALSE;
	return ((year % 4) == 0) ? 	PIM_TRUE : PIM_FALSE;
}

static void converIntTimeToPIMTm(const PIM_Time_t *tt, PIM_Tm *tm)
{
	PIM_Time_t t = *tt;
	PIM_Int32 day_count, day_sec;
	PIM_Int32 year, month;

	day_count = t / (24 * 60 * 60);
	day_sec = t % (24 * 60 * 60);

	for (year=1970; day_count>=0; year++) {
		PIM_Int32 year_day = 365;
		if (isLeapYear(year)) year_day++;

		if (day_count >= year_day) day_count -= year_day;
		else break;
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = day_count;
	tm->tm_wday = (day_count + 4) % 7;	// 1970.1.1 was Thursday.

	for (month=0; day_count>=0; month++) {
		if (month == 1 && isLeapYear(year)) {
			if (day_count >= 29) day_count -= 29;
			else break;
		} else {
			if (day_count >= DAYS_PER_MONTH[month]) day_count -= DAYS_PER_MONTH[month];
			else break;
		}
	}
	tm->tm_mon = month + 1;
	tm->tm_mday = day_count + 1;

	tm->tm_sec = day_sec % 60;		day_sec /= 60;
	tm->tm_min = day_sec % 60;		day_sec /= 60;
	tm->tm_hour = day_sec;
}

/* Get system time of day */
static PIM_Void getTimeOfDay(PIM_Tm *t)
{
	PIM_Time_t tm = (PIM_Time_t)time(NULL);
	converIntTimeToPIMTm( &tm, t);	
}

PIM_Result PIM_Util_CheckTimeLimit()
{
#ifndef USE_TIME_LIMIT
	return PIM_SUCCESS;
#else
	PIM_Tm current_time;
	PIM_Result ret = PIM_SUCCESS;

	getTimeOfDay(&current_time);

	LOGD("current years_ = %d, month = %d, day = %d",current_time.tm_year+1900, current_time.tm_mon, current_time.tm_mday);
	LOGD("expiring years_ = %d, month = %d, day = %d",TimeSecurity_LimitYear, TimeSecurity_LimitMonth, TimeSecurity_LimitDay);

	if((current_time.tm_year+1900) > TimeSecurity_LimitYear)
		ret = PIM_ERROR_EXPIRED;
	else if((current_time.tm_year+1900) == TimeSecurity_LimitYear)
	{
		if(current_time.tm_mon > TimeSecurity_LimitMonth)
			ret = PIM_ERROR_EXPIRED;
		else if(current_time.tm_mon == TimeSecurity_LimitMonth)
		{
			if(current_time.tm_mday >= TimeSecurity_LimitDay)
				ret = PIM_ERROR_EXPIRED; 
		}
	}

	if(ret != PIM_SUCCESS) 
	{
		LOGE("ERROR : Library was expired");
	}
	return ret;
#endif
}

