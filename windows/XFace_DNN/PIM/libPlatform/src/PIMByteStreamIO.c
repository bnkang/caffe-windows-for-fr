
#include "PIMTypes.h"
#include "libPlatform/inc/PIMByteStreamIO.h"
#include "libPlatform/inc/PIMMemory.h"

#define LIMIT_OK(stream, bytes)	 (((stream)->pos + (bytes)) < (stream)->maxSize)


PIM_ByteStream *PIM_ByteStream_Create(PIM_Uint8 *data, PIM_Size_t max_size)
{
	PIM_ByteStream *stream = (PIM_ByteStream *)PIM_Malloc(sizeof(PIM_ByteStream));

	if (stream) {
		stream->data = data;
		stream->pos = 0;
		stream->maxSize = max_size;
	}
	return stream;
}

void PIM_ByteStream_Destroy(PIM_ByteStream *stream)
{
	if (stream) PIM_Free(stream);
}


#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Int32 PIM_ByteStream_SetPos(PIM_ByteStream *stream, PIM_Int32 newpos)
{
	if (newpos < stream->maxSize) stream->pos = newpos;
	return stream->maxSize;
}



PIM_Int32 PIM_ByteStream_ReadBytes(PIM_ByteStream *stream, PIM_Uint8 *buf, PIM_Int32 size)
{
	if (!LIMIT_OK(stream, size)) return -1;
	
	PIM_Memcpy(buf, &stream->data[stream->pos], size);
	stream->pos += size;

	return size;
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


PIM_Int32 PIM_ByteStream_ReadUint8(PIM_ByteStream *stream)
{
	if (!LIMIT_OK(stream, 1)) return -1;
	return (PIM_Int32)stream->data[stream->pos++] & 0xFF;
}

PIM_Int32 PIM_ByteStream_ReadInt32(PIM_ByteStream *stream)
{
	PIM_Int32 val;
	if (!LIMIT_OK(stream, 4)) return -1;
	val = (PIM_Int32)stream->data[stream->pos++] & 0xFF;
	val = (val << 8) | ((PIM_Int32)stream->data[stream->pos++] & 0xFF);
	val = (val << 8) | ((PIM_Int32)stream->data[stream->pos++] & 0xFF);
	val = (val << 8) | ((PIM_Int32)stream->data[stream->pos++] & 0xFF);

	return val;
}


#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Uint32 PIM_ByteStream_ReadUint32(PIM_ByteStream *stream)
{
	PIM_Uint32 val;
	if (!LIMIT_OK(stream, 4)) return (PIM_Uint32)-1;
	val = (PIM_Uint32)stream->data[stream->pos++] & 0xFF;
	val = (val << 8) | ((PIM_Uint32)stream->data[stream->pos++] & 0xFF);
	val = (val << 8) | ((PIM_Uint32)stream->data[stream->pos++] & 0xFF);
	val = (val << 8) | ((PIM_Uint32)stream->data[stream->pos++] & 0xFF);

	return val;
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


PIM_Int32 PIM_ByteStream_ReadPoint(PIM_ByteStream *stream, PIM_Point *point)
{
	if (!LIMIT_OK(stream, 8)) return -1;
	point->x = PIM_ByteStream_ReadInt32(stream);
	point->y = PIM_ByteStream_ReadInt32(stream);
	return 0;
}


#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Int32 PIM_ByteStream_WriteBytes(PIM_ByteStream *stream, const PIM_Uint8 *buf, PIM_Int32 size)
{
	if (!LIMIT_OK(stream, size)) return -1;
	
	PIM_Memcpy(&stream->data[stream->pos], buf, size);
	stream->pos += size;

	return size;
}



PIM_Int32 PIM_ByteStream_WriteUint8(PIM_ByteStream *stream, PIM_Uint8 val)
{
	if (!LIMIT_OK(stream, 1)) return -1;
	stream->data[stream->pos++] = val;
	return 1;
}



PIM_Int32 PIM_ByteStream_WriteInt32(PIM_ByteStream *stream, PIM_Int32 val)
{
	if (!LIMIT_OK(stream, 4)) return -1;
	stream->data[stream->pos++] = (PIM_Uint8)(val >> 24);
	stream->data[stream->pos++] = (PIM_Uint8)(val >> 16);
	stream->data[stream->pos++] = (PIM_Uint8)(val >> 8);
	stream->data[stream->pos++] = (PIM_Uint8)(val);
	return 4;
}

PIM_Int32 PIM_ByteStream_WriteUint32(PIM_ByteStream *stream, PIM_Uint32 val)
{
	return PIM_ByteStream_WriteInt32(stream, val);
}

PIM_Int32 PIM_ByteStream_WritePoint(PIM_ByteStream *stream, PIM_Point *point)
{
	PIM_Int32 ret;
	if (!LIMIT_OK(stream, 8)) return -1;
	ret = PIM_ByteStream_WriteInt32(stream, point->x);
	ret += PIM_ByteStream_WriteInt32(stream, point->y);
	return ret;
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


