/**
 *	@file PIMDeviceApi.h
 *
 *	@brief Declares device(platform) dependent interface.
 *	Some of functions in this file have to implemented by device vendor.
 *
 *	Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */

#ifndef __PIMDEVICEAPI_H__
#define __PIMDEVICEAPI_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************
 *                  OEM system
 ***************************************************/
/**
 *  @brief make random value.
 *
 *  @return : it returns a value between 0 and RAND_MAX(0x7fff). RAND_MAX is defined in libPlt/inc/PIMUtil.h
 */
extern PIM_Uint32 PIM_Device_Random();

/**
 *  @brief query the hardware clock. increased by 1 per 1 ms
 */
extern PIM_Uint32 PIM_Device_msClock();

/**
 *  @brief query the hardware clock. increased by 1 per 1 us
 */
extern PIM_Uint32 PIM_Device_usClock();

/**
 *  @brief delay for a specified amount of time
 *
 *  @param milisec [IN] : number for miliseconds.
 */
extern PIM_Void PIM_Device_Sleep(PIM_Uint32 milisec);

/**
 *  @brief write a line of log data
 *
 *  @param level [IN] : 
 *  @param tag [IN] : 
 *  @param file_name [IN] : 
 *  @param method_name [IN] : 
 *  @param line [IN] : 
 *  @param str [IN] : 
 */
extern PIM_Void PIM_Device_WriteLog(PIM_Int32 level, const PIM_Int8* tag, const PIM_Int8* file_name, const PIM_Int8* method_name, const PIM_Int32 line, const PIM_Int8* str); /* Write a line of log data */
		

/****************************************************
 *                  OEM memory
 ***************************************************/
/**
 *  @brief allocate dynamic memory.
 *
 *  @param size [IN] : the size of the allocated memory
 *
 *  @return the pointer of allocated memory.
 */
extern PIM_pVoid PIM_Device_Malloc(PIM_Int32 size);

/**
 *  @brief changes the size of the memory block pointed to by ptr to size bytes.
 *
 *  @param ptr [IN] :  a memory pointer
 *  @param size [IN] : the size of the allocated memory
 *
 *  @return the pointer of allocated memory.
 */
extern PIM_pVoid PIM_Device_Realloc(PIM_pVoid ptr, PIM_Int32 size);

/**
 *  @brief allocate dynamic memory, it was set to zero.
 *
 *  @param num_obj [IN] : the count of objects.
 *  @param size_obj [IN] : the size of an object.
 *
 *  @return the pointer of allocated memory.
 */
extern PIM_pVoid PIM_Device_Calloc(PIM_Int32 num_obj, PIM_Int32 size_obj);

/**
 *  @brief free dynamic memory
 *
 *  @param ptr [IN] : memory pointer
 */
extern PIM_Void  PIM_Device_Free(PIM_pVoid ptr);

/**
 *  @brief fill memory with a constant byte
 *
 *  @param ptr [IN] : a destination memory block
 *  @param c [IN] : a constant byte
 *  @param size [IN] : a memory size
 *
 *  @return the pointer of the 'ptr'.
 */
extern PIM_pVoid PIM_Device_Memset(PIM_pVoid ptr, PIM_Uint8 c, PIM_Int32 size);

/**
 *  @brief copy memory area
 *
 *  @param dst [IN] : a destination memory block
 *  @param src [IN] : a source memory block
 *  @param size [IN] : a memory size
 *
 *  @return the pointer of the 'dst'.
 */
extern PIM_pVoid PIM_Device_Memcpy(PIM_pVoid dst, PIM_pVoid src, PIM_Int32 size);

/**
 *  @brief compare memory areas
 *
 *  @param cmpA [IN] : a memory block
 *  @param cmpB [IN] : a memory block
 *  @param size [IN] : a memory size
 *
 *  @return it returns an integer less than, equal to, or greater than zero if the first n bytes of 'cmpA' is found, respectively, to be less than, to match, or be greater than the first n bytes of 'cmpB'.
 */
extern PIM_Int32 PIM_Device_Memcmp(PIM_pVoid cmpA, PIM_pVoid cmpB, PIM_Int32 size);
	

/****************************************************
 *                  OEM math
 ***************************************************/
/**
 *  @brief it returns the sine of the input value.
 *
 *  @param x [IN] : x is given in radians.
 *
 *  @return 
 */
extern double PIM_Device_Sin(double x);

/**
 *  @brief it returns the cosine of the input value.
 *
 *  @param x [IN] : x is given in radians.
 *
 *  @return
 */
extern double PIM_Device_Cos(double x);

/**
 *  @brief it returns the arc cosine of the input value.
 *
 *  @param x [IN] : x is given in radians.
 *
 *  @return
 */
extern double PIM_Device_Acos(double x);

/**
 *  @brief The function returns the non-negative square root of the input value.
 *
 *  @param x [IN] : 
 *
 *  @return
 */
extern double PIM_Device_Sqrt(double x);

/**
 *  @brief The function returns the largest integral value that is not greater than the input value.
 *
 *  @param x [IN] : 
 *
 *  @return
 */
extern double PIM_Device_Floor(double x);

/**
 *  @brief The function returns the smallest integral value that is not less than the input value.
 *
 *  @param x [IN] : 
 *
 *  @return
 */
extern double PIM_Device_Ceil(double x);

/**
 *  @brief The function returns the value of 'x' raised to the power of 'y'.
 *
 *  @param x [IN] : 
 *  @param y [IN] : 
 *
 *  @return
 */
extern double PIM_Device_Pow(double x, double y);


/****************************************************
 *                  OEM file
 ***************************************************/
/**
 *  @brief The function opens the file whose name is the string pointed to by 'path' and associates a stream with it.
 *
 *  @param path [IN] : 
 *  @param mode [IN] : 
 *
 *  @return the pointer of the opened file.
 */
extern PIM_pFile  PIM_Device_FileOpen(const PIM_Char *path, PIM_File_OpenMode mode);

/**
 *  @brief The function flushes the stream pointed to by 'stream'
 *
 *  @param stream [IN] : 
 */
extern PIM_Void   PIM_Device_FileClose(PIM_pFile stream); 

/**
 *  @brief The function tests the end-of-file indicator for the stream pointed to by stream.
 *
 *  @param stream [IN] : 
 *
 *  @return returning PIM_TRUE if it is set.
 */
extern PIM_Bool   PIM_Device_FileisEnd(PIM_pFile stream);

/**
 *  @brief The function tests the error indicator for the stream pointed to by stream.
 *
 *  @param stream [IN] : 
 *
 *  @return returning PIM_TRUE if it is set.
 */
extern PIM_Bool   PIM_Device_FileisError(PIM_pFile stream);

/**
 *  @brief The function reads 'obj_count' elements of data, each 'obj_size' bytes long, from the stream pointed to by stream, storing them at the location given by 'buff'.
 *
 *  @param stream [IN] : 
 *  @param buff [IN] : 
 *  @param obj_size [IN] : 
 *  @param obj_count [IN] : 
 *
 *  @return return the number of items successfully read. If an error occurs, or the end-of-file is reached, the return value is a short item count (or zero).
 */
extern PIM_Int32  PIM_Device_FileRead(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count);

/**
 *  @brief The function writes 'obj_count' elements of data, each 'obj_size' bytes long, to the stream pointed to by stream, obtaining them from the location given by 'buff'.
 *
 *  @param stream [IN] : 
 *  @param buff [IN] : 
 *  @param obj_size [IN] : 
 *  @param obj_count [IN] : 
 *
 *  @return return the number of items successfully read. If an error occurs, or the end-of-file is reached, the return value is a short item count (or zero).
 */
extern PIM_Int32  PIM_Device_FileWrite(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count);

/**
 *  @brief The function sets the file position indicator for the stream pointed to by 'stream'.
 *
 *  @param stream [IN] : 
 *  @param offset [IN] : 
 *  @param mode [IN] : 
 *
 *  @return
 */
extern PIM_Int32  PIM_Device_FileSeek(PIM_pFile stream, PIM_Long offset, PIM_File_SeekMode mode);

/**
 *  @brief The function obtains the current value of the file position indicator for the stream pointed to by 'stream'.
 *
 *  @param stream [IN] : 
 *
 *  @return
 */
extern PIM_Long   PIM_Device_FileTell(PIM_pFile stream);

/**
 *  @brief The function deletes a name from the file system.
 *
 *  @param path [IN] : 
 *
 *  @return On success, zero is returned.  On error, -1 is returned.
 */
extern PIM_Result PIM_Device_FileRemove(const PIM_Char *path);

/**
 *  @brief The function renames the filenames supplied according to the rule specified as the first argument.
 *
 *  @param oldpath [IN] : 
 *  @param newpath [IN] : 
 *
 *  @return
 */
extern PIM_Result PIM_Device_FileRename(const PIM_Char *oldpath, const PIM_Char *newpath);

/**
 *  @brief The function shrinks or extends the size of each FILE to the specified size.
 *
 *  @param path [IN] : 
 *  @param length [IN] : 
 *
 *  @return
 */
extern PIM_Int32  PIM_Device_FileTruncate(const PIM_Char *path, PIM_Int32 length);

#ifdef __cplusplus
}
#endif

#endif /*__PIMDEVICEAPI_H__*/
