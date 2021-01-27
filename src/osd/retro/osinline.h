/*
empty
*/
//============================================================
//
//  osinline.h - GNU C inline functions
//
//============================================================

//============================================================
//
//  osinline.h - GNU C inline functions
//
//============================================================

#ifndef __OSINLINE__
#define __OSINLINE__

//============================================================
//  INLINE FUNCTIONS
//============================================================

#if defined(__i386__) || defined(__x86_64__)
#define osd_yield_processor() __asm__ __volatile__ ( " rep ; nop ;" )
#if defined(__x86_64__)
#define _osd_exchange64(ptr, exchange) (register INT64 ret; __asm__ __volatile__ (" lock ; xchg %[exchange], %[ptr] ;": [ptr]      "+m" (*ptr), [ret]      "=r" (ret): [exchange] "1"  (exchange)); ret)
#define osd_exchange64 _osd_exchange64
#endif /* __x86_64__ */
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
#define osd_yield_pocessor() __asm__ __volatile__ ( " nop \n nop \n" )
#if defined(__ppc64__) || defined(__PPC64__)
#define _osd_exchange64(ptr, exchange) (register INT64 ret; __asm__ __volatile__ ("1: ldarx  %[ret], 0, %[ptr]      \n""   stdcx. %[exchange], 0, %[ptr] \n""   bne-   1b                     \n": [ret]      "=&r" (ret): [ptr]      "r"   (ptr), [exchange] "r"   (exchange): "cr0"); ret)
#define osd_exchange64 _osd_exchange64
#endif /* __ppc64__ || __PPC64__ */

#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#ifndef YieldProcessor
#define YieldProcessor() do {} while (0)
#define osd_yield_processor() YieldProcessor()
#endif

#endif

#include "eminline.h"

#endif /* __OSINLINE__ */
