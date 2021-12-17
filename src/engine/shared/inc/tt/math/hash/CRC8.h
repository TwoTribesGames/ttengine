#ifndef INC_TT_MATH_HASH_CRC8_H
#define INC_TT_MATH_HASH_CRC8_H

#include <stddef.h>

#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

class CRC8
{
public:
	CRC8();
	CRC8(const CRC8& p_rhs);
	~CRC8();
	
	CRC8& operator=(const CRC8& p_rhs);
	
	u8 getCRC() const;
	u8 getHash() const;
	void update(const void* p_input, size_t p_length);
	u8 calcCRC(const void* p_data, size_t p_length) const;
	
private:
	enum
	{
		StandardPoly = 0x07,
		StandardInit = 0x00
	};
	
	struct CRCTable
	{
		u8 table[256];
	};
	
	// basic reference counting
	static void addReference();
	static void removeReference();
	static void calcCRC(u8& p_crc, const void* p_data, size_t p_length);

	static CRCTable* m_table;
	static int       m_references;
	
	u8 m_context;
};

// Namespace end
}
}
}

#endif // !defined(INC_TT_MATH_HASH_CRC8_H)
