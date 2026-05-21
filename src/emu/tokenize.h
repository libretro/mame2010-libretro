/***************************************************************************

    tokenize.h

    Common definitions and macros for tokenizing definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* tokens per item */
#define TOKENS_PER_PTR			(1)
#define TOKENS_PER_UINT32		(1)
#define TOKENS_PER_UINT64		(8 / sizeof(FPTR))
#define TOKENS_PER_ATTOTIME		(TOKENS_PER_UINT32 + TOKENS_PER_UINT64)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* include this at the top of your union to get the standard fields */
#define TOKEN_COMMON_FIELDS		\
	FPTR			i;			\
	const char *	stringptr;	\
	const void *	voidptr;	\
	const uint8_t *	ui8ptr;		\
	const int8_t *	i8ptr;		\
	const uint16_t *	ui16ptr;	\
	const int16_t *	i16ptr;		\
	const uint32_t *	ui32ptr;	\
	const int32_t *	i32ptr;		\
	const uint64_t *	ui64ptr;	\
	const int64_t *	i64ptr;		\


/* generic_token can be used when there are no particularly special types */
typedef struct _generic_token generic_token;
struct _generic_token
{
	TOKEN_COMMON_FIELDS
};



/***************************************************************************
    MACROS
***************************************************************************/

/* ----- compile-time token generation macros ----- */

/* GCC and C99 compilers can use designated initializers for type safety */
#if (defined(__GNUC__) && (__GNUC__ >= 3) && !defined(__cplusplus)) || (defined(_STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#define TOKEN_VALUE(field,a)	{ .field = (a) }
#else
#define TOKEN_VALUE(field,a)	{ (FPTR)(a) }
#endif

/* token output primitives */
/* note that regardless of the endianness, UINT64s are packed LSW first */
#define TOKEN_PTR(field,p)		TOKEN_VALUE(field, p)
#define TOKEN_STRING(p)			TOKEN_VALUE(stringptr, p)
#define TOKEN_UINT32(a)			TOKEN_VALUE(i, a)
#ifdef PTR64
#define TOKEN_UINT64(a)			TOKEN_VALUE(i, a)
#else
#define TOKEN_UINT64(a)			TOKEN_VALUE(i, (uint32_t)(a)), TOKEN_VALUE(i, (uint32_t)((a) >> 32))
#endif

/* mask a value to a fixed number of bits and then shift it */
#define SHIFT_AND_MASK32(val, bits, shift)		(((uint32_t)(val) & ((1 << (bits)) - 1)) << (shift))
#define SHIFT_AND_MASK64(val, bits, shift)		(((uint64_t)(val) & (((uint64_t)1 << (bits)) - 1)) << (shift))

/* 32-bit integer packing */
#define TOKEN_UINT32_PACK1(val1, bits1) \
	TOKEN_UINT32(SHIFT_AND_MASK32((val1), (bits1), 0))

#define TOKEN_UINT32_PACK2(val1, bits1, val2, bits2) \
	TOKEN_UINT32(SHIFT_AND_MASK32((val1), (bits1), 0) | \
				 SHIFT_AND_MASK32((val2), (bits2), (bits1)))

#define TOKEN_UINT32_PACK3(val1, bits1, val2, bits2, val3, bits3) \
	TOKEN_UINT32(SHIFT_AND_MASK32((val1), (bits1), 0) | \
				 SHIFT_AND_MASK32((val2), (bits2), (bits1)) | \
				 SHIFT_AND_MASK32((val3), (bits3), (bits1)+(bits2)))

#define TOKEN_UINT32_PACK4(val1, bits1, val2, bits2, val3, bits3, val4, bits4) \
	TOKEN_UINT32(SHIFT_AND_MASK32((val1), (bits1), 0) | \
				 SHIFT_AND_MASK32((val2), (bits2), (bits1)) | \
				 SHIFT_AND_MASK32((val3), (bits3), (bits1)+(bits2)) | \
				 SHIFT_AND_MASK32((val4), (bits4), (bits1)+(bits2)+(bits3)))

/* 64-bit integer packing */
#define TOKEN_UINT64_PACK1(val1, bits1) \
	TOKEN_UINT64(SHIFT_AND_MASK64((val1), (bits1), 0))

#define TOKEN_UINT64_PACK2(val1, bits1, val2, bits2) \
	TOKEN_UINT64(SHIFT_AND_MASK64((val1), (bits1), 0) | \
				 SHIFT_AND_MASK64((val2), (bits2), (bits1)))

#define TOKEN_UINT64_PACK3(val1, bits1, val2, bits2, val3, bits3) \
	TOKEN_UINT64(SHIFT_AND_MASK64((val1), (bits1), 0) | \
				 SHIFT_AND_MASK64((val2), (bits2), (bits1)) | \
				 SHIFT_AND_MASK64((val3), (bits3), (bits1)+(bits2)))

#define TOKEN_UINT64_PACK4(val1, bits1, val2, bits2, val3, bits3, val4, bits4) \
	TOKEN_UINT64(SHIFT_AND_MASK64((val1), (bits1), 0) | \
				 SHIFT_AND_MASK64((val2), (bits2), (bits1)) | \
				 SHIFT_AND_MASK64((val3), (bits3), (bits1)+(bits2)) | \
				 SHIFT_AND_MASK64((val4), (bits4), (bits1)+(bits2)+(bits3)))



/* ----- run-time token extraction macros ----- */

/* token fetch and advance primitives */
#define TOKEN_GET_PTR(tp,field)		(((tp)++)->field)
#define TOKEN_GET_STRING(tp)		(((tp)++)->stringptr)
#define TOKEN_GET_UINT32(tp)		(((tp)++)->i)
#ifdef PTR64
#define TOKEN_EXTRACT_UINT64(tp,a)	do { (a) = (tp)->i; (tp)++; } while (0)
#else
#define TOKEN_EXTRACT_UINT64(tp,a)	do { (a) = (tp)[0].i | ((uint64_t)(tp)[1].i << 32); (tp) += 2; } while (0)
#endif

/* token unfetch primitives */
#define TOKEN_UNGET_PTR(tp)			((tp)--)
#define TOKEN_UNGET_STRING(tp)		((tp)--)
#define TOKEN_UNGET_UINT32(tp)		((tp)--)
#define TOKEN_UNGET_UINT64(tp)		((tp) -= 8 / sizeof(FPTR))

/* token skip primitives */
#define TOKEN_SKIP_PTR(tp)			((tp)++)
#define TOKEN_SKIP_STRING(tp)		((tp)++)
#define TOKEN_SKIP_UINT32(tp)		((tp)++)
#define TOKEN_SKIP_UINT64(tp)		((tp) += 8 / sizeof(FPTR))

/* extract a value from a fixed number of bits; if bits is negative, treat it as a signed value */
#define UNSHIFT_AND_MASK32(src, val, bits, shift)	do { \
	if ((bits) < 0) \
		(val) = token_sign_extend32((src) >> (shift), (uint8_t)-(bits)); \
	else \
		(val) = token_zero_extend32((src) >> (shift), (uint8_t)(bits)); \
} while (0)

#define UNSHIFT_AND_MASK64(src, val, bits, shift)	do { \
	if ((bits) < 0) \
		(val) = token_sign_extend64((src) >> (shift), (uint8_t)-(bits)); \
	else \
		(val) = token_zero_extend64((src) >> (shift), (uint8_t)(bits)); \
} while (0)

/* cheesy inline absolute value */
#define TOKENABS(v)		(((v) < 0) ? -(v) : (v))

/* 32-bit integer unpacking */
#define TOKEN_GET_UINT32_UNPACK1(tp, val1, bits1) do { \
	uint32_t token32 = TOKEN_GET_UINT32(tp); \
	UNSHIFT_AND_MASK32(token32, val1, (bits1), 0); \
} while (0)

#define TOKEN_GET_UINT32_UNPACK2(tp, val1, bits1, val2, bits2) do { \
	uint32_t token32 = TOKEN_GET_UINT32(tp); \
	uint8_t shift = 0; \
	UNSHIFT_AND_MASK32(token32, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK32(token32, val2, (bits2), shift); \
} while (0)

#define TOKEN_GET_UINT32_UNPACK3(tp, val1, bits1, val2, bits2, val3, bits3) do { \
	uint32_t token32 = TOKEN_GET_UINT32(tp); \
	uint8_t shift = 0; \
	UNSHIFT_AND_MASK32(token32, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK32(token32, val2, (bits2), shift); shift += TOKENABS(bits2); \
	UNSHIFT_AND_MASK32(token32, val3, (bits3), shift); \
} while (0)

#define TOKEN_GET_UINT32_UNPACK4(tp, val1, bits1, val2, bits2, val3, bits3, val4, bits4) do { \
	uint32_t token32 = TOKEN_GET_UINT32(tp); \
	uint8_t shift = 0; \
	UNSHIFT_AND_MASK32(token32, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK32(token32, val2, (bits2), shift); shift += TOKENABS(bits2); \
	UNSHIFT_AND_MASK32(token32, val3, (bits3), shift); shift += TOKENABS(bits3); \
	UNSHIFT_AND_MASK32(token32, val4, (bits4), shift); \
} while (0)

/* 64-bit integer unpacking */
#define TOKEN_GET_UINT64_UNPACK1(tp, val1, bits1) do { \
	uint64_t token64; \
	TOKEN_EXTRACT_UINT64(tp, token64); \
	UNSHIFT_AND_MASK64(token64, val1, (bits1), 0); \
} while (0)

#define TOKEN_GET_UINT64_UNPACK2(tp, val1, bits1, val2, bits2) do { \
	uint64_t token64; \
	uint8_t shift = 0; \
	TOKEN_EXTRACT_UINT64(tp, token64); \
	UNSHIFT_AND_MASK64(token64, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK64(token64, val2, (bits2), shift); \
} while (0)

#define TOKEN_GET_UINT64_UNPACK3(tp, val1, bits1, val2, bits2, val3, bits3) do { \
	uint64_t token64; \
	uint8_t shift = 0; \
	TOKEN_EXTRACT_UINT64(tp, token64); \
	UNSHIFT_AND_MASK64(token64, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK64(token64, val2, (bits2), shift); shift += TOKENABS(bits2); \
	UNSHIFT_AND_MASK64(token64, val3, (bits3), shift); \
} while (0)

#define TOKEN_GET_UINT64_UNPACK4(tp, val1, bits1, val2, bits2, val3, bits3, val4, bits4) do { \
	uint64_t token64; \
	uint8_t shift = 0; \
	TOKEN_EXTRACT_UINT64(tp, token64); \
	UNSHIFT_AND_MASK64(token64, val1, (bits1), shift); shift += TOKENABS(bits1); \
	UNSHIFT_AND_MASK64(token64, val2, (bits2), shift); shift += TOKENABS(bits2); \
	UNSHIFT_AND_MASK64(token64, val3, (bits3), shift); shift += TOKENABS(bits3); \
	UNSHIFT_AND_MASK64(token64, val4, (bits4), shift); \
} while (0)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE uint32_t token_zero_extend32(uint32_t val, uint8_t bits)
{
	return val & (((uint32_t)1 << bits) - 1);
}

INLINE int32_t token_sign_extend32(uint32_t val, uint8_t bits)
{
	return (int32_t)(val << (32 - bits)) >> (32 - bits);
}

INLINE uint64_t token_zero_extend64(uint64_t val, uint8_t bits)
{
	return val & (((uint64_t)1 << bits) - 1);
}

INLINE int64_t token_sign_extend64(uint64_t val, uint8_t bits)
{
	return (int64_t)(val << (64 - bits)) >> (64 - bits);
}

#endif
