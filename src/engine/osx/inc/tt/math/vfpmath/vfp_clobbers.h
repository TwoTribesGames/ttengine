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


// VFP registers clobber list specified by intervals [s0, s31]
// Header file is created by the following snippet.

/*
#include <iostream>

int main() {
  int min_reg = 0;
  int max_reg = 31;
  
  for (int i = min_reg; i < max_reg; ++i) {
    for (int j = i+1; j <= max_reg; ++j) {
      std::cout << "#define VFP_CLOBBER_S" << i << "_"
      <<  "S" << j << " ";
      for (int k = i; k <= j; ++k) {
        std::cout << "\"s" << k << "\"";
        if (k != j) {
          std::cout << ", ";
          if (k > i && (k-i) % 8 == 0) {
            std::cout << " \\\n                          ";
          }
        }
      }
      
      std::cout << "\n";
    }
  }
}
*/

#define VFP_CLOBBER_S0_S1 "s0", "s1"
#define VFP_CLOBBER_S0_S2 "s0", "s1", "s2"
#define VFP_CLOBBER_S0_S3 "s0", "s1", "s2", "s3"
#define VFP_CLOBBER_S0_S4 "s0", "s1", "s2", "s3", "s4"
#define VFP_CLOBBER_S0_S5 "s0", "s1", "s2", "s3", "s4", "s5"
#define VFP_CLOBBER_S0_S6 "s0", "s1", "s2", "s3", "s4", "s5", "s6"
#define VFP_CLOBBER_S0_S7 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"
#define VFP_CLOBBER_S0_S8 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S0_S9 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9"
#define VFP_CLOBBER_S0_S10 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10"
#define VFP_CLOBBER_S0_S11 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11"
#define VFP_CLOBBER_S0_S12 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S0_S13 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S0_S14 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S0_S15 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S0_S16 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S0_S17 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17"
#define VFP_CLOBBER_S0_S18 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18"
#define VFP_CLOBBER_S0_S19 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19"
#define VFP_CLOBBER_S0_S20 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S0_S21 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S0_S22 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S0_S23 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S0_S24 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S0_S25 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25"
#define VFP_CLOBBER_S0_S26 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26"
#define VFP_CLOBBER_S0_S27 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27"
#define VFP_CLOBBER_S0_S28 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S0_S29 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S0_S30 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S0_S31 "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8",  \
                          "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S1_S2 "s1", "s2"
#define VFP_CLOBBER_S1_S3 "s1", "s2", "s3"
#define VFP_CLOBBER_S1_S4 "s1", "s2", "s3", "s4"
#define VFP_CLOBBER_S1_S5 "s1", "s2", "s3", "s4", "s5"
#define VFP_CLOBBER_S1_S6 "s1", "s2", "s3", "s4", "s5", "s6"
#define VFP_CLOBBER_S1_S7 "s1", "s2", "s3", "s4", "s5", "s6", "s7"
#define VFP_CLOBBER_S1_S8 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S1_S9 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S1_S10 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10"
#define VFP_CLOBBER_S1_S11 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11"
#define VFP_CLOBBER_S1_S12 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12"
#define VFP_CLOBBER_S1_S13 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S1_S14 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S1_S15 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S1_S16 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S1_S17 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S1_S18 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18"
#define VFP_CLOBBER_S1_S19 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19"
#define VFP_CLOBBER_S1_S20 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20"
#define VFP_CLOBBER_S1_S21 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S1_S22 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S1_S23 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S1_S24 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S1_S25 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S1_S26 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26"
#define VFP_CLOBBER_S1_S27 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27"
#define VFP_CLOBBER_S1_S28 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28"
#define VFP_CLOBBER_S1_S29 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S1_S30 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S1_S31 "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",  \
                          "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S2_S3 "s2", "s3"
#define VFP_CLOBBER_S2_S4 "s2", "s3", "s4"
#define VFP_CLOBBER_S2_S5 "s2", "s3", "s4", "s5"
#define VFP_CLOBBER_S2_S6 "s2", "s3", "s4", "s5", "s6"
#define VFP_CLOBBER_S2_S7 "s2", "s3", "s4", "s5", "s6", "s7"
#define VFP_CLOBBER_S2_S8 "s2", "s3", "s4", "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S2_S9 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S2_S10 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S2_S11 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11"
#define VFP_CLOBBER_S2_S12 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12"
#define VFP_CLOBBER_S2_S13 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13"
#define VFP_CLOBBER_S2_S14 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S2_S15 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S2_S16 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S2_S17 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S2_S18 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S2_S19 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19"
#define VFP_CLOBBER_S2_S20 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20"
#define VFP_CLOBBER_S2_S21 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21"
#define VFP_CLOBBER_S2_S22 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S2_S23 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S2_S24 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S2_S25 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S2_S26 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S2_S27 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27"
#define VFP_CLOBBER_S2_S28 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28"
#define VFP_CLOBBER_S2_S29 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29"
#define VFP_CLOBBER_S2_S30 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S2_S31 "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",  \
                          "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S3_S4 "s3", "s4"
#define VFP_CLOBBER_S3_S5 "s3", "s4", "s5"
#define VFP_CLOBBER_S3_S6 "s3", "s4", "s5", "s6"
#define VFP_CLOBBER_S3_S7 "s3", "s4", "s5", "s6", "s7"
#define VFP_CLOBBER_S3_S8 "s3", "s4", "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S3_S9 "s3", "s4", "s5", "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S3_S10 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S3_S11 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S3_S12 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12"
#define VFP_CLOBBER_S3_S13 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13"
#define VFP_CLOBBER_S3_S14 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14"
#define VFP_CLOBBER_S3_S15 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S3_S16 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S3_S17 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S3_S18 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S3_S19 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S3_S20 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20"
#define VFP_CLOBBER_S3_S21 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21"
#define VFP_CLOBBER_S3_S22 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22"
#define VFP_CLOBBER_S3_S23 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S3_S24 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S3_S25 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S3_S26 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S3_S27 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S3_S28 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28"
#define VFP_CLOBBER_S3_S29 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29"
#define VFP_CLOBBER_S3_S30 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30"
#define VFP_CLOBBER_S3_S31 "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",  \
                          "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S4_S5 "s4", "s5"
#define VFP_CLOBBER_S4_S6 "s4", "s5", "s6"
#define VFP_CLOBBER_S4_S7 "s4", "s5", "s6", "s7"
#define VFP_CLOBBER_S4_S8 "s4", "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S4_S9 "s4", "s5", "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S4_S10 "s4", "s5", "s6", "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S4_S11 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S4_S12 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S4_S13 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13"
#define VFP_CLOBBER_S4_S14 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14"
#define VFP_CLOBBER_S4_S15 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15"
#define VFP_CLOBBER_S4_S16 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S4_S17 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S4_S18 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S4_S19 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S4_S20 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S4_S21 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21"
#define VFP_CLOBBER_S4_S22 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22"
#define VFP_CLOBBER_S4_S23 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23"
#define VFP_CLOBBER_S4_S24 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S4_S25 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S4_S26 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S4_S27 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S4_S28 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S4_S29 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29"
#define VFP_CLOBBER_S4_S30 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30"
#define VFP_CLOBBER_S4_S31 "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12",  \
                          "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30", "s31"
#define VFP_CLOBBER_S5_S6 "s5", "s6"
#define VFP_CLOBBER_S5_S7 "s5", "s6", "s7"
#define VFP_CLOBBER_S5_S8 "s5", "s6", "s7", "s8"
#define VFP_CLOBBER_S5_S9 "s5", "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S5_S10 "s5", "s6", "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S5_S11 "s5", "s6", "s7", "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S5_S12 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S5_S13 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S5_S14 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14"
#define VFP_CLOBBER_S5_S15 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15"
#define VFP_CLOBBER_S5_S16 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16"
#define VFP_CLOBBER_S5_S17 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S5_S18 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S5_S19 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S5_S20 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S5_S21 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S5_S22 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22"
#define VFP_CLOBBER_S5_S23 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23"
#define VFP_CLOBBER_S5_S24 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24"
#define VFP_CLOBBER_S5_S25 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S5_S26 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S5_S27 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S5_S28 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S5_S29 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S5_S30 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30"
#define VFP_CLOBBER_S5_S31 "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13",  \
                          "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30", "s31"
#define VFP_CLOBBER_S6_S7 "s6", "s7"
#define VFP_CLOBBER_S6_S8 "s6", "s7", "s8"
#define VFP_CLOBBER_S6_S9 "s6", "s7", "s8", "s9"
#define VFP_CLOBBER_S6_S10 "s6", "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S6_S11 "s6", "s7", "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S6_S12 "s6", "s7", "s8", "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S6_S13 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S6_S14 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S6_S15 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15"
#define VFP_CLOBBER_S6_S16 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16"
#define VFP_CLOBBER_S6_S17 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17"
#define VFP_CLOBBER_S6_S18 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S6_S19 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S6_S20 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S6_S21 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S6_S22 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S6_S23 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23"
#define VFP_CLOBBER_S6_S24 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24"
#define VFP_CLOBBER_S6_S25 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25"
#define VFP_CLOBBER_S6_S26 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S6_S27 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S6_S28 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S6_S29 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S6_S30 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S6_S31 "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14",  \
                          "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30",  \
                          "s31"
#define VFP_CLOBBER_S7_S8 "s7", "s8"
#define VFP_CLOBBER_S7_S9 "s7", "s8", "s9"
#define VFP_CLOBBER_S7_S10 "s7", "s8", "s9", "s10"
#define VFP_CLOBBER_S7_S11 "s7", "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S7_S12 "s7", "s8", "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S7_S13 "s7", "s8", "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S7_S14 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S7_S15 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S7_S16 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16"
#define VFP_CLOBBER_S7_S17 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17"
#define VFP_CLOBBER_S7_S18 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18"
#define VFP_CLOBBER_S7_S19 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S7_S20 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S7_S21 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S7_S22 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S7_S23 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S7_S24 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24"
#define VFP_CLOBBER_S7_S25 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25"
#define VFP_CLOBBER_S7_S26 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26"
#define VFP_CLOBBER_S7_S27 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S7_S28 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S7_S29 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S7_S30 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S7_S31 "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",  \
                          "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S8_S9 "s8", "s9"
#define VFP_CLOBBER_S8_S10 "s8", "s9", "s10"
#define VFP_CLOBBER_S8_S11 "s8", "s9", "s10", "s11"
#define VFP_CLOBBER_S8_S12 "s8", "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S8_S13 "s8", "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S8_S14 "s8", "s9", "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S8_S15 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S8_S16 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S8_S17 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17"
#define VFP_CLOBBER_S8_S18 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18"
#define VFP_CLOBBER_S8_S19 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19"
#define VFP_CLOBBER_S8_S20 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S8_S21 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S8_S22 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S8_S23 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S8_S24 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S8_S25 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25"
#define VFP_CLOBBER_S8_S26 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26"
#define VFP_CLOBBER_S8_S27 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27"
#define VFP_CLOBBER_S8_S28 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S8_S29 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S8_S30 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S8_S31 "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16",  \
                          "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S9_S10 "s9", "s10"
#define VFP_CLOBBER_S9_S11 "s9", "s10", "s11"
#define VFP_CLOBBER_S9_S12 "s9", "s10", "s11", "s12"
#define VFP_CLOBBER_S9_S13 "s9", "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S9_S14 "s9", "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S9_S15 "s9", "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S9_S16 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S9_S17 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S9_S18 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18"
#define VFP_CLOBBER_S9_S19 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19"
#define VFP_CLOBBER_S9_S20 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20"
#define VFP_CLOBBER_S9_S21 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S9_S22 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S9_S23 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S9_S24 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S9_S25 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S9_S26 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26"
#define VFP_CLOBBER_S9_S27 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27"
#define VFP_CLOBBER_S9_S28 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28"
#define VFP_CLOBBER_S9_S29 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S9_S30 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S9_S31 "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17",  \
                          "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S10_S11 "s10", "s11"
#define VFP_CLOBBER_S10_S12 "s10", "s11", "s12"
#define VFP_CLOBBER_S10_S13 "s10", "s11", "s12", "s13"
#define VFP_CLOBBER_S10_S14 "s10", "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S10_S15 "s10", "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S10_S16 "s10", "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S10_S17 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S10_S18 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S10_S19 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19"
#define VFP_CLOBBER_S10_S20 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20"
#define VFP_CLOBBER_S10_S21 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21"
#define VFP_CLOBBER_S10_S22 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S10_S23 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S10_S24 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S10_S25 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S10_S26 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S10_S27 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27"
#define VFP_CLOBBER_S10_S28 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28"
#define VFP_CLOBBER_S10_S29 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29"
#define VFP_CLOBBER_S10_S30 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S10_S31 "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18",  \
                          "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S11_S12 "s11", "s12"
#define VFP_CLOBBER_S11_S13 "s11", "s12", "s13"
#define VFP_CLOBBER_S11_S14 "s11", "s12", "s13", "s14"
#define VFP_CLOBBER_S11_S15 "s11", "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S11_S16 "s11", "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S11_S17 "s11", "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S11_S18 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S11_S19 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S11_S20 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20"
#define VFP_CLOBBER_S11_S21 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21"
#define VFP_CLOBBER_S11_S22 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22"
#define VFP_CLOBBER_S11_S23 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S11_S24 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S11_S25 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S11_S26 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S11_S27 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S11_S28 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28"
#define VFP_CLOBBER_S11_S29 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29"
#define VFP_CLOBBER_S11_S30 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30"
#define VFP_CLOBBER_S11_S31 "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19",  \
                          "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S12_S13 "s12", "s13"
#define VFP_CLOBBER_S12_S14 "s12", "s13", "s14"
#define VFP_CLOBBER_S12_S15 "s12", "s13", "s14", "s15"
#define VFP_CLOBBER_S12_S16 "s12", "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S12_S17 "s12", "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S12_S18 "s12", "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S12_S19 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S12_S20 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S12_S21 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21"
#define VFP_CLOBBER_S12_S22 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22"
#define VFP_CLOBBER_S12_S23 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23"
#define VFP_CLOBBER_S12_S24 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S12_S25 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S12_S26 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S12_S27 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S12_S28 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S12_S29 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29"
#define VFP_CLOBBER_S12_S30 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30"
#define VFP_CLOBBER_S12_S31 "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20",  \
                          "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30", "s31"
#define VFP_CLOBBER_S13_S14 "s13", "s14"
#define VFP_CLOBBER_S13_S15 "s13", "s14", "s15"
#define VFP_CLOBBER_S13_S16 "s13", "s14", "s15", "s16"
#define VFP_CLOBBER_S13_S17 "s13", "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S13_S18 "s13", "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S13_S19 "s13", "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S13_S20 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S13_S21 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S13_S22 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22"
#define VFP_CLOBBER_S13_S23 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23"
#define VFP_CLOBBER_S13_S24 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24"
#define VFP_CLOBBER_S13_S25 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S13_S26 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S13_S27 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S13_S28 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S13_S29 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S13_S30 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30"
#define VFP_CLOBBER_S13_S31 "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21",  \
                          "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30", "s31"
#define VFP_CLOBBER_S14_S15 "s14", "s15"
#define VFP_CLOBBER_S14_S16 "s14", "s15", "s16"
#define VFP_CLOBBER_S14_S17 "s14", "s15", "s16", "s17"
#define VFP_CLOBBER_S14_S18 "s14", "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S14_S19 "s14", "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S14_S20 "s14", "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S14_S21 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S14_S22 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S14_S23 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23"
#define VFP_CLOBBER_S14_S24 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24"
#define VFP_CLOBBER_S14_S25 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25"
#define VFP_CLOBBER_S14_S26 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S14_S27 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S14_S28 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S14_S29 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S14_S30 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S14_S31 "s14", "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22",  \
                          "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30",  \
                          "s31"
#define VFP_CLOBBER_S15_S16 "s15", "s16"
#define VFP_CLOBBER_S15_S17 "s15", "s16", "s17"
#define VFP_CLOBBER_S15_S18 "s15", "s16", "s17", "s18"
#define VFP_CLOBBER_S15_S19 "s15", "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S15_S20 "s15", "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S15_S21 "s15", "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S15_S22 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S15_S23 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S15_S24 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24"
#define VFP_CLOBBER_S15_S25 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25"
#define VFP_CLOBBER_S15_S26 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26"
#define VFP_CLOBBER_S15_S27 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S15_S28 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S15_S29 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S15_S30 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S15_S31 "s15", "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",  \
                          "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S16_S17 "s16", "s17"
#define VFP_CLOBBER_S16_S18 "s16", "s17", "s18"
#define VFP_CLOBBER_S16_S19 "s16", "s17", "s18", "s19"
#define VFP_CLOBBER_S16_S20 "s16", "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S16_S21 "s16", "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S16_S22 "s16", "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S16_S23 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S16_S24 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S16_S25 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25"
#define VFP_CLOBBER_S16_S26 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26"
#define VFP_CLOBBER_S16_S27 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27"
#define VFP_CLOBBER_S16_S28 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S16_S29 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S16_S30 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S16_S31 "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24",  \
                          "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S17_S18 "s17", "s18"
#define VFP_CLOBBER_S17_S19 "s17", "s18", "s19"
#define VFP_CLOBBER_S17_S20 "s17", "s18", "s19", "s20"
#define VFP_CLOBBER_S17_S21 "s17", "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S17_S22 "s17", "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S17_S23 "s17", "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S17_S24 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S17_S25 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S17_S26 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26"
#define VFP_CLOBBER_S17_S27 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27"
#define VFP_CLOBBER_S17_S28 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28"
#define VFP_CLOBBER_S17_S29 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S17_S30 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S17_S31 "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25",  \
                          "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S18_S19 "s18", "s19"
#define VFP_CLOBBER_S18_S20 "s18", "s19", "s20"
#define VFP_CLOBBER_S18_S21 "s18", "s19", "s20", "s21"
#define VFP_CLOBBER_S18_S22 "s18", "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S18_S23 "s18", "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S18_S24 "s18", "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S18_S25 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S18_S26 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S18_S27 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27"
#define VFP_CLOBBER_S18_S28 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28"
#define VFP_CLOBBER_S18_S29 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29"
#define VFP_CLOBBER_S18_S30 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S18_S31 "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",  \
                          "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S19_S20 "s19", "s20"
#define VFP_CLOBBER_S19_S21 "s19", "s20", "s21"
#define VFP_CLOBBER_S19_S22 "s19", "s20", "s21", "s22"
#define VFP_CLOBBER_S19_S23 "s19", "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S19_S24 "s19", "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S19_S25 "s19", "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S19_S26 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S19_S27 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S19_S28 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28"
#define VFP_CLOBBER_S19_S29 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29"
#define VFP_CLOBBER_S19_S30 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30"
#define VFP_CLOBBER_S19_S31 "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27",  \
                          "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S20_S21 "s20", "s21"
#define VFP_CLOBBER_S20_S22 "s20", "s21", "s22"
#define VFP_CLOBBER_S20_S23 "s20", "s21", "s22", "s23"
#define VFP_CLOBBER_S20_S24 "s20", "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S20_S25 "s20", "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S20_S26 "s20", "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S20_S27 "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S20_S28 "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S20_S29 "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29"
#define VFP_CLOBBER_S20_S30 "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30"
#define VFP_CLOBBER_S20_S31 "s20", "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28",  \
                          "s29", "s30", "s31"
#define VFP_CLOBBER_S21_S22 "s21", "s22"
#define VFP_CLOBBER_S21_S23 "s21", "s22", "s23"
#define VFP_CLOBBER_S21_S24 "s21", "s22", "s23", "s24"
#define VFP_CLOBBER_S21_S25 "s21", "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S21_S26 "s21", "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S21_S27 "s21", "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S21_S28 "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S21_S29 "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S21_S30 "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30"
#define VFP_CLOBBER_S21_S31 "s21", "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29",  \
                          "s30", "s31"
#define VFP_CLOBBER_S22_S23 "s22", "s23"
#define VFP_CLOBBER_S22_S24 "s22", "s23", "s24"
#define VFP_CLOBBER_S22_S25 "s22", "s23", "s24", "s25"
#define VFP_CLOBBER_S22_S26 "s22", "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S22_S27 "s22", "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S22_S28 "s22", "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S22_S29 "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S22_S30 "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S22_S31 "s22", "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30",  \
                          "s31"
#define VFP_CLOBBER_S23_S24 "s23", "s24"
#define VFP_CLOBBER_S23_S25 "s23", "s24", "s25"
#define VFP_CLOBBER_S23_S26 "s23", "s24", "s25", "s26"
#define VFP_CLOBBER_S23_S27 "s23", "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S23_S28 "s23", "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S23_S29 "s23", "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S23_S30 "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S23_S31 "s23", "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S24_S25 "s24", "s25"
#define VFP_CLOBBER_S24_S26 "s24", "s25", "s26"
#define VFP_CLOBBER_S24_S27 "s24", "s25", "s26", "s27"
#define VFP_CLOBBER_S24_S28 "s24", "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S24_S29 "s24", "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S24_S30 "s24", "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S24_S31 "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S25_S26 "s25", "s26"
#define VFP_CLOBBER_S25_S27 "s25", "s26", "s27"
#define VFP_CLOBBER_S25_S28 "s25", "s26", "s27", "s28"
#define VFP_CLOBBER_S25_S29 "s25", "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S25_S30 "s25", "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S25_S31 "s25", "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S26_S27 "s26", "s27"
#define VFP_CLOBBER_S26_S28 "s26", "s27", "s28"
#define VFP_CLOBBER_S26_S29 "s26", "s27", "s28", "s29"
#define VFP_CLOBBER_S26_S30 "s26", "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S26_S31 "s26", "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S27_S28 "s27", "s28"
#define VFP_CLOBBER_S27_S29 "s27", "s28", "s29"
#define VFP_CLOBBER_S27_S30 "s27", "s28", "s29", "s30"
#define VFP_CLOBBER_S27_S31 "s27", "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S28_S29 "s28", "s29"
#define VFP_CLOBBER_S28_S30 "s28", "s29", "s30"
#define VFP_CLOBBER_S28_S31 "s28", "s29", "s30", "s31"
#define VFP_CLOBBER_S29_S30 "s29", "s30"
#define VFP_CLOBBER_S29_S31 "s29", "s30", "s31"
#define VFP_CLOBBER_S30_S31 "s30", "s31"
