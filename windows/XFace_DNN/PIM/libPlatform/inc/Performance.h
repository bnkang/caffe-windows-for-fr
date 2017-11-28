/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PERFORM_EAllClear  0x00000000
#define PERFORM_EOverDelay 0x00000001
#define PERFORM_SUCCESS_ALL 7777

/**
 *  @struct Performance Performance.h "Performance.h"
 *  @brief Performance structure.
 */
typedef struct __Performance {
    PIM_Uint32 total_count;
    PIM_Uint32 total_delay;
	PIM_Uint32 total_delay_avg;
    PIM_Uint32 success_count;
    PIM_Uint32 success_delay;
    PIM_Uint32 success_delay_avg;
    PIM_Uint32 success_delay_min;
    PIM_Uint32 success_delay_max;
    PIM_Uint32 fail_count;
    PIM_Uint32 fail_delay;
    PIM_Uint32 fail_delay_avg;
    PIM_Uint32 fail_delay_min;
    PIM_Uint32 fail_delay_max;

	PIM_Uint32 status;
	PIM_Uint8  process_name[256];
    PIM_Char   *unit_str;

	/* image info */
	PIM_Int32  width;
	PIM_Int32  height;
	PIM_ImageFormat  imageFormat;
} Performance;

/**
 *  @brief 
 *
 *  @param perform [IN/OUT] : 
 *  @param process_name [IN] : 
 *  @param bitmap [IN] : 
 */
extern void Performance_InitResult(Performance* perform, const PIM_Uint8* process_name, PIM_Bitmap* bitmap);
extern void Performance_SetUnit(Performance* perform, PIM_Char *unit_str);

/**
 *  @brief 
 *
 *  @param perform [IN/OUT] : 
 *  @param is_success [IN] : 
 *  @param delay_ms [IN] : 
 */
extern void Performance_AddResult(Performance* perform, PIM_Bool is_success, PIM_Uint32 delay_ms);

/**
 *  @brief 
 *
 *  @param perform [IN/OUT] : 
 */
extern PIM_Uint32 Performance_GetStatus(Performance* perform);

/**
 *  @brief 
 *
 *  @param perform [IN/OUT] : 
 */
extern void Performance_ShowResult(Performance* perform);


/**
 *  @brief 
 *
 *  @param avg_count [IN] : 
 */
extern void Performance_Init(PIM_Uint32 avg_count);

/**
 *  @brief 
 */
extern void Performance_Begin();

/**
 *  @brief 
 */
extern void Performance_End();

/**
 *  @brief 
 */
extern void Performance_Print();

/**
 *  @brief 
 *
 *  @return : 
 */
extern PIM_Uint32 Performance_FPS();

#ifdef __cplusplus
}
#endif

#endif /*__PERFORMANCE_H__*/
