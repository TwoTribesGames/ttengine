#if !defined(INC_TT_STREAMS_BOSTREAM_H)
#define INC_TT_STREAMS_BOSTREAM_H


#include <cstddef>

#include <tt/streams/BIOStreamBase.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace streams {

/*! \brief Binary Output Stream class. */
class BOStream : public BIOStreamBase
{
public:
	class Sentry
	{
	public:
		explicit Sentry(BOStream& p_bos);
		~Sentry();
		
		operator bool() const {return m_ok;}
	private:
		bool m_ok;
		BOStream& m_bos;
		
		Sentry(const Sentry&);
		Sentry& operator=(const Sentry&);
		
		friend class BOStream;
	};
	friend class Sentry;
	
	
	BOStream();
	virtual ~BOStream();
	
	virtual std::ptrdiff_t writeBytes(const u8* p_buffer, std::ptrdiff_t p_number) = 0;
	
	BOStream& operator<<(BOStream& (*function)(BOStream&));
	
	BOStream& operator<<(bool p_n);
	BOStream& operator<<(u8 p_n);
	BOStream& operator<<(s8 p_n);
	BOStream& operator<<(u16 p_n);
	BOStream& operator<<(s16 p_n);
	BOStream& operator<<(u32 p_n);
	BOStream& operator<<(s32 p_n);
	BOStream& operator<<(u64 p_n);
	BOStream& operator<<(s64 p_n);
	BOStream& operator<<(float p_n);
	BOStream& operator<<(double p_n);
	
	BOStream& put(u8 p_n);
	BOStream& write(const u8* p_buffer, streamsize p_length);
	
private:
	BOStream(const BOStream&);
	BOStream& operator=(const BOStream&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STREAMS_BOSTREAM_H)
