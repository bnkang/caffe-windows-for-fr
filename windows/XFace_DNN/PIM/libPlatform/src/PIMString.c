
#include "PIMTypes.h"
#include "PIMDeviceApi.h"
#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMString.h"
#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"

#define PIM_KMaxLogLineLength 255


PIM_Int32 PIM_StrlenMp(const PIM_Char* str)
{
	PIM_Int32 i;
	for (i=0; str[i]; i++) {}
	
	return i;
}

PIM_Char* PIM_StrcpyMp(PIM_Char* dst, const PIM_Char* src)
{
	PIM_Int32 i;
	
	for (i=0; src[i]; i++) dst[i] = src[i];
	dst[i] = 0;
	
	return dst;	  
}

PIM_Char* PIM_StrncpyMp(PIM_Char* dst, const PIM_Char* src, PIM_Int32 maxLength)
{
	PIM_Int32 i;
	
	for (i=0; i<maxLength; i++)
	{
		if(src[i] == '\0')
		{
			dst[i] = '\0';
			break;
		}
		dst[i] = src[i];
	}
	
	return dst;	  
}

PIM_Int32 PIM_StrcmpMp(const PIM_Char* cs, const PIM_Char* ct)
{
	PIM_Int32 i;
	
	for (i=0; cs[i] || ct[i]; i++) {
		if (cs[i] < ct[i]) return -1;
		else if (cs[i] > ct[i]) return 1;
	}
	
	return 0;		
}



PIM_Int32 PIM_Itoa(PIM_Char* out, PIM_Int32 num)
{
	PIM_Int32 i, j = 0;
	PIM_Char buf[12];
	
	if (num == 0) {
		out[0] = '0';
		out[1] = 0;
		return 1;
	}
	 
	if (num < 0) {
		num = -num;
		out[j++] = '-';
	}
	
	for (i=0; num>0; i++) {
		buf[i] = (num % 10) + '0';
		num /= 10;
	}
	
	for (--i; i>=0; j++, i--) out[j] = buf[i];
	
	out[j] = 0;
	return j;
}

PIM_Int32 PIM_Atoi(const PIM_Char* nptr)
{
	PIM_Int32 c;
	PIM_Int32 total;
	PIM_Int32 sign;
	const PIM_Char *p = nptr;

 	while (PIM_Isspace((PIM_Int32) *p)) ++p;

	c = (PIM_Int32) *p++;
	sign = c;
	if (c == '-' || c == '+') c = (PIM_Int32)*p++;

	total = 0;
	while (PIM_Isdigit(c)) {
		total = 10 * total + (c - '0');
		c = (PIM_Int32)*p++;
	}

	if (sign == '-') return -total;
	else return total;	
}




PIM_Int32 PIM_Toupper(PIM_Int32 c)
{
	if (PIM_Islower(c)) c -= 'a' - 'A';
	return c;	
}

PIM_Int32 PIM_Tolower(PIM_Int32 c)
{
	if (PIM_Isupper(c)) c += 'a' - 'A';
	return c;
}





void PIM_Log_WriteStr(const PIM_Char* str)
{
	PIM_Device_WriteLog(PIM_LOG_FUNC, NULL, NULL, NULL, 0, str);
}


void PIM_Log_WriteNumArray(const PIM_Char* str, PIM_Int32 *val, PIM_Int32 count)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_Int32 pos;
	PIM_Int32 i;
	
	PIM_StrcpyMp((PIM_Char*)buf, str);
	pos = PIM_StrlenMp((PIM_Char*)buf);
	
	for (i=0; i<count; i++) {
		if (pos >= PIM_KMaxLogLineLength) {
			buf[PIM_KMaxLogLineLength] = 0;
			break;
		}
		if (i > 0) buf[pos++] = ',';
		pos += PIM_Itoa(&buf[pos], val[i]);
	}
	
	PIM_Log_WriteStr(buf);				
}


void PIM_Log_WriteInt1(const PIM_Char* str, PIM_Int32 arg1)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt1(buf, PIM_KMaxLogLineLength, str, arg1);
	PIM_Log_WriteStr(buf);
}

void PIM_Log_WriteInt2(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt2(buf, PIM_KMaxLogLineLength, str, arg1, arg2);
	PIM_Log_WriteStr(buf);
}

void PIM_Log_WriteInt3(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt3(buf, PIM_KMaxLogLineLength, str, arg1, arg2, arg3);
	PIM_Log_WriteStr(buf);
}

void PIM_Log_WriteInt4(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt4(buf, PIM_KMaxLogLineLength, str, arg1, arg2, arg3, arg4);
	PIM_Log_WriteStr(buf);
}

void PIM_Log_WriteInt5(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt5(buf, PIM_KMaxLogLineLength, str, arg1, arg2, arg3, arg4, arg5);
	PIM_Log_WriteStr(buf);
}

void PIM_Log_WriteInt6(const PIM_Char* str, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5, PIM_Int32 arg6)
{
	PIM_Char buf[PIM_KMaxLogLineLength + 1];
	PIM_SnprintfInt6(buf, PIM_KMaxLogLineLength, str, arg1, arg2, arg3, arg4, arg5, arg6);
	PIM_Log_WriteStr(buf);
}





// These stuffs are for implementing Sprintf series.
// Original code was got from sqlite source tree. 
////////////////////////////////////////////////////////////////////////////////////////////////

// Format types
#define etRADIX       1 /* Integer types.  %d, %x, %o, and so forth */
#define etSIZE        5 /* Return number of characters processed so far. %n */
#define etSTRING      6 /* Strings. %s */
#define etPERCENT     8 /* Percent symbol. %% */
#define etCHARX       9 /* Characters. %c */
#define etPOINTER    15 /* The %p conversion */

typedef struct et_info {   /* Information about each format field */
  PIM_Char fmttype;            /* The format field code letter */
  PIM_Uint8 base;             /* The base for radix conversion */
  PIM_Uint8 flags;            /* One or more of FLAG_ constants below */
  PIM_Uint8 type;             /* Conversion paradigm */
  PIM_Uint8 charset;          /* Offset into aDigits[] of the digits string */
  PIM_Uint8 prefix;           /* Offset into aPrefix[] of the prefix string */
} et_info;

#define FLAG_SIGNED  1     /* True if the value to convert is signed */
#define etBUFSIZE	32

static const et_info fmtinfo[] = {
  {  'd', 10, 1, etRADIX,      0,  0 },
  {  's',  0, 0, etSTRING,     0,  0 },
  {  'c',  0, 0, etCHARX,      0,  0 },
  {  'o',  8, 0, etRADIX,      0,  2 },
  {  'u', 10, 0, etRADIX,      0,  0 },
  {  'x', 16, 0, etRADIX,      16, 1 },
  {  'X', 16, 0, etRADIX,      0,  4 },
  {  'i', 10, 1, etRADIX,      0,  0 },
  {  'n',  0, 0, etSIZE,       0,  0 },
  {  '%',  0, 0, etPERCENT,    0,  0 },
  {  'p', 16, 0, etPOINTER,    0,  1 },
};
#define etNINFO  (sizeof(fmtinfo)/sizeof(fmtinfo[0]))


static void Snprintf_WriteString(PIM_Char* outbuf, PIM_Int32 maxbuf, PIM_Int32 *n_char, const PIM_Char* newstr, PIM_Int32 n_new)
{
	if (*n_char + n_new + 1 > maxbuf) n_new =  maxbuf - *n_char - 1;
	PIM_Memcpy(&outbuf[*n_char], (PIM_pVoid)newstr, n_new);
	*n_char += n_new;
	outbuf[*n_char] = 0;
}

static void Snprintf_WriteSpace(PIM_Char* outbuf, PIM_Int32 maxbuf, PIM_Int32 *n_char, PIM_Int32 n_spaces)
{
	if (*n_char + n_spaces + 1 > maxbuf) n_spaces =  maxbuf - *n_char - 1;
	for (; n_spaces>0; n_spaces--) outbuf[(*n_char)++] = ' ';
	outbuf[*n_char] = 0;
}

PIM_Int32 PIM_Snprintf(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 *args, PIM_Int32 num_args)
{
	static const char NULL_STRING[1] = { 0 };
	static const char aDigits[] = "0123456789ABCDEF0123456789abcdef";
	static const char aPrefix[] = "-x0\000X0";

	PIM_Int32 c;                     /* Next character in the format string */
	PIM_Char *bufpt;               /* Pointer to the conversion buffer */
	PIM_Int32 precision;             /* Precision of the current field */
	PIM_Int32 length;                /* Length of the field */
	PIM_Int32 idx;                   /* A general purpose loop counter */
	PIM_Int32 width;                 /* Width of the current field */
	PIM_Uint8 flag_leftjustify;   /* True if "-" flag is present */
	PIM_Uint8 flag_plussign;      /* True if "+" flag is present */
	PIM_Uint8 flag_blanksign;     /* True if " " flag is present */
	PIM_Uint8 flag_alternateform; /* True if "#" flag is present */
	PIM_Uint8 flag_zeropad;       /* True if field width constant starts with zero */
	PIM_Uint8 done;               /* Loop termination flag */
	PIM_Uint32 longvalue;   /* Value for integer types */
	const et_info *infop;      /* Pointer to the appropriate info structure */
	PIM_Char buf[etBUFSIZE];       /* Conversion buffer */
	char prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
	PIM_Int32 current_arg;
	PIM_Int32 current_chars;

	str[0] = 0;
	current_arg = current_chars = length = 0;
	bufpt = NULL;

	for (; (c=(*format))!=0; ++format) {
		if ( c!='%' ){
			PIM_Int32 amt;
			//bufpt = format;
			amt = 1;
			while ( (c=(*++format))!='%' && c!=0 ) amt++;
			Snprintf_WriteString(str, size, &current_chars, format, amt);
			if ( c==0 ) break;
		}

		if( (c=(*++format))==0 ){		// '%' appeared at the end of the format string
			Snprintf_WriteString(str, size, &current_chars, (PIM_Char*)"%", 1);
			break;
		}

		// Find out what flags are present */
		flag_leftjustify = flag_plussign = flag_blanksign = flag_alternateform = flag_zeropad = 0;
		done = 0;
		do {
			switch( c ){
				case '-':   flag_leftjustify = 1;     break;
				case '+':   flag_plussign = 1;        break;
				case ' ':   flag_blanksign = 1;       break;
				case '#':   flag_alternateform = 1;   break;
				case '0':   flag_zeropad = 1;         break;
				default:    done = 1;                 break;
			}
		} while( !done && (c=(*++format))!=0 );

		// Get the field width 
		width = 0;
		if ( c=='*' ){
			width = args[current_arg++];
			if (width < 0){
				flag_leftjustify = 1;
				width = -width;
			}
			c = *++format;
		} else {
			while( c>='0' && c<='9' ){
				width = width*10 + c - '0';
				c = *++format;
			}
		}
		if ( width > etBUFSIZE-10 ) width = etBUFSIZE-10;

		// Get the precision
		if ( c=='.' ) {
			c = *++format;
			if ( c=='*' ) {
				precision = args[current_arg++];
				if( precision<0 ) precision = -precision;
				c = *++format;
			} else {
				precision = 0;
				while ( c>='0' && c<='9' ){
					precision = precision*10 + c - '0';
					c = *++format;
				}
			}
		} else precision = -1;

		// Ignore 'l' or 'll'
		if (c == 'l') {
			c = *++format;
			if (c == 'l') c = *++format;
		}
		//  Fetch the info entry for the field 
		infop = 0;			
		for (idx=0; idx<etNINFO; idx++){
			if (c==fmtinfo[idx].fmttype ){
				infop = &fmtinfo[idx];
				break;
			}
		}
		if (infop == 0) infop = &fmtinfo[0];	// For unknown format type, treat as 'd'

		/*
		** At this point, variables are initialized as follows:
		**
		**   flag_alternateform          TRUE if a '#' is present.
		**   flag_plussign               TRUE if a '+' is present.
		**   flag_leftjustify            TRUE if a '-' is present or if the
		**                               field width was negative.
		**   flag_zeropad                TRUE if the width began with 0.
		**   flag_blanksign              TRUE if a ' ' is present.
		**   width                       The specified field width.  This is
		**                               always non-negative.  Zero is the default.
		**   precision                   The specified precision.  The default
		**                               is -1.
		**   infop                       Pointer to the appropriate info struct.
		*/
		switch (infop->type){
			case etPOINTER:
			case etRADIX:
				if( infop->flags & FLAG_SIGNED ){
					PIM_Int32 v = args[current_arg++];
					if( v<0 ){
					longvalue = -v;
					prefix = '-';
					}else{
					longvalue = v;
					if( flag_plussign )        prefix = '+';
					else if( flag_blanksign )  prefix = ' ';
					else                       prefix = 0;
					}
				} else {
					longvalue = (PIM_Uint32)args[current_arg++];
					prefix = 0;
				}
				if( longvalue==0 ) flag_alternateform = 0;
				if( flag_zeropad && precision<width-(prefix!=0) ){
					precision = width-(prefix!=0);
				}
				bufpt = &buf[etBUFSIZE-1];
				{
					register const char *cset;      /* Use registers for speed */
					register PIM_Int32 base;
					cset = &aDigits[infop->charset];
					base = infop->base;
					do{                                           /* Convert to ascii */
					*(--bufpt) = cset[longvalue%base];
					longvalue = longvalue/base;
					}while( longvalue>0 );
				}
				length = &buf[etBUFSIZE-1]-bufpt;
				for(idx=precision-length; idx>0; idx--){
					*(--bufpt) = '0';                             /* Zero pad */
				}
				if( prefix ) *(--bufpt) = prefix;               /* Add sign */
				if( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
					const char *pre;
					char x;
					pre = &aPrefix[infop->prefix];
					if( *bufpt!=pre[0] ){
						for(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
					}
				}
				length = &buf[etBUFSIZE-1]-bufpt;
				break;

			case etSIZE:
				args[current_arg++] = current_chars;
				length = width = 0;
				break;
			case etPERCENT:
				buf[0] = '%';
				bufpt = buf;
				length = 1;
				break;
			case etCHARX:
				c = buf[0] = args[current_arg++];
				if( precision>=0 ){
					for(idx=1; idx<precision; idx++) buf[idx] = c;
					length = precision;
				}else{
					length =1;
				}
				bufpt = buf;
				break;
			case etSTRING:
				bufpt = (PIM_Char*)args[current_arg++];
				if ( bufpt==0 ) bufpt = (PIM_Char*)NULL_STRING;
				length = PIM_StrlenMp(bufpt);
				if( precision>=0 && precision<length ) length = precision;
				break;
		}/* End switch over the format type */
  
		if( !flag_leftjustify ){
			register PIM_Int32 nspace;
			nspace = width-length;
			if( nspace>0 ){
				Snprintf_WriteSpace(str, size, &current_chars, nspace);
			}
		}

		if( length>0 ){
			Snprintf_WriteString(str, size, &current_chars, bufpt, length);
		}

		if( flag_leftjustify ){
			register PIM_Int32 nspace;
			nspace = width-length;
			if( nspace>0 ){
				Snprintf_WriteSpace(str, size, &current_chars, nspace);
			}
		}
	}/* End for loop over the format string */

	return current_chars;
}

PIM_Int32 PIM_SnprintfInt1(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1)
{
	return PIM_Snprintf(str, size, format, &arg1, 1);
}

PIM_Int32 PIM_SnprintfInt2(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2)
{
	PIM_Int32 args[2];
	args[0] = arg1;		args[1] = arg2;
	return PIM_Snprintf(str, size, format, args, 2);
}

PIM_Int32 PIM_SnprintfInt3(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3)
{
	PIM_Int32 args[3];
	args[0] = arg1;		args[1] = arg2;		args[2] = arg3;
	return PIM_Snprintf(str, size, format, args, 3);
}

PIM_Int32 PIM_SnprintfInt4(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4)
{
	PIM_Int32 args[4];
	args[0] = arg1;		args[1] = arg2;		args[2] = arg3;		args[3] = arg4;
	return PIM_Snprintf(str, size, format, args, 4);
}

PIM_Int32 PIM_SnprintfInt5(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5)
{
	PIM_Int32 args[5];
	args[0] = arg1;		args[1] = arg2;		args[2] = arg3;		args[3] = arg4;		args[4] = arg5;
	return PIM_Snprintf(str, size, format, args, 5);
}

PIM_Int32 PIM_SnprintfInt6(PIM_Char* str, PIM_Size_t size, const PIM_Char* format, PIM_Int32 arg1, PIM_Int32 arg2, PIM_Int32 arg3, PIM_Int32 arg4, PIM_Int32 arg5, PIM_Int32 arg6)
{
	PIM_Int32 args[6];
	args[0] = arg1;		args[1] = arg2;		args[2] = arg3;		args[3] = arg4;		args[4] = arg5;		args[5] = arg6;
	return PIM_Snprintf(str, size, format, args, 6);
}



#if 0
// Deprecated
void PIM_Log_WriteNum1(const PIM_Char* str, PIM_Int32 val)
{
	PIM_Log_WriteNumArray(str, &val, 1);
}

// Deprecated
void PIM_Log_WriteNum2(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2)
{
	PIM_Int32 val[2];
	val[0] = val1; val[1] = val2;
	PIM_Log_WriteNumArray(str, val, 2);	
}

// Deprecated
void PIM_Log_WriteNum3(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2, PIM_Int32 val3)
{
	PIM_Int32 val[3];
	val[0] = val1; val[1] = val2; val[2] = val3;
	PIM_Log_WriteNumArray(str, val, 3);	
}

// Deprecated
void PIM_Log_WriteNum4(const PIM_Char* str, PIM_Int32 val1, PIM_Int32 val2, PIM_Int32 val3, PIM_Int32 val4)
{
	PIM_Int32 val[4];
	val[0] = val1; val[1] = val2; val[2] = val3; val[3] = val4;
	PIM_Log_WriteNumArray(str, val, 4);	
}


void PIM_Log_DumpHex(const PIM_Uint8 *data, PIM_Int32 size)
{
#define one_line_width 40
	static const char dec2hex[] = "0123456789ABCDEF";
	PIM_Int32 i, lcnt;
	PIM_Char buf[one_line_width * 2 + 1];
	
	for (i=0, lcnt=0; i<size; i++) {
		buf[lcnt++] = dec2hex[(data[i] >> 4) & 0x0F];
		buf[lcnt++] = dec2hex[data[i] & 0x0F];
		
		if (lcnt >= (one_line_width * 2)) {
			buf[lcnt] = 0;
			PIM_Log_WriteStr(buf);
			lcnt = 0;
		}	
	}

	if (lcnt > 0) {
		buf[lcnt] = 0;
		PIM_Log_WriteStr(buf);
	}
	
#undef one_line_width
}

PIM_Int32 PIM_Strcm(PIM_Char* str1, PIM_Char* str2)
{
	PIM_Int32 d;
	
	while(*str1 && *str2) {
		if(d =*str1++ - *str2++) return d;
	}
	return *str1-*str2;
}
#endif
