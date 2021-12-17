//******************************************************************************
//* HMAC_SHA1.cpp : Implementation of HMAC SHA1 algorithm
//*                 Comfort to RFC 2104
//*
//******************************************************************************
#include <iostream>
#include <memory>

#include <tt/math/hash/hmac_sha1.h>
#include <tt/mem/util.h>


namespace tt {
namespace math {
namespace hash {

void calcHmacSha1Digest(const u8* p_text, u32 p_textLen,
                        const u8* p_key,  u32 p_keyLen,
                        u8* p_digest)
{
	// STEP 1
	u8 sha1Key[SHA1::SHA1_BLOCK_SIZE] = { 0 };
	
	if (p_keyLen > SHA1::SHA1_BLOCK_SIZE)
	{
		SHA1 keyHasher;
		keyHasher.Reset();
		keyHasher.Update(p_key, p_keyLen);
		keyHasher.Final();
		
		keyHasher.GetHash(sha1Key);
	}
	else
	{
		mem::copy8(sha1Key, p_key, static_cast<mem::size_type>(p_keyLen));
	}
	
	// STEP 2
	u8 innerPad[SHA1::SHA1_BLOCK_SIZE];
	mem::fill8(innerPad, 0x36, sizeof(innerPad));
	
	for (size_t i = 0; i < sizeof(innerPad); ++i)
	{
		innerPad[i] ^= sha1Key[i];
	}
	
	// STEP 3
	u8 report[SHA1::SHA1_DIGEST_LENGTH] = { 0 };
	const u32 appendBuf1Size = static_cast<u32>(sizeof(innerPad) + p_textLen);
	u8* appendBuf1 = new u8[appendBuf1Size];
	mem::copy8(appendBuf1, innerPad, static_cast<mem::size_type>(sizeof(innerPad)));
	mem::copy8(appendBuf1 + sizeof(innerPad), p_text, static_cast<mem::size_type>(p_textLen));
	
	// STEP 4
	{
		SHA1 sha1;
		sha1.Update(appendBuf1, appendBuf1Size);
		sha1.Final();
		
		sha1.GetHash(report);
	}
	
	delete[] appendBuf1;
	appendBuf1 = 0;
	
	// STEP 5
	u8 outerPad[SHA1::SHA1_BLOCK_SIZE];
	mem::fill8(outerPad, 0x5C, sizeof(outerPad));
	for (size_t j = 0; j < sizeof(outerPad); ++j)
	{
		outerPad[j] ^= sha1Key[j];
	}
	
	// STEP 6
	static const u32 appendBuf2Size = SHA1::SHA1_BLOCK_SIZE + SHA1::SHA1_DIGEST_LENGTH;
	u8 appendBuf2[appendBuf2Size] = { 0 };
	mem::copy8(appendBuf2, outerPad, static_cast<mem::size_type>(sizeof(outerPad)));
	mem::copy8(appendBuf2 + sizeof(outerPad), report, SHA1::SHA1_DIGEST_LENGTH);
	
	// STEP 7
	{
		SHA1 sha1;
		sha1.Reset();
		sha1.Update(appendBuf2, appendBuf2Size);
		sha1.Final();
		
		sha1.GetHash(p_digest);
	}
}


void calcHmacSha1Digest(const std::string& p_text,
                        const std::string& p_key,
                        u8*                p_digest)
{
	calcHmacSha1Digest(
		reinterpret_cast<const u8*>(p_text.c_str()), static_cast<u32>(p_text.length()),
		reinterpret_cast<const u8*>(p_key.c_str()),  static_cast<u32>(p_key.length()),
		p_digest);
}

// Namespace end
}
}
}
