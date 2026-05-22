/***************************************************************************

    osdcomm.h

    Common definitions shared by the OSD layer. This includes the most
    fundamental integral types as well as compiler-specific tweaks.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
//#include <stdlib.h>


/***************************************************************************
    POINTER-WIDTH DETECTION
***************************************************************************/

/* Auto-detect 64-bit pointer ABI from the compiler. PTR64 was historically
 * set via -D in the Makefile; sources should not have to depend on a build
 * flag to know how wide their own pointers are. If something upstream still
 * defines PTR64 manually we honour it; otherwise we figure it out here.
 *
 * Predicates, in order of preference:
 *   - __SIZEOF_POINTER__ : GCC >= 4.5, Clang >= 3.0, MSVC >= 2017
 *   - _WIN64             : MSVC fallback for older versions
 *   - __LP64__ / _LP64   : older Unix toolchains
 *
 * The choice is target-aware (it asks the compiler about its target ABI),
 * which is strictly better than uname -m on the build host. */
#ifndef PTR64
  #if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
    #define PTR64 1
  #elif defined(_WIN64) || defined(__LP64__) || defined(_LP64)
    #define PTR64 1
  #endif
#endif


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define ATTR_UNUSED				__attribute__((__unused__))
#define ATTR_NORETURN			__attribute__((noreturn))
#define ATTR_PRINTF(x,y)		__attribute__((format(printf, x, y)))
#define ATTR_MALLOC				__attribute__((malloc))
#define ATTR_PURE				__attribute__((pure))
#define ATTR_CONST				__attribute__((const))
#define ATTR_FORCE_INLINE		__attribute__((always_inline))
#define ATTR_NONNULL(...)		__attribute__((nonnull(__VA_ARGS__)))
#define UNEXPECTED(exp)			__builtin_expect(!!(exp), 0)
#define EXPECTED(exp)			__builtin_expect(!!(exp), 1)
#define RESTRICT				__restrict__
#define SETJMP_GNUC_PROTECT()	(void)__builtin_return_address(1)
#else
#define ATTR_UNUSED
#define ATTR_NORETURN
#define ATTR_PRINTF(x,y)
#define ATTR_MALLOC
#define ATTR_PURE
#define ATTR_CONST
#define ATTR_FORCE_INLINE
#define ATTR_NONNULL(...)
#define UNEXPECTED(exp)			(exp)
#define EXPECTED(exp)			(exp)
#define RESTRICT
#define SETJMP_GNUC_PROTECT()	do {} while (0)
#endif


/* And some MSVC optimizations/warnings */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define DECL_NORETURN			__declspec(noreturn)
#else
#define DECL_NORETURN
#endif



/***************************************************************************
    FUNDAMENTAL TYPES
***************************************************************************/

/* The fundamental integer types are <stdint.h>'s exact-width types
   (uint8_t/int8_t .. uint64_t/int64_t), included near the top of this
   header. The legacy UINT8../INT8.. aliases have been removed now that
   all code uses the stdint spellings directly. */



/***************************************************************************
    FUNDAMENTAL CONSTANTS
***************************************************************************/

/* Ensure that TRUE/FALSE are defined */
#ifndef TRUE
#define TRUE    			1
#endif

#ifndef FALSE
#define FALSE				0
#endif



/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)			((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)			((x) > (y) ? (x) : (y))
#endif


/* U64 and S64 are used to wrap long integer constants. */
#ifdef __GNUC__
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif


/* Concatenate/extract 32-bit halves of 64-bit values */
#define CONCAT_64(hi,lo)	(((uint64_t)(hi) << 32) | (uint32_t)(lo))
#define EXTRACT_64HI(val)	((uint32_t)((val) >> 32))
#define EXTRACT_64LO(val)	((uint32_t)(val))


/* MINGW has adopted the MSVC formatting for 64-bit ints as of gcc 4.4 */
#if (defined(__MINGW32__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4))) || defined(_MSVC_VER)
#define I64FMT   "I64"
#else
#define I64FMT   "ll"
#endif


/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)		(sizeof(x) / sizeof(x[0]))


/* Macros for normalizing data into big or little endian formats */
#define FLIPENDIAN_INT16(x)	(((((uint16_t) (x)) >> 8) | ((x) << 8)) & 0xffff)
#define FLIPENDIAN_INT32(x)	((((uint32_t) (x)) << 24) | (((uint32_t) (x)) >> 24) | \
	(( ((uint32_t) (x)) & 0x0000ff00) << 8) | (( ((uint32_t) (x)) & 0x00ff0000) >> 8))
#define FLIPENDIAN_INT64(x)	\
	(												\
		(((((uint64_t) (x)) >> 56) & ((uint64_t) 0xFF)) <<  0)	|	\
		(((((uint64_t) (x)) >> 48) & ((uint64_t) 0xFF)) <<  8)	|	\
		(((((uint64_t) (x)) >> 40) & ((uint64_t) 0xFF)) << 16)	|	\
		(((((uint64_t) (x)) >> 32) & ((uint64_t) 0xFF)) << 24)	|	\
		(((((uint64_t) (x)) >> 24) & ((uint64_t) 0xFF)) << 32)	|	\
		(((((uint64_t) (x)) >> 16) & ((uint64_t) 0xFF)) << 40)	|	\
		(((((uint64_t) (x)) >>  8) & ((uint64_t) 0xFF)) << 48)	|	\
		(((((uint64_t) (x)) >>  0) & ((uint64_t) 0xFF)) << 56)		\
	)

#ifdef MSB_FIRST
#define BIG_ENDIANIZE_INT16(x)		(x)
#define BIG_ENDIANIZE_INT32(x)		(x)
#define BIG_ENDIANIZE_INT64(x)		(x)
#define LITTLE_ENDIANIZE_INT16(x)	(FLIPENDIAN_INT16(x))
#define LITTLE_ENDIANIZE_INT32(x)	(FLIPENDIAN_INT32(x))
#define LITTLE_ENDIANIZE_INT64(x)	(FLIPENDIAN_INT64(x))
#else
#define BIG_ENDIANIZE_INT16(x)		(FLIPENDIAN_INT16(x))
#define BIG_ENDIANIZE_INT32(x)		(FLIPENDIAN_INT32(x))
#define BIG_ENDIANIZE_INT64(x)		(FLIPENDIAN_INT64(x))
#define LITTLE_ENDIANIZE_INT16(x)	(x)
#define LITTLE_ENDIANIZE_INT32(x)	(x)
#define LITTLE_ENDIANIZE_INT64(x)	(x)
#endif


#endif	/* __OSDCOMM_H__ */
