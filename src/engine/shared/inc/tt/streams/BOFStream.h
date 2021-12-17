#if !defined(INC_TT_STREAMS_BOFSTREAM_H)
#define INC_TT_STREAMS_BOFSTREAM_H


#include <cstddef>
#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace streams {

/*! \brief Binary Output File Stream class. */
class BOFStream : public BOStream
{
public:
	explicit BOFStream(const std::string& p_filename, fs::identifier p_type = 0);
	explicit BOFStream(const fs::FilePtr& p_file);
	virtual ~BOFStream();
	
private:
	BOFStream(const BOFStream&);
	BOFStream& operator=(const BOFStream&);
	
	virtual std::ptrdiff_t writeBytes(const u8* p_buffer, std::ptrdiff_t p_number);
	
	fs::FilePtr m_file;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STREAMS_BOFSTREAM_H)
