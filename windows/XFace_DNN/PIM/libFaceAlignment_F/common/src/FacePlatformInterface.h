/******************************************************************************
* Copyright 2012 by embedded dept., ACROEM, Co., LTD.
*
* @file		FacePlatformInterface.h
* @brief	interface header file for platform-independent Face application
*
* @author	hj0914.jeon(hjjeon@acroem.com)
* @author	Karthikeyan (s.karthik@samsung.com)
* @date		2012. 02. 04
*
* @abbreviation	ALPI : Algorithm Lap Platform Interface
* 
* @history
*
*******************************************************************************/
#ifndef __FACE_PLATFORM_INTERFACE__
#define __FACE_PLATFORM_INTERFACE__
	
/******************************************************************************
* INCLUDE common & for each Platform
******************************************************************************/
#include "FD_header.h"

#if defined (ZORAN_FRM)
#include "Types.h"
#include "HceDirect.h"
#elif defined (DRIME3_FRM)
#include <tfs4_types.h>
#include <CBIMMPhotoDataTypes.h>
#include <fx_memory.h>
#include <ICpoIpc.h>
#include <FileManagement/CSFileAPI.h>
#elif defined (DRIME4_FRM)
#include <stdio.h>
#include <stdlib.h>
#elif defined (FUJITSU_FRM)
#elif defined(ANDROID_FRM)
#include <stdio.h>
#include <stdlib.h>
#if defined(USE_ASSETMGR)
/* for native asset manager */
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif
#else
#endif

/******************************************************************************
* DEFINE common & for each Platform
******************************************************************************/
#if defined (ZORAN_FRM)
#define MAX_FILENAME_LEN		128
#elif defined (DRIME3_FRM)
#define MAX_FILENAME_LEN		128
#elif defined (DRIME4_FRM)
#elif defined (FUJITSU_FRM)
#else
#endif


#ifdef ANDROID
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__)
#else
#define LOGV(...) printf( __VA_ARGS__)
#define LOGD(...) printf( __VA_ARGS__)
#define LOGI(...) printf( __VA_ARGS__)
#define LOGW(...) printf( __VA_ARGS__)
#define LOGE(...) printf( __VA_ARGS__)
#endif // #ifndef USE_SSE
	
#ifdef __cplusplus
	extern "C" {
#endif  /* __cplusplus */

/******************************************************************************
* EXTERNAL ENUM
******************************************************************************/
typedef enum _eFILE_OPEN
{
	eFILE_CREATE	=0,
	eFILE_READ_ONLY,
	eFILE_WRITE_ONLY,
	eFILE_OVER_WRITE,
	eFILE_READ_WRITE
}eFILE_OPEN;
	
typedef enum _eRESULT
{
	eRESULT_SUCCESS =0,
	eRESULT_FAIL = -1
}eRESULT;

#if defined (ZORAN_FRM)
enum ePTEntryType	//! Directory entry types
{
	TYPE_DIR  = 0, //!< Directory
	TYPE_FILE = 1  //!< File
}PTEntryType;

#elif defined (DRIME3_FRM)
#elif defined (DRIME4_FRM)
#elif defined (FUJITSU_FRM)
#else
#endif

/******************************************************************************
* EXTERNAL STRUCT
******************************************************************************/
#if defined (ZORAN_FRM)
	typedef struct sPTFile
	{
		char			name[MAX_FILENAME_LEN];
		UINT 			type;
		UINT64			size;
		SSTRING 		unicode;
	}PTFile;
#elif defined (DRIME3_FRM)
#elif defined (DRIME4_FRM)
#elif defined (FUJITSU_FRM)
	typedef ULONG FileIO;
	typedef ULONG* PFileIO;
#elif defined(ANDROID_FRM)
	#if defined(USE_ASSETMGR)
		typedef struct _tAssetInfo {
			AAssetManager* asset_mgr;
			AAsset* asset_file;
		}AssetInfo;
		typedef AssetInfo ResFileIO;
		typedef AssetInfo* PResFileIO;
	#endif
	typedef int FileIO;
	typedef int* PFileIO;
#else
	typedef int FileIO;
	typedef int* PFileIO;
#endif

/******************************************************************************
* EXTERNAL VALIABLE 
******************************************************************************/

/******************************************************************************
* EXTERNAL FUNCTION DECLARE
******************************************************************************/
unsigned int ALPI_MakeDir(char* pDirName);
int ALPI_ChangeDir(void* pTargetDir, void* pDirIO, unsigned int uiDirIOLen);
int ALPI_DestroyDirIO(void* pDirIO);
int ALPI_GetFileInfo(unsigned char is_first_file, void* pDirIO, void* pFileInfo);

int ALPI_FileOpen(PFileIO pFileIO, char* pFileName, eFILE_OPEN openType);
void ALPI_FileClose(PFileIO pFileIO);
int ALPI_FileRead(PFileIO pFileIO, void* dest, unsigned long* byteReadSize);
int ALPI_FileWrite(PFileIO pFileIO, void* src, unsigned long* byteWriteSize);

eRESULT ALPI_ResourceReadInit(PResFileIO *pResFileIO, void *data);
eRESULT ALPI_ResourceFileOpen(PResFileIO pResFileIO, const char* pFileName);
void ALPI_ResourceFileClose(PResFileIO pResFileIO);
eRESULT ALPI_ResourceFileRead(PResFileIO pResFileIO, void* dest, unsigned long* byteReadSize);
eRESULT ALPI_ResourceFileSeek(PResFileIO pResFileIO, int offset, int whence);
eRESULT ALPI_ResourceFileGetPos(PResFileIO pResFileIO, int *position);

int ALPI_Malloc(void* ptr, unsigned long size);
void ALPI_Free(void* ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __FACE_PLATFORM_INTERFACE__ */

