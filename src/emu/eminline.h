/***************************************************************************

    eminline.h

    Definitions for inline functions that can be overriden by OSD-
    specific code.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __EMINLINE__
#define __EMINLINE__

/* we come with implementations for GCC x86 and PPC */
#if defined(__GNUC__) && !defined(SDLMAME_NOASM)

#if defined(__i386__) || defined(__x86_64__)
#include "eigccx86.h"
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
#include "eigccppc.h"
#else
#include "osinline.h"
#endif

#else

#include "osinline.h"

#endif

#ifdef WEBOS
#include <stdint.h>
#endif

/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef mul_32x32
INLINE int64_t mul_32x32(int32_t a, int32_t b)
{
	return (int64_t)a * (int64_t)b;
}
#endif


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

#ifndef mulu_32x32
INLINE uint64_t mulu_32x32(uint32_t a, uint32_t b)
{
	return (uint64_t)a * (uint64_t)b;
}
#endif


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#ifndef mul_32x32_hi
INLINE int32_t mul_32x32_hi(int32_t a, int32_t b)
{
	return (uint32_t)(((int64_t)a * (int64_t)b) >> 32);
}
#endif


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

#ifndef mulu_32x32_hi
INLINE uint32_t mulu_32x32_hi(uint32_t a, uint32_t b)
{
	return (uint32_t)(((uint64_t)a * (uint64_t)b) >> 32);
}
#endif


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mul_32x32_shift
INLINE int32_t mul_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	return (int32_t)(((int64_t)a * (int64_t)b) >> shift);
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mulu_32x32_shift
INLINE uint32_t mulu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return (uint32_t)(((uint64_t)a * (uint64_t)b) >> shift);
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef div_64x32
INLINE int32_t div_64x32(int64_t a, int32_t b)
{
	return a / (int64_t)b;
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_64x32
INLINE uint32_t divu_64x32(uint64_t a, uint32_t b)
{
	return a / (uint64_t)b;
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#ifndef div_64x32_rem
INLINE int32_t div_64x32_rem(int64_t a, int32_t b, int32_t *remainder)
{
	*remainder = a % (int64_t)b;
	return a / (int64_t)b;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef divu_64x32_rem
INLINE uint32_t divu_64x32_rem(uint64_t a, uint32_t b, uint32_t *remainder)
{
	*remainder = a % (uint64_t)b;
	return a / (uint64_t)b;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef div_32x32_shift
INLINE int32_t div_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	return ((int64_t)a << shift) / (int64_t)b;
}
#endif


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_32x32_shift
INLINE uint32_t divu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return ((uint64_t)a << shift) / (uint64_t)b;
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef mod_64x32
INLINE int32_t mod_64x32(int64_t a, int32_t b)
{
	return a % (int64_t)b;
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef modu_64x32
INLINE uint32_t modu_64x32(uint64_t a, uint32_t b)
{
	return a % (uint64_t)b;
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifndef recip_approx
INLINE float recip_approx(float value)
{
	return 1.0f / value;
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
INLINE uint8_t count_leading_zeros(uint32_t val)
{
	uint8_t count;
	for (count = 0; (int32_t)val >= 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
INLINE uint8_t count_leading_ones(uint32_t val)
{
	uint8_t count;
	for (count = 0; (int32_t)val < 0; count++) val <<= 1;
	return count;
}
#endif



/***************************************************************************
    INLINE SYNCHRONIZATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    compare_exchange32 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifndef compare_exchange32
INLINE int32_t compare_exchange32(int32_t volatile *ptr, int32_t compare, int32_t exchange)
{
	int32_t oldval = *ptr;
	if (*ptr == compare)
		*ptr = exchange;
	return oldval;
}
#endif


/*-------------------------------------------------
    compare_exchange64 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifdef PTR64
#ifndef compare_exchange64
INLINE int64_t compare_exchange64(int64_t volatile *ptr, int64_t compare, int64_t exchange)
{
	int64_t oldval = *ptr;
	if (*ptr == compare)
		*ptr = exchange;
	return oldval;
}
#endif
#endif


/*-------------------------------------------------
    compare_exchange_ptr - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#ifndef compare_exchange_ptr
INLINE void *compare_exchange_ptr(void * volatile *ptr, void *compare, void *exchange)
{
#ifdef PTR64
	int64_t result;
	result = compare_exchange64((int64_t volatile *)ptr, (int64_t)compare, (int64_t)exchange);
#else
	int32_t result;
#ifdef WEBOS
    result = compare_exchange32(
        (int32_t volatile *)ptr,
        (int32_t)(uintptr_t)compare,
        (int32_t)(uintptr_t)exchange
    );
#else
    result = compare_exchange32((int32_t volatile *)ptr, (int32_t)compare, (int32_t)exchange);
#endif
#endif
	return (void *)result;
}
#endif


/*-------------------------------------------------
    atomic_exchange32 - atomically exchange the
    exchange value with the memory at 'ptr',
    returning the original value.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifndef atomic_exchange32
INLINE int32_t atomic_exchange32(int32_t volatile *ptr, int32_t exchange)
{
	int32_t oldval = *ptr;
	*ptr = exchange;
	return oldval;
}
#endif


/*-------------------------------------------------
    atomic_add32 - atomically add the delta value
    to the memory at 'ptr', returning the final
    result.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifndef atomic_add32
INLINE int32_t atomic_add32(int32_t volatile *ptr, int32_t delta)
{
	return (*ptr += delta);
}
#endif


/*-------------------------------------------------
    atomic_increment32 - atomically increment the
    32-bit value in memory at 'ptr', returning the
    final result.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifndef atomic_increment32
INLINE int32_t atomic_increment32(int32_t volatile *ptr)
{
	return atomic_add32(ptr, 1);
}
#endif


/*-------------------------------------------------
    atomic_decrement32 - atomically decrement the
    32-bit value in memory at 'ptr', returning the
    final result.

    Note that the default implementation does
    no synchronization. You MUST override this
    in osinline.h for it to be useful in a
    multithreaded environment!
-------------------------------------------------*/

#ifndef atomic_decrement32
INLINE int32_t atomic_decrement32(int32_t volatile *ptr)
{
	return atomic_add32(ptr, -1);
}
#endif



/***************************************************************************
    INLINE TIMING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_profile_ticks - return a tick counter
    from the processor that can be used for
    profiling. It does not need to run at any
    particular rate.
-------------------------------------------------*/

#ifndef get_profile_ticks
INLINE int64_t get_profile_ticks(void)
{
	return osd_ticks();
}
#endif

#endif /* __EMINLINE__ */
