/*********************************************************************************

* Licensed under the Apache License, Version 2.0 (the "License"); you may not use
* this file except in compliance with the License.  You may obtain a copy of the
* License at
* 
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software distributed
* under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations under the License.
**********************************************************************************
*/

#include <math.h>
#include "securec.h"
#include "securecutil.h"
#include "secureprintoutput.h"

#include <string.h>
#include <stdarg.h>

#ifdef ANDROID
#include <wchar.h>

int wctomb(char *s, wchar_t wc) { return wcrtomb(s,wc,NULL); }
int mbtowc(wchar_t *pwc, const char *s, size_t n) { return mbrtowc(pwc, s, n, NULL); }
#endif

/*verNumber<->verStr like:0X502<->SPC002;0X503<->SPC003...;0X510<->SPC010;0X511<->SPC011...*/
void getHwSecureCVersion(char* verStr, int bufSize, unsigned short* verNumber)
{
    if (verStr != NULL && bufSize > 0 )
    {
       (void)strcpy_s(verStr, (size_t)bufSize, "Secure C V100R001C01SPC003");
    }
    if (verNumber != NULL)
    {
        *verNumber = (5 << 8 | 3);/*high Num << 8 | num of spc Ver*/
    }
}

void util_memmove (void* dst, const void* src, size_t count)
{
    UINT8T* pDest = (UINT8T*)dst;
    UINT8T* pSrc = (UINT8T*)src;

    if (dst <= src || pDest >= (pSrc + count))    _CHECK_BUFFER_OVERLAP
    {
        /*
        * Non-Overlapping Buffers
        * copy from lower addresses to higher addresses
        */
        while (count--)
        {
            *pDest = *(UINT8T*)pSrc;
            ++pDest;
            ++pSrc;
        }
    }
    else
    {
        /*
        * Overlapping Buffers
        * copy from higher addresses to lower addresses
        */
        pDest = pDest + count - 1;
        pSrc = pSrc + count - 1;

        while (count--)
        {
            *pDest = *pSrc;

            --pDest;
            --pSrc;
        }
    }
}
/*put a char to output stream */
#define SECUREC_PUTC_NOLOCK(_c,_stream)    (--(_stream)->_cnt >= 0 ? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  EOF)

int putWcharStrEndingZero(SECUREC_XPRINTF_STREAM* str, int zerosNum)
{
    int succeed = 0, i = 0;

    for (; i < zerosNum && (SECUREC_PUTC_NOLOCK('\0', str) != EOF ); ++i)
    {
    }
    if (i == zerosNum)
    {
        succeed = 1;
    }
    return succeed;
}

int vsnprintf_helper (char* string, size_t count, const char* format, va_list arglist)
{
    SECUREC_XPRINTF_STREAM str;
    int retval;
    
    str._cnt = (int)count;
    str._ptr = string;

    retval = securec_output_s(&str, format, arglist );
    if ((retval >= 0) && (SECUREC_PUTC_NOLOCK('\0', &str) != EOF))
    {
        return (retval);
    }

    if (string != NULL)
    {
        string[count - 1] = 0;
    }

    if (str._cnt < 0)
    {
        /* the buffer was too small; we return -2 to indicate truncation */
        return -2;
    }
    
    return -1;
}

/*
remove it */
void write_char_a( char ch, SECUREC_XPRINTF_STREAM* f, int* pnumwritten)
{
    if (SECUREC_PUTC_NOLOCK(ch, f) == EOF)
    {
        *pnumwritten = -1;
    }
    else
    {
        ++(*pnumwritten);
    }
}


void write_multi_char_a(char ch, int num, SECUREC_XPRINTF_STREAM* f, int* pnumwritten)
{
    while (num-- > 0)
    {
        if (SECUREC_PUTC_NOLOCK(ch, f) == EOF)
        {
            *pnumwritten = -1;
            break;
        }
        else
        {
            ++(*pnumwritten);
        }
    }
}



void write_string_a (char* string, int len, SECUREC_XPRINTF_STREAM* f, int* pnumwritten)
{
    while (len-- > 0)
    {
        if (SECUREC_PUTC_NOLOCK(*string, f) == EOF)
        {
            *pnumwritten = -1;
            break;
        }
        else
        {
            ++(*pnumwritten);
            ++string;
        }
    }
}

/* Following function "U64Div32" realized the operation of division between an unsigned 64-bits 
 *     number and an unsigned 32-bits number.
 * these codes are contributed by Dopra team in syslib.
 */
#if defined(VXWORKS_VERSION)

#define SEC_MAX_SHIFT_NUM           32
#define SEC_MASK_BIT_ALL            0xFFFFFFFF
#define SEC_MASK_BIT_32             0x80000000
#define SEC_MASK_BIT_01             0x00000001
#define SEC_MASK_HI_NBITS(x)        (SEC_MASK_BIT_ALL << (SEC_MAX_SHIFT_NUM - (x)))

typedef enum tagbit64SecCompareResult
{
    SEC_BIT64_GREAT,
    SEC_BIT64_EQUAL,
    SEC_BIT64_LESS
} bit64SecCompareResult;

#define SEC_BIT64_SUB(argAHi, argALo, argBHi, argBLo) \
    do \
    { \
        if ((argALo) < (argBLo)) \
        { \
            (argAHi) -= ((argBHi) + 1); \
        } \
        else \
        { \
            (argAHi) -= (argBHi); \
        } \
        (argALo) -= (argBLo); \
    } while (0)
    
#define SEC_BIT64_COMPARE(argAHi, argALo, argBHi, argBLo, result) \
    do \
    { \
        if ((argAHi) > (argBHi)) \
        { \
            result = SEC_BIT64_GREAT; \
        } \
        else if (((argAHi) == (argBHi)) && ((argALo) > (argBLo))) \
        { \
            result = SEC_BIT64_GREAT; \
        } \
        else if (((argAHi) == (argBHi)) && ((argALo) == (argBLo))) \
        { \
            result = SEC_BIT64_EQUAL; \
        } \
        else \
        { \
            result = SEC_BIT64_LESS; \
        } \
    } while (0)

INT32T U64Div64(UINT32T uiDividendHigh,
                       UINT32T uiDividendLow,
                       UINT32T uiDivisorHigh,
                       UINT32T uiDivisorLow,
                       UINT32T *puiQuotientHigh,
                       UINT32T *puiQuotientLow,
                       UINT32T *puiRemainderHigh,
                       UINT32T *puiRemainderLow)
{
    INT8T   scShiftNumHi = 0, scShiftNumLo = 0;
    UINT32T uiTmpQuoHi, uiTmpQuoLo;
    UINT32T uiTmpDividendHi, uiTmpDividendLo;
    UINT32T uiTmpDivisorHi, uiTmpDivisorLo;
    bit64SecCompareResult etmpResult;

    if ((NULL == puiQuotientHigh) || (NULL == puiQuotientLow))
    {
        return -1;
    }

    if (0 == uiDivisorHigh)
    {
        if (0 == uiDivisorLow)
        {
            return -1;
        }
        else if (1 == uiDivisorLow)
        {
            *puiQuotientHigh = uiDividendHigh;
            *puiQuotientLow  = uiDividendLow;

            if (NULL != puiRemainderHigh
                && NULL != puiRemainderLow)
            {
                *puiRemainderHigh = 0;
                *puiRemainderLow  = 0;
            }

            return 0;
        }
    }

    uiTmpQuoHi = uiTmpQuoLo = 0;
    uiTmpDividendHi = uiDividendHigh;
    uiTmpDividendLo = uiDividendLow;

    /* if divisor is larger than dividend, quotient equals to zero,
     * remainder equals to dividends */
    SEC_BIT64_COMPARE(uiDividendHigh, uiDividendLow,
                  uiDivisorHigh, uiDivisorLow, etmpResult);
    
    if (SEC_BIT64_LESS == etmpResult)
    {
        goto returnHandle;
    }
    
    else if (SEC_BIT64_EQUAL == etmpResult)
    {
        *puiQuotientHigh = 0;
        *puiQuotientLow  = 1;

        if ((NULL != puiRemainderHigh) && (NULL != puiRemainderLow))
        {
            *puiRemainderHigh = 0;
            *puiRemainderLow  = 0;
        }

        return 0;
    }

    /* get shift number to implement divide arithmetic */
    if (uiDivisorHigh > 0)
    {
        for (scShiftNumHi = 0; scShiftNumHi < SEC_MAX_SHIFT_NUM; scShiftNumHi++)
        {
            if ( (uiDivisorHigh << scShiftNumHi) & SEC_MASK_BIT_32 )
            {
                break;
            }
        }
    }
    else
    {
        for (scShiftNumLo = 0; scShiftNumLo < SEC_MAX_SHIFT_NUM; scShiftNumLo++)
        {
            if ( (uiDivisorLow << scShiftNumLo) & SEC_MASK_BIT_32 )
            {
                break;
            }
        }
    }

    if (uiDivisorHigh > 0)
    {
        /* divisor's high 32 bits doesn't equal to zero */
        
        for (; scShiftNumHi >= 0; scShiftNumHi--)
        {

            if (0 == scShiftNumHi)
            {
                uiTmpDivisorHi = uiDivisorHigh;
            }
            else
            {
                uiTmpDivisorHi = (uiDivisorHigh << scShiftNumHi)
                            | (uiDivisorLow >> (SEC_MAX_SHIFT_NUM - scShiftNumHi));
            }

            uiTmpDivisorLo = uiDivisorLow << scShiftNumHi;

            SEC_BIT64_COMPARE(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, uiTmpDivisorLo, etmpResult);

            if (SEC_BIT64_LESS != etmpResult)
            {
                SEC_BIT64_SUB(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, uiTmpDivisorLo);

                uiTmpQuoLo |= (1 << scShiftNumHi);

                if ((0 == uiTmpDividendHi) && (0 == uiTmpDividendLo))
                {
                    goto returnHandle;
                }
            }
            if (0 == scShiftNumHi)
                break;
        }

    }
    else
    {
        /* divisor's high 32 bits equals to zero */
        
        scShiftNumHi = scShiftNumLo;
        
        for (; scShiftNumHi >= 0; scShiftNumHi--)
        {
            uiTmpDivisorHi = uiDivisorLow << scShiftNumHi;
            SEC_BIT64_COMPARE(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, 0, etmpResult);

            if (SEC_BIT64_LESS != etmpResult)
            {   
                UINT32T uiTmp = 0;
                SEC_BIT64_SUB(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, uiTmp);
                
                uiTmpQuoHi |= (1 << scShiftNumHi);

                if ((0 == uiTmpDividendHi) && (0 == uiTmpDividendLo))
                {
                    goto returnHandle;
                }
            }
            if (0 == scShiftNumHi)
                break;
        }

        for (scShiftNumHi = SEC_MAX_SHIFT_NUM - 1; scShiftNumHi >= 0; scShiftNumHi--)
        {
            if (0 == scShiftNumHi)
            {
                uiTmpDivisorHi = 0;
            }
            else
            {
                uiTmpDivisorHi = uiDivisorLow  >> (SEC_MAX_SHIFT_NUM - scShiftNumHi);
            }

            uiTmpDivisorLo = uiDivisorLow << scShiftNumHi;

            SEC_BIT64_COMPARE(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, uiTmpDivisorLo, etmpResult);

            if (SEC_BIT64_LESS != etmpResult)
            {
                SEC_BIT64_SUB(uiTmpDividendHi, uiTmpDividendLo,
                          uiTmpDivisorHi, uiTmpDivisorLo);

                uiTmpQuoLo |= (1 << scShiftNumHi);

                if ((0 == uiTmpDividendHi) && (0 == uiTmpDividendLo))
                {
                    goto returnHandle;
                }
            }
            if (0 == scShiftNumHi)
                break;
        }

    }

returnHandle:

    *puiQuotientHigh = uiTmpQuoHi;
    *puiQuotientLow  = uiTmpQuoLo;

    if ((NULL != puiRemainderHigh)
        && (NULL != puiRemainderLow))
    {
        *puiRemainderHigh = uiTmpDividendHi;
        *puiRemainderLow  = uiTmpDividendLo;
    }

    return 0;
}

INT32T U64Div32(UINT32T uiDividendHigh,
                       UINT32T uiDividendLow,
                       UINT32T uiDivisor,
                       UINT32T *puiQuotientHigh,
                       UINT32T *puiQuotientLow,
                       UINT32T *puiRemainder)
{
    UINT32T uiTmpRemainderHi = 0, uiTmpRemainderLo = 0;
    UINT32T uiRet = 0;

    if ((NULL == puiQuotientHigh) 
        || (NULL == puiQuotientLow)
        || 0 == uiDivisor  || NULL == puiRemainder)
    {
        return -1;
    }
    
    uiDividendHigh &= SEC_MASK_BIT_ALL;
    uiDividendLow  &= SEC_MASK_BIT_ALL;
    uiDivisor      &= SEC_MASK_BIT_ALL;
    *puiQuotientHigh = 0;
    *puiQuotientLow  = 0;
    *puiRemainder    = 0;

    uiRet = U64Div64( uiDividendHigh,
                         uiDividendLow,
                         0,
                         uiDivisor,
                         puiQuotientHigh,
                         puiQuotientLow,
                         &uiTmpRemainderHi,
                         &uiTmpRemainderLo );
    if (0 != uiRet)
    {
        return uiRet;
    }

    if (NULL != puiRemainder)
    {
        if(0 != uiTmpRemainderHi)
        {
            return -1;
        }
        *puiRemainder = uiTmpRemainderLo;
    }

    return 0;
}
#endif
