#include <tt/compression/lzma.h>
#include <tt/mem/mem.h>

#include "lzma/LzmaEnc.h"
#include "lzma/LzmaDec.h"


static void *MyAlloc(void *, size_t size)
{
	return tt::mem::alloc(static_cast<tt::mem::size_type>(size));
}


static void MyFree(void *, void *address)
{
	tt::mem::free(address);
}


u32 lzma_compress(const void* p_input, u32 p_inputSize, void* p_output, u32 p_level,
                  u32 p_dictionarySize)
{
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level        = p_level;
	props.dictSize     = p_dictionarySize;
	props.writeEndMark = 0;
	ISzAlloc allocator = { MyAlloc, MyFree };
	
	Byte*       outBuf = static_cast<      Byte*>(p_output);
	const Byte* inBuf  = static_cast<const Byte*>(p_input );
	
	const SizeT srcLen = p_inputSize;
	SizeT dstLen       = p_inputSize;
	SizeT propsSize    = LZMA_PROPS_SIZE;
	
	SRes rc = LzmaEncode(
	    &outBuf[LZMA_PROPS_SIZE], &dstLen,
	    &inBuf[0], srcLen,
	    &props, &outBuf[0], &propsSize, props.writeEndMark, 0, &allocator, &allocator);
	
	if (rc != SZ_OK)
	{
		return 0;
	}
	return static_cast<u32>(dstLen + LZMA_PROPS_SIZE);
}


u32 lzma_decompress(const void* p_input, u32 p_inputSize, void* p_output, u32 p_outputSize)
{
	ISzAlloc allocator = { MyAlloc, MyFree };
	
	Byte*       outBuf = static_cast<      Byte*>(p_output);
	const Byte* inBuf  = static_cast<const Byte*>(p_input );
	
	SizeT srcLen = p_inputSize - LZMA_PROPS_SIZE;
	SizeT dstLen = p_outputSize;
	
	ELzmaStatus nStatus;
	
	SRes rc = LzmaDecode(
		&outBuf[0], &dstLen,
		&inBuf[LZMA_PROPS_SIZE], &srcLen,
		&inBuf[0], LZMA_PROPS_SIZE,
		LZMA_FINISH_END, &nStatus, &allocator);
	
	if (rc != SZ_OK || dstLen != p_outputSize)
	{
		return 0;
	}
	
	return static_cast<u32>(dstLen);
}
