/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implemantion of RFC 1321

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*/

#include <tt/http/helpers.h>
#include <tt/math/hash/MD5.h>
#include <tt/mem/util.h>


namespace tt {
namespace math {
namespace hash {

// Constants for MD5Transform routine.
enum
{
	S11 = 7,
	S12 = 12,
	S13 = 17,
	S14 = 22,
	S21 = 5,
	S22 = 9,
	S23 = 14,
	S24 = 20,
	S31 = 4,
	S32 = 11,
	S33 = 16,
	S34 = 23,
	S41 = 6,
	S42 = 10,
	S43 = 15,
	S44 = 21
};

// default ctor, just initailize
MD5::MD5()
:
m_finalized(false)
{
	init();
}


MD5::MD5(const std::string& p_text)
:
m_finalized(false)
{
	init();
	update(p_text.c_str(), static_cast<size_type>(p_text.length()));
	finalize();
}


void MD5::init()
{
	m_finalized = false;
	
	m_count[0] = 0;
	m_count[1] = 0;
	
	// Load magic initialization constants.
	m_state[0] = 0x67452301;
	m_state[1] = 0xEFCDAB89;
	m_state[2] = 0x98BADCFE;
	m_state[3] = 0x10325476;
}


void MD5::decode(u32* p_output, const u8* p_input, size_type p_len)
{
	for (size_type i = 0, j = 0; j < p_len; ++i, j += 4)
	{
		p_output[i] =
			 static_cast<u32>(p_input[j    ])        |
			(static_cast<u32>(p_input[j + 1]) << 8 ) |
			(static_cast<u32>(p_input[j + 2]) << 16) |
			(static_cast<u32>(p_input[j + 3]) << 24);
	}
}


void MD5::encode(u8* p_output, const u32* p_input, size_type p_len)
{
	for (size_type i = 0, j = 0; j < p_len; ++i, j += 4)
	{
		p_output[j    ] = static_cast<u8>( p_input[i]        & 0xFF);
		p_output[j + 1] = static_cast<u8>((p_input[i] >> 8 ) & 0xFF);
		p_output[j + 2] = static_cast<u8>((p_input[i] >> 16) & 0xFF);
		p_output[j + 3] = static_cast<u8>((p_input[i] >> 24) & 0xFF);
	}
}


void MD5::transform(const u8 p_block[blocksize])
{
	u32 a = m_state[0];
	u32 b = m_state[1];
	u32 c = m_state[2];
	u32 d = m_state[3];
	u32 x[16] = { 0 };
	decode(x, p_block, blocksize);
	
	// Round 1
	FF(a, b, c, d, x[ 0], S11, 0xD76AA478); // 1
	FF(d, a, b, c, x[ 1], S12, 0xE8C7B756); // 2
	FF(c, d, a, b, x[ 2], S13, 0x242070DB); // 3
	FF(b, c, d, a, x[ 3], S14, 0xC1BDCEEE); // 4
	FF(a, b, c, d, x[ 4], S11, 0xF57C0FAF); // 5
	FF(d, a, b, c, x[ 5], S12, 0x4787C62A); // 6
	FF(c, d, a, b, x[ 6], S13, 0xA8304613); // 7
	FF(b, c, d, a, x[ 7], S14, 0xFD469501); // 8
	FF(a, b, c, d, x[ 8], S11, 0x698098D8); // 9
	FF(d, a, b, c, x[ 9], S12, 0x8B44F7AF); // 10
	FF(c, d, a, b, x[10], S13, 0xFFFF5BB1); // 11
	FF(b, c, d, a, x[11], S14, 0x895CD7BE); // 12
	FF(a, b, c, d, x[12], S11, 0x6B901122); // 13
	FF(d, a, b, c, x[13], S12, 0xFD987193); // 14
	FF(c, d, a, b, x[14], S13, 0xA679438E); // 15
	FF(b, c, d, a, x[15], S14, 0x49B40821); // 16
	
	// Round 2
	GG(a, b, c, d, x[ 1], S21, 0xF61E2562); // 17
	GG(d, a, b, c, x[ 6], S22, 0xC040B340); // 18
	GG(c, d, a, b, x[11], S23, 0x265E5A51); // 19
	GG(b, c, d, a, x[ 0], S24, 0xE9B6C7AA); // 20
	GG(a, b, c, d, x[ 5], S21, 0xD62F105D); // 21
	GG(d, a, b, c, x[10], S22,  0X2441453); // 22
	GG(c, d, a, b, x[15], S23, 0xD8A1E681); // 23
	GG(b, c, d, a, x[ 4], S24, 0xE7D3FBC8); // 24
	GG(a, b, c, d, x[ 9], S21, 0x21E1CDE6); // 25
	GG(d, a, b, c, x[14], S22, 0xC33707D6); // 26
	GG(c, d, a, b, x[ 3], S23, 0xF4D50D87); // 27
	GG(b, c, d, a, x[ 8], S24, 0x455A14ED); // 28
	GG(a, b, c, d, x[13], S21, 0xA9E3E905); // 29
	GG(d, a, b, c, x[ 2], S22, 0xFCEFA3F8); // 30
	GG(c, d, a, b, x[ 7], S23, 0x676F02D9); // 31
	GG(b, c, d, a, x[12], S24, 0x8D2A4C8A); // 32
	
	// Round 3
	HH(a, b, c, d, x[ 5], S31, 0xFFFA3942); // 33
	HH(d, a, b, c, x[ 8], S32, 0x8771F681); // 34
	HH(c, d, a, b, x[11], S33, 0x6D9D6122); // 35
	HH(b, c, d, a, x[14], S34, 0xFDE5380C); // 36
	HH(a, b, c, d, x[ 1], S31, 0xA4BEEA44); // 37
	HH(d, a, b, c, x[ 4], S32, 0x4BDECFA9); // 38
	HH(c, d, a, b, x[ 7], S33, 0xF6BB4B60); // 39
	HH(b, c, d, a, x[10], S34, 0xBEBFBC70); // 40
	HH(a, b, c, d, x[13], S31, 0x289B7EC6); // 41
	HH(d, a, b, c, x[ 0], S32, 0xEAA127FA); // 42
	HH(c, d, a, b, x[ 3], S33, 0xD4EF3085); // 43
	HH(b, c, d, a, x[ 6], S34,  0X4881D05); // 44
	HH(a, b, c, d, x[ 9], S31, 0xD9D4D039); // 45
	HH(d, a, b, c, x[12], S32, 0xE6DB99E5); // 46
	HH(c, d, a, b, x[15], S33, 0x1FA27CF8); // 47
	HH(b, c, d, a, x[ 2], S34, 0xC4AC5665); // 48
	
	// Round 4
	II(a, b, c, d, x[ 0], S41, 0xF4292244); // 49
	II(d, a, b, c, x[ 7], S42, 0x432AFF97); // 50
	II(c, d, a, b, x[14], S43, 0xAB9423A7); // 51
	II(b, c, d, a, x[ 5], S44, 0xFC93A039); // 52
	II(a, b, c, d, x[12], S41, 0x655B59C3); // 53
	II(d, a, b, c, x[ 3], S42, 0x8F0CCC92); // 54
	II(c, d, a, b, x[10], S43, 0xFFEFF47D); // 55
	II(b, c, d, a, x[ 1], S44, 0x85845DD1); // 56
	II(a, b, c, d, x[ 8], S41, 0x6FA87E4F); // 57
	II(d, a, b, c, x[15], S42, 0xFE2CE6E0); // 58
	II(c, d, a, b, x[ 6], S43, 0xA3014314); // 59
	II(b, c, d, a, x[13], S44, 0x4E0811A1); // 60
	II(a, b, c, d, x[ 4], S41, 0xF7537E82); // 61
	II(d, a, b, c, x[11], S42, 0xBD3AF235); // 62
	II(c, d, a, b, x[ 2], S43, 0x2AD7D2BB); // 63
	II(b, c, d, a, x[ 9], S44, 0xEB86D391); // 64
	
	m_state[0] += a;
	m_state[1] += b;
	m_state[2] += c;
	m_state[3] += d;
	
	// Zeroize sensitive information.
	mem::zero8(x, sizeof(x));
}


void MD5::update(const u8* p_input, size_type p_length)
{
	// Compute number of bytes mod 64
	size_type index = m_count[0] / 8 % blocksize;
	
	// Update number of bits
	if ((m_count[0] += (p_length << 3)) < (p_length << 3))
	{
		++m_count[1];
	}
	m_count[1] += (p_length >> 29);
	
	// Number of bytes we need to fill in buffer
	size_type firstpart = 64 - index;
	
	size_type i = 0;
	
	// Transform as many times as possible.
	if (p_length >= firstpart)
	{
		// Fill buffer first, transform
		mem::copy8(&m_buffer[index], p_input, static_cast<mem::size_type>(firstpart));
		transform(m_buffer);
		
		// Transform chunks of blocksize (64 bytes)
		for (i = firstpart; i + blocksize <= p_length; i += blocksize)
		{
			transform(&p_input[i]);
		}
		
		index = 0;
	}
	
	// Buffer remaining input
	mem::copy8(&m_buffer[index], &p_input[i], static_cast<mem::size_type>(p_length - i));
}


void MD5::update(const char* p_input, size_type p_length)
{
	update(reinterpret_cast<const u8*>(p_input), p_length);
}


MD5& MD5::finalize()
{
	static u8 padding[64] =
	{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	
	if (m_finalized == false)
	{
		// Save number of bits
		u8 bits[8] = { 0 };
		encode(bits, m_count, 8);
		
		// Pad out to 56 mod 64.
		size_type index  = m_count[0] / 8 % 64;
		size_type padLen = (index < 56) ? (56 - index) : (120 - index);
		update(padding, padLen);
		
		// Append length (before padding)
		update(bits, 8);
		
		// Store state in digest
		encode(m_digest, m_state, 16);
		
		// Zeroize sensitive information.
		mem::zero8(m_buffer, static_cast<mem::size_type>(sizeof(m_buffer)));
		mem::zero8(m_count,  static_cast<mem::size_type>(sizeof(m_count)));
		
		m_finalized = true;
	}
	
	return *this;
}


std::string MD5::hexdigest() const
{
	if (m_finalized == false)
	{
		return std::string();
	}
	
	return http::bytesToHex(m_digest, 16, true);
}

// Namespace end
}
}
}
