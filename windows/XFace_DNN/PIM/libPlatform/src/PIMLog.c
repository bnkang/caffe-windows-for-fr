#if defined(WP7)
#include "libplatform/inc/PIMLog.h"
#else
#include <stdio.h>
#include <stdarg.h>
#include "PIMDeviceApi.h"
#include "libPlatform/inc/PIMLog.h"
#endif

#define LOG_BUF_SIZE    1024

void PIM_Log_Print(PIM_Int32 level, const PIM_Int8* tag, const PIM_Int8* file_name, const PIM_Int8* method_name, const PIM_Int32 line, const PIM_Int8* fmt, ...)
{
#if defined(WP7)
	PIM_Int8 buf[1] = {0};
	PIM_Device_WriteLog(level, tag, file_name, method_name, line, buf);
#else
	va_list ap;
    PIM_Int8 buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

	PIM_Device_WriteLog(level, tag, file_name, method_name, line, (const PIM_Int8*)buf);
#endif
}

