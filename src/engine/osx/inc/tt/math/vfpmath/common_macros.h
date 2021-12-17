/*
VFP math library for the iPhone / iPod touch

Copyright (c) 2007-2008 Wolfgang Engel and Matthias Grundmann
http://code.google.com/p/vfpmathlibrary/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
*/

#ifndef COMMON_MACROS_H__
#define COMMON_MACROS_H__

// Usage for any VFP routine is:

/*
    asm volatile (VFP_SWITCH_TO_ARM

                  <--  your code goes here

                  VFP_SWITCH_TO_THUMB
                  : <list of modified INPUT registers>
                  : <list of input registers>
                  : "r0", <additional registers you used>
                  );
*/

// NOTE: Lots of VFP macros overwrite register r0, therefore you have to make sure 
//       to include "r0" in the list of registers used, as in above example.

#define NO_THUMB // Two Tribes define. (no way to check compiler setting?)

#ifndef NO_THUMB
#error "Compiling in Thumb Mode. Mode switches activated."
#else
//#warning "Compiling in ARM mode. Mode switches deactivated."
#endif

// Switches to from THUMB to ARM mode.
#ifndef NO_THUMB
  #define VFP_SWITCH_TO_ARM ".align 4               \n\t" \
                            "mov     r0, pc         \n\t" \
                            "bx      r0             \n\t" \
                            ".arm                   \n\t" 
#else
  #define VFP_SWITCH_TO_ARM
#endif

// Switches from ARM to THUMB mode.
#ifndef NO_THUMB
  #define VFP_SWITCH_TO_THUMB "add     r0, pc, #1     \n\t" \
                              "bx      r0             \n\t" \
                              ".thumb                 \n\t" 
#else
  #define VFP_SWITCH_TO_THUMB
#endif

// NOTE: Both VFP_VECTOR_LENGTH* macros will stall the FP unit, 
//       until all currently processed operations have been executed.
//       Call wisely.
//       FP operations (except load/stores) will interpret a command like
//	 fadds s8, s16, s24
//	 AS
//	 fadds {s8-s11}, {s16-s19}, {s24-s27} in case length is set to zero.


// Sets length and stride to 0.
#define VFP_VECTOR_LENGTH_ZERO "fmrx    r0, fpscr            \n\t" \
                               "bic     r0, r0, #0x00370000  \n\t" \
                               "fmxr    fpscr, r0            \n\t" 
  
// Set vector length. VEC_LENGTH has to be bitween 0 for length 1 and 3 for length 4.
#define VFP_VECTOR_LENGTH(VEC_LENGTH) "fmrx    r0, fpscr                         \n\t" \
                                      "bic     r0, r0, #0x00370000               \n\t" \
                                      "orr     r0, r0, #0x000" #VEC_LENGTH "0000 \n\t" \
                                      "fmxr    fpscr, r0                         \n\t"

// Fixed vector operation for vectors of length 1, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_1_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, s8        \n\t" \
                "fldmias  %2, s16       \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                "fstmias  %0, s8        \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0" \
                );

// Fixed vector operation for vectors of length 2, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_2_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s9}   \n\t" \
                "fldmias  %2, {s16-s17} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                "fstmias  %0, {s8-s9}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0" \
                );

// Fixed vector operation for vectors of length 3, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_3_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s10}  \n\t" \
                "fldmias  %2, {s16-s18} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                VFP_OP  " s10, s10, s18 \n\t" \
                "fstmias  %0, {s8-s10}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0" \
                );

// Fixed vector operation for vectors of length 4, i.e. scalars.
// Expects pointers to source and destination data.
// Use VFP_OP_* macros for VFP_OP or any FP assembler opcode that fits.
#define VFP_FIXED_4_VECTOR_OP(VFP_OP, P_SRC_1, P_SRC_2, P_DST) \
  asm volatile (VFP_SWITCH_TO_ARM \
                "fldmias  %1, {s8-s11}  \n\t" \
                "fldmias  %2, {s16-s19} \n\t" \
                VFP_OP  " s8, s8, s16   \n\t" \
                VFP_OP  " s9, s9, s17   \n\t" \
                VFP_OP  " s10, s10, s18 \n\t" \
                VFP_OP  " s11, s11, s19 \n\t" \
                "fstmias  %0, {s8-s11}   \n\t" \
                VFP_SWITCH_TO_THUMB \
                : \
                : "r" (P_DST), "r" (P_SRC_1), "r" (P_SRC_2) \
                : "r0" \
                );

// NOTE: Usage example for VFP_FIXED_?_VECTOR_OP
// 	 float* src_ptr_1;
// 	 float* src_ptr_2;
// 	 float* dst_ptr; 
//	 VFP_FIXED_4_VECTOR_OP(VFP_OP_ADD, src_ptr_1, src_ptr_2, dst_ptr)

#define VFP_OP_ADD "fadds"
#define VFP_OP_SUB "fsubs"
#define VFP_OP_MUL "fmuls"
#define VFP_OP_DIV "fdivs"
#define VFP_OP_ABS "fabss"
#define VFP_OP_SQRT "fsqrts"

#endif // COMMON_MACROS_H__
