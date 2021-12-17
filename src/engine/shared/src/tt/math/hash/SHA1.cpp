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

#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/math/hash/SHA1.h>
#include <tt/mem/util.h>


namespace tt {
namespace math {
namespace hash {

#ifdef SHA1_UTILITY_FUNCTIONS
#define SHA1_MAX_FILE_BUFFER 8000
#endif

// Rotate x bits to the left
#ifndef ROL32
#ifdef _MSC_VER
#define ROL32(_val32, _nBits) _rotl(_val32, _nBits)
#else
#define ROL32(_val32, _nBits) (((_val32)<<(_nBits))|((_val32)>>(32-(_nBits))))
#endif
#endif

#ifdef SHA1_LITTLE_ENDIAN
#define SHABLK0(i) (m_block->l[i] = \
	(ROL32(m_block->l[i],24) & 0xFF00FF00) | (ROL32(m_block->l[i],8) & 0x00FF00FF))
#else
#define SHABLK0(i) (m_block->l[i])
#endif

#define SHABLK(i) (m_block->l[i&15] = ROL32(m_block->l[(i+13)&15] ^ m_block->l[(i+8)&15] \
	^ m_block->l[(i+2)&15] ^ m_block->l[i&15],1))

// SHA-1 rounds
#define _R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
#define _R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
#define _R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }


SHA1::SHA1()
{
	(void)m_reserved1[0];
	(void)m_reserved2[0];

	m_block = (SHA1_WORKSPACE_BLOCK*)m_workspace;
	
	Reset();
}


SHA1::~SHA1()
{
	Reset();
}


void SHA1::Reset()
{
	// SHA1 initialization constants
	m_state[0] = 0x67452301;
	m_state[1] = 0xEFCDAB89;
	m_state[2] = 0x98BADCFE;
	m_state[3] = 0x10325476;
	m_state[4] = 0xC3D2E1F0;
	
	m_count[0] = 0;
	m_count[1] = 0;
}


void SHA1::Transform(u32* p_state, const u8* p_buffer)
{
	// Copy p_state[] to working vars
	u32 a = p_state[0];
	u32 b = p_state[1];
	u32 c = p_state[2];
	u32 d = p_state[3];
	u32 e = p_state[4];
	
	mem::copy8(m_block, p_buffer, SHA1_BLOCK_SIZE);
	
	// 4 rounds of 20 operations each. Loop unrolled.
	_R0(a,b,c,d,e, 0); _R0(e,a,b,c,d, 1); _R0(d,e,a,b,c, 2); _R0(c,d,e,a,b, 3);
	_R0(b,c,d,e,a, 4); _R0(a,b,c,d,e, 5); _R0(e,a,b,c,d, 6); _R0(d,e,a,b,c, 7);
	_R0(c,d,e,a,b, 8); _R0(b,c,d,e,a, 9); _R0(a,b,c,d,e,10); _R0(e,a,b,c,d,11);
	_R0(d,e,a,b,c,12); _R0(c,d,e,a,b,13); _R0(b,c,d,e,a,14); _R0(a,b,c,d,e,15);
	_R1(e,a,b,c,d,16); _R1(d,e,a,b,c,17); _R1(c,d,e,a,b,18); _R1(b,c,d,e,a,19);
	_R2(a,b,c,d,e,20); _R2(e,a,b,c,d,21); _R2(d,e,a,b,c,22); _R2(c,d,e,a,b,23);
	_R2(b,c,d,e,a,24); _R2(a,b,c,d,e,25); _R2(e,a,b,c,d,26); _R2(d,e,a,b,c,27);
	_R2(c,d,e,a,b,28); _R2(b,c,d,e,a,29); _R2(a,b,c,d,e,30); _R2(e,a,b,c,d,31);
	_R2(d,e,a,b,c,32); _R2(c,d,e,a,b,33); _R2(b,c,d,e,a,34); _R2(a,b,c,d,e,35);
	_R2(e,a,b,c,d,36); _R2(d,e,a,b,c,37); _R2(c,d,e,a,b,38); _R2(b,c,d,e,a,39);
	_R3(a,b,c,d,e,40); _R3(e,a,b,c,d,41); _R3(d,e,a,b,c,42); _R3(c,d,e,a,b,43);
	_R3(b,c,d,e,a,44); _R3(a,b,c,d,e,45); _R3(e,a,b,c,d,46); _R3(d,e,a,b,c,47);
	_R3(c,d,e,a,b,48); _R3(b,c,d,e,a,49); _R3(a,b,c,d,e,50); _R3(e,a,b,c,d,51);
	_R3(d,e,a,b,c,52); _R3(c,d,e,a,b,53); _R3(b,c,d,e,a,54); _R3(a,b,c,d,e,55);
	_R3(e,a,b,c,d,56); _R3(d,e,a,b,c,57); _R3(c,d,e,a,b,58); _R3(b,c,d,e,a,59);
	_R4(a,b,c,d,e,60); _R4(e,a,b,c,d,61); _R4(d,e,a,b,c,62); _R4(c,d,e,a,b,63);
	_R4(b,c,d,e,a,64); _R4(a,b,c,d,e,65); _R4(e,a,b,c,d,66); _R4(d,e,a,b,c,67);
	_R4(c,d,e,a,b,68); _R4(b,c,d,e,a,69); _R4(a,b,c,d,e,70); _R4(e,a,b,c,d,71);
	_R4(d,e,a,b,c,72); _R4(c,d,e,a,b,73); _R4(b,c,d,e,a,74); _R4(a,b,c,d,e,75);
	_R4(e,a,b,c,d,76); _R4(d,e,a,b,c,77); _R4(c,d,e,a,b,78); _R4(b,c,d,e,a,79);
	
	// Add the working vars back into p_state
	p_state[0] += a;
	p_state[1] += b;
	p_state[2] += c;
	p_state[3] += d;
	p_state[4] += e;
	
	// Wipe variables
#ifdef SHA1_WIPE_VARIABLES
	a = b = c = d = e = 0;
#endif
}


void SHA1::Update(const u8* p_data, u32 p_len)
{
	u32 j = (m_count[0] >> 3) & 63;
	
	if ((m_count[0] += p_len << 3) < (p_len << 3)) ++m_count[1];
	
	m_count[1] += (p_len >> 29);
	
	u32 i = 0;
	if ((j + p_len) > (SHA1_BLOCK_SIZE - 1))
	{
		i = SHA1_BLOCK_SIZE - j;
		mem::copy8(&m_buffer[j], p_data, i);
		Transform(m_state, m_buffer);
		
		for ( ; i + (SHA1_BLOCK_SIZE - 1) < p_len; i += SHA1_BLOCK_SIZE)
		{
			Transform(m_state, &p_data[i]);
		}
		
		j = 0;
	}
	
	mem::copy8(&m_buffer[j], &p_data[i], p_len - i);
}


#ifdef SHA1_UTILITY_FUNCTIONS
bool SHA1::HashFile(const std::string& p_filename)
{
	if (fs::fileExists(p_filename) == false) return false;
	
	fs::FilePtr file(fs::open(p_filename, fs::OpenMode_Read));
	if (file == 0) return false;
	
	fs::size_type fileSize = file->getLength();
	fs::size_type blocks = 0;
	fs::size_type rest   = 0;
	
	if (fileSize != 0)
	{
		blocks = fileSize / SHA1_MAX_FILE_BUFFER;
		rest   = fileSize % SHA1_MAX_FILE_BUFFER;
	}
	
	u8 data[SHA1_MAX_FILE_BUFFER] = { 0 };
	for (fs::size_type i = 0; i < blocks; ++i)
	{
		file->read(data, SHA1_MAX_FILE_BUFFER);
		Update((u8*)data, SHA1_MAX_FILE_BUFFER);
	}
	
	if (rest != 0)
	{
		file->read(data, rest);
		Update((u8*)data, static_cast<u32>(rest));
	}
	
	return true;
}
#endif


void SHA1::Final()
{
	u32 i;
	u8 finalcount[8];
	
	for (i = 0; i < 8; ++i)
	{
		finalcount[i] = static_cast<u8>((m_count[((i >= 4) ? 0 : 1)]
			>> ((3 - (i & 3)) * 8) ) & 255); // Endian independent
	}
	
	Update(reinterpret_cast<const u8*>("\200"), 1);
	
	while ((m_count[0] & 504) != 448)
	{
		Update(reinterpret_cast<const u8*>("\0"), 1);
	}
	
	Update(finalcount, 8); // Cause a SHA1Transform()
	
	for (i = 0; i < SHA1_DIGEST_LENGTH; ++i)
	{
		m_digest[i] = static_cast<u8>((m_state[i >> 2] >> ((3 - (i & 3)) * 8) ) & 255);
	}
	
	// Wipe variables for security reasons
#ifdef SHA1_WIPE_VARIABLES
	i = 0;
	mem::zero8(m_buffer,   SHA1_BLOCK_SIZE);
	mem::zero8(m_state,    5 * sizeof(m_state[0]));
	mem::zero8(m_count,    2 * sizeof(m_count[0]));
	mem::zero8(finalcount, 8);
	Transform(m_state, m_buffer);
#endif
}


#ifdef SHA1_UTILITY_FUNCTIONS
void SHA1::ReportHash(char* p_report, ReportType p_reportType) const
{
	if (p_report == 0) return;
	
	char tempStr[16] = { 0 };
	
	if (p_reportType == REPORT_HEX)
	{
		sprintf(tempStr, "%02X", m_digest[0]);
		strcat(p_report, tempStr);
		
		for (s32 i = 1; i < SHA1_DIGEST_LENGTH; ++i)
		{
			sprintf(tempStr, " %02X", m_digest[i]);
			strcat(p_report, tempStr);
		}
	}
	else if (p_reportType == REPORT_DIGIT)
	{
		sprintf(tempStr, "%u", m_digest[0]);
		strcat(p_report, tempStr);
		
		for (s32 i = 1; i < SHA1_DIGEST_LENGTH; ++i)
		{
			sprintf(tempStr, " %u", m_digest[i]);
			strcat(p_report, tempStr);
		}
	}
	else strcpy(p_report, "Error: Unknown report type!");
}
#endif


void SHA1::GetHash(u8* p_dest) const
{
	mem::copy8(p_dest, m_digest, SHA1_DIGEST_LENGTH);
}

// Namespace end
}
}
}
