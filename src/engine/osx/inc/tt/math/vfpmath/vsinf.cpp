/*
 vsinf.cpp

 Created by damien on 8/03/09.
 Copyright 2009 Damien Morton. All rights reserved.
 
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


#ifdef __APPLE__
#include <TargetConditionals.h>
#if (TARGET_IPHONE_SIMULATOR == 0) && (TARGET_OS_IPHONE == 1)

#include "vsinf.h"
#include <math.h>

float fast_sin(float x);

inline float abs(float x) { return (x < 0) ? -x : x; }

float fast_sin(float x)
{
	// fast sin function; maximum error is 0.001
	const float P = 0.225f;
	
	x = x * (float)M_1_PI;
	int k = (int) round(x);
	x = x - k;

	float y = (4 - 4 * abs(x)) * x;

	y = P * (y * abs(y) - y) + y;

	return (k&1) ? -y : y;
}


void vsinf(float *x, float *y, int n) // input x, output y, n multiple of 4
{
	// source and destination arrays can be the same
	const float P = 0.225f;
	float consts[4] = { (float)M_1_PI, 1.0f, 4.0f, P};
	const float neg1 = -1.0f;
	
	int a, b, c, d;
	a = b = c = d = 0; //TWO TRIBES 'fix'.
	
	b = (int) consts;
	asm volatile (
				  "bic %[n], %[n], #3					\n\t"   // n has to be a multiple of 4 - lets enforce that (and avoid infinte loops)
				  "fldmias %[b], {s0-s3}				\n\t"	// load up our constants	
				  "fldmias %[x]!, {s8-s11}				\n\t"   // v2 = x	 // our loop is a little bit overlapped - preload some stuff		
				  
				  "fmrx %[a], fpscr						\n\t"	
				  "stmfd  sp!, {%[a]}					\n\t"	// save fpscr to the stack (dont forget to restore it and the stack pointer)
				  "bic %[a], %[a], #0x001f				\n\t"   // for runfast - clear CumulativeException[4:0] bits
				  "bic %[a], %[a], #0x1f00				\n\t"   // for runfast - clear ExceptionTrapEnable[12:8] bits
				  "orr %[a], %[a], #(0x3 << 24)			\n\t"   // for runfast - set DefaultNaN[25] and FushToZero[24] bits
				  "bic %[a], %[a], #(0x37 << 16)		\n\t"   // clear the vector stride[21:20] and len[18:16] bits       
				  "orr %[a], %[a], #(0x03 << 16)		\n\t"	// set stride[21:20] to 1(0), vector len[18:16] to 4(3)	
				  "fmxr fpscr, %[a]						\n\t"	
				  
				  "fmuls s8, s8, s0						\n\t"	// v3: = x * (1/PI)						  

				  "ftosis s12, s8						\n\t"	// v3: k = round(x)
				  "ftosis s13, s9						\n\t"
				  "ftosis s14, s10						\n\t"
				  "ftosis s15, s11						\n\t"			
			  
				  "fmrrs %[a],%[b], {s12,s13}			\n\t"	
				  "fsitos s12, s12						\n\t"	// k = (float) k
				  "fsitos s13, s13						\n\t"
				  "fmrrs %[c],%[d], {s14,s15}			\n\t"					   
				  "fsitos s14, s14						\n\t"
				  "fsitos s15, s15						\n\t"				  
				  
				"1:										\n\t"
			  
				  //OOO out of order instructions marked with OOO and are usually suffixed with the ne condition
		
				  "fsubs s8, s8, s12					\n\t"   // x = x - k	
				  "fcpys s28, s1						\n\t"	// v7 = 1111					  
				  "fcpys s24, s2						\n\t"   // OOO v6 = 4444		
				  
				  //float x = x * M_1_PI;
				  //int k = round(x);
				  //float x = x - k;
				  
				  "fstmiasne %[y]!, {s20-s23}			\n\t"	// OOO: from previous loop		
				  
				  "tst %[a], #1							\n\t"   // if odd(n) then negate result at end
				  "fmsrne s28, %[neg1]					\n\t"   // shove a -1 into the appropriate part of v7 
				  "tst %[b], #1							\n\t"   // 4 tests, spread through code - result not needed till end
				  "fmsrne s29, %[neg1]					\n\t"				   
				  
  				  "fabss s16, s8						\n\t"   // x' = abs(x)
				  //	y = (4 - 4 * abs(x)) * x;
				  
				  "tst %[c], #1							\n\t"
				  "fmsrne s30, %[neg1]					\n\t"			
				  
				  "fnmacs s24, s16, s2					\n\t"   // y' = 4 - 4*abs(x)
				  
				  "tst %[d], #1							\n\t"
				  "fmsrne s31, %[neg1]					\n\t"			
				  
			  
				  "fmuls s16, s24, s8					\n\t"   // y' *= x
				  
				  "subs %[n], %[n], #4					\n\t"	// testing early here 					  
				  "fldmiasne %[x]!, {s8-s11}			\n\t"   // v2 = x	 // our loop is a little bit overlapped - preload some stuff
				  "pld [%[x],#64]						\n\t"   // In the ARM1176JZF-S processor, in Non-secure state, the PLD instruction behaves like a NOP. Dammit.
				  
				  // y += P * (y * abs(y) - y);					   

				  "fabss s20, s16						\n\t"	// v4 = abs(y)
				  "fcpys s24, s16                       \n\t"							  
				  
				  "fmulsne s8, s8, s0					\n\t"	// OOO v2 = x * (1/PI)		
				  
				  "fmscs s16, s16, s20					\n\t"	// v3 = -y + y*abs(y)
				  
				  "ftosisne s12, s8						\n\t"	// OOO v3: k = round(x)
				  "ftosisne s13, s9						\n\t"	
				  "ftosisne s14, s10					\n\t"
				  "ftosisne s15, s11					\n\t"						  
			  
				  "fmacs s24, s16, s3					\n\t"	// y += P*v3	
				  
				  "fmrrsne %[a],%[b], {s12,s13}			\n\t"	
				  "fsitosne s12, s12					\n\t"	// k = (float) k
				  "fsitosne s13, s13					\n\t"
				  "fmrrsne %[c],%[d], {s14,s15}			\n\t"					   
				  "fsitosne s14, s14					\n\t"
				  "fsitosne s15, s15					\n\t"
				  
				  "fmuls s20, s28, s24					\n\t"	// multiply result by +1/-1 according to oddness of n			  				  
				  "bne 1b								\n\t"	
				  
				  "fstmias %[y]!, {s20-s23}				\n\t"				  
				  "ldmfd  sp!, {%[b]}					\n\t"	// restore the fpscr
				  "fmxr fpscr, %[b]						\n\t"		

				  
				  :	  [x] "+&r" (x), 
					  [y] "+&r" (y), 
					  [n] "+&r" (n), 			   
					  [neg1] "+&r" (neg1), 
					  [a] "+&r" (a),
					  [b] "+&r" (b),
					  [c] "+&r" (c),
					  [d] "+&r" (d)
				  :
				  :	"memory", "cc", 
				  "s0",  "s1",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
				  "s8",  "s9",  "s10", "s11", "s12", "s13", "s14", "s15",
				  "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",
				  "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"		   
				  );	 
}

  
#endif
#endif
