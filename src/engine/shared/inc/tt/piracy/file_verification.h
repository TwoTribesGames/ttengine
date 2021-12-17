#if !defined(INC_TT_PIRACTY_FILE_VERIFICATION_H)
#define INC_TT_PIRACTY_FILE_VERIFICATION_H

#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/piracy/obfuscate_string.h>
#include <tt/platform/tt_error.h>


#define VERIFY_FILE_SIZE_A(p_root, p_file, p_fileMask, p_fileSize, p_sizeObfuscator, p_OUT) \
{ \
	p_OUT = p_sizeObfuscator(tt::fs::getFileContent((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file)))->getSize()) != p_sizeObfuscator(p_fileSize); \
	TT_ASSERTMSG(p_OUT == false, "Incorrect filesize of '%s'.", ((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file))).c_str());\
}


#define VERIFY_FILE_SIZE_B(p_root, p_file, p_fileMask, p_fileSize, p_sizeObfuscator, p_OUT) \
{ \
	p_OUT = p_sizeObfuscator(tt::fs::open((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file)), tt::fs::OpenMode_Read)->getLength()) != p_sizeObfuscator(p_fileSize); \
	TT_ASSERTMSG(p_OUT == false, "Incorrect filesize of '%s'.", ((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file))).c_str());\
}


#define VERIFY_FILE_HEADER(p_root, p_file, p_fileMask, p_header, p_headerObfuscator, p_size, p_sizeObfuscator, p_OUT) \
{ \
	tt::code::BufferPtr _buffer(tt::fs::getFileContent((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file)))); \
	if (_buffer == 0 || _buffer->getData() == 0 || _buffer->getSize() == 0) \
	{ \
		p_OUT = true; \
	} \
	else \
	{ \
		const u8* _data = reinterpret_cast<const u8*>(_buffer->getData()); \
		for (int _i = 0; p_sizeObfuscator(_i) != p_sizeObfuscator(p_size) && p_OUT == false; ++_i) \
		{ \
			p_OUT = u8(p_headerObfuscator(_data[_i])) != p_header[_i]; \
			TT_ASSERTMSG(p_OUT == false, "%u: %u(%u) != %u", _i, p_headerObfuscator(_data[_i]), _data[_i], p_header[_i]);\
		} \
	} \
	TT_ASSERTMSG(p_OUT == false, "Incorrect header in '%s'.", ((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file))).c_str());\
}



#define VERIFY_FILE_CONTENT(p_root, p_file, p_fileMask, p_content, p_contentObfuscator, p_size, p_sizeObfuscator, p_OUT) \
{ \
	tt::code::BufferPtr _buffer(tt::fs::getFileContent((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file)))); \
	if (_buffer == 0 || _buffer->getData() == 0 || _buffer->getSize() == 0) \
	{ \
		p_OUT = true; \
	} \
	else \
	{ \
		const u8* _data = reinterpret_cast<const u8*>(_buffer->getData()); \
		for (int _i = 0; (_i < static_cast<int>(_buffer->getSize() - p_size)) && p_OUT == false; ++_i) \
		{ \
			p_OUT = true; \
			for (int _j = 0; (p_sizeObfuscator(_j) != p_sizeObfuscator(p_size)) && p_OUT; ++_j) \
			{ \
				p_OUT = p_contentObfuscator(_data[_i + _j]) == p_content[_j]; \
			} \
		} \
	} \
	TT_ASSERTMSG(p_OUT == false, "Found illegal string in '%s'.", ((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file))).c_str());\
}


// CRC values can be generated with Hex Workshop:
// Select "Custom CRC", fill in polynomial and initial value.
// Reflection (both in and out) should be OFF, XOR out should be 0.
#define VERIFY_FILE_CRC(p_root, p_file, p_fileMask, p_crc, p_crcObfuscator, p_init, p_polynomial, p_bits, p_OUT) \
{ \
	u##p_bits _r; \
	u##p_bits _t[256]; \
	for (u##p_bits _i = 0; _i < 256; _i++) \
	{ \
		_r = _i << (p_bits - 8); \
		for (u##p_bits _j = 0; _j < 8; _j++) \
		{ \
			if (_r & (1 << (p_bits - 1))) \
			{ \
				_r = (_r << 1) ^ (p_polynomial); \
			} \
			else \
			{ \
				_r <<= 1; \
			} \
		} \
		_t[_i] = _r; \
	} \
	\
	_r = u##p_bits(p_init); \
	tt::code::BufferPtr _buffer(tt::fs::getFileContent((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file)))); \
	const u8* _data = reinterpret_cast<const u8*>(_buffer->getData()); \
	 \
	for (u32 _i = 0; _i < static_cast<u32>(_buffer->getSize()); ++_i) \
	{ \
		_r = (_r << 8) ^ _t[((_r >> (p_bits - 8)) ^ *_data) & 0xff]; \
		_data++; \
	} \
	p_OUT = p_crcObfuscator(p_crc) != p_crcObfuscator(_r); \
	TT_ASSERTMSG(p_OUT == false, "Incorrect crc of '%s', expected %08X found %08X.", ((p_root) + tt::piracy::obfuscate((p_fileMask), (p_file))).c_str(), p_crc, _r);\
}


#endif // !defined(INC_TT_PIRACTY_FILE_VERIFICATION_H)
