/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMLOG_H__
#define __PIMLOG_H__

#include "PIMTypes.h"
#include "PIMDeviceApi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Defines debug level of engines.
 */
typedef enum
{
    PIM_LOG_ERROR,        // Only errors are printed, if occurrs.
    PIM_LOG_INFO,         // library info message
    PIM_LOG_DEBUG,        // debug message
    PIM_LOG_FUNC          // function trace message
} PIM_DebugLevel;

#if defined __LOG_INFO__
	#define LOGE(...) PIM_Log_Print(PIM_LOG_ERROR,   TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGI(...) PIM_Log_Print(PIM_LOG_INFO,    TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGD(...)
	#define LOGF(...)
#elif defined __LOG_DEBUG__
	#define LOGE(...) PIM_Log_Print(PIM_LOG_ERROR,   TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGI(...) PIM_Log_Print(PIM_LOG_INFO,    TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGD(...) PIM_Log_Print(PIM_LOG_DEBUG,   TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGF(...)
#else	// __LOG_FUNC__
	#define LOGE(...) PIM_Log_Print(PIM_LOG_ERROR,   TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGI(...) PIM_Log_Print(PIM_LOG_INFO,    TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGD(...) PIM_Log_Print(PIM_LOG_DEBUG,   TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define LOGF(...) PIM_Log_Print(PIM_LOG_FUNC,    TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#if defined __LOG_INFO__
    #define LOG_TIME_DEFINE(n)
    #define LOG_TIME_START(n)
    #define LOG_TIME_END(n)
    #define LOG_TIME(n)
    #define LOG_TIME_PRINT(...)
#else
    #define LOG_TIME_DEFINE(n)    PIM_Uint32 clock_start_##n, clock_stop_##n, clock_log_##n = 0
    #define LOG_TIME_START(n)     clock_start_##n = PIM_Device_msClock()
    #define LOG_TIME_END(n)       clock_stop_##n = PIM_Device_msClock(), clock_log_##n = clock_stop_##n - clock_start_##n
    #define LOG_TIME(n)           clock_log_##n
    #define LOG_TIME_PRINT(...)   PIM_Log_Print(PIM_LOG_INFO, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

/**
 *  @brief 
 *  @note 
 *
 *  @param level [IN] : 
 *  @param tag [IN] : 
 *  @param file_name [IN] : 
 *  @param method_name [IN] : 
 *  @param line [IN] : 
 *  @param fmt [IN] : 
 */
extern void PIM_Log_Print(PIM_Int32 level, const PIM_Int8* tag, const PIM_Int8* file_name, const PIM_Int8* method_name, const PIM_Int32 line, const PIM_Int8* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __PIMLOG_H__ */
