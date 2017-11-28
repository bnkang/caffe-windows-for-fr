
#include "PIMTypes.h"
#include "libPlatform/inc/PIMMath.h"

PIM_Int32 PIM_Point_Dist(const PIM_Point* p1, const PIM_Point* p2)
{
	PIM_Int32 dx,dy;
	dx = p1->x - p2->x;
	dy = p1->y - p2->y;
	return PIM_Math_Sqrt(dx*dx+dy*dy);
}



PIM_Void PIM_Point_Rotate(PIM_Point* dst, PIM_Point *src, PIM_Point *zerop, PIM_Int32 angle360)
{	
	PIM_Int32 x, y;
	
	x = src->x - zerop->x;
	y = zerop->y - src->y; // - zerop->y;
	
	dst->x = ( x * PIM_Math_CosS(angle360) - y * PIM_Math_SinS(angle360)) >> PIM_Math_KCosSBaseShifter;
	dst->y = ( x * PIM_Math_SinS(angle360) + y * PIM_Math_CosS(angle360)) >> PIM_Math_KCosSBaseShifter;
	
	dst->x += zerop->x;
	dst->y = zerop->y - dst->y;
}



PIM_Void PIM_Point_Move(PIM_Point* dest, PIM_Int32 offset_x, PIM_Int32 offset_y)
{
	dest->x += offset_x;
	dest->y += offset_y;
}
