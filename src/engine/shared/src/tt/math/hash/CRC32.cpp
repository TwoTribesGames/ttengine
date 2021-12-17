#include <tt/math/hash/CRC32.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


CRC32::CRCTable* CRC32::m_table = 0;
int              CRC32::m_references = 0;


CRC32::CRC32()
:
m_context(u32(StandardInit))
{
	addReference();
}


CRC32::CRC32(const CRC32& p_rhs)
:
m_context(p_rhs.m_context)
{
	addReference();
}


CRC32::~CRC32()
{
	removeReference();
}


CRC32& CRC32::operator=(const CRC32& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u32 CRC32::getCRC() const
{
	return m_context;
}


u32 CRC32::getHash() const
{
	return ~m_context;
}


void CRC32::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "CRC32::update: input buffer is 0");
	
	calcCRC(m_context, p_input, p_length);
}


u32 CRC32::calcCRC(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "CRC32::calcCRC: input buffer is 0");
	
	u32 crc = u32(StandardInit);
	calcCRC(crc, p_data, p_length);
	return ~crc;
}


void CRC32::addReference()
{
	if ( m_references == 0 )
	{
		m_table = new CRCTable;
		
		u32 r;
		u32* t = m_table->table;
		
		for ( int i = 0; i < 256; i++ )
		{
			r = static_cast<u32>(i);
			for ( int j = 0; j < 8; j++ )
			{
				if ( r & 1 )
				{
					r = (r >> 1) ^ StandardPoly;
				}
				else
				{
					r >>= 1;
				}
			}
			t[i] = r;
		}
	}
	++m_references;
}


void CRC32::removeReference()
{
	--m_references;
	
	if ( m_references == 0 )
	{
		delete m_table;
		m_table = 0;
	}
}


void CRC32::calcCRC(u32& p_crc, const void* p_input, size_t p_length)
{
	u32 r;
	const u32* t = m_table->table;
	const u8* data = static_cast<const u8*>(p_input);
	
	r = p_crc;
	for ( int i = 0; i < static_cast<int>(p_length); i++ )
	{
		r = (r >> 8) ^ t[(r ^ *data) & 0xff];
		data++;
	}
	p_crc = r;
}

// Namespace end
}
}
}
