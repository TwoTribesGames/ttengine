#include <tt/mem/util.h>
#include <tt/platform/tt_compile_time_error.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace streams {


BOStream::Sentry::Sentry(BOStream& p_bos)
:
m_ok(false),
m_bos(p_bos)
{
	if ( m_bos.isGood() )
	{
		m_ok = true;
	}
	else
	{
		m_bos.setState(BIOStreamBase::IOState_FailBit);
	}
}


BOStream::Sentry::~Sentry()
{
	if ( m_bos.hasFailed() == false )
	{
		//m_bos.flush();
	}
}


BOStream::BOStream()
{
}


BOStream::~BOStream()
{
	// flush?
}


BOStream& BOStream::operator<<(BOStream& (*function)(BOStream&))
{
	return function(*this);
}


BOStream& BOStream::operator<<(bool p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		put(u8(p_n ? 1 : 0));
	}
	return *this;
}


BOStream& BOStream::operator<<(u8 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		put(p_n);
	}
	return *this;
}


BOStream& BOStream::operator<<(s8 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		put(static_cast<u8>(p_n));
	}
	return *this;
}


BOStream& BOStream::operator<<(u16 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u16)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(u16); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(u16); i > 0; --i)
			{
				data[sizeof(u16) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(u16));
	}
	return *this;
}


BOStream& BOStream::operator<<(s16 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s16)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(s16); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(s16); i > 0; --i)
			{
				data[sizeof(s16) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(s16));
	}
	return *this;
}


BOStream& BOStream::operator<<(u32 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u32)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(u32); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(u32); i > 0; --i)
			{
				data[sizeof(u32) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(u32));
	}
	return *this;
}


BOStream& BOStream::operator<<(s32 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s32)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(s32); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(s32); i > 0; --i)
			{
				data[sizeof(s32) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(s32));
	}
	return *this;
}


BOStream& BOStream::operator<<(u64 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u64)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(u64); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(u64); i > 0; --i)
			{
				data[sizeof(u64) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(u64));
	}
	return *this;
}


BOStream& BOStream::operator<<(s64 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s64)];
		
		if ( usesLittleEndian() )
		{
			for (std::size_t i = 0; i < sizeof(s64); ++i)
			{
				data[i] = static_cast<u8>((p_n >> (i * 8)) & 0xFF);
			}
		}
		else
		{
			for (std::size_t i = sizeof(s64); i > 0; --i)
			{
				data[sizeof(s64) - i] = static_cast<u8>((p_n >> ((i - 1) * 8)) & 0xFF);
			}
		}
		
		writeBytes(data, sizeof(s64));
	}
	return *this;
}


BOStream& BOStream::operator<<(float p_n)
{
	u32 n = 0;
	TT_STATIC_ASSERT(sizeof(float) == sizeof(u32));
	mem::copy8(&n, &p_n, sizeof(u32));
	return operator<<(n);
}


BOStream& BOStream::operator<<(double p_n)
{
	u64 n = 0;
	TT_STATIC_ASSERT(sizeof(double) == sizeof(u64));
	mem::copy8(&n, &p_n, sizeof(u64));
	return operator<<(n);
}


BOStream& BOStream::put(u8 p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		writeBytes(&p_n, sizeof(p_n));
	}
	return *this;
}


BOStream& BOStream::write(const u8* p_buffer, streamsize p_length)
{
	Sentry ok(*this);
	if ( ok )
	{
		writeBytes(p_buffer, p_length);
	}
	return *this;
}

// Namespace end
}
}
