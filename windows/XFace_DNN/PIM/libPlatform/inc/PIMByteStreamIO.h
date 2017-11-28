/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMBYTESTREAMIO_H__
#define __PIMBYTESTREAMIO_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  @struct PIM_ByteStream PIMByteStreamIO.h "PIMByteStreamIO.h"
 *  @brief PIM_ByteStream structure.
 *  @note This module is to support platform-independent stream read/write.
 *        Data is always read/written in big-endian format regardless endianness of platform.
 */
typedef struct PIM_ByteStream 
{
	PIM_Uint8 *data;
	PIM_Int32 pos;
	PIM_Int32 maxSize;
} PIM_ByteStream;

/**
 *  @brief 
 *
 *  @param data [IN] : 
 *  @param max_size [IN] : 
 *
 *  @return 
 */
extern PIM_ByteStream *PIM_ByteStream_Create(PIM_Uint8 *data, PIM_Size_t max_size);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 */
extern PIM_Void PIM_ByteStream_Destroy(PIM_ByteStream *stream);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param newpos [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_SetPos(PIM_ByteStream *stream, PIM_Int32 newpos);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_ReadUint8(PIM_ByteStream *stream);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_ReadInt32(PIM_ByteStream *stream);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *
 *  @return 
 */
extern PIM_Uint32 PIM_ByteStream_ReadUint32(PIM_ByteStream *stream);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param point [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_ReadPoint(PIM_ByteStream *stream, PIM_Point *point);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_ReadBytes(PIM_ByteStream *stream, PIM_Uint8 *buf, PIM_Int32 size);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param val [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_WriteUint8(PIM_ByteStream *stream, PIM_Uint8 val);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param val [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_WriteInt32(PIM_ByteStream *stream, PIM_Int32 val);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param val [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_WriteUint32(PIM_ByteStream *stream, PIM_Uint32 val);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param point [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_WritePoint(PIM_ByteStream *stream, PIM_Point *point);

/**
 *  @brief 
 *
 *  @param stream [IN] : 
 *  @param buf [IN] : 
 *  @param size [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_ByteStream_WriteBytes(PIM_ByteStream *stream, const PIM_Uint8 *buf, PIM_Int32 size);

#ifdef __cplusplus
}
#endif

#endif /*__PIMBYTESTREAMIO_H__*/
