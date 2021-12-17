#if 0	// Flagged for deletion

#ifndef INC_TT_MATH_HASH_CHECKSUM16_H
#define INC_TT_MATH_HASH_CHECKSUM16_H

#include <stddef.h>

#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

class Checksum16
{
public:
	Checksum16();
	Checksum16(const Checksum16& p_rhs);
	~Checksum16();
	
	Checksum16& operator=(const Checksum16& p_rhs);
	
	u16 getHash() const;
	void update(const void* p_input, size_t p_length);
	u16 calcChecksum(const void* p_data, size_t p_length) const;
	
private:
	
	u32 calcChecksumI(const void* p_data, size_t p_length) const;
	u16 m_context;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TT_MATH_HASH_CHECKSUM16_H)

#endif