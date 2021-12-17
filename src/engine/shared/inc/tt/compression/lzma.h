#ifndef INC_LZMA_H
#define INC_LZMA_H

#include <tt/platform/tt_types.h>

/**
  Compress a block of data in the input buffer and returns the size of 
  compressed block. The size of input buffer is specified by length.
  
  If an errors occurs, 0 will be returned
  
  The input buffer and the output buffer can not overlap.
  
  p_level [0-9] specifies the compression ratio; 0 being worst compression, 9 being highest compression
  p_dictionarySize a compression-dictionary size; minimum should be 64k. The higher the size, the better the
  compression.
  
  Please note that lzma_compress allocates approximately p_dictionarySize * 10 bytes of memory!
*/

u32 lzma_compress(const void* p_input, u32 p_inputSize, void* p_output, u32 p_level = 9,
                  u32 p_dictionarySize = 64 * 1024);

/**
  Decompress a block of compressed data and returns the size of the 
  decompressed block. If error occurs, e.g. the compressed data is 
  corrupted or the output buffer is not large enough, then 0 (zero) 
  will be returned instead.
  
  The input buffer and the output buffer can not overlap.
 */

u32 lzma_decompress(const void* p_input, u32 p_inputSize, void* p_output, u32 p_outputSize); 


#endif /* INC_LZMA_H */
