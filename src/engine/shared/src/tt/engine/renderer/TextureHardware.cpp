#include <tt/engine/renderer/TextureHardware.h>


namespace tt {
namespace engine {
namespace renderer {

// Set inital value based on platform defines.
TextureHardware::Requirements TextureHardware::ms_activePlatformEmulationRequirements;


const char* TextureHardware::getName(SizeRequirement p_sizeRequirement)
{
	switch (p_sizeRequirement)
	{
	case SizeRequirement_None:           return "none";
	case SizeRequirement_MultipleOfFour: return "multiple of four";
	case SizeRequirement_PowerOfTwo:     return "power of two";
	default:
		TT_PANIC("Unknown size requirement %d", p_sizeRequirement);
		return "";
	}
}


s32 TextureHardware::getMinDimensionForPlatform(app::Platform p_platform)
{
	switch (p_platform)
	{
	case app::Platform_WIN:
	case app::Platform_MAC:
	case app::Platform_LNX:
		return 1;
		
	default:
		TT_PANIC("Unknown platform: %d", p_platform);
		return -1;
	}
}


s32 TextureHardware::getMaxDimensionForPlatform(app::Platform p_platform)
{
	switch (p_platform)
	{
	case app::Platform_WIN:
	case app::Platform_MAC:
	case app::Platform_LNX:
		return 4096;
		
	default:
		TT_PANIC("Unknown platform: %d", p_platform);
		return -1;
	}
}


TextureHardware::SizeRequirement TextureHardware::getSizeRequirementForPlatform(app::Platform p_platform)
{
	switch (p_platform)
	{
	case app::Platform_WIN:
		return SizeRequirement_None;
		
	case app::Platform_LNX:
	case app::Platform_MAC:
		return SizeRequirement_PowerOfTwo;
		
	default:
		TT_PANIC("Unknown platform: %d", p_platform);
		return SizeRequirement_Invalid;
	}
}


s32 TextureHardware::correctSize(SizeRequirement p_sizeRequirement, s32 p_dimension)
{
	switch (p_sizeRequirement)
	{
	case SizeRequirement_None:
		return p_dimension;
		
	case SizeRequirement_MultipleOfFour:
		return tt::math::roundUp(p_dimension, 4);
		
	case SizeRequirement_PowerOfTwo:
		return tt::math::roundToPowerOf2(p_dimension);
		
	default:
		TT_PANIC("Unknown sizeRequirement: %d", p_sizeRequirement);
		return p_dimension;
	}
}


TextureHardware::Requirements TextureHardware::getRequirementsForPlatform(app::Platform p_platform)
{
	return Requirements(p_platform);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

TextureHardware::Requirements::Requirements(app::Platform p_platform)
:
platform(p_platform),
sizeRequirement(getSizeRequirementForPlatform(p_platform)),
minDimension(getMinDimensionForPlatform(p_platform)),
maxDimension(getMaxDimensionForPlatform(p_platform))
{
}

// Namespace end
}
}
}
