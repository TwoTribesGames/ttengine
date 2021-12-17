#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/compression/compression.h>
#include <tt/compression/fastlz.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace compression {

//--------------------------------------------------------------------------------------------------
// Declaration of internal helpers

u32 CXGetUncompressedSize(const void* srcp);
void CXUncompressRL(const void* srcp, void* destp);
void CXUncompressLZ(const void* srcp, void* destp);
void CXUncompressHuffman(const void* srcp, void* destp);
void CXUnfilterDiff(const void* srcp, void* destp);
void CXUncompressAny(const void* srcp, void* destp);
void CXInitUncompContextRL(RLContext* context, void* dest);
void CXInitUncompContextLZ(LZContext* context, void* dest);
void CXInitUncompContextHuffman(HuffContext* context, void* dest);
s32 CXReadUncompRL(RLContext* context, const void* data, u32 len);
s32 CXReadUncompLZ(LZContext* context, const void* data, u32 len);
s32 CXReadUncompHuffman(HuffContext* context, const void* data, u32 len);

u32 CXCompressLZImpl(const u8 *srcp, u32 size, u8 *dstp, void *work, bool exFormat);

inline u32 CXCompressLZEx(const u8 *srcp, u32 size, u8 *dstp, void* work)
{
	return CXCompressLZImpl( srcp, size, dstp, work, true );
}
inline u32 CXCompressLZ(const u8 *srcp, u32 size, u8 *dstp, void *work)
{
	return CXCompressLZImpl( srcp, size, dstp, work, false );
}

u32 CXCompressRL( const u8 *srcp, u32 size, u8 *dstp );

u32 CXCompressHuffman( const u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize, void *work );


#define CX_LZ_COMPRESS_WORK_SIZE   ( (4096 + 256 + 256) * sizeof(s16) )
#define CX_HUFFMAN_COMPRESS_WORK_SIZE   ( 12288 + 512 + 1536 )

static inline u32 CXiConvertEndian_( u32 x )
{
#if defined( CX_PLATFORM_IS_BIGENDIAN )
    // Reflect the endianness on a big-endian platform
    return ((x & 0xFF000000) >> 24) |
           ((x & 0x00FF0000) >>  8) |
           ((x & 0x0000FF00) <<  8) |
           ((x & 0x000000FF) << 24);
#else
    return x;
#endif
}

static inline u16 CXiConvertEndian16_( u16 x )
{
#if defined( CX_PLATFORM_IS_BIGENDIAN )
    // Reflect the endianness on a big-endian platform
    return (u16)( ((x & 0xFF00) >> 8) |
                  ((x & 0x00FF) << 8) );
#else
    return x;
#endif
}


s32  getPackedBits(void* p_buffer, s32 p_index, s32 p_offset, s32 p_bits);
void setPackedBits(void* p_buffer, s32 p_index, s32 p_offset, s32 p_bits, s32 p_value);


//--------------------------------------------------------------------------------------------------
// Public functions

s32 getPackedValue(void* p_buffer, s32 p_bits, s32 p_index)
{
	s32 start = (p_index * p_bits) / 8;
	s32 offset = (p_index * p_bits) % 8;
	s32 todo = p_bits;
	s32 value = 0;
	
	while(todo > 0)
	{
		s32 do_now = std::min(8 - offset, todo);
		value |= getPackedBits(p_buffer, start, offset, do_now) << (p_bits - todo);
		++start;
		todo -= do_now;
		offset = 0;
	}
	return value;
}


void setPackedValue(void* p_buffer, s32 p_bits, s32 p_index, s32 p_value)
{
	s32 start = (p_index * p_bits) / 8;
	s32 offset = (p_index * p_bits) % 8;
	s32 todo = p_bits;
	
	while (todo > 0)
	{
		s32 do_now = std::min(8 - offset, todo);
		setPackedBits(p_buffer, start, offset, do_now, p_value &((1 << do_now) - 1));
		todo -= do_now;
		++start;
		p_value >>= do_now;
		offset = 0;
	}
}


u32 getUncompressedSize(const void* p_src)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return 0;
	}
	
	return CXGetUncompressedSize(p_src);
}


CompressionType getCompressionType(const void* p_src)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return static_cast<CompressionType>(0);
	}
	
	return static_cast<CompressionType>(*(u8*)p_src & 0xF0);
}


void uncompressAny(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	const CompressionType type = getCompressionType(p_src);
	switch (type)
	{
	case Compression_None:
		// No compression, just copy
		uncompressNone(p_src, p_dst);
		break;
		
	case Compression_RL:
		// Run-length compressed data
		uncompressRL(p_src, p_dst);
		break;
		
	case Compression_LZ:
		// LZ77 compressed data
		uncompressLZ(p_src, p_dst);
		break;
		
	case Compression_FastLZ:
		// FastLZ compressed data
		uncompressFastLZ(p_src, p_dst);
		break;
		
	case Compression_Huffman:
		// Huffman compressed data
		uncompressHuff(p_src, p_dst);
		break;
		
	case Compression_Dif:
		// Difference filter
		unfilterDiff(p_src, p_dst);
		break;
		
	default:
		TT_PANIC("Unsupported compressed format: %d. Do not know how to uncompress.", type);
		break;
	}
}


void uncompressNone(const void* p_src, void* p_dst)
{
	const u8* buffer = static_cast<const u8*>(p_src);
	size_t bufsize = 4;
	u32 size = code::bufferutils::get<u32>(buffer, bufsize) >> 8;
	if (size == 0)
	{
		bufsize = 4;
		size = code::bufferutils::get<u32>(buffer, bufsize);
	}
	mem::copy8(p_dst, buffer, static_cast<mem::size_type>(size));
}


void uncompressRL(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	CXUncompressRL(p_src, p_dst);
}


void uncompressLZ(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	CXUncompressLZ(p_src, p_dst);
}


void uncompressFastLZ(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	const u8* sourceBytes      = static_cast<const u8*>(p_src);
	u32       uncompressedSize = CXiConvertEndian_(*(u32*)sourceBytes) >> 8;
	
	sourceBytes += 4;
	
	if (uncompressedSize == 0)
	{
		uncompressedSize = CXiConvertEndian_(*(u32*)sourceBytes);
		sourceBytes += 4;
	}
	
	const u32 compressedSize = CXiConvertEndian_(*(u32*)sourceBytes);
	sourceBytes += 4;
	
	fastlz_decompress(sourceBytes, static_cast<int>(compressedSize),
	                  p_dst,       static_cast<int>(uncompressedSize));
}


void uncompressHuff(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	CXUncompressHuffman(p_src, p_dst);
}


void unfilterDiff(const void* p_src, void* p_dst)
{
	if (p_src == 0)
	{
		TT_PANIC("No source specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	if ((reinterpret_cast<uintptr>(p_src) & 3) != 0)
	{
		TT_PANIC("Source must be 4 byte aligned.");
		return;
	}
	
	CXUnfilterDiff(p_src, p_dst);
}


void initUncompContextRL(RLContext* p_context, u8* p_dst, CompressionHeader*)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	CXInitUncompContextRL(p_context, p_dst);
}


void initUncompContextLZ(LZContext* p_context, u8* p_dst, CompressionHeader*)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	CXInitUncompContextLZ(p_context, p_dst);
}


void initUncompContextHuff(HuffContext* p_context, u8* p_dst, CompressionHeader*)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return;
	}
	
	if (p_dst == 0)
	{
		TT_PANIC("No destination specified.");
		return;
	}
	
	CXInitUncompContextHuffman(p_context, p_dst);
}


s32 readUncompRL(RLContext* p_context, const u8* p_data, u32 p_len)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return 0;
	}
	
	if (p_data == 0)
	{
		TT_PANIC("No data specified.");
		return 0;
	}
	
	return CXReadUncompRL(p_context, p_data, p_len);
}


s32 readUncompLZ(LZContext* p_context, const u8* p_data, u32 p_len)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return 0;
	}
	
	if (p_data == 0)
	{
		TT_PANIC("No data specified.");
		return 0;
	}
	
	return CXReadUncompLZ(p_context, p_data, p_len);
}


s32 readUncompHuff(HuffContext* p_context, const u8* p_data, u32 p_len)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return 0;
	}
	
	if (p_data == 0)
	{
		TT_PANIC("No data specified.");
		return 0;
	}
	
	return CXReadUncompHuffman(p_context, p_data, p_len);
}


bool isFinishedUncompRL(const RLContext* p_context)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return true;
	}
	
	return (p_context->destCount <= 0 && p_context->headerSize <= 0);
}


bool isFinishedUncompLZ(const LZContext* p_context)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return true;
	}
	
	return (p_context->destCount <= 0 && p_context->headerSize <= 0);
}


bool isFinishedUncompHuff(const HuffContext* p_context)
{
	if (p_context == 0)
	{
		TT_PANIC("No context specified.");
		return true;
	}
	
	return (p_context->destCount <= 0 && p_context->headerSize <= 0);
}


u32 compressNone(const u8* p_src, u32 p_size, u8* p_dst)
{
	u8* buffer = p_dst;
	size_t size = static_cast<size_t>(p_size + 8);
	u32 ret = p_size;
	
	u32 header = 0;
	if ((p_size & 0xFF000000) != 0)
	{
		code::bufferutils::put(header, buffer, size);
		code::bufferutils::put(p_size, buffer, size);
		p_size += 8;
	}
	else
	{
		code::bufferutils::put(p_size << 8, buffer, size);
		p_size += 4;
	}
	
	mem::copy8(buffer, p_src, static_cast<mem::size_type>(p_size));
	return ret;
}


u32 compressRL(const u8* p_src, u32 p_size, u8* p_dst)
{
	TT_ASSERTMSG(p_src != 0, "No source specified.");
	TT_ASSERTMSG(p_dst != 0, "No destination specified.");
	
	return CXCompressRL(p_src, p_size, p_dst);
}


u32 compressLZ(const u8* p_src, u32 p_size, u8* p_dst, bool p_ex, bool p_fast)
{
	(void)p_fast;
	TT_ASSERTMSG(p_src != 0, "No source specified.");
	TT_ASSERTMSG(p_dst != 0, "No destination specified.");
	
	u8* work = new u8[CX_LZ_COMPRESS_WORK_SIZE];
	
	u32 result;
	if (p_ex)
	{
		result = CXCompressLZEx(p_src, p_size, p_dst, work);
	}
	else
	{
		result = CXCompressLZ(p_src, p_size, p_dst, work);
	}
	delete[] work;
	
	return result;
}


u32 compressFastLZ(const u8* p_src, u32 p_size, u8* p_dst)
{
	TT_ASSERTMSG(p_src != 0, "No source specified.");
	TT_ASSERTMSG(p_dst != 0, "No destination specified.");
	
	// Write the uncompressed size and compression type
	if (p_size < (1 << 24))
	{
		*(u32*)p_dst = CXiConvertEndian_(p_size << 8 | Compression_FastLZ);  // Data header
		p_dst += 4;
	}
	else
	{
		// Use extended header if the size is larger than 24 bits
		*(u32*)p_dst = CXiConvertEndian_(Compression_FastLZ);  // Data header
		p_dst += 4;
		*(u32*)p_dst = CXiConvertEndian_(p_size);              // Size extended header
		p_dst += 4;
	}
	
	// Skip 4 bytes: we will place the compressed size here after compression
	// (this size is needed when uncompressing: the fastlz library requires it as input)
	u32* compressedSizeLocation = (u32*)p_dst;
	p_dst += 4;
	
	// Compress the data
	const int compressedSize = fastlz_compress(p_src, static_cast<int>(p_size), p_dst);
	if (compressedSize > static_cast<int>(p_size))
	{
		// Data got larger after compression: return 0 to indicate this
		return 0;
	}
	
	// Save the compressed size
	*compressedSizeLocation = CXiConvertEndian_(static_cast<u32>(compressedSize));
	
	return static_cast<u32>(compressedSize);
}


u32 compressHuff(const u8* p_src, u32 p_size, u8* p_dst, bool p_4bit)
{
	TT_ASSERTMSG(p_src != 0, "No source specified.");
	TT_ASSERTMSG(p_dst != 0, "No destination specified.");
	
	u8* work = new u8[CX_HUFFMAN_COMPRESS_WORK_SIZE];
	
	u32 result = CXCompressHuffman(p_src, p_size, p_dst, u8(p_4bit ? 4 : 8), work);
	
	delete[] work;
	
	return result;
}


//--------------------------------------------------------------------------------------------------
// Implementation of internal helpers

// FIXME: Remove this duplicate enum: use tt::compression::CompressionType instead!
//---- Compression type
typedef enum
{
    CX_COMPRESSION_LZ           = 0x10,     // LZ77
    CX_COMPRESSION_HUFFMAN      = 0x20,     // Huffman
    CX_COMPRESSION_RL           = 0x30,     // Run Length
    //CX_COMPRESSION_LH           = 0x40,     // LH(LZ77+Huffman)
    //CX_COMPRESSION_LRC          = 0x50,     // LRC(LZ77+RangeCoder)
    CX_COMPRESSION_DIFF         = 0x80,     // Differential filter

    CX_COMPRESSION_TYPE_MASK    = 0xF0,
    CX_COMPRESSION_TYPE_EX_MASK = 0xFF
}
CXCompressionType;

#define CX_ERR_SUCCESS              0
#define CX_ERR_UNSUPPORTED          -1
#define CX_ERR_SRC_SHORTAGE         -2
#define CX_ERR_SRC_REMAINDER        -3
#define CX_ERR_DEST_OVERRUN         -4
#define CX_ERR_ILLEGAL_TABLE        -5


//======================================================================
//          Expanding compressed data
//======================================================================

/*---------------------------------------------------------------------------*
  Name:         CXGetUncompressedSize

  Description:  Gets the data size after decompression.
                This function can be used for data in all compression formats handled by CX.

  Arguments:    srcp :  Starting address of the compressed data

  Returns:      Data size after decompression
 *---------------------------------------------------------------------------*/
u32 CXGetUncompressedSize( const void *srcp )
{
    u32 size = CXiConvertEndian_( *(u32*)srcp ) >> 8;
    if ( size == 0 )
    {
        size = CXiConvertEndian_( *((u32*)srcp + 1) );
    }
    return size;
}


/*---------------------------------------------------------------------------*
  Name:         CXUncompressRL

  Description:  8-bit decompression of run-length compressed data

  - Decompresses run-length compressed data, writing in 8-bit units.
  - Use 4 byte alignment for the source address.

  - Data header
      u32 :4  :                Reserved
          compType:4          Compression type (=3)
          destSize:24 :        Data size after decompression
  
  - Flag data format
      u8  length:7 :           Decompressed data length - 1 (When not compressed)
                              Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
          flag:1 :             (0, 1) = (not compressed, compressed)
  
  Arguments:    *srcp   Source address
                *destp  Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CXUncompressRL( const void *srcp, void *destp )
{
    const u8 *pSrc  = static_cast<const u8*>(srcp);
    u8       *pDst  = static_cast<u8*>(destp);
    u32      destCount = CXiConvertEndian_( *(u32*)pSrc ) >> 8;
    pSrc += 4;
    
    if ( destCount == 0 )
    {
        destCount = CXiConvertEndian_( *(u32*)pSrc );
        pSrc += 4;
    }
    
    while ( destCount > 0 )
    {
        u8  flags  = *pSrc++;
        u32 length = flags & 0x7fU;
        if ( !(flags & 0x80) )
        {   
            length++;
            if ( length > destCount )
            // Measures for buffer overrun when invalid data is decompressed.
            {
                length = destCount;
            }
            
            destCount -= length;
            do
            {
                *pDst++ = *pSrc++;
            } while ( --length > 0 );
        }
        else
        {
            u8 srcTmp;
            
            length    += 3;
            if ( length > destCount )
            // Measures for buffer overrun when invalid data is decompressed.
            {
                length = destCount;
            }
            
            destCount -= length;
            srcTmp    = *pSrc++;
            do
            {
                *pDst++ =  srcTmp;
            } while ( --length > 0 );
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         CXUncompressLZ
  
  Description:  8-bit decompression of LZ77 compressed data
  
  * Expands LZ77 compressed data, and writes it in 8-bit units.
  - Use 4 byte alignment for the source address.
  
  - Data header
      u32 :4  :                Reserved
          compType:4 :         Compression type( = 1)
          destSize:24 :        Data size after decompression
  
  - Flag data format
      u8  flags :              Compression/no compression flag
                              (0, 1) = (not compressed, compressed)
  - Code data format (Big Endian)
      u16 length:4 :           Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
          offset:12 :          Match data offset - 1
  
  Arguments:    *srcp   Source address
                *destp  Destination address
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void CXUncompressLZ( const void *srcp, void *destp )
{
    const u8* pSrc      = static_cast<const u8*>(srcp);
    u8*       pDst      = static_cast<u8*>(destp);
    u32       destCount = CXiConvertEndian_( *(u32 *)pSrc ) >> 8;
    bool      exFormat  = (*pSrc & 0x0F) != 0;
    
    pSrc += 4;
    
    if ( destCount == 0 )
    {
        destCount = CXiConvertEndian_( *(u32*)pSrc );
        pSrc += 4;
    }
    
    while ( destCount > 0 )
    {
        u32 i;
        u32 flags = *pSrc++;
        for ( i = 0; i < 8; ++i )
        {
            if ( !(flags & 0x80) )
            {
                *pDst++ = *pSrc++;
                destCount--;
            }
            else
            {
                s32 length = (*pSrc >> 4);
                s32 offset;
                
                if ( ! exFormat )
                {
                    length += 3;
                }
                else
                {
                    // LZ77 extended format
                    if ( length == 1 )
                    {
                        length =  (*pSrc++ & 0x0F) << 12;
                        length |= (*pSrc++) << 4;
                        length |= (*pSrc >> 4);
                        length += 0xFF + 0xF + 3;
                    }
                    else if ( length == 0 )
                    {
                        length =  (*pSrc++ & 0x0F) << 4;
                        length |= (*pSrc >> 4);
                        length += 0xF + 2;
                    }
                    else
                    {
                        length += 1;
                    }
                }
                offset = (*pSrc++ & 0x0f) << 8;
                offset = (offset | *pSrc++) + 1;
                if ( length > static_cast<s32>(destCount) )
                // Measures for buffer overrun when invalid data is decompressed.
                {
                    length = (s32)destCount;
                }
                destCount -= length;
                do
                {
                    u8 val = pDst[ -offset ];
                    *pDst++ = val;
                    //*pDst++ = pDst[ -offset ];
                } while ( --length > 0 );
            }
            if ( destCount <= 0 )
            {
                break;
            }
            flags <<= 1;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         CXUncompressHuffman
  
  Description:  Decompression of Huffman compressed data
  
  * Decompresses Huffman compressed data, writing in 32 bit units.
  - Use 4 byte alignment for the source address.
  - Use 4 byte alignment for the destination address.
  - The target decompression buffer size must be prepared in 4 byte multiples.
  
  - Data header
      u32 bitSize:4 :          Bit size of 1 data (Normally 4|8)
          compType:4          Compression type (=2)
          destSize:24 :        Data size after decompression
  
  - Tree table
      u8           treeSize :       tree table size/2 - 1
      TreeNodeData nodeRoot :       Root node
  
      TreeNodeData nodeLeft :       Root left node
      TreeNodeData nodeRight :      Root right node
  
      TreeNodeData nodeLeftLeft :   Left left node
      TreeNodeData nodeLeftRight :  Left right node
  
      TreeNodeData nodeRightLeft :  Right left node
      TreeNodeData nodeRightRight : Right right node
  
              .
              .
  
    The compressed data itself follows
  
  - TreeNodeData structure
      u8  nodeNextOffset:6 :   Offset to the next node data - 1 (2 byte units)
          rightEndFlag:1 :     Right node end flag
          leftEndzflag:1 :     Left node end flag
                              When the end flag is set
                              There is data in next node
  
  Arguments:    *srcp   Source address
                *destp  Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CXUncompressHuffman( const void *srcp, void *destp )
{
#define TREE_END 0x80
    const u32 *pSrc          = static_cast<const u32*>(srcp);
    u32       *pDst          = static_cast<u32*>(destp);
    s32       destCount      = (s32)( CXiConvertEndian_( *pSrc ) >> 8 );
    u8        *treep         = (destCount != 0)? ((u8*)pSrc + 4) : ((u8*)pSrc + 8);
    u8        *treeStartp    = treep + 1;
    u32       dataBit        = *(u8*)pSrc & 0x0FU;
    u32       destTmp        = 0;
    u32       destTmpCount   = 0;
    u32       destTmpDataNum = 4 + ( dataBit & 0x7 );
    
    if ( destCount == 0 )
    {
        destCount = (s32)( CXiConvertEndian_( *(pSrc + 1) ) );
    }
    
    pSrc  = (u32*)( treep + ((*treep + 1) << 1) );
    treep = treeStartp;
    
    while ( destCount > 0 )
    {
        s32 srcCount = 32;
        u32 srcTmp   = CXiConvertEndian_( *pSrc++ );      // Endian strategy
        while ( --srcCount >= 0 )
        {
            u32 treeShift = (srcTmp >> 31) & 0x1;
            u32 treeCheck = *treep;
            treeCheck <<= treeShift;
            treep = (u8*)( (((uintptr)treep >> 1) << 1) + (((*treep & 0x3f) + 1) << 1) + treeShift );
            if ( treeCheck & TREE_END )
            {
                destTmp >>= dataBit;
                destTmp |= *treep << (32 - dataBit);
                treep = treeStartp;
                if ( ++destTmpCount == destTmpDataNum )
                {
                    // Over-access until the last 4-byte alignment of the decompression buffer
                    *pDst++ = CXiConvertEndian_(destTmp); // Endian strategy
                    destCount -= 4;
                    destTmpCount = 0;
                }
            }
            if ( destCount <= 0 )
            {
                break;
            }
            srcTmp <<= 1;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         CXUnfilterDiff

  Description:  8-bit decompression to restore differential filter conversion.
  
    - Decompresses a differential filter, writing in 8 bit units.
    - Use 4 byte alignment for the source address.

  Arguments:    *srcp   Source address
                *destp  Destination address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CXUnfilterDiff(const void *srcp, void *destp )
{
    const u8* pSrc = static_cast<const u8*>(srcp);
    u8*       pDst = static_cast<u8*>(destp);
    u32 bitSize    = *pSrc & 0xFU;
    s32 destCount  = (s32)( CXiConvertEndian_( *(u32*)pSrc ) >> 8 );
    u32 sum = 0;
    
    pSrc += 4;
    
    if ( bitSize != 1 )
    {
        // Difference calculation in units of 8 bits
        do 
        {
            u8 tmp = *(pSrc++);
            destCount--;
            sum += tmp;
            *(pDst++) = (u8)sum;
        } while ( destCount > 0 );
    }
    else
    {
        // Difference calculation in units of 16 bits
        do 
        {
            u16 tmp = CXiConvertEndian16_( *(u16*)pSrc );
            pSrc += 2;
            destCount -= 2;
            sum += tmp;
            *(u16*)pDst = CXiConvertEndian16_( (u16)sum );
            pDst += 2;
        } while ( destCount > 0 );
    }
}


/*---------------------------------------------------------------------------*
  Name        : CXInitUncompContextRL

  Description : Initializes the streaming decompression context for run-length compressed data.
                

  Arguments   : context: Pointer to the run-length uncompressed context
                dest: Destination address for uncompressed data

  Returns     : Can get the data size after decompression.

 *---------------------------------------------------------------------------*/
void CXInitUncompContextRL( RLContext* context, void* dest )
{
    context->destp      = (u8*)dest;
    context->destCount  = 0;
    context->flags      = 0;
    context->length     = 0;
    context->headerSize = 8;
    context->forceDestCount = 0;
}


/*---------------------------------------------------------------------------*
  Name        : CXInitUncompContextLZ

  Description : Initializes the streaming decompression context for LZ-compressed data.

  Arguments   : context: Pointer to the LZ-uncompressed context
                dest:    Destination address for uncompressed data
                header:  Pointer to the start data for the compressed data

 *---------------------------------------------------------------------------*/
void CXInitUncompContextLZ( LZContext *context, void* dest )
{
    context->destp      = (u8*)dest;
    context->destCount  = 0;
    context->flags      = 0;
    context->flagIndex  = 0;
    context->length     = 0;
    context->lengthFlg  = 3;
    context->headerSize = 8;
    context->exFormat   = 0;
    context->forceDestCount = 0;
}


/*---------------------------------------------------------------------------*
  Name        : CXInitUncompContextHuffman

  Description : Initializes the streaming decompression context for Huffman-compressed data.

  Arguments   : context: Pointer to the Huffman-uncompressed context
                dest:    Destination address for uncompressed data
                header:  Pointer to the start data for the compressed data

 *---------------------------------------------------------------------------*/
void CXInitUncompContextHuffman( HuffContext *context, void* dest )
{
    context->destp      = (u8*)dest;
    context->destCount  = 0;
    context->bitSize    = 0;
    context->treeSize   = -1;
    context->treep      = &context->tree[ 0 ];
    context->destTmp    = 0;
    context->destTmpCnt = 0;
    context->srcTmp     = 0;
    context->srcTmpCnt  = 0;
    context->headerSize = 8;
    context->forceDestCount = 0;
}


/*---------------------------------------------------------------------------*
  Name:         CXiReadHeader

  Description:  Header parsing

  Arguments:    headerSize    Pointer to the remaining size of the header to be read
                *destCount    Pointer to the uncompressed data size
                srcp          Pointer to a buffer containing the header information
                srcSize       Pointer to the size of the buffer containing the header information
                forceDestSize Uncompressed data size (if 0, use the binary header information as is)

  Returns:      Size of the loaded source data
 *---------------------------------------------------------------------------*/
static inline u32 CXiReadHeader( u8* headerSize, s32 *destCount, const u8* srcp, u32 srcSize, s32 forceDestSize )
{
    u32 readLen = 0;
    while ( *headerSize > 0 )
    {
        --(*headerSize);
        if ( *headerSize <= 3 )
        {
            *destCount |= (*srcp << ((3 - *headerSize) * 8));
        }
        else if ( *headerSize <= 6 )
        {
            *destCount |= (*srcp << ((6 - *headerSize) * 8));
        }
        ++srcp;
        ++readLen;
        if ( *headerSize == 4 && *destCount > 0 )
        {
            *headerSize = 0;
        }
        if ( --srcSize == 0 && *headerSize > 0 )
        {
            return readLen;
        }
    }
    
    if ( ( forceDestSize > 0          ) &&
         ( forceDestSize < *destCount ) )
    {
        *destCount = forceDestSize;
    }
    return readLen;
}


/*---------------------------------------------------------------------------*
  Name        : CXReadUncompRL

  Description : This function performs streaming decompression of run-length compressed data.
                Data is written in units of 8 bits. Data cannot be directly uncompressed to VRAM.

  Arguments   : context: Pointer to the run-length uncompressed context
                data: Pointer to the next data
                len: Data size

  Returns     : Size of remaining uncompressed data.
                Returns a negative error code if failed.

 *---------------------------------------------------------------------------*/
s32 CXReadUncompRL( RLContext *context, const void* data, u32 len )
{
    const u8* srcp = (const u8*)data;
    u8  srcTmp;
    
    // Header parsing
    if ( context->headerSize > 0 )
    {
        u32 read_len;
        if ( context->headerSize == 8 )
        {
            if ( (*srcp & CX_COMPRESSION_TYPE_MASK) != CX_COMPRESSION_RL )
            {
                return CX_ERR_UNSUPPORTED;
            }
            if ( (*srcp & 0x0F ) != 0 )
            {
                return CX_ERR_UNSUPPORTED;
            }
        }
        read_len = CXiReadHeader( &context->headerSize, &context->destCount, srcp, len, context->forceDestCount );
        srcp += read_len;
        len  -= read_len;
        if ( len == 0 )
        {
            return (context->headerSize == 0)? context->destCount : -1;
        }
    }
    
    while ( context->destCount > 0 )
    {
        // Process if length > 0.
        if ( ! (context->flags & 0x80) )
        // Uncompressed data has a length not equal to 0
        {
            while ( context->length > 0 )
            {
                *context->destp++ = *srcp++;
                context->length--;
                context->destCount--;
                len--;
                // End when the prepared buffer has been read in full
                if ( len == 0 )
                {
                    return context->destCount;
                }
            }
        }
        else if ( context->length > 0 )
        // Compressed data has a length not equal to 0
        {
            srcTmp = *srcp++;
            len--;
            while ( context->length > 0 )
            {
                *context->destp++ = srcTmp;
                context->length--;
                context->destCount--;
            }
            if ( len == 0 )
            {
                return context->destCount;
            }
        }
        
        // Reading the flag byte
        context->flags = *srcp++;
        len--;
        context->length = (u16)(context->flags & 0x7F);
        if ( context->flags & 0x80 )
        {
            context->length += 3;
        }
        else
        {
            context->length += 1;
        }
        
        if ( context->length > context->destCount )
        // A buffer overrun handler for when invalid data is decompressed.
        {
            if ( context->forceDestCount == 0 )
            {
                return CX_ERR_DEST_OVERRUN;
            }
            context->length = (u16)context->destCount;
        }
        if ( len == 0 )
        {
            return context->destCount;
        }
    }
    
    // Processing to perform in the event that context->destCount  == 0
    if ( (context->forceDestCount == 0) && (len > 32) )
    {
        return CX_ERR_SRC_REMAINDER;
    }
    return 0;
}


/*---------------------------------------------------------------------------*
  Name        : CXReadUncompLZ

  Description : This function performs streaming decompression of LZ-compressed data.
                Data is written in units of 8 bits. Data cannot be directly uncompressed to VRAM.

  Arguments   : context: Pointer to the LZ-uncompressed context
                data:    Pointer to the next data
                len:     Data size

  Returns     : Size of remaining uncompressed data.
                Returns a negative error code if failed.

 *---------------------------------------------------------------------------*/
s32 CXReadUncompLZ( LZContext *context, const void* data, u32 len )
{
    const u8* srcp = (const u8*)data;
    s32 offset;
    
    // Header parsing
    if ( context->headerSize > 0 )
    {
        u32 read_len;
        // Process the first byte
        if ( context->headerSize == 8 )
        {
            if ( ( *srcp & CX_COMPRESSION_TYPE_MASK ) != CX_COMPRESSION_LZ )
            {
                return CX_ERR_UNSUPPORTED;
            }
            // Record as an LZ compression parameter
            context->exFormat = (u8)( *srcp & 0x0F );
            if ( (context->exFormat != 0x0) && (context->exFormat != 0x1) )
            {
                return CX_ERR_UNSUPPORTED;
            }
        }
        read_len = CXiReadHeader( &context->headerSize, &context->destCount, srcp, len, context->forceDestCount );
        srcp += read_len;
        len  -= read_len;
        if ( len == 0 )
        {
            return (context->headerSize == 0)? context->destCount : -1;
        }
    }
    
    while ( context->destCount > 0 )
    {
        while ( context->flagIndex > 0 )
        {
            if ( len == 0 )
            {
                return context->destCount;
            }
            
            if ( ! (context->flags & 0x80) )
            // Process for non-compressed data
            {
                *context->destp++ = *srcp++;
                context->destCount--;
                len--;
            }
            else
            // Process for compressed data
            {
                while ( context->lengthFlg > 0 )
                {
                    --context->lengthFlg;
                    if ( ! context->exFormat )
                    {
                        context->length  = *srcp++;
                        context->length += (3 << 4);
                        context->lengthFlg = 0;
                    }
                    else
                    {
                        switch ( context->lengthFlg )
                        {
                        case 2:
                            {
                                context->length = *srcp++;
                                if ( (context->length >> 4) == 1 )
                                {
                                    // Read two more bytes
                                    context->length =  (context->length & 0x0F) << 16;
                                    context->length += ( (0xFF + 0xF + 3) << 4 );
                                }
                                else if ( (context->length >> 4) == 0 )
                                {
                                    // Read one more byte
                                    context->length =  (context->length & 0x0F) << 8;
                                    context->length += ( (0xF + 2) << 4 );
                                    context->lengthFlg = 1;
                                }
                                else
                                {
                                    context->length += (1 << 4);
                                    context->lengthFlg = 0;
                                }
                            }
                            break;
                        case 1:
                            {
                                context->length += (*srcp++ << 8);
                            }
                            break;
                        case 0:
                            {
                                context->length += *srcp++;
                            }
                            break;
                        }
                    }
                    if ( --len == 0 )
                    {
                        return context->destCount;
                    }
                }
                
                offset = (context->length & 0xF) << 8;
                context->length = context->length >> 4;
                offset = (offset | *srcp++) + 1;
                len--;
                context->lengthFlg = 3;
                
                // A buffer overrun handler for when invalid data is decompressed.
                if ( context->length > context->destCount )
                {
                    if ( context->forceDestCount == 0 )
                    {
                        return CX_ERR_DEST_OVERRUN;
                    }
                    context->length = context->destCount;
                }
                // Copy a length amount of data located at the offset position
                while ( context->length > 0 )
                {
                    *context->destp = context->destp[ -offset ];
                    context->destp++;
                    context->destCount--;
                    context->length--;
                }
            }
            
            if ( context->destCount == 0 )
            {
                goto out;
            }
            context->flags <<= 1;
            context->flagIndex--;
        }
        
        if ( len == 0 )
        {
            return context->destCount;
        }
        // Read a new flag
        context->flags     = *srcp++;
        context->flagIndex = 8;
        len--;
    }
    
out:
    // Processing to perform in the event that context->destCount  == 0
    if ( (context->forceDestCount == 0) && (len > 32) )
    {
        return CX_ERR_SRC_REMAINDER;
    }
    return 0;
}


// Get the next node in the Huffman signed table
static inline u8* GetNextNode( const u8* pTree, u32 select )
{
    return (u8*)( ((uintptr)pTree & ~0x1) + ( ( (*pTree & 0x3F) + 1 ) * 2 ) + select );
}


/*---------------------------------------------------------------------------*
  Name:         CXiVerifyHuffmanTable_

  Description:  Huffman table integrity check

  Arguments:    Pointer to the Huffman table

  Returns:      TRUE when the table is valid
                FALSE when the table is invalid
 *---------------------------------------------------------------------------*/
bool CXiVerifyHuffmanTable_(const void* pTable, u8 bit)
{
    const u32 FLAGS_ARRAY_NUM = 512 / 8; /* 64 byte */
    u8* treep = (u8*)pTable;
    u8* treeStartp = treep + 1;
    u32 treeSize   = *treep;
    u8* treeEndp   = (u8*)pTable + (treeSize + 1) * 2;
    u32 i;
    u8  end_flags[ FLAGS_ARRAY_NUM ];
    u32 idx;
    
    for ( i = 0; i < FLAGS_ARRAY_NUM; i++ )
    {
        end_flags[ i ] = 0;
    }
    
    if ( bit == 4 )
    {
        if ( treeSize >= 0x10 )
        {
            return false;
        }
    }
    
    idx = 1;
    treep = treeStartp;
    while ( treep < treeEndp )
    {
        if ( (end_flags[ idx / 8 ] & (1 << (idx % 8) )) == 0 )
        {
            u32  offset = (u32)( ( (*treep & 0x3F) + 1 ) << 1);
            u8*  nodep  = (u8*)( (((uintptr)treep >> 1) << 1) + offset );
            
            // Skip data added at the end for alignment.
            if ( *treep == 0 && idx >= (treeSize * 2) )
            {
                goto next;
            }
            
            if ( nodep >= treeEndp )
            {
                return false;
            }
            if ( *treep & 0x80 )
            {
                u32 left = (idx & ~0x1) + offset;
                end_flags[ left / 8 ] |= (u8)( 1 << (left % 8) );
            }
            if ( *treep & 0x40 )
            {
                u32 right = (idx & ~0x1) + offset + 1;
                end_flags[ right / 8 ] |= (u8)( 1 << (right % 8) );
            }
        }
    next:
        ++idx;
        ++treep;
    }
    return true;
}


/*---------------------------------------------------------------------------*
  Name        : CXReadUncompHuffman

  Description : This function performs streaming decompression of Huffman-compressed data.
                Returns a negative error code if failed.

  Arguments   : context: Pointer to the Huffman-uncompressed context
                data:    Pointer to the next data
                len:     Data size

  Returns     : Size of remaining uncompressed data.
                Returns a negative error code if failed.

 *---------------------------------------------------------------------------*/
s32 CXReadUncompHuffman( HuffContext *context, const void* data, u32 len )
{
#define TREE_END_MASK   0x80U
    const u8* srcp = (const u8*)data;
    u32  select;
    u32  endFlag;
    
    // Header parsing
    if ( context->headerSize > 0 )
    {
        u32 read_len;
        // Process the first byte
        if ( context->headerSize == 8 )
        {
            context->bitSize = (u8)(*srcp & 0xF);
            
            if ( ( *srcp & CX_COMPRESSION_TYPE_MASK ) != CX_COMPRESSION_HUFFMAN )
            {
                return CX_ERR_UNSUPPORTED;
            }
            if ( (context->bitSize != 4) && (context->bitSize != 8) )
            {
                return CX_ERR_UNSUPPORTED;
            }
        }
        read_len = CXiReadHeader( &context->headerSize, &context->destCount, srcp, len, context->forceDestCount );
        srcp += read_len;
        len  -= read_len;
        if ( len == 0 )
        {
            return (context->headerSize == 0)? context->destCount : -1;
        }
    }
    
    // treeSize is set to -1 in CXInitUncompContextHuffman.
    // When context->treeSize is negative, the data's beginning is used.
    if ( context->treeSize < 0 )
    {
        context->treeSize = (s16)( ( *srcp + 1 ) * 2 - 1 );
        *context->treep++ = *srcp++;
        len--;
    }
    
    // Load the Huffman signed table
    while ( context->treeSize > 0 )
    {
        if ( len == 0 )
        {
            return context->destCount;
        }
        *context->treep++ = *srcp++;
        context->treeSize--;
        len--;
        if ( context->treeSize == 0 )
        {
            context->treep = &context->tree[ 1 ];
            if ( ! CXiVerifyHuffmanTable_( &context->tree[ 0 ], context->bitSize ) )
            {
                return CX_ERR_ILLEGAL_TABLE;
            }
        }
    }
    
    // Decoding process
    while ( context->destCount > 0 )
    {
        // src data is read in 4-byte units
        while ( context->srcTmpCnt < 32 )
        {
            if ( len == 0 )
            {
                return context->destCount;
            }
            context->srcTmp |= (*srcp++) << context->srcTmpCnt;
            len--;
            context->srcTmpCnt += 8;
        }
        
        // Decode the 32 bits that were loaded. After those 32 bits are processed, the next 4 bytes are read.
        while ( context->srcTmpCnt > 0 )
        {
            select  = context->srcTmp >> 31;
            endFlag = (*context->treep << select) & TREE_END_MASK;
            context->treep = GetNextNode( context->treep, select );
            context->srcTmp <<= 1;
            context->srcTmpCnt--;
            
            if ( ! endFlag )
            {
                continue;
            }
            
            // When the Huffman tree's terminal flag is set, data is stored at the end of the offset.
            // 
            context->destTmp >>= context->bitSize;
            context->destTmp |= *context->treep << ( 32 - context->bitSize );
            context->treep = &context->tree[ 1 ];
            context->destTmpCnt += context->bitSize;
            
            if ( context->destCount <= (context->destTmpCnt / 8) )
            {
                context->destTmp    >>= (32 - context->destTmpCnt);
                context->destTmpCnt = 32;
            }
            
            // Write in 4-byte units
            if ( context->destTmpCnt == 32 )
            {
                *(u32*)context->destp = CXiConvertEndian_( context->destTmp );
                context->destp     += 4;
                context->destCount -= 4;
                context->destTmpCnt = 0;
                if ( context->destCount <= 0 )
                {
                    goto out;
                }
            }
        }
    }
    
out:
    // Processing to perform in the event that context->destCount  == 0
    if ( (context->forceDestCount == 0) && (len > 32) )
    {
        return CX_ERR_SRC_REMAINDER;
    }
    return 0;
}



//===========================================================================
//  LZ Encoding
//===========================================================================

// Temporary information for LZ high-speed encoding
typedef struct
{
    u16     windowPos;                 // Top position of the history window
    u16     windowLen;                 // Length of the history window
    
    s16    *LZOffsetTable;             // Offset buffer of the history window
    s16    *LZByteTable;               // Pointer to the most recent character history
    s16    *LZEndTable;                // Pointer to the oldest character history
}
LZCompressInfo;

static void        LZInitTable( LZCompressInfo * info, void *work );
static u32         SearchLZ   ( LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength );
static void        SlideByte  ( LZCompressInfo * info, const u8 *srcp );
static inline void LZSlide    ( LZCompressInfo * info, const u8 *srcp, u32 n );


/*---------------------------------------------------------------------------*
  Name:         CXCompressLZ

  Description:  Function that performs LZ77 compression

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp:            Pointer to destination for compressed data
                                The buffer must be larger than the size of the compression source data.
                work:            Temporary buffer for compression
                                Requires a region of at least size CX_LZ_FAST_COMPRESS_WORK_SIZE.

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 CXCompressLZImpl(const u8 *srcp, u32 size, u8 *dstp, void *work, bool exFormat)
{
    u32     LZDstCount;                // Number of bytes of compressed data
    u8      LZCompFlags;               // Flag series indicating whether there is compression
    u8     *LZCompFlagsp;              // Points to memory region storing LZCompFlags
    u16     lastOffset;                // Offset to matching data (the longest matching data at the time) 
    u32     lastLength;                // Length of matching data (the longest matching data at the time)
    u8      i;
    u32     dstMax;
    LZCompressInfo info;               // Temporary LZ compression information
    const u32 MAX_LENGTH = (exFormat)? (0xFFFF + 0xFF + 0xF + 3U) : (0xF + 3U);
    
    TT_ASSERT( ((uintptr)srcp & 0x1) == 0 );
    TT_ASSERT( work != NULL );
    TT_ASSERT( size > 4 );
    
    if ( size < (1 << 24) )
    {
        *(u32 *)dstp = CXiConvertEndian_( size << 8 | CX_COMPRESSION_LZ | (exFormat? 1 : 0) );  // Data header
        dstp += 4;
        LZDstCount = 4;
    }
    else
    // Use extended header if the size is larger than 24 bits
    {
        *(u32 *)dstp = CXiConvertEndian_( CX_COMPRESSION_LZ | (exFormat? 1U : 0U) );  // Data header
        dstp += 4;
        *(u32 *)dstp = CXiConvertEndian_( size ); // Size extended header
        dstp += 4;
        LZDstCount = 8;
    }
    dstMax = size;
    LZInitTable(&info, work);
    
    while ( size > 0 )
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // Destination for storing flag series
        LZDstCount++;

        // Since flag series is stored as 8-bit data, loop eight times
        for ( i = 0; i < 8; i++ )
        {
            LZCompFlags <<= 1;         // The first time (i=0) has no real meaning
            if (size <= 0)
            {
                // When the end terminator is reached, quit after shifting flag through to the last
                continue;
            }

            if ( (lastLength = SearchLZ( &info, srcp, size, &lastOffset, MAX_LENGTH)) != 0 )
            {
                u32 length;
                // Enable flag if compression is possible
                LZCompFlags |= 0x1;

                if (LZDstCount + 2 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }
                
                if ( exFormat )
                {
                    if ( lastLength >= 0xFF + 0xF + 3 )
                    {
                        length  = (u32)( lastLength - 0xFF - 0xF - 3 );
                        *dstp++ = (u8)( 0x10 | (length >> 12) );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 2;
                    }
                    else if ( lastLength >= 0xF + 2 )
                    {
                        length  = (u32)( lastLength - 0xF - 2 );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 1;
                    }
                    else
                    {
                        length = (u32)( lastLength - 1 );
                    }
                }
                else
                {
                    length = (u32)( lastLength - 3 );
                }
                
                // Divide offset into upper 4 bits and lower 8 bits and store
                *dstp++ = (u8)( length << 4 | (lastOffset - 1) >> 8 );
                *dstp++ = (u8)( (lastOffset - 1) & 0xff);
                LZDstCount += 2;
                LZSlide(&info, srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // No compression
                if (LZDstCount + 1 >= dstMax)       // Quit on error if size becomes larger than source
                {
                    return 0;
                }
                LZSlide(&info, srcp, 1);
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // Completed eight loops
        *LZCompFlagsp = LZCompFlags;   // Store flag series
    }
    
    // 4-byte boundary alignment
    //   Data size does not include Data0, used for alignment
    i = 0;
    while ( (LZDstCount + i) & 0x3 )
    {
        *dstp++ = 0;
        i++;
    }
    
    return LZDstCount;
}

//--------------------------------------------------------
// With LZ77 compression, searches for the longest matching string in the slide window.
//  Arguments:    startp:              Pointer to starting position of data
//                nextp:               Pointer to data where search will start
//                remainSize:          Size of remaining data
//                offset:              Pointer to region storing matched offset
//  Return   :    TRUE if matching string is found
//                FALSE if not found
//--------------------------------------------------------
static u32 SearchLZ( LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength )
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     maxOffset = 0;
    u32     currLength = 2;
    u32     tmpLength;
    s32     w_offset;
    s16    *const LZOffsetTable = info->LZOffsetTable;
    const u16 windowPos = info->windowPos;
    const u16 windowLen = info->windowLen;

    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = info->LZByteTable[*nextp];

    while (w_offset != -1)
    {
        if (w_offset < windowPos)
        {
            searchp = nextp - windowPos + w_offset;
        }
        else
        {
            searchp = nextp - windowLen - windowPos + w_offset;
        }

        /* This isn't needed, but it seems to make it a little faster */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[w_offset];
            continue;
        }

        if (nextp - searchp < 2)
        {
            // VRAM is accessed in 2-byte units (since data is sometimes read from VRAM), so the data to search must start 2 bytes prior to the start location actually desired.
            // 
            // 
            // Since the offset is stored in 12 bits, the value is 4096 or less
            break;
        }
        tmpLength = 3;
        searchHeadp = searchp + 3;
        headp = nextp + 3;

        // Increments the compression size until a data terminator or different data is encountered.
        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            // Since the data length is stored in 4 bits, the value is 18 or less (3 is added)
            if (tmpLength == maxLength)
            {
                break;
            }
        }

        if (tmpLength > currLength)
        {
            // Update the maximum-length offset
            currLength = tmpLength;
            maxOffset = (u16)(nextp - searchp);
            if (currLength == maxLength || currLength == remainSize)
            {
                // This is the longest matching length, so end search.
                break;
            }
        }
        w_offset = LZOffsetTable[w_offset];
    }

    if (currLength < 3)
    {
        return 0;
    }
    *offset = maxOffset;
    return currLength;
}

//--------------------------------------------------------
// Initialize the dictionary index
//--------------------------------------------------------
static void LZInitTable(LZCompressInfo * info, void *work)
{
    u16     i;
    
    info->LZOffsetTable = (s16*)work;
    info->LZByteTable   = (s16*)( (uintptr)work + 4096 * sizeof(s16)         );
    info->LZEndTable    = (s16*)( (uintptr)work + (4096 + 256) * sizeof(s16) );
    
    for ( i = 0; i < 256; i++ )
    {
        info->LZByteTable[i] = -1;
        info->LZEndTable [i]  = -1;
    }
    info->windowPos = 0;
    info->windowLen = 0;
}

//--------------------------------------------------------
// Slide the dictionary 1 byte
//--------------------------------------------------------
static void SlideByte(LZCompressInfo * info, const u8 *srcp)
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;

    s16    *const LZByteTable   = info->LZByteTable;
    s16    *const LZOffsetTable = info->LZOffsetTable;
    s16    *const LZEndTable    = info->LZEndTable;
    const u16 windowPos = info->windowPos;
    const u16 windowLen = info->windowLen;

    if (windowLen == 4096)
    {
        u8      out_data = *(srcp - 4096);
        if ((LZByteTable[out_data] = LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data] = -1;
        }
        insert_offset = windowPos;
    }
    else
    {
        insert_offset = windowLen;
    }

    offset = LZEndTable[in_data];
    if (offset == -1)
    {
        LZByteTable[in_data] = (s16)insert_offset;
    }
    else
    {
        LZOffsetTable[offset] = (s16)insert_offset;
    }
    LZEndTable[in_data] = (s16)insert_offset;
    LZOffsetTable[insert_offset] = -1;

    if (windowLen == 4096)
    {
        info->windowPos = (u16)((windowPos + 1) % 0x1000);
    }
    else
    {
        info->windowLen++;
    }
}

//--------------------------------------------------------
// Slide the dictionary n bytes
//--------------------------------------------------------
static inline void LZSlide(LZCompressInfo * info, const u8 *srcp, u32 n)
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(info, srcp++);
    }
}


//===========================================================================
//  Run-Length Encoding
//===========================================================================

/*---------------------------------------------------------------------------*
  Name:         CXCompressRL

  Description:  Function that performs run-length compression

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp:            Pointer to destination for compressed data
                                The buffer must be larger than the size of the compression source data.

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 CXCompressRL(const u8 *srcp, u32 size, u8 *dstp)
{
    u32 RLDstCount;                    // Number of bytes of compressed data
    u32 RLSrcCount;                    // Processed data volume of the compression target data (in bytes)
    u8  RLCompFlag;                    // 1 if performing run-length encoding
    u8  runLength;                     // Run length
    u8  rawDataLength;                 // Length of non-run data
    u32 i;
    
    const u8 *startp;                  // Point to the start of compression target data for each process loop
    
    TT_ASSERT( srcp != NULL );
    TT_ASSERT( dstp != NULL );
    TT_ASSERT( size > 4     );
    
    //  Data header (For the size after decompression)
    // To create the same output data as Nitro, work on on the endian.
    if ( size < (1 << 24) )
    {
        *(u32 *)dstp = CXiConvertEndian_(size << 8 | CX_COMPRESSION_RL);       // Data header
        RLDstCount   = 4;
    }
    else
    // Use extended header if the size is larger than 24 bits
    {
        *(u32 *)dstp       = CXiConvertEndian_(CX_COMPRESSION_RL);       // Data header
        *(u32 *)(dstp + 4) = CXiConvertEndian_(size);                  // Extend header size
        RLDstCount         = 8;
    }
    RLSrcCount = 0;
    rawDataLength = 0;
    RLCompFlag = 0;

    while (RLSrcCount < size)
    {
        startp = &srcp[RLSrcCount];    // Set compression target data

        for (i = 0; i < 128; i++)      // Data volume that can be expressed in 7 bits is 0 to 127
        {
            // Reach the end of the compression target data
            if (RLSrcCount + rawDataLength >= size)
            {
                rawDataLength = (u8)(size - RLSrcCount);
                break;
            }

            if (RLSrcCount + rawDataLength + 2 < size)
            {
                if (startp[i] == startp[i + 1] && startp[i] == startp[i + 2])
                {
                    RLCompFlag = 1;
                    break;
                }
            }
            rawDataLength++;
        }

        // Store data that will not be encoded
        // If the 8th bit of the data length storage byte is 0, this is a data sequence that is not encoded.
        // The data length is x - 1, so 0-127 becomes 1-128.
        if (rawDataLength)
        {
            if (RLDstCount + rawDataLength + 1 >= size) // Quit on error if size becomes larger than source
            {
                return 0;
            }
            dstp[RLDstCount++] = (u8)(rawDataLength - 1);       // Store "data length - 1" (7 bits)
            for (i = 0; i < rawDataLength; i++)
            {
                dstp[RLDstCount++] = srcp[RLSrcCount++];
            }
            rawDataLength = 0;
        }

        // Run-Length Encoding
        if (RLCompFlag)
        {
            runLength = 3;
            for (i = 3; i < 128 + 2; i++)
            {
                // Reach the end of the data for compression
                if (RLSrcCount + runLength >= size)
                {
                    runLength = (u8)(size - RLSrcCount);
                    break;
                }

                // If run was interrupted
                if (srcp[RLSrcCount] != srcp[RLSrcCount + runLength])
                {
                    break;
                }
                // Run continues
                runLength++;
            }

            // If the 8th bit of the data length storage byte is 1, this is an encoded data sequence
            if (RLDstCount + 2 >= size) // Quit on error if size becomes larger than source
            {
                return 0;
            }
            dstp[RLDstCount++] = (u8)(0x80 | (runLength - 3));  // Add 3, and store from 3 to 130
            dstp[RLDstCount++] = srcp[RLSrcCount];
            RLSrcCount += runLength;
            RLCompFlag = 0;
        }
    }

    // 4-byte boundary alignment
    //   Data size does not include Data0, used for alignment
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }
    return RLDstCount;
}


//===========================================================================
//  Huffman encoding
//===========================================================================
#define HUFF_END_L  0x80
#define HUFF_END_R  0x40

typedef struct
{
    u32 Freq;                          // Frequency of occurrence
    u16 No;                            // Data number
    s16 PaNo;                          // Parent number 
    s16 ChNo[2];                       // Child Number (0: left side, 1: right side)
    u16 PaDepth;                       // Parent node depth
    u16 LeafDepth;                     // Depth to leaf
    u32 HuffCode;                      // Huffman code
    u8  Bit;                           // Node's bit data
    u8  _padding;
    u16 HWord;                         // For each intermediate node, the amount of memory needed to store in HuffTree the subtree that has that node as its root
}
HuffData;                              // Total of 24 bytes

typedef struct
{
    u8  leftOffsetNeed;                // 1 if offset to left child node is required
    u8  rightOffsetNeed;               // 1 if an offset to the right child node is required
    u16 leftNodeNo;                    // The left child node's number
    u16 rightNodeNo;                   // Right child node's number
}
HuffTreeCtrlData;                      // Total of 6 bytes

// Structure of the Huffman work buffer
typedef struct
{
    HuffData         *huffTable;    //  huffTable[ 512 ];      12288B
    u8               *huffTree;     //  huffTree[ 256 * 2 ];     512B
    HuffTreeCtrlData *huffTreeCtrl; //  huffTreeCtrl[ 256 ];    1536B
    u8               huffTreeTop;   //  
    u8               padding_[3];   //  
}
HuffCompressionInfo;                // Total is 14340B

static void HuffInitTable( HuffCompressionInfo* info, void* work, u16 dataNum );
static void HuffCountData( HuffData* table, const u8 *srcp, u32 size, u8 bitSize );
static u16  HuffConstructTree( HuffData* table, u32 dataNum );
static u32  HuffConvertData( const HuffData *table, const u8* srcp, u8* dstp, u32 srcSize, u32 maxSize, u8 bitSize );

static void HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo );
static void HuffAddCodeToTable       ( HuffData *table, u16 nodeNo, u32 paHuffCode );
static u8   HuffAddCountHWordToTable ( HuffData *table, u16 nodeNo );

static void HuffMakeHuffTree             ( HuffCompressionInfo* info, u16 rootNo );
static void HuffMakeSubsetHuffTree       ( HuffCompressionInfo* info, u16 huffTreeNo, u8 rightNodeFlag );
static u8   HuffRemainingNodeCanSetOffset( HuffCompressionInfo* info, u8  costHWord );
static void HuffSetOneNodeOffset         ( HuffCompressionInfo* info, u16 huffTreeNo, u8 rightNodeFlag );


/*---------------------------------------------------------------------------*
  Name:         CXCompressHuffman

  Description:  Function that performs Huffman compression

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp:            Pointer to destination for compressed data
                                The buffer must be larger than the size of the compression source data.
                huffBitSize:    The number of bits to encode.
                work:            Work buffer for Huffman compression

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 CXCompressHuffman( const u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize, void *work )
{
    u32 huffDstCount;                  // Number of bytes of compressed data
    s32 i;
    u16 rootNo;                        // Binary tree's root number
    u16 huffDataNum  = (u16)(1 << huffBitSize);      // 8->256, 4->16
    u32 tableOffset;
    HuffCompressionInfo info;
    
    TT_ASSERT( srcp != NULL );
    TT_ASSERT( dstp != NULL );
    TT_ASSERT( huffBitSize == 4 || huffBitSize == 8 );
    TT_ASSERT( work != NULL );
    TT_ASSERT( ((uintptr)work & 0x3) == 0 );
    TT_ASSERT( size > 4 );
    
    // Initialize table
    HuffInitTable( &info, work, huffDataNum );
    
    // Check frequency of occurrence
    HuffCountData( info.huffTable, srcp, size, huffBitSize );
    
    // Create tree table
    rootNo = HuffConstructTree( info.huffTable, huffDataNum );
    
    // Create HuffTree
    HuffMakeHuffTree( &info, rootNo );
    info.huffTree[0] = --info.huffTreeTop;
    
    // Data header
    // To create the same compression data as Nitro, work on the endian.
    if ( size < (1 << 24) )
    {
        *(u32 *)dstp = CXiConvertEndian_(size << 8 | CX_COMPRESSION_HUFFMAN | huffBitSize);
        tableOffset  = 4;
    }
    else
    // Use extended header if the size is larger than 24 bits
    {
        *(u32 *)dstp       = CXiConvertEndian_( (u32)(CX_COMPRESSION_HUFFMAN | huffBitSize) );
        *(u32 *)(dstp + 4) = CXiConvertEndian_( size );
        tableOffset        = 8;
    }
    huffDstCount = tableOffset;
    
    if ( huffDstCount + (info.huffTreeTop + 1) * 2 >= size )   // Quit on error if size becomes larger than source
    {
        return 0;
    }
    
    for ( i = 0; i < (u16)( (info.huffTreeTop + 1) * 2 ); i++ )  // Tree table
    {
        dstp[ huffDstCount++ ] = ((u8*)info.huffTree)[ i ];
    }
    
    // 4-byte boundary alignment
    //   Data0 used for alignment is included in data size (as per the decoder algorithm)
    while ( huffDstCount & 0x3 )
    {
        if ( huffDstCount & 0x1 )
        {
            info.huffTreeTop++;
            dstp[ tableOffset ]++;
        }
        dstp[ huffDstCount++ ] = 0;
    }
    
    // Data conversion via the Huffman table
    {
        u32 convSize = HuffConvertData( info.huffTable, srcp, &dstp[ huffDstCount ], size, size - huffDstCount, huffBitSize );
        if ( convSize == 0 )
        {
            // Compression fails because the compressed data is larger than the source
            return 0;
        }
        huffDstCount += convSize;
    }
    
    return huffDstCount;
}


/*---------------------------------------------------------------------------*
  Name:         HuffInitTable
  Description:  Initializes the Huffman table.
  Arguments:    table   
                size    
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffInitTable( HuffCompressionInfo* info, void* work, u16 dataNum )
{
    u32 i;
    info->huffTable    = (HuffData*)(work);
    info->huffTree     = (u8*)( (uintptr)work + sizeof(HuffData) * 512 );
    info->huffTreeCtrl = (HuffTreeCtrlData*)( (uintptr)info->huffTree + sizeof(u8) * 512 );
    info->huffTreeTop  = 1;
    
    // Initialize huffTable
    {
        HuffData* table = info->huffTable;
        
        const HuffData  HUFF_TABLE_INIT_DATA  = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0, 0};
        for ( i = 0; i < dataNum * 2U; i++ )
        {
            table[ i ]    = HUFF_TABLE_INIT_DATA;
            table[ i ].No = (u16)i;
        }
    }
    
    // Initialize huffTree and huffTreeCtrl
    {
        const HuffTreeCtrlData HUFF_TREE_CTRL_INIT_DATA = { 1, 1, 0, 0 };
        u8*               huffTree     = info->huffTree;
        HuffTreeCtrlData* huffTreeCtrl = info->huffTreeCtrl;
        
        for ( i = 0; i < 256; i++ )
        {
            huffTree[ i * 2 ]     = 0;
            huffTree[ i * 2 + 1 ] = 0;
            huffTreeCtrl[ i ]     = HUFF_TREE_CTRL_INIT_DATA;
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HuffCountData
  Description:  Count of frequency of appearance.
  Arguments:    table   
                *srcp   
                size    
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffCountData( HuffData* table, const u8 *srcp, u32 size, u8 bitSize )
{
    u32 i;
    u8  tmp;
    
    if ( bitSize == 8 )
    {
        for ( i = 0; i < size; i++ )
        {
            table[ srcp[ i ] ].Freq++; // 8-bit encoding
        }
    }
    else
    {
        for ( i = 0; i < size; i++ )   // 4-bit encoding
        {
            tmp = (u8)( (srcp[ i ] & 0xf0) >> 4 );
            table[ tmp ].Freq++;       // Store from upper 4 bits first // Either is OK
            tmp = (u8)( srcp[ i ] & 0x0f );
            table[ tmp ].Freq++;       // The problem is the encoding
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HuffConstructTree
  Description:  Constructs a Huffman tree
  Arguments:    *table  
                dataNum 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16 HuffConstructTree( HuffData *table, u32 dataNum )
{
    s32     i;
    s32     leftNo, rightNo;         // Node's numbers at time of binary tree's creation
    u16     tableTop = (u16)dataNum; // The table top number at time of table's creation
    u16     rootNo;                  // Binary tree's root number
    //u16     nodeNum;                 // Number of valid nodes
    
    leftNo  = -1;
    rightNo = -1;
    while ( 1 )
    {
        // Search for two subtree vertices with low Freq value. At least one should be found.
        // Search child vertices (left)
        for ( i = 0; i < tableTop; i++ )
        {
            if ( ( table[i].Freq == 0 ) ||
                 ( table[i].PaNo != 0 ) )
            {
                continue;
            }
            
            if ( leftNo < 0 )
            {
                leftNo = i;
            }
            else if ( table[i].Freq < table[ leftNo ].Freq )
            {
                leftNo = i;
            }
        }
        
        // Search child vertices (right)
        for ( i = 0; i < tableTop; i++ )
        {
            if ( ( table[i].Freq == 0 ) || 
                 ( table[i].PaNo != 0 ) || 
                 ( i == leftNo ) )
            {
                continue;
            }
            
            if ( rightNo < 0 )
            {
                rightNo = i;
            }
            else if ( table[i].Freq < table[rightNo].Freq )
            {
                rightNo = i;
            }
        }
        
        // If only one, then end table creation
        if ( rightNo < 0 )
        {
            // When only one type of value exists, then create one node that gives the same value for both 0 and 1.
            if ( tableTop == dataNum )
            {
                table[ tableTop ].Freq      = table[ leftNo ].Freq;
                table[ tableTop ].ChNo[0]   = (s16)leftNo;
                table[ tableTop ].ChNo[1]   = (s16)leftNo;
                table[ tableTop ].LeafDepth = 1;
                table[ leftNo   ].PaNo      = (s16)tableTop;
                table[ leftNo   ].Bit       = 0;
                table[ leftNo   ].PaDepth   = 1;
            }
            else
            {
                tableTop--;
            }
            rootNo  = tableTop;
            //nodeNum = (u16)( (rootNo - dataNum + 1) * 2 + 1 );
            break;
        }
        
        // Create vertex that combines left subtree and right subtree
        table[ tableTop ].Freq = table[ leftNo ].Freq + table[ rightNo ].Freq;
        table[ tableTop ].ChNo[0] = (s16)leftNo;
        table[ tableTop ].ChNo[1] = (s16)rightNo;
        if ( table[ leftNo ].LeafDepth > table[ rightNo ].LeafDepth )
        {
            table[ tableTop ].LeafDepth = (u16)( table[ leftNo ].LeafDepth + 1 );
        }
        else
        {
            table[ tableTop ].LeafDepth = (u16)( table[ rightNo ].LeafDepth + 1 );
        }
        
        table[ leftNo  ].PaNo = table[ rightNo ].PaNo = (s16)(tableTop);
        table[ leftNo  ].Bit  = 0;
        table[ rightNo ].Bit  = 1;
        
        HuffAddParentDepthToTable( table, (u16)leftNo, (u16)rightNo );
        
        tableTop++;
        leftNo = rightNo = -1;
    }
    
    // Generate Huffman code (In table[i].HuffCode)
    HuffAddCodeToTable( table, rootNo, 0x00 );        // The Huffman code is the code with HuffCode's lower bits masked for PaDepth bits
    
    // For each intermediate node, calculate the amount of memory needed to store as a huffTree the subtree that has that node as the root.
    HuffAddCountHWordToTable( table, rootNo );
    
    return rootNo;
}

//-----------------------------------------------------------------------
// When creating binary tree and when combining subtrees, add 1 to the depth of every node in the subtree.
//-----------------------------------------------------------------------
static void HuffAddParentDepthToTable( HuffData *table, u16 leftNo, u16 rightNo )
{
    table[ leftNo  ].PaDepth++;
    table[ rightNo ].PaDepth++;
    
    if ( table[ leftNo ].LeafDepth != 0 )
    {
        HuffAddParentDepthToTable( table, (u16)table[ leftNo  ].ChNo[0], (u16)table[ leftNo  ].ChNo[1] );
    }
    if ( table[ rightNo ].LeafDepth != 0 )
    {
        HuffAddParentDepthToTable( table, (u16)table[ rightNo ].ChNo[0], (u16)table[ rightNo ].ChNo[1] );
    }
}

//-----------------------------------------------------------------------
// Create Huffman code
//-----------------------------------------------------------------------
static void HuffAddCodeToTable( HuffData* table, u16 nodeNo, u32 paHuffCode )
{
    table[ nodeNo ].HuffCode = (paHuffCode << 1) | table[ nodeNo ].Bit;
    
    if ( table[ nodeNo ].LeafDepth != 0 )
    {
        HuffAddCodeToTable( table, (u16)table[ nodeNo ].ChNo[0], table[ nodeNo ].HuffCode );
        HuffAddCodeToTable( table, (u16)table[ nodeNo ].ChNo[1], table[ nodeNo ].HuffCode );
    }
}


//-----------------------------------------------------------------------
// Data volume required by intermediate nodes to create huffTree
//-----------------------------------------------------------------------
static u8 HuffAddCountHWordToTable( HuffData *table, u16 nodeNo)
{
    u8      leftHWord, rightHWord;
    
    switch ( table[ nodeNo ].LeafDepth )
    {
    case 0:
        return 0;
    case 1:
        leftHWord = rightHWord = 0;
        break;
    default:
        leftHWord  = HuffAddCountHWordToTable( table, (u16)table[nodeNo].ChNo[0] );
        rightHWord = HuffAddCountHWordToTable( table, (u16)table[nodeNo].ChNo[1] );
        break;
    }
    
    table[ nodeNo ].HWord = (u16)( leftHWord + rightHWord + 1 );
    return (u8)( leftHWord + rightHWord + 1 );
}


//-----------------------------------------------------------------------
// Create Huffman code table
//-----------------------------------------------------------------------
static void HuffMakeHuffTree( HuffCompressionInfo* info, u16 rootNo )
{
    s16 i;
    s16 costHWord, tmpCostHWord;       // Make subtree code table for most-costly node when subtree code table has not been created.
    s16 costOffsetNeed, tmpCostOffsetNeed;
    s16 costMaxKey, costMaxRightFlag = 0;  // Information for singling out the least costly node from HuffTree
    u8  offsetNeedNum;
    //u8  tmpKey;
    u8  tmpRightFlag;
    
    info->huffTreeTop = 1;
    costOffsetNeed    = 0;
    
    info->huffTreeCtrl[0].leftOffsetNeed = 0; // Do not use (used as table size)
    info->huffTreeCtrl[0].rightNodeNo    = rootNo;
    
    while ( 1 )                          // Until return 
    {
        // Calculate the number of nodes whose offset needs setting
        offsetNeedNum = 0;
        for ( i = 0; i < info->huffTreeTop; i++ )
        {
            if ( info->huffTreeCtrl[ i ].leftOffsetNeed )
            {
                offsetNeedNum++;
            }
            if ( info->huffTreeCtrl[ i ].rightOffsetNeed )
            {
                offsetNeedNum++;
            }
        }

        // Search for node with greatest cost
        costHWord    = -1;
        costMaxKey   = -1;
        //tmpKey       =  0;
        tmpRightFlag =  0;

        for ( i = 0; i < info->huffTreeTop; i++ )
        {
            tmpCostOffsetNeed = (u8)( info->huffTreeTop - i );
            
            // Evaluate cost of left child node
            if (info->huffTreeCtrl[i].leftOffsetNeed)
            {
                tmpCostHWord = (s16)info->huffTable[ info->huffTreeCtrl[i].leftNodeNo ].HWord;
                
                if ( (tmpCostHWord + offsetNeedNum) > 64 )
                {
                    goto leftCostEvaluationEnd;
                }
                if ( ! HuffRemainingNodeCanSetOffset( info, (u8)tmpCostHWord ) )
                {
                    goto leftCostEvaluationEnd;
                }
                if ( tmpCostHWord > costHWord )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
                else if ( (tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed) )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
            }
leftCostEvaluationEnd:{}
            
            // Evaluate cost of right child node
            if ( info->huffTreeCtrl[i].rightOffsetNeed)
            {
                tmpCostHWord = (s16)info->huffTable[info->huffTreeCtrl[i].rightNodeNo].HWord;
                
                if ( (tmpCostHWord + offsetNeedNum) > 64 )
                {
                    goto rightCostEvaluationEnd;
                }
                if ( ! HuffRemainingNodeCanSetOffset( info, (u8)tmpCostHWord ) )
                {
                    goto rightCostEvaluationEnd;
                }
                if ( tmpCostHWord > costHWord )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
                else if ( (tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed) )
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
            }
rightCostEvaluationEnd:{}
        }

        // Store entire subtree in huffTree
        if ( costMaxKey >= 0 )
        {
            HuffMakeSubsetHuffTree( info, (u8)costMaxKey, (u8)costMaxRightFlag);
            goto nextTreeMaking;
        }
        else
        {
            // Search for node with largest required offset
            for ( i = 0; i < info->huffTreeTop; i++ )
            {
                u16 tmp = 0;
                tmpRightFlag = 0;
                if (info->huffTreeCtrl[i].leftOffsetNeed)
                {
                    tmp = info->huffTable[ info->huffTreeCtrl[i].leftNodeNo ].HWord;
                }
                if (info->huffTreeCtrl[i].rightOffsetNeed)
                {
                    if ( info->huffTable[info->huffTreeCtrl[i].rightNodeNo ].HWord > tmp )
                    {
                        tmpRightFlag = 1;
                    }
                }
                if ( (tmp != 0) || (tmpRightFlag) )
                {
                    HuffSetOneNodeOffset( info, (u8)i, tmpRightFlag);
                    goto nextTreeMaking;
                }
            }
        }
        return;
nextTreeMaking:{}
    }
}

//-----------------------------------------------------------------------
// Store entire subtree in huffTree
//-----------------------------------------------------------------------
static void HuffMakeSubsetHuffTree( HuffCompressionInfo* info, u16 huffTreeNo, u8 rightNodeFlag )
{
    u8  i;
    
    i = info->huffTreeTop;
    HuffSetOneNodeOffset( info, huffTreeNo, rightNodeFlag);
    
    if ( rightNodeFlag )
    {
        info->huffTreeCtrl[ huffTreeNo ].rightOffsetNeed = 0;
    }
    else
    {
        info->huffTreeCtrl[ huffTreeNo ].leftOffsetNeed = 0;
    }
    
    while ( i < info->huffTreeTop )
    {
        if ( info->huffTreeCtrl[ i ].leftOffsetNeed )
        {
            HuffSetOneNodeOffset( info, i, 0);
            info->huffTreeCtrl[ i ].leftOffsetNeed = 0;
        }
        if ( info->huffTreeCtrl[ i ].rightOffsetNeed )
        {
            HuffSetOneNodeOffset( info, i, 1);
            info->huffTreeCtrl[ i ].rightOffsetNeed = 0;
        }
        i++;
    }
}

//-----------------------------------------------------------------------
// Check if there is any problems with huffTree construction if subtree of obtained data size is decompressed.
//-----------------------------------------------------------------------
static u8 HuffRemainingNodeCanSetOffset( HuffCompressionInfo* info, u8 costHWord )
{
    u8  i;
    s16 capacity;
    
    capacity = (s16)( 64 - costHWord );
    
    // The offset value is larger for smaller values of i, so you should calculate without sorting, with i=0 -> huffTreeTop
    for ( i = 0; i < info->huffTreeTop; i++ )
    {
        if ( info->huffTreeCtrl[i].leftOffsetNeed )
        {
            if ( (info->huffTreeTop - i) <= capacity )
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
        if ( info->huffTreeCtrl[i].rightOffsetNeed )
        {
            if ( (info->huffTreeTop - i) <= capacity )
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
    }
    
    return 1;
}

/*---------------------------------------------------------------------------*
  Name:         HuffSetOneNodeOffset
  Description:  Create Huffman code table for one node
  Arguments:    *info :          Pointer to data for constructing a Huffman tree
                huffTreeNo      
                rightNodeFlag :  Flag for whether node is at right
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void HuffSetOneNodeOffset( HuffCompressionInfo* info, u16 huffTreeNo, u8 rightNodeFlag )
{
    u16 nodeNo;
    u8  offsetData = 0;
    
    HuffData*         huffTable    = info->huffTable;
    u8*               huffTree     = info->huffTree;
    HuffTreeCtrlData* huffTreeCtrl = info->huffTreeCtrl;
    u8                huffTreeTop  = info->huffTreeTop;
    
    if (rightNodeFlag)
    {
        nodeNo = huffTreeCtrl[ huffTreeNo ].rightNodeNo;
        huffTreeCtrl[ huffTreeNo ].rightOffsetNeed = 0;
    }
    else
    {
        nodeNo = huffTreeCtrl[ huffTreeNo ].leftNodeNo;
        huffTreeCtrl [huffTreeNo ].leftOffsetNeed = 0;
    }
    
    // Left child node
    if ( huffTable[ huffTable[nodeNo].ChNo[0] ].LeafDepth == 0)
    {
        offsetData |= 0x80;
        huffTree[ huffTreeTop * 2 + 0 ] = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u8)huffTable[ nodeNo ].ChNo[0];
        huffTreeCtrl[ huffTreeTop ].leftOffsetNeed = 0;   // Offset no longer required
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].leftNodeNo = (u16)huffTable[ nodeNo ].ChNo[0];  // Offset is required
    }
    
    // Right child node
    if ( huffTable[ huffTable[ nodeNo ].ChNo[1] ].LeafDepth == 0 )
    {
        offsetData |= 0x40;
        huffTree[ huffTreeTop * 2 + 1 ] = (u8)huffTable[nodeNo].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u8)huffTable[ nodeNo ].ChNo[1];
        huffTreeCtrl[ huffTreeTop ].rightOffsetNeed = 0;  // Offset no longer required
    }
    else
    {
        huffTreeCtrl[ huffTreeTop ].rightNodeNo = (u16)huffTable[ nodeNo ].ChNo[1]; // Offset is required
    }
    
    offsetData |= (u8)( huffTreeTop - huffTreeNo - 1 );
    huffTree[ huffTreeNo * 2 + rightNodeFlag ] = offsetData;
    
    info->huffTreeTop++;
}


/*---------------------------------------------------------------------------*
  Name:         HuffConvertData
  Description:  Data conversion based on Huffman table.
  Arguments:    *table  
                srcp    
                dstp    
                srcSize 
                bitSize 
  Returns:      None.
 *---------------------------------------------------------------------------*/
static u32 HuffConvertData( const HuffData *table, const u8* srcp, u8* dstp, u32 srcSize, u32 maxSize, u8 bitSize )
{
    u32     i, ii, iii;
    u8      srcTmp;
    u32     bitStream    = 0;
    u32     streamLength = 0;
    u32     dstSize      = 0;
    
    // Huffman encoding
    for ( i = 0; i < srcSize; i++ )
    {                                  // Data compression
        u8 val = srcp[ i ];
        if ( bitSize == 8 )
        {                              // 8-bit Huffman
            bitStream = (bitStream << table[ val ].PaDepth) | table[ val ].HuffCode;
            streamLength += table[ val ].PaDepth;
            
            if ( dstSize + (streamLength / 8) >= maxSize )
            {
                // Error if size becomes larger than source
                return 0;
            }
            
            for ( ii = 0; ii < streamLength / 8; ii++ )
            {
                dstp[ dstSize++ ] = (u8)(bitStream >> (streamLength - (ii + 1) * 8));
            }
            streamLength %= 8;
        }
        else                           // 4-bit Huffman
        {
            for ( ii = 0; ii < 2; ii++ )
            {
                if ( ii )
                {
                    srcTmp = (u8)( val >> 4 );     // First four bits come later
                }
                else
                {
                    srcTmp = (u8)( val & 0x0F );   // Last four bits come first (because the decoder accesses in a Little-Endian method)
                }
                bitStream = (bitStream << table[ srcTmp ].PaDepth) | table[ srcTmp ].HuffCode;
                streamLength += table[ srcTmp ].PaDepth;
                if ( dstSize + (streamLength / 8) >= maxSize )
                {
                    // Error if size becomes larger than source
                    return 0;
                }
                for ( iii = 0; iii < streamLength / 8; iii++ )
                {
                    dstp[ dstSize++ ] = (u8)(bitStream >> (streamLength - (iii + 1) * 8));
                }
                streamLength %= 8;
            }
        }
    }
    
    if ( streamLength != 0 )
    {
        if ( dstSize + 1 >= maxSize )
        {
            // Error if size becomes larger than source
            return 0;
        }
        dstp[ dstSize++ ] = (u8)( bitStream << (8 - streamLength) );
    }
    
    // 4-byte boundary alignment
    //   Data0 for alignment is included in data size 
    //   This is special to Huffman encoding! Data subsequent to the alignment-boundary data is stored because little-endian conversion is used.
    while ( dstSize & 0x3 )
    {
        dstp[ dstSize++ ] = 0;
    }
    
    // Little endian conversion
    for ( i = 0; i < dstSize / 4; i++ )
    {
        u8 tmp;
        tmp = dstp[i * 4 + 0];
        dstp[i * 4 + 0] = dstp[i * 4 + 3];
        dstp[i * 4 + 3] = tmp;         // Swap
        tmp = dstp[i * 4 + 1];
        dstp[i * 4 + 1] = dstp[i * 4 + 2];
        dstp[i * 4 + 2] = tmp;         // Swap
    }
    return dstSize;
}


s32 getPackedBits(void* p_buffer, s32 p_index, s32 p_offset, s32 p_bits)
{
	s32 mask = (1 << p_bits) - 1;
	mask <<= p_offset;
	s32 value = ((u8*)p_buffer)[p_index] & mask;
	value >>= p_offset;
	return value;
}


void setPackedBits(void* p_buffer, s32 p_index, s32 p_offset, s32 p_bits, s32 p_value)
{
	// At this point we're working per byte. Make sure the input fits.
	TT_ASSERT(p_bits   <= 8);
	TT_ASSERT(p_offset <= 8);
	TT_ASSERT( (p_value & 0xFFFFFF00) == 0);
	
	s32 mask = (1 << p_bits) - 1;
	mask <<= p_offset;
	mask = ~mask;
	((u8*)p_buffer)[p_index] &= static_cast<u8>(mask);
	((u8*)p_buffer)[p_index] |= static_cast<u8>(p_value << p_offset);
}

// Namespace end
}
}
