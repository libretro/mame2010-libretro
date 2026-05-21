/*******************************************************************************
*                                                                              *
*   Define size independent data types and operations.                         *
*                                                                              *
*   The following types must be supported by all platforms:                    *
*                                                                              *
*   uint8_t  - Unsigned 8-bit Integer     int8_t  - Signed 8-bit integer           *
*   uint16_t - Unsigned 16-bit Integer    int16_t - Signed 16-bit integer          *
*   uint32_t - Unsigned 32-bit Integer    int32_t - Signed 32-bit integer          *
*   uint64_t - Unsigned 64-bit Integer    int64_t - Signed 64-bit integer          *
*                                                                              *
*                                                                              *
*   The macro names for the artithmatic operations are composed as follows:    *
*                                                                              *
*   XXX_R_A_B, where XXX - 3 letter operation code (ADD, SUB, etc.)            *
*                    R   - The type of the result                              *
*                    A   - The type of operand 1                               *
*                    B   - The type of operand 2 (if binary operation)         *
*                                                                              *
*                    Each type is one of: U8,8,U16,16,U32,32,U64,64            *
*                                                                              *
*******************************************************************************/
#pragma once

#ifndef OSD_CPU_H
#define OSD_CPU_H

/* Combine two 32-bit integers into a 64-bit integer */
#define COMBINE_64_32_32(A,B)     ((((uint64_t)(A))<<32) | (uint32_t)(B))
#define COMBINE_U64_U32_U32(A,B)  COMBINE_64_32_32(A,B)

/* Return upper 32 bits of a 64-bit integer */
#define HI32_32_64(A)		  (((uint64_t)(A)) >> 32)
#define HI32_U32_U64(A)		  HI32_32_64(A)

/* Return lower 32 bits of a 64-bit integer */
#define LO32_32_64(A)		  ((A) & 0xffffffff)
#define LO32_U32_U64(A)		  LO32_32_64(A)

#define DIV_64_64_32(A,B)	  ((A)/(B))
#define DIV_U64_U64_U32(A,B)  ((A)/(uint32_t)(B))

#define MOD_32_64_32(A,B)	  ((A)%(B))
#define MOD_U32_U64_U32(A,B)  ((A)%(uint32_t)(B))

#define MUL_64_32_32(A,B)	  ((A)*(int64_t)(B))
#define MUL_U64_U32_U32(A,B)  ((A)*(uint64_t)(uint32_t)(B))

#endif	/* defined OSD_CPU_H */
