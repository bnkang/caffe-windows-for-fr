#include "PIMTypes.h"
#include "PIMDeviceApi.h"
#include "libPlatform/inc/Performance.h"
#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"

#ifdef __cplusplus
extern "C" {
#endif

static const PIM_Char *DEFAULT_UNIT_STR = "ms";

void Performance_SetUnit(Performance* perform, PIM_Char *unit_str)
{
    perform->unit_str = unit_str;
}

void Performance_InitResult(Performance* perform, const PIM_Uint8* process_name, PIM_Bitmap* bitmap)
{
	int i = 0;
	PIM_Memset(perform, 0, sizeof(Performance));

	//sprintf((char*)perform->process_name, "%s", process_name);	// API
	for(i = 0 ; i < 256 ; i++)
	{
		if(process_name[i] == '\0')
		{
			perform->process_name[i] = '\0';
			break;
		}
		perform->process_name[i] = process_name[i];
	}
	

	perform->success_delay_min  = 100000;
	perform->success_delay_max  = 0;
	perform->fail_delay_min     = 100000;
	perform->fail_delay_max     = 0;

    if(bitmap != NULL) {
        perform->width  = bitmap->width;
        perform->height = bitmap->height;
        perform->imageFormat = bitmap->imageFormat;
    }

    perform->unit_str = DEFAULT_UNIT_STR;
}

void Performance_AddResult(Performance* perform, PIM_Bool is_success, PIM_Uint32 delay)
{
	PIM_Uint32 past_total_delay;

	if(perform->status != 0) {
		LOGE("perform status error (0x%08x)!!!", perform->status);
		return;
	}

	past_total_delay = perform->total_delay;

	perform->total_count++;
	perform->total_delay += delay;

	if(past_total_delay > perform->total_delay) {
		perform->status |= PERFORM_EOverDelay;
	}

	if(is_success == PIM_TRUE) {
		perform->success_count++;
		perform->success_delay += delay;

		if(perform->success_delay_min > delay) perform->success_delay_min = delay;
		if(perform->success_delay_max < delay) perform->success_delay_max = delay;
	} else {
		perform->fail_count++;
		perform->fail_delay += delay;

		if(perform->fail_delay_min > delay) perform->fail_delay_min = delay;
		if(perform->fail_delay_max < delay) perform->fail_delay_max = delay;
	}

	if(perform->total_count>0)   perform->total_delay_avg   = perform->total_delay   / perform->total_count;
	if(perform->success_count>0) perform->success_delay_avg = perform->success_delay / perform->success_count;
	if(perform->fail_count>0)    perform->fail_delay_avg    = perform->fail_delay    / perform->fail_count;
}

void Performance_ShowResult(Performance* perform)
{
	LOGI(" ");
	LOGI("================================");
	LOGI("process [%s] loop(%d)", perform->process_name, perform->total_count);
	LOGI("info size(%dx%d) format(0x%08x)", perform->width, perform->height, perform->imageFormat);
	Performance_GetStatus(perform);
	//LOGI("total");
	//LOGI("\tcount (%8u)", perform->total_count);
	//LOGI("\tdelay (%8u) %s", perform->total_delay,     perform->unit_str);
	//LOGI("\tavg   (%8u) %s", perform->total_delay_avg, perform->unit_str);
    if(perform->success_count > 0) {
        LOGI("success");
        LOGI("\tcount (%8u)", perform->success_count);
        LOGI("\tdelay (%8u) %s", perform->success_delay,     perform->unit_str);
        LOGI("\tavg   (%8u) %s", perform->success_delay_avg, perform->unit_str);
        LOGI("\tmin   (%8u) %s", perform->success_delay_min, perform->unit_str);
        LOGI("\tmax   (%8u) %s", perform->success_delay_max, perform->unit_str);
    }

    if(perform->fail_count > 0) {
        LOGI("fail");
        LOGI("\tcount (%8u)", perform->fail_count);
        LOGI("\tdelay (%8u) %s", perform->fail_delay,     perform->unit_str);
        LOGI("\tavg   (%8u) %s", perform->fail_delay_avg, perform->unit_str);
        LOGI("\tmin   (%8u) %s", perform->fail_delay_min, perform->unit_str);
        LOGI("\tmax   (%8u) %s", perform->fail_delay_max, perform->unit_str);
    }
}

PIM_Uint32 Performance_GetStatus(Performance* perform)
{
	LOGI("status [0x%08x]", perform->status);
	if(perform->status & PERFORM_EOverDelay) LOGI("\tOverDelay");

	return perform->status;
}


static unsigned long g_total_delay;
static PIM_Uint32 g_begin_clock;
static PIM_Uint32 g_avg_count = 30;
static PIM_Uint32 g_cur_count;

void Performance_Init(PIM_Uint32 avg_count)
{
	g_avg_count = avg_count;
	g_cur_count = 0;
	g_total_delay = 0;
}

void Performance_Begin()
{
	g_begin_clock = PIM_Device_msClock();
}

void Performance_End()
{
	PIM_Uint32 delay = PIM_Device_msClock() - g_begin_clock;
	g_total_delay += delay;
	g_cur_count++;

	if(g_cur_count >= g_avg_count) {
		Performance_Print();

		g_total_delay = 0;
		g_cur_count = 0;
	}
}

void Performance_Print()
{
	PIM_Uint32 count = (g_cur_count < g_avg_count)?g_cur_count:g_avg_count;
	if(count == 0) {
		LOGE("count is zero...");
		return;
	}

	LOGI("total_delay (%8u)", g_total_delay);
	LOGI("total_count (%8u)", count);
	LOGI("avg delay   (%8u)", g_total_delay / count);
}

static PIM_Uint32 g_frame_count;				//The number of calls
static PIM_Uint32 g_frame_time;		//Cumulative time
static PIM_Uint32 g_frame_old_clock;
static PIM_Uint32 g_frame_fps;

PIM_Uint32 Performance_FPS()
{
	PIM_Uint32 cur_clock = PIM_Device_msClock(); 
	g_frame_count++;
	g_frame_time += (cur_clock - g_frame_old_clock);

	if(g_frame_time > 1000) {	// 1000ms
		g_frame_fps = g_frame_count * 1000 / g_frame_time;
		LOGI("fps (%3d)", g_frame_fps);

		g_frame_count = 0;
		g_frame_time = 0;
	}

	g_frame_old_clock = cur_clock;

	return g_frame_fps;
}

#ifdef __cplusplus
}
#endif
