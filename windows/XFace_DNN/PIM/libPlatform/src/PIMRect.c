#include "PIMTypes.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMPoint.h"



PIM_Void PIM_Rect_SetRect(PIM_Rect* rect, PIM_Int32 left, PIM_Int32 top, PIM_Int32 right, PIM_Int32 bottom) 
{
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;

}



PIM_Int32 PIM_Rect_GetWidth(const PIM_Rect *rect)
{
	return (rect->right - rect->left) + 1;
}

PIM_Int32 PIM_Rect_GetHeight(const PIM_Rect *rect)
{
	return (rect->bottom - rect->top) + 1;
}

PIM_Int32 PIM_Rect_GetArea(const PIM_Rect *rect)
{
	return (rect->bottom - rect->top)*(rect->right - rect->left);
}



PIM_Void PIM_Rect_SetCenterPoint(PIM_Rect *dr, const PIM_Point* sp, PIM_Int32 wh)
{
	if (wh <= 0) wh = PIM_Rect_GetWidth(dr);
		
	dr->left = sp->x - wh/2;
	dr->right = dr->left + (wh-1);
	dr->top = sp->y - wh/2;
	dr->bottom = dr->top + (wh-1) ;
}

PIM_Void PIM_Rect_GetCenterPoint(PIM_Point *p, const PIM_Rect * r)
{
	p->x = ( r->left + r->right ) /2;
	p->y = ( r->top + r->bottom ) /2;	
}



PIM_Bool PIM_Rect_IsInRect(const PIM_Int32 x, const PIM_Int32 y, const PIM_Rect* rect)
{
	return (x>=rect->left && x<=rect->right && y>=rect->top && y<=rect->bottom);	
}

PIM_Bool PIM_Rect_Intersects(const PIM_Rect *base_rect, const PIM_Rect *rect)
{
	if(rect->left >= base_rect->right || base_rect->left >= rect->right ||
       rect->top >= base_rect->bottom || base_rect->top >= rect->bottom)
    {
        return PIM_FALSE;
    } else {
        return PIM_TRUE;
    }
}

PIM_Void PIM_Rect_GetIntersectionRect(PIM_Rect *result, const PIM_Rect *rect1, const PIM_Rect *rect2)
{
	result->left = PIM_Math_Max(rect1->left, rect2->left);
	result->top = PIM_Math_Max(rect1->top, rect2->top);
	result->right = PIM_Math_Min(rect1->right, rect2->right);
	result->bottom = PIM_Math_Min(rect1->bottom, rect2->bottom);
}

PIM_Int32 PIM_Rect_GetIntersectionArea(const PIM_Rect *rect1, const PIM_Rect *rect2)
{
	PIM_Rect r;
	PIM_Rect_GetIntersectionRect(&r, rect1, rect2);
	return PIM_Rect_GetArea	(&r);
}



PIM_Void PIM_Rect_GetBoundingRect(PIM_Rect *result, const PIM_Rect *rect1, const PIM_Rect *rect2)
{
	result->left   = PIM_Math_Min(rect1->left, rect2->left);
	result->top    = PIM_Math_Min(rect1->top, rect2->top);
	result->right  = PIM_Math_Max(rect1->right, rect2->right);
	result->bottom = PIM_Math_Max(rect1->bottom, rect2->bottom);	
}

PIM_Void PIM_Rect_ClipToBound(PIM_Rect *rect, const PIM_Rect *bound)
{
	if (!rect || !bound) return;

	if (rect->left < bound->left) rect->left = bound->left;
	if (rect->right > bound->right) rect->right = bound->right;
	if (rect->top < bound->top) rect->top = bound->top;
	if (rect->bottom > bound->bottom) rect->bottom = bound->bottom;

	if((rect->left > rect->right) || (rect->top > rect->bottom)) {
		rect->left   = 0;
		rect->top    = 0;
		rect->right  = 0;
		rect->bottom = 0;
	}
}



//20080624 cbHeo
PIM_Void PIM_Rect_Rotate(PIM_Rect* dest, PIM_Rect* src, PIM_Point* cp, PIM_Int32 angle)
{
	PIM_Point src_cp;
	PIM_Rect_GetCenterPoint(&src_cp, src);
	PIM_Point_Rotate(&src_cp, &src_cp, cp, angle);	
	PIM_Rect_SetCenterPoint(dest, &src_cp, PIM_Rect_GetWidth(src));
}

PIM_Void PIM_Rect_Move(PIM_Rect* dest, PIM_Int32 offset_x, PIM_Int32 offset_y)
{
	dest->left   += offset_x;
	dest->top    += offset_y;
	dest->right  += offset_x;
	dest->bottom += offset_y;
}

PIM_Void PIM_Rect_Margin(PIM_Rect* dst, PIM_Rect*src, PIM_Int32 percent)
{
	PIM_Int32 w = (src->right - src->left) * percent / 100;

	dst->left   = PIM_Math_Max(0, src->left - w);
	dst->top    = PIM_Math_Max(0, src->top - w);
	dst->right  = PIM_Math_Max(0, src->right + w);
	dst->bottom = PIM_Math_Max(0, src->bottom + w);

}
