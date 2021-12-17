#include <tt/mem/util.h>
#include <tt/platform/tt_compile_time_error.h>
#include <tt/streams/BIStream.h>


namespace tt {
namespace streams {


BIStream::Sentry::Sentry(BIStream& p_bis)
:
m_ok(false)
{
	if (p_bis.isGood())
	{
		m_ok = true;
	}
	else
	{
		p_bis.setState(BIOStreamBase::IOState_FailBit);
	}
}


BIStream::BIStream()
{
}


BIStream::~BIStream()
{
}


BIStream& BIStream::operator>>(BIStream& (*function)(BIStream&))
{
	return function(*this);
}


BIStream& BIStream::operator>>(bool& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 val = 0;
		if ( readBytes(&val, 1) == 1 )
		{
			if ( val == 0 )
			{
				p_n = false;
			}
			else
			{
				p_n = true;
			}
		}
		else
		{
			clear(IOState_EOFBit);
		}
	}
	return *this;
}


BIStream& BIStream::operator>>(u8& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		if ( readBytes(&p_n, 1) == 0 )
		{
			clear(IOState_EOFBit);
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(s8& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		if ( readBytes(reinterpret_cast<u8*>(&p_n), 1) == 0 )
		{
			clear(IOState_EOFBit);
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(u16& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u16)] = {0};
		if ( readBytes(data, sizeof(u16)) != sizeof(u16) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(u16); ++i)
				{
					p_n += static_cast<u16>(data[i] << (u16(i) * 8));
				}
			}
			else
			{
				for (std::size_t i = sizeof(u16); i > 0; --i)
				{
					p_n += static_cast<u16>(data[sizeof(u16) - i] << (u16(i - 1) * 8));
				}
			}
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(s16& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s16)] = {0};
		if ( readBytes(data, sizeof(s16)) != sizeof(s16) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(s16); ++i)
				{
					p_n += static_cast<s16>(data[i] << (s16(i) * 8));
				}
			}
			else
			{
				for (std::size_t i = sizeof(s16); i > 0; --i)
				{
					p_n += static_cast<s16>(data[sizeof(s16) - i] << (s16(i - 1) * 8));
				}
			}
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(u32& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u32)] = {0};
		if ( readBytes(data, sizeof(u32)) != sizeof(u32) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(u32); ++i)
				{
					p_n += data[i] << (i * 8);
				}
			}
			else
			{
				for (std::size_t i = sizeof(u32); i > 0; --i)
				{
					p_n += data[sizeof(u32) - i] << (u32(i - 1) * 8);
				}
			}
		}
	}
	return (*this);
}

BIStream& BIStream::operator>>(s32& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s32)] = {0};
		if ( readBytes(data, sizeof(s32)) != sizeof(s32) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(s32); ++i)
				{
					p_n += data[i] << (i * 8);
				}
			}
			else
			{
				for (std::size_t i = sizeof(s32); i > 0; --i)
				{
					p_n += data[sizeof(s32) - i] << (s32(i - 1) * 8);
				}
			}
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(u64& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(u64)] = {0};
		if ( readBytes(data, sizeof(u64)) != sizeof(u64) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(u64); ++i)
				{
					p_n += u64(data[i]) << (i * 8);
				}
			}
			else
			{
				for (std::size_t i = sizeof(u64); i > 0; --i)
				{
					p_n += data[sizeof(u64) - i] << (u64(i - 1) * 8);
				}
			}
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(s64& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		u8 data[sizeof(s64)] = {0};
		if ( readBytes(data, sizeof(s64)) != sizeof(s64) )
		{
			clear(IOState_EOFBit);
		}
		else
		{
			p_n = 0;
			if ( usesLittleEndian() )
			{
				for (std::size_t i = 0; i < sizeof(s64); ++i)
				{
					p_n += s64(data[i]) << (i * 8);
				}
			}
			else
			{
				for (std::size_t i = sizeof(s64); i > 0; --i)
				{
					p_n += data[sizeof(s64) - i] << (s64(i - 1) * 8);
				}
			}
		}
	}
	return (*this);
}


BIStream& BIStream::operator>>(float& p_n)
{
	u32 n = 0;
	BIStream& ret = operator>>(n);
	
	TT_STATIC_ASSERT(sizeof(float) == sizeof(u32));
	mem::copy8(&p_n, &n, sizeof(float));
	
	return ret;
}


BIStream& BIStream::operator>>(double& p_n)
{
	u64 n = 0;
	BIStream& ret = operator>>(n);
	
	TT_STATIC_ASSERT(sizeof(double) == sizeof(u64));
	mem::copy8(&p_n, &n, sizeof(double));
	
	return ret;
}


BIStream& BIStream::get(u8& p_n)
{
	Sentry ok(*this);
	if ( ok )
	{
		if ( readBytes(&p_n, 1) == 0 )
		{
			clear(IOState_EOFBit);
		}
	}
	return (*this);
}


BIStream& BIStream::read(u8* p_buffer, streamsize p_length)
{
	Sentry ok(*this);
	if ( ok )
	{
		if ( readBytes(p_buffer, p_length) != p_length )
		{
			clear(IOState_EOFBit);
		}
	}
	return (*this);
}

// Namespace end
}
}
