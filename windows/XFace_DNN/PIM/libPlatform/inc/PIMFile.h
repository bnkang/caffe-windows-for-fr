/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMFILE_H__
#define __PIMFILE_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief 
 *
 *  @param path [IN] : 
 *  @param mode [IN] : 
 *
 *  @return : file stream pointer.
 */
extern PIM_pFile PIM_File_Open(const PIM_Char *path, PIM_File_OpenMode mode);

/**
 *  @brief 
 *
 *  @param file [IN] : file stream pointer
 */
extern PIM_Void PIM_File_Close(PIM_pFile file); 

/**
 *  @brief check end of file
 *
 *  @param file [IN] : file stream pointer
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_File_isEnd(PIM_pFile file);

/**
 *  @brief check errors in the file.
 *
 *  @param file [IN] : file stream pointer
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_File_isError(PIM_pFile file);

/**
 *  @brief Reading data into block units
 *
 *  @param file [IN] : file stream pointer
 *  @param buff [OUT] : pre-allocated memory block, (buff_length >= obj_size * obj_count)
 *  @param obj_size [IN] : The size of one object
 *  @param obj_count [IN] : The length of object
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret >= PIM_SUCCESS : file size.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_File_Read(PIM_pFile file, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count);

/**
 *  @brief Writing data to block units
 *
 *  @param file [IN] : file stream pointer
 *  @param buff [IN] : pre-allocated memory block, (buff_length >= obj_size * obj_count)
 *  @param obj_size [IN] : The size of one object
 *  @param obj_count [IN] : The length of object
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret >= PIM_SUCCESS : file size.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_File_Write(PIM_pFile file, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count);

/**
 *  @brief move the position of read/write File pointer
 *
 *  @param file [IN] : file stream pointer
 *  @param offset [IN] : Based on the location from "PIM_File_SeekMode"
 *  @param mode [IN] : 
 *
 *  @see PIM_File_SeekMode : see "libPlatform/inc/PIMFile.h"
 */
extern PIM_Int32 PIM_File_Seek(PIM_pFile file, PIM_Long offset, PIM_File_SeekMode mode);

/**
 *  @brief get the position of read/write File pointer
 *
 *  @param file [IN] : file stream pointer
 *
 *  @return current position of file stream.
 */
extern PIM_Long PIM_File_Tell(PIM_pFile file);

/**
 *  @brief remove file.
 *
 *  @param path [IN] : the file name with directory path.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_File_Remove(const PIM_Char *path);

/**
 *  @brief get the position of read/write File pointer
 *
 *  @param oldpath [IN] : the existing file name with directory path.
 *  @param newpath [IN] : the new file name with directory path.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_File_Rename(const PIM_Char *oldpath, const PIM_Char *newpath);

/**
 *  @brief Cut to the size of the file specified
 *
 *  @param path [IN] : the file name with directory path.
 *  @param length [IN] : the size of file
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_File_Truncate(const PIM_Char *path, PIM_Int32 length);


#ifdef __cplusplus
}
#endif

#endif /*__PIMFILE_H__*/
