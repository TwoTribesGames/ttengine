#if !defined(INC_TT_MATH_HASH_SHA1_H)
#define INC_TT_MATH_HASH_SHA1_H

/*
	100% free public domain implementation of the SHA-1 algorithm
	by Dominik Reichl <dominik.reichl@t-online.de>
	Web: http://www.dominik-reichl.de/

	Version 1.6 - 2005-02-07 (thanks to Howard Kapustein for patches)
	- You can set the endianness in your files, no need to modify the
	  header file of the SHA1 class any more
	- Aligned data support
	- Made support/compilation of the utility functions (ReportHash
	  and HashFile) optional (useful, if bytes count, for example in
	  embedded environments)

	Version 1.5 - 2005-01-01
	- 64-bit compiler compatibility added
	- Made variable wiping optional (define SHA1_WIPE_VARIABLES)
	- Removed unnecessary variable initializations
	- ROL32 improvement for the Microsoft compiler (using _rotl)

	======== Test Vectors (from FIPS PUB 180-1) ========

	SHA1("abc") =
		A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D

	SHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
		84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1

	SHA1(A million repetitions of "a") =
		34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#if !defined(SHA1_UTILITY_FUNCTIONS) && !defined(SHA1_NO_UTILITY_FUNCTIONS)
#define SHA1_UTILITY_FUNCTIONS
#endif

#include <string>

#ifdef SHA1_UTILITY_FUNCTIONS
#include <stdio.h>  // Needed for file access and sprintf
#include <string.h> // Needed for strcat and strcpy
#endif

#ifdef _MSC_VER
#include <stdlib.h>
#endif

#include <tt/platform/tt_types.h>



// You can define the endian mode in your files, without modifying the SHA1
// source files. Just #define SHA1_LITTLE_ENDIAN or #define SHA1_BIG_ENDIAN
// in your files, before including the SHA1.h header file. If you don't
// define anything, the class defaults to little endian.

#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#define SHA1_LITTLE_ENDIAN
#endif

// Same here. If you want variable wiping, #define SHA1_WIPE_VARIABLES, if
// not, #define SHA1_NO_WIPE_VARIABLES. If you don't define anything, it
// defaults to wiping.

#if !defined(SHA1_WIPE_VARIABLES) && !defined(SHA1_NO_WIPE_VARIABLES)
#define SHA1_WIPE_VARIABLES
#endif


namespace tt {
namespace math {
namespace hash {

/////////////////////////////////////////////////////////////////////////////
// Declare SHA1 workspace

class SHA1
{
public:
	enum
	{
		SHA1_DIGEST_LENGTH = 20,
		SHA1_BLOCK_SIZE    = 64
	};
	
#ifdef SHA1_UTILITY_FUNCTIONS
	// Two different formats for ReportHash(...)
	enum ReportType
	{
		REPORT_HEX   = 0,
		REPORT_DIGIT = 1
	};
#endif
	
	SHA1();
	~SHA1();
	
	void Reset();
	
	// Update the hash value
	// Use this function to hash in binary data and strings
	void Update(const u8* p_data, u32 p_len);
	
#ifdef SHA1_UTILITY_FUNCTIONS
	// Hash in file contents
	bool HashFile(const std::string& p_filename);
#endif
	
	// Finalize hash and report
	void Final();
	
	// Report functions: as pre-formatted and raw data
#ifdef SHA1_UTILITY_FUNCTIONS
	// Get the final hash as a pre-formatted string
	// FIXME: This function is unsafe! Buffer size argument needed!
	void ReportHash(char* p_report, ReportType p_reportType = REPORT_HEX) const;
#endif
	
	/*! \brief Get the raw message digest.
	    \param p_dest Buffer to store hash digest in. Must be at least SHA1_DIGEST_LENGTH bytes in size. */
	void GetHash(u8* p_dest) const;
	
private:
	union SHA1_WORKSPACE_BLOCK
	{
		u8  c[SHA1_BLOCK_SIZE];
		u32 l[SHA1_BLOCK_SIZE / 4];
	};
	
	
	// Private SHA-1 transformation
	void Transform(u32* p_state, const u8* p_buffer);
	
	
	u32 m_state[5];
	u32 m_count[2];
	u32 m_reserved1[1];
	u8  m_buffer[SHA1_BLOCK_SIZE];
	u8  m_digest[SHA1_DIGEST_LENGTH];
	u32 m_reserved2[3];
	
	u8 m_workspace[SHA1_BLOCK_SIZE];
	SHA1_WORKSPACE_BLOCK* m_block; // SHA1 pointer to the byte array above
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MATH_HASH_SHA1_H)
