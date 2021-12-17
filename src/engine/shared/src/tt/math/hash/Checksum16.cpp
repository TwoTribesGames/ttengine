#if 0	// Flagged for deletion

#include <tt/math/hash/Checksum16.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


Checksum16::Checksum16()
:
m_context(0)
{
}


Checksum16::Checksum16(const Checksum16& p_rhs)
:
m_context(p_rhs.m_context)
{
}


Checksum16::~Checksum16()
{
}


Checksum16& Checksum16::operator=(const Checksum16& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u16 Checksum16::getHash() const
{
	return static_cast<u16>(~m_context);
}


void Checksum16::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "Checksum16::update: input buffer is 0");
	u32 sum = calcChecksumI(p_input, p_length);
	
	sum += m_context;
	sum += (sum >> 16);
	
	m_context = static_cast<u16>(sum);
}


u16 Checksum16::calcChecksum(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "Checksum16::calcChecksum: input buffer is 0");
	
	u32 sum = calcChecksumI(p_data, p_length);
	
	return static_cast<u16>(~sum);
}


u32 Checksum16::calcChecksumI(const void* p_input, size_t p_length) const
{
	u32 sum = 0;
	bool fSwap = false;
	
	const u8* input = reinterpret_cast<const u8*>(p_input);
	
	if (reinterpret_cast<u32>(input) & 1)
	{
		sum += static_cast<u32>(*input) << 8; // BIGENDIAN
		++input;
		p_length--;
		fSwap = true;
	}
	
	const u16* input2 = reinterpret_cast<const u16*>(input);
	
	while ( ( p_length >> 17) > 0 )
	{
		p_length -= (1 << 17);
		for ( u32 n = (1 << 16); n > 0; n-- )
		{
			sum += *input2;
			++input2;
		}
		
		sum = (sum >> 16) + (sum & 0xffff);
		sum = static_cast<u16>((sum >> 16) + sum);
	}
	
	for (u32 n = static_cast<u32>(p_length >> 1); n > 0; --n)
	{
		sum += *input2;
		++input2;
	}
	
	if ( p_length & 1 )
	{
		u16 input16 = (*input2); // Temp fix for weird internal compiler error with iPhone SDK 3.2 beta 3 when compiling OSX_Test for OS X 10.5
		sum += *reinterpret_cast<const u8*>(&input16);
	}
	
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + (sum & 0xffff);
	
	if ( fSwap )
	{
		sum = ((sum << 24) | (sum << 8)) >> 16;
	}
	
	return sum;
}

// Namespace end
}
}
}

#endif
