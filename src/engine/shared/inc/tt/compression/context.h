#ifndef INC_TT_COMPRESSION_CONTEXT_H
#define INC_TT_COMPRESSION_CONTEXT_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace compression {

struct RLContext
{
    u8 *destp;                         // Write-destination pointer:                     4B
    s32 destCount;                     // Remaining size to write:                     4B
    s32 forceDestCount;                // Forcibly set the decompression size             4B
    u16 length;                        // Remaining size of continuous write:      2B
    u8  flags;                         // Compression flag:                      1B
    u8  headerSize;                    // Size of header being read             1B
};


struct LZContext
{
    u8 *destp;                         // Write-destination pointer:                     4B
    s32 destCount;                     // Remaining size to write:                     4B
    s32 forceDestCount;                // Forcibly set the decompression size             4B
    s32 length;                        // Remaining length of continuous write:              4B
    u8  lengthFlg;                     // 'length' obtained flag:              1B
    u8  flags;                         // Compression flag:                      1B
    u8  flagIndex;                     // Current compression flag index:  1B
    u8  headerSize;                    // Size of header being read:             1B
    u8  exFormat;                      // LZ77 compression extension option:          1B
    u8  padding_[3];                   //                                 3B
                                       //                             Total is 24B
};


struct HuffContext
{
    u8 *destp;                         // Write-destination pointer:                     4B
    s32 destCount;                     // Remaining size to write:                     4B
    s32 forceDestCount;                // Forcibly set the decompression size:             4B
    u8 *treep;                         // Huffman encoding table, current pointer: 4B
    u32 srcTmp;                        // Data being read:                   4B
    u32 destTmp;                       // Data being decoded:                     4B
    s16 treeSize;                      // Size of Huffman encoding table:             2B
    u8  srcTmpCnt;                     // Size of data being read:             1B
    u8  destTmpCnt;                    // Number of bits that have been decoded:                     1B
    u8  bitSize;                       // Size of encoded bits:                     1B
    u8  headerSize;                    // Size of header being read:             1B
    u8  padding_[2];                   //                                        2B
    u8  tree[0x200];                   // Huffman encoding table:                 512B (32B is enough for 4-bit encoding, but allocated enough for 8-bit encoding)
                                       //                                   Total = 544B  (60B sufficient if 4-bit encoding)
};


struct CompressionHeader
{
    u8  compType;   // Compression type
    u8  compParam;  // Compression parameter
    u8  padding_[2];
    u32 destSize;   // Expanded size
};

// Namespace end
}
}


#endif // !defined(INC_TT_COMPRESSION_CONTEXT_H)
