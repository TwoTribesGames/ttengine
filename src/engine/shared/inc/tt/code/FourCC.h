#if !defined(INC_TT_CODE_FOURCC_H)
#define INC_TT_CODE_FOURCC_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

/*! \brief Helper to easily create FourCCs (FourCC: four-character code; see http://en.wikipedia.org/wiki/FourCC )
Example usage:

typedef tt::code::FourCC<'C', 'H', 'N', 'K'> ChunkMarker;

tt::fs::writeInteger(file, ChunkMarker::value);

*/
template<char a, char b, char c, char d>
struct FourCC
{
	static const u32 value = (static_cast<u32>(d) << 24) |
	                         (static_cast<u32>(c) << 16) |
	                         (static_cast<u32>(b) << 8)  |
	                          static_cast<u32>(a);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_FOURCC_H)
