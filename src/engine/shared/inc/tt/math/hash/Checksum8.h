#if 0	// Flagged for deletion

#ifndef INC_TT_MATH_HASH_CHECKSUM8_H
#define INC_TT_MATH_HASH_CHECKSUM8_H

#include <stddef.h>

#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

class Checksum8
{
public:
	Checksum8();
	Checksum8(const Checksum8& p_rhs);
	~Checksum8();
	
	Checksum8& operator=(const Checksum8& p_rhs);
	
	u8 getHash() const;
	void update(const void* p_input, size_t p_length);
	u8 calcChecksum(const void* p_data, size_t p_length) const;
	
private:
	
	u32 calcChecksumI(const void* p_data, size_t p_length) const;
	u16 m_context;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_MATH_HASH_CHECKSUM8_H)

#endif