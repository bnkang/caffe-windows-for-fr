#include "PIMDeviceApi.h"
#include "libPlatform/inc/PIMFile.h"

PIM_pFile  PIM_File_Open(const PIM_Char *path, PIM_File_OpenMode mode)
{
	return PIM_Device_FileOpen(path, mode);
}



PIM_Void PIM_File_Close(PIM_pFile stream)
{
	PIM_Device_FileClose(stream);
}
 


PIM_Result PIM_File_isEnd(PIM_pFile stream)
{
	return PIM_Device_FileisEnd(stream);
}



PIM_Result PIM_File_isError(PIM_pFile stream)
{
	return PIM_Device_FileisError(stream);
}



PIM_Int32 PIM_File_Read(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count)
{
	return PIM_Device_FileRead(stream, buff, obj_size, obj_count);
}



PIM_Int32  PIM_File_Write(PIM_pFile stream, PIM_Uint8 *buff, PIM_Int32 obj_size, PIM_Int32 obj_count)
{
	return PIM_Device_FileWrite(stream, buff, obj_size, obj_count);
}



PIM_Int32  PIM_File_Seek(PIM_pFile stream, PIM_Long offset, PIM_File_SeekMode mode)
{
	return PIM_Device_FileSeek(stream, offset, mode);
}



PIM_Long   PIM_File_Tell(PIM_pFile stream)
{
	return PIM_Device_FileTell(stream);
}



PIM_Result PIM_File_Remove(const PIM_Char *path)
{
	return PIM_Device_FileRemove(path);
}



PIM_Result PIM_File_Rename(const PIM_Char *oldpath, const PIM_Char *newpath)
{
	return PIM_Device_FileRename(oldpath, newpath);
}



PIM_Result PIM_File_Truncate(const PIM_Char *path, PIM_Int32 length)
{
	return  PIM_Device_FileTruncate(path, length);
}

