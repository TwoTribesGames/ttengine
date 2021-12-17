#if !defined(INC_TT_STREAMS_BIFSTREAM_H)
#define INC_TT_STREAMS_BIFSTREAM_H


#include <cstddef>
#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/streams/BIStream.h>


namespace tt {
namespace streams {

/*! \brief Binary Input File Stream class. */
class BIFStream : public BIStream
{
public:
	explicit BIFStream(const std::string& p_filename, fs::identifier p_type = 0);
	explicit BIFStream(const fs::FilePtr& p_file);
	virtual ~BIFStream();
	
private:
	BIFStream(const BIFStream&);
	BIFStream& operator=(const BIFStream&);
	
	virtual std::ptrdiff_t readBytes(u8* p_buffer, std::ptrdiff_t p_number);
	
	fs::FilePtr m_file;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STREAMS_BIFSTREAM_H)
