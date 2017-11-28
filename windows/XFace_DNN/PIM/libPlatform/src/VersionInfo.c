#include "PIMTypes.h"
#include "exp/PIMVersion.h"
#include "libPlatform/inc/VersionInfo.h"

PIM_Version gReleaseVersion = {
	DEF_VER_RELEASE<<8 + DEF_VER_RELEASE_CANDIDATE,
	DEF_VER_BETA<<8 + DEF_VER_ALPHA,
	DEF_VER_CONFIG,
	DEF_VER_COPYRIGHT
};

PIM_Version PIM_Version_GetVersion(void)
{
	return gReleaseVersion;
}

PIM_Uint32 PIM_Version_GetVersionNumber(void)
{
	return (PIM_Uint32)(gReleaseVersion.major<<16 + gReleaseVersion.minor);
}
