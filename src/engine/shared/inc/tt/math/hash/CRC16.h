#ifndef INC_TT_MATH_HASH_CRC16_H
#define INC_TT_MATH_HASH_CRC16_H

#include <stddef.h>

#include <tt/platform/tt_types.h>


namespace tt {
namespace math {
namespace hash {

class CRC16
{
public:
	CRC16();
	CRC16(const CRC16& p_rhs);
	~CRC16();
	
	CRC16& operator=(const CRC16& p_rhs);
	
	u16 getCRC() const;
	u16 getHash() const;
	void update(const void* p_input, size_t p_length);
	u16 calcCRC(const void* p_data, size_t p_length) const;
	
private:
	enum
	{
		StandardPoly = 0xa001,
		StandardInit = 0x0000
	};
	
	struct CRCTable
	{
		u16 table[256];
	};
	
	// basic reference counting
	static void addReference();
	static void removeReference();
	static void calcCRC(u16& p_crc, const void* p_data, size_t p_length);
	
	static CRCTable* m_table;
	static int       m_references;
	
	u16 m_context;
};

// Namespace end
}
}
}

#endif // !defined(INC_TT_MATH_HASH_CRC16_H)
