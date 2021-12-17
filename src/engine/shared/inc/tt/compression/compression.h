#if !defined(INC_TT_COMPRESSION_COMPRESSION_H)
#define INC_TT_COMPRESSION_COMPRESSION_H


#include <tt/compression/context.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace compression {

enum CompressionType
{
	Compression_None    = 0x00, //!< Not compressed      0000 0000b
	Compression_LZ      = 0x10, //!< LZ77 (extended)     0001 0000b
	Compression_Huffman = 0x20, //!< Huffman             0010 0000b
	Compression_RL      = 0x30, //!< Run Length          0011 0000b
	Compression_FastLZ  = 0x40, //!< FastLZ              0100 0000b
	Compression_Dif     = 0x80, //!< Differential filter 1000 0000b
	
	Compression_TypeMask   = 0xf0, //                    1111 0000b
	Compression_TypeExMask = 0xff  //                    1111 1111b
};


/*! \brief Gets a bit packed value from a buffer
    \param p_buffer Buffer containing bit packed values.
    \param p_bits Size of elements in buffer in bits.
    \param p_index Index of value to get.
    \return Value at p_index in p_buffer.*/
s32  getPackedValue(void* p_buffer, s32 p_bits, s32 p_index);

/*! \brief Sets a bit packed value in a buffer
    \param p_buffer Buffer containing bit packed values.
    \param p_bits Size of elements in buffer in bits.
    \param p_index Index of value to set.
    \param p_value Value to set.*/
void setPackedValue(void* p_buffer, s32 p_bits, s32 p_index, s32 p_value);

/*! \brief Gets the size of the buffer after decompression.
    \param p_src Source buffer to get size of.
    \return The size of the buffer after decompression.*/
u32 getUncompressedSize(const void* p_src);

/*! \brief Gets the type compression of the compressed buffer.
    \param p_src Source buffer to get compression type of.
    \return The type of compression.*/
CompressionType getCompressionType(const void* p_src);

/*! \brief Determines the compression type and then executes the appropriate decompression process.
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressAny(const void* p_src, void* p_dst);

/*! \brief Decompresses a buffer that has not been compressed (removes compression header).
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressNone(const void* p_src, void* p_dst);

/*! \brief Uncompresses Run Length Encoded data.
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressRL(const void* p_src, void* p_dst);

/*! \brief Uncompresses LZ77 (extended) compressed data.
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressLZ(const void* p_src, void* p_dst);

/*! \brief Uncompresses FastLZ compressed data.
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressFastLZ(const void* p_src, void* p_dst);

/*! \brief Uncompresses Huffman compressed data.
    \param p_src Source buffer with compressed data.
    \param p_dst Destination buffer.*/
void uncompressHuff(const void* p_src, void* p_dst);

/*! \brief Performs difference filtering.
    \param p_src Source buffer with difference filtered data.
    \param p_dst Destination buffer.*/
void unfilterDiff(const void* p_src, void* p_dst);


/*! \brief Initializes decompression context and header for Run Length Encoding.
    \param p_context Context data for streaming decompression.
    \param p_dst Destination buffer.
    \param p_header Header of compressed data.*/
void initUncompContextRL(RLContext* p_context, u8* p_dst, CompressionHeader* p_header);

/*! \brief Initializes decompression context and header for LZ77 (ex).
    \param p_context Context data for streaming decompression.
    \param p_dst Destination buffer.
    \param p_header Header of compressed data.*/
void initUncompContextLZ(LZContext* p_context, u8* p_dst, CompressionHeader* p_header);

/*! \brief Initializes decompression context and header for Huffman.
    \param p_context Context data for streaming decompression.
    \param p_dst Destination buffer.
    \param p_header Header of compressed data.*/
void initUncompContextHuff(HuffContext* p_context, u8* p_dst, CompressionHeader* p_header);


/*! \brief Decompresses streaming Run Length Encoded data.
    \param p_context Context data for streaming decompression.
    \param p_data Compressed data.
    \param p_len Amount of compressed data.
    \return Amount of decompressed data.*/
s32 readUncompRL(RLContext* p_context, const u8* p_data, u32 p_len);

/*! \brief Decompresses streaming LZ77 (ex) compressed data.
    \param p_context Context data for streaming decompression.
    \param p_data Compressed data.
    \param p_len Amount of compressed data.
    \return Amount of decompressed data.*/
s32 readUncompLZ(LZContext* p_context, const u8* p_data, u32 p_len);

/*! \brief Decompresses streaming Huffman compressed data.
    \param p_context Context data for streaming decompression.
    \param p_data Compressed data.
    \param p_len Amount of compressed data.
    \return Amount of decompressed data.*/
s32 readUncompHuff(HuffContext* p_context, const u8* data, u32 p_len);


/*! \brief Checks whether streaming Run Length Encoding decompression has finished.
    \param p_context Context data for streaming decompression.
    \return Whether decompression has finished.*/
bool isFinishedUncompRL(const RLContext* p_context);

/*! \brief Checks whether streaming LZ77 (ex) decompression has finished.
    \param p_context Context data for streaming decompression.
    \return Whether decompression has finished.*/
bool isFinishedUncompLZ(const LZContext* p_context);

/*! \brief Checks whether streaming Huffman decompression has finished.
    \param p_context Context data for streaming decompression.
    \return Whether decompression has finished.*/
bool isFinishedUncompHuff(const HuffContext* p_context);

/*! \brief Compresses data using no compression.
    \param p_src Source buffer.
    \param p_size Size of source buffer.
    \param p_dst Destination buffer, at least p_size + 8 bytes large.
    \return Size of compressed data, at most p_bytes + 8 bytes.*/
u32 compressNone(const u8* p_src, u32 p_size, u8* p_dst);

/*! \brief Compresses data using Run Length Encoding.
    \param p_src Source buffer.
    \param p_size Size of source buffer.
    \param p_dst Destination buffer, at least p_size bytes large.
    \return Size of compressed data, 0 when compressed data is larger than source.*/
u32 compressRL(const u8* p_src, u32 p_size, u8* p_dst);

/*! \brief Compresses data using LZ77 (extended) compression.
    \param p_src Source buffer.
    \param p_size Size of source buffer.
    \param p_dst Destination buffer, at least p_size bytes large.
    \param p_ex Whether to use extended LZ77 (slower, but better compression).
    \param p_fast Whether to optimize the compression at the cost of some memory.
    \return Size of compressed data, 0 when compressed data is larger than source.*/
u32 compressLZ(const u8* p_src, u32 p_size, u8* p_dst, bool p_ex, bool p_fast = true);

/*! \brief Compresses data using FastLZ compression.
    \param p_src Source buffer.
    \param p_size Size of source buffer.
    \param p_dst Destination buffer, at least p_size + 5% + 8 bytes large.
    \return Size of compressed data, 0 when compressed data is larger than source. */
u32 compressFastLZ(const u8* p_src, u32 p_size, u8* p_dst);

/*! \brief Compresses data using Huffman encoding.
    \param p_src Source buffer.
    \param p_size Size of source buffer.
    \param p_dst Destination buffer, at least p_size bytes large.
    \param p_4bit Whether the bit size should be 4 or 8.
    \return Size of compressed data, 0 when compressed data is larger than source.*/
u32 compressHuff(const u8* p_src, u32 p_size, u8* p_dst, bool p_4bit);

//u32 compressRangeCoder

// Namespace end
}
}


#endif  // !defined(INC_TT_COMPRESSION_COMPRESSION_H)
