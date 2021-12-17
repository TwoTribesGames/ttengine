#include <tt/math/hash/CRC16.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


CRC16::CRCTable* CRC16::m_table = 0;
int              CRC16::m_references = 0;


CRC16::CRC16()
:
m_context(StandardInit)
{
	addReference();
}


CRC16::CRC16(const CRC16& p_rhs)
:
m_context(p_rhs.m_context)
{
	addReference();
}


CRC16::~CRC16()
{
	removeReference();
}


CRC16& CRC16::operator=(const CRC16& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u16 CRC16::getCRC() const
{
	return m_context;
}


u16 CRC16::getHash() const
{
	return m_context;
}


void CRC16::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "CRC16::update: input buffer is 0");
	
	calcCRC(m_context, p_input, p_length);
}


u16 CRC16::calcCRC(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "CRC16::calcCRC: input buffer is 0");
	
	u16 crc = StandardInit;
	calcCRC(crc, p_data, p_length);
	return crc;
}


void CRC16::addReference()
{
	if ( m_references == 0 )
	{
		m_table = new CRCTable;
		
		u32 r;
		u16* t = m_table->table;
		
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
			t[i] = static_cast<u16>(r);
		}
	}
	++m_references;
}


void CRC16::removeReference()
{
	--m_references;
	
	if ( m_references == 0 )
	{
		delete m_table;
		m_table = 0;
	}
}


void CRC16::calcCRC(u16& p_crc, const void* p_input, size_t p_length)
{
	u32 r;
	const u16* t = m_table->table;
	const u8* data = static_cast<const u8*>(p_input);
	
	r = static_cast<u32>(p_crc);
	for ( int i = 0; i < static_cast<int>(p_length); i++ )
	{
		r = (r >> 8) ^ t[(r ^ *data) & 0xff];
		data++;
	}
	p_crc = static_cast<u16>(r);
}

// Namespace end
}
}
}
