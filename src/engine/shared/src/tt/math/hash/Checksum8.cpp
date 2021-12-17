#if 0	// Flagged for deletion

#include <tt/math/hash/Checksum8.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace math {
namespace hash {


Checksum8::Checksum8()
:
m_context(0)
{
	
}


Checksum8::Checksum8(const Checksum8& p_rhs)
:
m_context(p_rhs.m_context)
{
	
}


Checksum8::~Checksum8()
{
	
}


Checksum8& Checksum8::operator=(const Checksum8& p_rhs)
{
	m_context = p_rhs.m_context;
	return *this;
}


u8 Checksum8::getHash() const
{
	u32 sum = m_context;
	sum = (sum >> 8) + (sum & 0xff);
	sum += (sum >> 8);
	
	return static_cast<u8>(~sum);
}


// HACK: Because of an ICE - seg fault don't compile this.
//       Need to proper fix if used. (Will get linker error.)
#if !defined(TT_PLATFORM_OSX) && !defined(TT_BUILD_FINAL)
void Checksum8::update(const void* p_input, size_t p_length)
{
	TT_ASSERTMSG(p_input != 0, "Checksum8::update: input buffer is 0");
	
	u32 sum = calcChecksumI(p_input, p_length);
	
	sum += m_context;
	sum += (sum >> 16);
	
	m_context = static_cast<u16>(sum);
}
#endif


u8 Checksum8::calcChecksum(const void* p_data, size_t p_length) const
{
	TT_ASSERTMSG(p_data != 0, "Checksum8::calcChecksum: input buffer is 0");
	
	u32 sum = calcChecksumI(p_data, p_length);
	
	sum = (sum >> 8) + (sum & 0xff);
	sum += (sum >> 8);
	
	return static_cast<u8>(~sum);
}


u32 Checksum8::calcChecksumI(const void* p_input, size_t p_length) const
{
	u32 sum = 0;
	
	const u8* input = reinterpret_cast<const u8*>(p_input);
	
	if ( reinterpret_cast<u32>(p_input) & 1 )
	{
		sum += *input;
		++input;
		p_length--;
	}
	
	const u16* input2 = reinterpret_cast<const u16*>(input);
	
	while ( (p_length >> 17) > 0 )
	{
		p_length -= (1 << 17);
		for ( u32 n = (1 << 16); n > 0; n-- )
		{
			sum += *input2;
			++input2;
		}
		
		sum = (sum >> 16) + (sum & 0xffff);
		sum = static_cast<u16>(sum + (sum >> 16));
	}
	
	for (u32 n = static_cast<u32>(p_length >> 1); n > 0; --n)
	{
		sum += *input2;
		++input2;
	}
	
	if ( p_length & 1 )
	{
		sum += *(reinterpret_cast<const u8*>(input2));
	}
	
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	
	return static_cast<u16>(sum);
}

// Namespace end
}
}
}

#endif
