#if !defined(INC_TT_PRES_SPRITESTRIP_H)
#define INC_TT_PRES_SPRITESTRIP_H
#include <string>


#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace pres {


class SpriteStrip
{
public:
	struct SpriteStripData
	{
		s32           totalFrameCount;
		math::Vector2 frameSize;
		std::string   spriteStripName; // AssetID
		std::string   spriteStripNamespace;
		
		
		inline SpriteStripData()
		:
		totalFrameCount(0),
		frameSize(0.0f, 0.0f),
		spriteStripName(),
		spriteStripNamespace()
		{ }
	};
	
	static bool load(const std::string& p_directory, SpriteStripData& p_data, 
	                 code::ErrorStatus* p_errStatus, bool p_makeFolder = true);
	static bool save(const std::string& p_outFileMeta,
	                 real p_frameHeight, real p_frameWidth, s32 p_frameCount);
	
	static void fail(const std::string& p_reason, const std::string& p_outFileMeta);
	
private:
	// No instantiation
	SpriteStrip();
	~SpriteStrip();
	SpriteStrip(const SpriteStrip&);
	SpriteStrip& operator=(const SpriteStrip&);
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_SPRITESTRIP_H)
