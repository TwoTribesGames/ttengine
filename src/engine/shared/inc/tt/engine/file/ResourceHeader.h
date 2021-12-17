#if !defined(INC_TT_ENGINE_FILE_RESOURCEHEADER_H)
#define INC_TT_ENGINE_FILE_RESOURCEHEADER_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/streams/BIStream.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace engine {
namespace file {


// Increase this when binary format changes!
static const u16 formatVersionMajor = 4;
static /*const*/ u16 formatVersionMinor = 0;
// Non-cost for 0 because of the following warning:
// ResourceHeader.h:58: warning: comparison is always false due to limited range of data type


static const s32 fixedStringSize = 64;


enum Platform
{
	Platform_Windows,
	Platform_OSX,
	Platform_Linux,

	Platform_Invalid
};


struct ResourceHeader
{
	u16 versionMajor;
	u16 versionMinor;
	u16 platform;
	u16 reserved;
	
	char name[fixedStringSize];
	char ns  [fixedStringSize];
	
	
	inline ResourceHeader()
	:
	versionMajor(0),
	versionMinor(0),
	platform(0),
	reserved(0)
	{
		for (s32 i = 0; i < fixedStringSize; ++i)
		{
			name[i] = 0;
			ns  [i] = 0;
		}
	}
	
	inline bool checkVersion() const
	{
		if (versionMajor > formatVersionMajor ||
		    (versionMajor == formatVersionMajor && versionMinor > formatVersionMinor))
		{
			TT_PANIC("Converter version higher than libs: get latest packbuild / binary."
			         " (Data v%d.%d, Libs v%d.%d)\n"
			         "Asset ID:        '%s'\nAsset Namespace: '%s'",
			         versionMajor, versionMinor,
			         formatVersionMajor, formatVersionMinor,
			         name, ns);
			return false;
		}
		else if (versionMajor < formatVersionMajor || 
		         (versionMajor == formatVersionMajor && versionMinor < formatVersionMinor))
		{
			TT_PANIC("Old data: update ttsdk & rebuild data."
			         " (Data v%d.%d, Libs v%d.%d)\n"
			         "Asset ID:        '%s'\nAsset Namespace: '%s'",
			         versionMajor, versionMinor,
			         formatVersionMajor, formatVersionMinor,
			         name, ns);
			return false;
		}
		
		return true;
	}
};


// namespace end
}
}
}

inline tt::streams::BOStream& operator<<(tt::streams::BOStream& p_s,
										 const tt::engine::file::ResourceHeader& p_rhs)
{
	p_s << p_rhs.versionMajor;
	p_s << p_rhs.versionMinor;
	p_s << p_rhs.platform;
	p_s << p_rhs.reserved;

	p_s.write(reinterpret_cast<const u8*>(p_rhs.name), tt::engine::file::fixedStringSize);
	p_s.write(reinterpret_cast<const u8*>(p_rhs.ns),   tt::engine::file::fixedStringSize);

	return p_s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& p_s,
										 tt::engine::file::ResourceHeader& p_rhs)
{
	p_s >> p_rhs.versionMajor;
	p_s >> p_rhs.versionMinor;
	p_s >> p_rhs.platform;
	p_s >> p_rhs.reserved;

	p_s.read(reinterpret_cast<u8*>(p_rhs.name), tt::engine::file::fixedStringSize);
	p_s.read(reinterpret_cast<u8*>(p_rhs.ns),   tt::engine::file::fixedStringSize);

	return p_s;
}



#endif // INC_TT_ENGINE_FILE_RESOURCEHEADER_H
