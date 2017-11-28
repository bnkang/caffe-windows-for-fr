/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMBASICAPI_H__
#define __PIMBASICAPI_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief get the length of string
 *
 *  @param str [IN] : a string
 *
 *  @return the length of string
 */
extern PIM_Int32 PIM_StrlenMp(const PIM_Char* str);

/**
 *  @brief copy a string
 *  @note this function copies the string pointed to by src, including the terminating null byte ('\0'), to the buffer pointed to by dst.
 *
 *  @param dst [IN] : the size of dst buffer equal to or greater than the src string
 *  @param src [IN] : a string
 *
 *  @return the pointer of the dst buffer
 */
extern PIM_Char* PIM_StrcpyMp(PIM_Char* dst, const PIM_Char* src);

/**
 *  @brief copy a string
 *  @note this function copies the string pointed to by src, including the terminating null byte ('\0'), to the buffer pointed to by dst.
 *
 *  @param dst [IN] : the size of dst buffer equal to or greater than the src string
 *  @param src [IN] : a string
 *  @param maxLength [IN] : max length of src
 *
 *  @return the pointer of the dst buffer
 */
extern PIM_Char* PIM_StrncpyMp(PIM_Char* dst, const PIM_Char* src, PIM_Int32 maxLength);

/**
 *  @brief compare two strings
 *
 *  @param cs [IN] : a string
 *  @param ct [IN] : a string
 *
 *  @return return an integer less than, equal to, or greater than zero if 'cs' (or the  first  n  bytes  thereof) is found, respectively, to be less than, to match, or be greater than 'ct'.
 */
extern PIM_Int32 PIM_StrcmpMp(const PIM_Char* cs, const PIM_Char* ct);


/**
 *  @brief convert a number to string in base 10.
 *
 *  @param out [OUT] : result data. (must be allocated.)
 *  @param num [IN] : a number
 *
 *  @return : number of characters returned.
 */
extern PIM_Int32 PIM_Itoa(PIM_Char* out, PIM_Int32 num);

/**
 *  @brief convert a string to an integer
 *
 *  @param nptr [IN] : a string with numbers
 *
 *  @return converting integer.
 */
extern PIM_Int32 PIM_Atoi(const PIM_Char* nptr);


/* ctype-compliant functions. */
#define PIM_Isdigit(c)    ((c) >= '0' && (c) <= '9')
#define PIM_Isspace(c)    (((c) >= 9 && (c) <= 13) || (c) == 32)
#define PIM_Isupper(c)    ((c) >= 'A' && (c) <= 'Z')
#define PIM_Islower(c)    ((c) >= 'a' && (c) <= 'z')
#define PIM_Isalpha(c)    (PIM_Isupper((c)) || PIM_Islower((c)))
#define PIM_Isalnum(c)    (PIM_Isalpha((c)) || PIM_Isdigit((c)))
#define PIM_Isblank(c)    ((c) == ' ' || (c) == '\t')
#define PIM_Isxdigit(c)   (PIM_Isdigit((c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define PIM_Isprint(c)    (((c) >= 32 && (c) <= 126) || ((c) >= 161 && (c) <= 254))
#define PIM_Iscntrl(c)    (!PIM_Isprint(c))
#define PIM_Ispunct(c)    (PIM_Isprint(c) && !PIM_Isalnum(c) && !PIM_Isspace(c))
#define PIM_Isgraph(c)    (PIM_Isprint(c) && (c) != ' ')

/**
 *  @brief convert letter to upper
 *
 *  @param c [IN] : a letter
 *
 *  @return : the converted letter
 */
extern PIM_Int32 PIM_Toupper(PIM_Int32 c);

/**
 *  @brief convert letter to lower
 *
 *  @param c [IN] : a letter
 *
 *  @return : the converted letter
 */
extern PIM_Int32 PIM_Tolower(PIM_Int32 c);


/********************************************************************************************
 * 
 *******************************************************************************************/
extern PIM_Int32 PIM_Snprintf(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 *args, PIM_Int32 num_args);
extern PIM_Int32 PIM_SnprintfInt1(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1);
extern PIM_Int32 PIM_SnprintfInt2(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2);
extern PIM_Int32 PIM_SnprintfInt3(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3);
extern PIM_Int32 PIM_SnprintfInt4(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4);
extern PIM_Int32 PIM_SnprintfInt5(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5);
extern PIM_Int32 PIM_SnprintfInt6(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5, PIM_Int32 arg6);

extern void PIM_Log_WriteStr(const PIM_Char* str);
extern void PIM_Log_WriteInt1(const PIM_Char* str, PIM_Int32 arg1);
extern void PIM_Log_WriteInt2(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2);
extern void PIM_Log_WriteInt3(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3);
extern void PIM_Log_WriteInt4(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4);
extern void PIM_Log_WriteInt5(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5);
extern void PIM_Log_WriteInt6(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5, PIM_Int32 arg6);

#define PIM_Snprintf1(str, size, format, a1)                        PIM_SnprintfInt1((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1))
#define PIM_Snprintf2(str, size, format, a1, a2)                    PIM_SnprintfInt2((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2))
#define PIM_Snprintf3(str, size, format, a1, a2, a3)                PIM_SnprintfInt3((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3))
#define PIM_Snprintf4(str, size, format, a1, a2, a3, a4)            PIM_SnprintfInt4((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4))
#define PIM_Snprintf5(str, size, format, a1, a2, a3, a4, a5)        PIM_SnprintfInt5((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4), (PIM_Int32)(a5))
#define PIM_Snprintf6(str, size, format, a1, a2, a3, a4, a5, a6)    PIM_SnprintfInt6((PIM_Char*)(str), (size), (const PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4), (PIM_Int32)(a5), (PIM_Int32)(a6))

#define PIM_Log_Write0(format)                          PIM_Log_WriteStr((PIM_Char*)(format))
#define PIM_Log_Write1(format, a1)                      PIM_Log_WriteInt1((PIM_Char*)(format), (PIM_Int32)(a1))
#define PIM_Log_Write2(format, a1, a2)                  PIM_Log_WriteInt2((PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2))
#define PIM_Log_Write3(format, a1, a2, a3)              PIM_Log_WriteInt3((PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3))
#define PIM_Log_Write4(format, a1, a2, a3, a4)          PIM_Log_WriteInt4((PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4))
#define PIM_Log_Write5(format, a1, a2, a3, a4, a5)      PIM_Log_WriteInt5((PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4), (PIM_Int32)(a5))
#define PIM_Log_Write6(format, a1, a2, a3, a4, a5, a6)  PIM_Log_WriteInt6((PIM_Char*)(format), (PIM_Int32)(a1), (PIM_Int32)(a2), (PIM_Int32)(a3), (PIM_Int32)(a4), (PIM_Int32)(a5), (PIM_Int32)(a6))

// Deprecated {
extern void PIM_Log_WriteNumArray(const PIM_Char* str, PIM_Int32 *val, PIM_Int32 count);
extern void PIM_Log_WriteNum1(const PIM_Char* str, PIM_Int32 val);
extern void PIM_Log_WriteNum2(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2);
extern void PIM_Log_WriteNum3(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2, PIM_Int32 val3);
extern void PIM_Log_WriteNum4(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2, PIM_Int32 val3, PIM_Int32 val4);
extern void PIM_Log_DumpHex(const PIM_Uint8 *data, PIM_Int32 size);
// }



#ifdef __cplusplus
}
#endif


#endif /*__PIMBASICAPI_H__*/

