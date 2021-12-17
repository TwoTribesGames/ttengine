#include <tt/math/hash/CRC8.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


CRC8::CRCTable* CRC8::m_table = 0;
int             CRC8::m_references = 0;


CRC8::CRC8()
:
m_context(StandardInit)
{
	addReference();
}


CRC8::CRC8(const CRC8& p_rhs)
:
m_context(p_rhs.m_context)
{
	addReference();
}


CRC8::~CRC8()
{
	removeReference();
}


CRC8& CRC8::operator=(const CRC8& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u8 CRC8::getCRC() const
{
	return m_context;
}


u8 CRC8::getHash() const
{
	return m_context;
}


void CRC8::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "CRC8::update: input buffer is 0");
	
	calcCRC(m_context, p_input, p_length);
}


u8 CRC8::calcCRC(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "CRC8::calcCRC: input buffer is 0");
	
	u8 crc = StandardInit;
	calcCRC(crc, p_data, p_length);
	return crc;
}


void CRC8::addReference()
{
	if ( m_references == 0 )
	{
		m_table = new CRCTable;
		
		u32 r;
		u8 *t = m_table->table;
		
		for ( u32 i = 0; i < 256; i++ )
		{
			r = i;
			for ( int j = 0; j < 8; j++ )
			{
				if (r & 0x80)
				{
					r = (r << 1) ^ StandardPoly;
				}
				else
				{
					r <<= 1;
				}
			}
			t[i] = (u8)r;
		}
	}
	++m_references;
}


void CRC8::removeReference()
{
	--m_references;
	
	if ( m_references == 0 )
	{
		delete m_table;
		m_table = 0;
	}
}


void CRC8::calcCRC(u8& p_crc, const void* p_input, size_t p_length)
{
	u32 r;
	const u8* t = m_table->table;
	const u8 *data = reinterpret_cast<const u8*>(p_input);
	
	r = static_cast<u32>(p_crc);
	for ( int i = 0; i < static_cast<int>(p_length); i++ )
	{
		r = t[(r ^ *data) & 0xff];
		data++;
	}
	p_crc = static_cast<u8>(r);
}

// Namespace end
}
}
}
