#if !defined(INC_TT_MATH_HASH_HMACSHA1_H)
#define INC_TT_MATH_HASH_HMACSHA1_H


/// 100% free public domain implementation of the HMAC-SHA1 algorithm
//  by Chien-Chung, Chung (Jim Chung) <jimchung1221@gmail.com>
// Heavily modified by Two Tribes to fix buffer issues and general code cleanliness.

#include <tt/math/hash/SHA1.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

/*! \brief Calculates the HMAC-SHA1 digest for the specified text and key.
    \param p_text Bytes to hash.
    \param p_textLen The number of bytes in p_text.
    \param p_key The key to use for hashing.
    \param p_keyLen The number of bytes in p_key.
    \param p_digest Buffer to store the resulting digest in. Must be at least SHA1_DIGEST_LENGTH bytes big. */
void calcHmacSha1Digest(const u8* p_text, u32 p_textLen,
                        const u8* p_key,  u32 p_keyLen,
                        u8* p_digest);

/*! \brief Convenience overload of the raw buffer version to facilitate hashing strings. */
void calcHmacSha1Digest(const std::string& p_text,
                        const std::string& p_key,
                        u8*                p_digest);

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MATH_HASH_HMACSHA1_H)
