#if !defined(INC_TT_STREAMS_BISTREAM_H)
#define INC_TT_STREAMS_BISTREAM_H


#include <cstddef>

#include <tt/platform/tt_types.h>
#include <tt/streams/BIOStreamBase.h>


namespace tt {
namespace streams {

/*! \brief Binary Input Stream class. */
class BIStream : public BIOStreamBase
{
public:
	class Sentry
	{
	public:
		explicit Sentry(BIStream& p_bis);
		~Sentry()
		{
		}
		operator bool() const { return m_ok; }
		
	private:
		Sentry(const Sentry&);
		Sentry& operator=(const Sentry&);
		
		bool m_ok;
	};
	friend class Sentry;
	
	
	BIStream();
	virtual ~BIStream();
	
	virtual std::ptrdiff_t readBytes(u8* p_buffer, std::ptrdiff_t) = 0;
	
	BIStream& operator>>(BIStream& (*function)(BIStream&));
	
	BIStream& operator>>(bool& p_n);
	BIStream& operator>>(u8& p_n);
	BIStream& operator>>(s8& p_n);
	BIStream& operator>>(u16& p_n);
	BIStream& operator>>(s16& p_n);
	BIStream& operator>>(u32& p_n);
	BIStream& operator>>(s32& p_n);
	BIStream& operator>>(u64& p_n);
	BIStream& operator>>(s64& p_n);
	BIStream& operator>>(float& p_n);
	BIStream& operator>>(double& p_n);
	
	BIStream& get(u8& p_n);
	BIStream& read(u8* p_buffer, streamsize p_length);
	
	
private:
	BIStream(const BIStream&);
	BIStream& operator=(const BIStream&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STREAMS_BISTREAM_H)
