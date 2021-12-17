#include <tt/math/hash/CRC16CCITT.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


CRC16CCITT::CRCTable* CRC16CCITT::m_table = 0;
int                   CRC16CCITT::m_references = 0;


CRC16CCITT::CRC16CCITT()
:
m_context(StandardInit)
{
	addReference();
}


CRC16CCITT::CRC16CCITT(const CRC16CCITT& p_rhs)
:
m_context(p_rhs.m_context)
{
	addReference();
}


CRC16CCITT::~CRC16CCITT()
{
	removeReference();
}


CRC16CCITT& CRC16CCITT::operator=(const CRC16CCITT& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u16 CRC16CCITT::getCRC() const
{
	return m_context;
}


u16 CRC16CCITT::getHash() const
{
	return m_context;
}


void CRC16CCITT::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "CRC16CCITT::update: input buffer is 0");
	
	calcCRC(m_context, p_input, p_length);
}


u16 CRC16CCITT::calcCRC(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "CRC16CCITT::calcCRC: input buffer is 0");
	
	u16 crc = StandardInit;
	calcCRC(crc, p_data, p_length);
	return crc;
}


void CRC16CCITT::addReference()
{
	if ( m_references == 0 )
	{
		m_table = new CRCTable;
		
		u32 r;
		u16* t = m_table->table;
		
		for ( int i = 0; i < 256; i++ )
		{
			r = static_cast<u32>(i) << 8;
			for ( int j = 0; j < 8; j++ )
			{
				if ( r & 0x8000 )
				{
					r = (r << 1) ^ StandardPoly;
				}
				else
				{
					r <<= 1;
				}
			}
			t[i] = static_cast<u16>(r);
		}
	}
	++m_references;
}


void CRC16CCITT::removeReference()
{
	--m_references;
	
	if ( m_references == 0 )
	{
		delete m_table;
		m_table = 0;
	}
}


void CRC16CCITT::calcCRC(u16& p_crc, const void* p_input, size_t p_length)
{
	u32 r;
	const u16* t = m_table->table;
	const u8* data = static_cast<const u8*>(p_input);
	
	r = static_cast<u32>(p_crc);
	for ( int i = 0; i < static_cast<int>(p_length); i++ )
	{
		r = (r << 8) ^ t[((r >> 8) ^ *data) & 0xff];
		data++;
	}
	p_crc = static_cast<u16>(r);
}

// Namespace end
}
}
}
