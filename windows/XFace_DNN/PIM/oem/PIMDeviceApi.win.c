
#include <conio.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <crtdbg.h>

#include "PIMTypes.h"
#include "PIMDeviceApi.h"
#include "libPlatform/inc/PIMLog.h" 
#define TAG "PIM"

const PIM_Char *PIM_Device_KPathDelimiter = "\\\\";


/****************************************************
 *                  OEM system
 ***************************************************/

/* return value : 0 ~ RAND_MAX (0x7fff, 32767) */
#define PIM_RANDOM_MAX   0x7fff
PIM_Uint32 PIM_Device_Random()
{
    //srand(time(NULL));    /* Choose a random number table */
    return rand()&PIM_RANDOM_MAX;        /* get a random number */
}

/* increased by 1 per 1 ms */
PIM_Uint32 PIM_Device_msClock()
{
    return clock();
}

/* increased by 1 per 1 us */
PIM_Uint32 PIM_Device_usClock()
{
    return clock() * 1000;
}

/* Sleep for a given miliseconds. */
PIM_Void PIM_Device_Sleep(PIM_Uint32 milisec)
{
    Sleep(milisec);
}

/* Write a line of log data */
PIM_Void PIM_Device_WriteLog(PIM_Int32 level, const PIM_Int8* tag, const PIM_Int8* file_name, const PIM_Int8* method_name, const PIM_Int32 line, const PIM_Int8* str)
{
    if(tag == NULL) {
        _cprintf("%s\n", str);
    } else {
        _cprintf("[%s:%d] %s\n", method_name, line, str);
	   //_CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "[%s:%d] %s\n", method_name, line, str);
    }
}



/****************************************************
 *                 OEM memory
 ***************************************************/

PIM_pVoid PIM_Device_Malloc(PIM_Int32 size)
{
    return malloc(size);
}

PIM_pVoid PIM_Device_Realloc(PIM_pVoid ptr, PIM_Int32 size)
{
    return realloc(ptr, size);
}

PIM_pVoid PIM_Device_Calloc(PIM_Int32 num_obj, PIM_Int32 size_obj)
{
    return calloc(num_obj, size_obj);
}

void  PIM_Device_Free(PIM_pVoid ptr)
{
    free(ptr);
}

PIM_pVoid PIM_Device_Memset(PIM_pVoid ptr, PIM_Uint8 c, PIM_Int32 size)
{
    return memset(ptr, c, size);
}

PIM_pVoid PIM_Device_Memcpy(PIM_pVoid dst, PIM_pVoid src, PIM_Int32 size)
{
    return memcpy(dst, src, size);
}

PIM_Int32 PIM_Device_Memcmp(PIM_pVoid cmpA, PIM_pVoid cmpB, PIM_Int32 size)
{
    return memcmp(cmpA, cmpB, size);
}

PIM_Void MemoryManager_InitPeakMem()
{
}

PIM_Uint32 MemoryManager_GetPeakMem()
{
    return 0;
}

PIM_Uint32 MemoryManager_GetUseMem()
{
    return 0;
}

PIM_Void  MemoryManager_ShowInfo()
{
}



/****************************************************
 *                  OEM math
 ***************************************************/
double PIM_Device_Sin(double x)
{
    return sin(x);
}

double PIM_Device_Cos(double x)
{
    return cos(x);
}

double PIM_Device_Acos(double x)
{
    return acos(x);
}

/* The function returns the non-negative square root of x. */
double PIM_Device_Sqrt(double x)
{
    return sqrt(x);
}

/* The function returns the largest integral value that is not greater than x. */
double PIM_Device_Floor(double x)
{
    return floor(x);
}

/* The function returns the smallest integral value that is not less than x. */
double PIM_Device_Ceil(double x)
{
    return floor(x);
}

/* The function returns the value of x raised to the power of y. */
double PIM_Device_Pow(double x, double y)
{
    return pow(x, y);
}



/****************************************************
 *                 OEM File
 ***************************************************/

/* fopen() mode
 * r  : read only,  The location of the stream pointer indicates the beginning of the file.
 * r+ : read/write,                                              the beginning of the file.
 * w  : write only,                                              the beginning of the file.
 * w+ : read/write,                                              the beginning of the file.
 * a  : write only,                                              the beginning of the file.
 * a+ : read/write,                                              the ending of the file.
 * t  : text mode
 * b  : binary mode
 */
PIM_pFile  PIM_Device_FileOpen(const PIM_Char *path, PIM_File_OpenMode mode)
{
    const PIM_Char* mode_str;
    FILE* openFile = NULL;
    errno_t retError = 0;
    char errorStr[256];
    memset(errorStr, 0, 256);

    switch(mode)
    {
        case PIM_FOPEN_EBINARY_READ:       mode_str = "rb";    break;
        case PIM_FOPEN_EBINARY_WRITE:      mode_str = "wb";    break;
        case PIM_FOPEN_EBINARY_NEW_WRITE:  mode_str = "wb";    break;
        case PIM_FOPEN_EBINARY_APPEND:     mode_str = "a+b";   break;
        case PIM_FOPEN_ETEXT_READ:         mode_str = "rb";    break;
        case PIM_FOPEN_ETEXT_WRITE:        mode_str = "wb";    break;
        case PIM_FOPEN_ETEXT_NEW_WRITE:    mode_str = "wb";    break;
        case PIM_FOPEN_ETEXT_APPEND:       mode_str = "a+b";   break;

        default:
        {
            LOGE("this mode(0x%08x) is not supported!", mode);
            return NULL;
        }
    }/* end switch */

    //return (PIM_pFile) fopen(path, mode_str);
    retError = fopen_s(&openFile, path, mode_str);
    if(retError != 0) {
        strerror_s(errorStr, 256, retError);
        _cprintf("fopen_s ret(%d:%s)\n", retError, errorStr);
    }
    return openFile;
}

void PIM_Device_FileClose(PIM_pFile stream)
{
    fclose((FILE*)stream);
}

PIM_Result PIM_Device_FileisEnd(PIM_pFile stream)
{
    return feof((FILE*)stream);
}

PIM_Result PIM_Device_FileisError(PIM_pFile stream)
{
    return ferror((FILE*)stream);
}

/* Reading data into block units */
PIM_Int32  PIM_Device_FileRead(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count)
{
    return fread(buff, obj_size, obj_count, (FILE*)stream);
}

/* Writing data to block units */
PIM_Int32  PIM_Device_FileWrite(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count)
{
    return fwrite(buff, obj_size, obj_count, (FILE*)stream);
}

PIM_Int32  PIM_Device_FileSeek(PIM_pFile stream, PIM_Long offset, PIM_File_SeekMode mode)
{
    return fseek((FILE*)stream, offset, (int)mode);
}

PIM_Long   PIM_Device_FileTell(PIM_pFile stream)
{
    return ftell((FILE*)stream);
}

PIM_Result PIM_Device_FileRemove(const PIM_Char *path)
{
    return remove(path);
}

PIM_Result PIM_Device_FileRename(const PIM_Char *oldpath, const PIM_Char *newpath)
{
    return rename(oldpath, newpath);
}

PIM_Int32 PIM_Device_FileTruncate(const PIM_Char *path, PIM_Int32 length)
{
    BOOL bRet;
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, length, NULL, FILE_BEGIN);

        bRet = SetEndOfFile(hFile);
        if(bRet == 0) {
            LOGE("error(%x)", GetLastError());
        }

        CloseHandle(hFile);
    }

    return 0;
}
