/**
 *	@file PIMVersion.h
 *
 *  @brief Declares PIM Solution Version interface.
 *
 *
 *  Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */

#ifndef __PIMVERSION_H__
#define __PIMVERSION_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Get solution release version information.
 *
 *  @return PIM_Version release version 
 *
 *  @see PIM_Version in PIMType.h
 */
extern PIM_Version PIM_Version_GetVersion(void);

/**
 *  @brief Get solution release version number.
 *
 *  @return PIM_Uint32 version number
 */
extern PIM_Uint32 PIM_Version_GetVersionNumber(void);


#ifdef __cplusplus
}
#endif

#endif /*__PIMVERSION_H__*/