/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implementation of RFC 1321

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

#if !defined(INC_TT_MATH_HASH_MD5_H)
#define INC_TT_MATH_HASH_MD5_H


#include <iostream>
#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

/*! \brief A small class for calculating MD5 hashes of strings or byte arrays.
           It is not meant to be fast or secure.

 Usage: 1) feed it blocks of uchars with update()
        2) finalize()
        3) get hexdigest() string
        or
        MD5(std::string).hexdigest() */
class MD5
{
public:
	typedef u32 size_type; // must be 32bit
	
	
	MD5();
	/*! \brief Shortcut constructor; compute MD5 for string and finalize it right away. */
	explicit MD5(const std::string& p_text);
	
	/*! \brief MD5 block update operation. Continues an MD5 message-digest operation,
	           processing another message block. */
	void update(const u8* p_buf, size_type p_length);
	
	/*! \brief Convenience overload of u8 version, so that strings can be passed more easily. */
	void update(const char* p_buf, size_type p_length);
	
	/*! \brief MD5 finalization. Ends an MD5 message-digest operation, writing
	           the message digest and zeroizing the context. */
	MD5& finalize();
	
	/*! \return Hex representation of MD5 digest as string. */
	std::string hexdigest() const;
	
	friend std::ostream& operator<<(std::ostream& p_out, const MD5& p_md5);
	
private:
	enum { blocksize = 64 }; // VC6 won't eat a const static int here
	
	void init();
	
	/*! \brief Apply MD5 algo on a block. */
	void transform(const u8 block[blocksize]);
	/*! \brief Decodes p_input (u8) into p_output (u32). Assumes p_len is a multiple of 4. */
	static void decode(u32* p_output, const u8* p_input, size_type p_len);
	/*! \brief Encodes p_input (u32) into p_output (u8). Assumes p_len is a multiple of 4. */
	static void encode(u8* p_output, const u32* p_input, size_type p_len);
	
	// low level logic operations
	// F, G, H and I are basic MD5 functions.
	static inline u32 F(u32 x, u32 y, u32 z) { return (x & y) | (~x & z); }
	static inline u32 G(u32 x, u32 y, u32 z) { return (x & z) | (y & ~z); }
	static inline u32 H(u32 x, u32 y, u32 z) { return x ^ y ^ z;          }
	static inline u32 I(u32 x, u32 y, u32 z) { return y ^ (x | ~z);       }
	
	/*! \brief Rotates x left n bits. */
	static inline u32 rotate_left(u32 x, int n) { return (x << n) | (x >> (32 - n)); }
	
	// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
	// Rotation is separate from addition to prevent recomputation.
	static inline void FF(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac)
	{ a = rotate_left(a+ F(b, c, d) + x + ac, static_cast<u8>(s)) + b; }
	static inline void GG(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac)
	{ a = rotate_left(a + G(b, c, d) + x + ac, static_cast<u8>(s)) + b; }
	static inline void HH(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac)
	{ a = rotate_left(a + H(b, c, d) + x + ac, static_cast<u8>(s)) + b; }
	static inline void II(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac)
	{ a = rotate_left(a + I(b, c, d) + x + ac, static_cast<u8>(s)) + b; }
	
	
	bool m_finalized;
	u8   m_buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
	u32  m_count[2];   // 64bit counter for number of bits (lo, hi)
	u32  m_state[4];   // m_digest so far
	u8   m_digest[16]; // the result
};


inline std::ostream& operator<<(std::ostream& p_out, const MD5& p_md5)
{
	return p_out << p_md5.hexdigest();
}

inline std::string md5(const std::string& p_str)
{
	MD5 m(p_str);
	return m.hexdigest();
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MATH_HASH_MD5_H)
