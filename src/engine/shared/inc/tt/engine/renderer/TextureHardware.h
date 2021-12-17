#if !defined(INC_TT_ENGINE_RENDERER_TEXTUREHARDWARE_H)
#define INC_TT_ENGINE_RENDERER_TEXTUREHARDWARE_H

#include <algorithm>

#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>
#include <tt/app/Platform.h>


namespace tt {
namespace engine {
namespace renderer {

/*! \brief Static class which can be used to know what the texture size limits are for a specific
           piece of hardware. It will also store which 'emulation-mode' is active.
           Windows can run in iPad mode and will test all textures for iPad size restrictions. */
class TextureHardware
{
public:
	enum SizeRequirement
	{
		SizeRequirement_None,
		SizeRequirement_MultipleOfFour,
		SizeRequirement_PowerOfTwo,
		
		SizeRequirement_Count,
		SizeRequirement_Invalid
	};
	
	static inline bool isValidSizeRequirement(SizeRequirement p_sizeRequirement)
	{ return p_sizeRequirement >= 0 && p_sizeRequirement < SizeRequirement_Count; }
	
	static const char* getName(SizeRequirement p_sizeRequirement);
	
	// Static helpers to get info for a specific platform
	
	static s32             getMinDimensionForPlatform(   app::Platform p_platform);
	static s32             getMaxDimensionForPlatform(   app::Platform p_platform);
	static SizeRequirement getSizeRequirementForPlatform(app::Platform p_platform);
	
	// Static Helpers for SizeRequirement
	static s32 correctSize(SizeRequirement p_sizeRequirement, s32 p_dimension);
	
	// Helper class for all texture Requirements.
	class Requirements
	{
	public:
		inline app::Platform   getPlatform()        const { return platform;        }
		inline SizeRequirement getSizeRequirement() const { return sizeRequirement; }
		inline s32             getMinDimension()    const { return minDimension;    }
		inline s32             getMaxDimension()    const { return maxDimension;    }
		
		inline Requirements()
		:
		platform(app::Platform_Invalid),
		sizeRequirement(SizeRequirement_Invalid),
		minDimension(-1),
		maxDimension(-1)
		{}
		
		/* \brief Dimensions can be corrected by making the texture bigger.
		          If the dimension is too large nothing is done.
		          (Use fitsDimensions to check for textures which are too large!) */
		inline s32 correctDimension(s32 p_dimension) const
		{
			return correctSize(getSizeRequirement(), std::max(p_dimension, minDimension));
		}
		
		inline math::Point2 correctDimension(const math::Point2& p_size) const
		{
			return math::Point2(correctDimension(p_size.x),
			                    correctDimension(p_size.y));
		}
		
		inline bool checkDimension(s32 p_dimension, bool p_panic,
		                           const std::string& p_name     = std::string(),
		                           const std::string& p_fileInfo = std::string()) const
		{
			TT_ASSERT(isValidPlatform(getPlatform()));
			
			if (p_dimension < minDimension)
			{
				TT_ASSERTMSG(p_panic == false,
				             "Texture %s is too small. Texture %s is %d; minimum is %d.\nInfo: %s",
				             p_name.c_str(), p_name.c_str(), p_dimension, minDimension, p_fileInfo.c_str());
				return false;
			}
			if (p_dimension > maxDimension)
			{
				TT_ASSERTMSG(p_panic == false,
				             "Texture %s is too large. Texture %s is %d; maximum is %d.\nInfo: %s",
				             p_name.c_str(), p_name.c_str(), p_dimension, maxDimension, p_fileInfo.c_str());
				return false;
			}
			switch (sizeRequirement)
			{
			case SizeRequirement_None:
				// Nothing to check.
				break;
				
			case SizeRequirement_MultipleOfFour:
			case SizeRequirement_PowerOfTwo:
				{
					const s32 correction = p_dimension;// correctSize(getSizeRequirement(), p_dimension);
					if (correction != p_dimension)
					{
						TT_ASSERTMSG(p_panic == false,
						             "Texture %s %d is not a %s (%s).\n"
						             "Please change the %s of this texture so that it is a %s "
						             "(suggestion: %d).",
						             p_name.c_str(), p_dimension, getName(getSizeRequirement()),
						             p_fileInfo.c_str(), p_name.c_str(),
						             getName(getSizeRequirement()), correction);
						return false;
					}
				}
				break;
				
			default:
				TT_PANIC("Unknown size requirement: %d! Info: %s", sizeRequirement, p_fileInfo.c_str());
				break;
			}
			return true;
		}
		
		inline bool fitsDimensions(s32 p_dimension) const
		{
			return p_dimension <= maxDimension;
		}
		
		inline bool fitsDimensions(const math::Point2& p_size) const
		{
			return fitsDimensions(p_size.x) && fitsDimensions(p_size.y);
		}
		
	private:
		Requirements(app::Platform p_platform);
		
		app::Platform   platform;
		SizeRequirement sizeRequirement;
		s32             minDimension;
		s32             maxDimension;
		
		friend class TextureHardware;
	};
	
	// Static helper to get the Requirement class for a specific platform
	static Requirements getRequirementsForPlatform(app::Platform p_platform);
	
	// Get the (Texture) Requirements for the current/active platform.
	static inline const Requirements& getRequirements()
	{
		if (app::getPlatform() != ms_activePlatformEmulationRequirements.getPlatform())
		{
			// Platform changed!
			ms_activePlatformEmulationRequirements = getRequirementsForPlatform(app::getPlatform());
		}
		return ms_activePlatformEmulationRequirements;
	}
	
private:
	TextureHardware();  // Not implemented. Static class.
	~TextureHardware(); // Not implemented. Static class.
	
	static Requirements ms_activePlatformEmulationRequirements;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_TEXTUREHARDWARE_H)
