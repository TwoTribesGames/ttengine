#include <tt/engine/renderer/enums.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {

// TextureBlendOperation

const char* getTextureBlendOperationName(TextureBlendOperation p_enum)
{
	switch (p_enum)
	{
	case TextureBlendOperation_Disable:    return "disable";
	case TextureBlendOperation_SelectArg1: return "select_arg1";
	case TextureBlendOperation_SelectArg2: return "select_arg2";
	case TextureBlendOperation_Modulate:   return "modulate";
	case TextureBlendOperation_Add:        return "add";
	case TextureBlendOperation_Subtract:   return "subtract";
	case TextureBlendOperation_Modulate2X: return "modulate2X";
	case TextureBlendOperation_Decal:      return "decal";
		
	default:
		TT_PANIC("Invalid TextureBlendOperation value: %d", p_enum);
		return "";
	}
}

TextureBlendOperation getTextureBlendOperationFromName(const std::string& p_name)
{
	for (s32 i = 0; i < TextureBlendOperation_Count; ++i)
	{
		TextureBlendOperation asEnum = static_cast<TextureBlendOperation>(i);
		if (p_name == getTextureBlendOperationName(asEnum))
		{
			return asEnum;
		}
	}
	
	return TextureBlendOperation_Invalid;
}


// TextureBlendSource

const char* getTextureBlendSourceName(TextureBlendSource p_enum)
{
	switch (p_enum)
	{
	case TextureBlendSource_Texture:          return "texture";
	case TextureBlendSource_Diffuse:          return "diffuse";
	case TextureBlendSource_Previous:         return "previous";
	case TextureBlendSource_Constant:         return "constant";
	case TextureBlendSource_OneMinusTexture:  return "inverse_texture";
	case TextureBlendSource_OneMinusDiffuse:  return "inverse_diffuse";
	case TextureBlendSource_OneMinusPrevious: return "inverse_previous";
	case TextureBlendSource_OneMinusConstant: return "inverse_constant";
		
	default:
		TT_PANIC("Invalid TextureBlendSource value: %d", p_enum);
		return "";
	}
}

TextureBlendSource getTextureBlendSourceFromName(const std::string& p_name)
{
	for (s32 i = 0; i < TextureBlendSource_Count; ++i)
	{
		TextureBlendSource asEnum = static_cast<TextureBlendSource>(i);
		if (p_name == getTextureBlendSourceName(asEnum))
		{
			return asEnum;
		}
	}
	
	return TextureBlendSource_Invalid;
}


// AlphaTestFunction

const char* getAlphaTestFunctionName(AlphaTestFunction p_enum)
{
	switch (p_enum)
	{
	case AlphaTestFunction_Never:        return "never";
	case AlphaTestFunction_Less:         return "less";
	case AlphaTestFunction_Equal:        return "equal";
	case AlphaTestFunction_LessEqual:    return "less_equal";
	case AlphaTestFunction_Greater:      return "greater";
	case AlphaTestFunction_NotEqual:     return "not_equal";
	case AlphaTestFunction_GreaterEqual: return "greater_equal";
	case AlphaTestFunction_Always:       return "always";
		
	default:
		TT_PANIC("Invalid AlphaTestFunction value: %d", p_enum);
		return "";
	}
}

AlphaTestFunction getAlphaTestFunctionFromName(const std::string& p_name)
{
	for (s32 i = 0; i < AlphaTestFunction_Count; ++i)
	{
		AlphaTestFunction asEnum = static_cast<AlphaTestFunction>(i);
		if (p_name == getAlphaTestFunctionName(asEnum))
		{
			return asEnum;
		}
	}
	
	return AlphaTestFunction_Invalid;
}


// BlendMode

const char* getBlendModeName(BlendMode p_enum)
{
	switch (p_enum)
	{
	case BlendMode_Blend:         return "blend";
	case BlendMode_Add:           return "add";
	case BlendMode_Modulate:      return "modulate";
	case BlendMode_Premultiplied: return "premultiplied";
		
	default:
		TT_PANIC("Invalid BlendMode value: %d", p_enum);
		return "";
	}
}

BlendMode getBlendModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < BlendMode_Count; ++i)
	{
		BlendMode asEnum = static_cast<BlendMode>(i);
		if (p_name == getBlendModeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return BlendMode_Invalid;
}

// BlendModeAlpha

const char* getBlendModeAlphaName(BlendModeAlpha p_enum)
{
	switch (p_enum)
	{
	case BlendModeAlpha_NoOverride: return "no_override";
	case BlendModeAlpha_Disabled:   return "disabled";
	case BlendModeAlpha_Lightmask:  return "lightmask";
	
	default:
		TT_PANIC("Invalid BlendModeAlpha value: %d", p_enum);
		return "";
	}
}


BlendModeAlpha getBlendModeAlphaFromName(const std::string& p_name)
{
	for (s32 i = 0; i < BlendModeAlpha_Count; ++i)
	{
		BlendModeAlpha asEnum = static_cast<BlendModeAlpha>(i);
		if (p_name == getBlendModeAlphaName(asEnum))
		{
			return asEnum;
		}
	}
	
	return BlendModeAlpha_Invalid;
}

// CullFrontOrder

const char* getCullFrontOrderName(CullFrontOrder p_enum)
{
	switch (p_enum)
	{
	case CullFrontOrder_ClockWise:        return "clockwise";
	case CullFrontOrder_CounterClockWise: return "counter_clockwise";
		
	default:
		TT_PANIC("Invalid CullFrontOrder value: %d", p_enum);
		return "";
	}
}

CullFrontOrder getCullFrontOrderFromName(const std::string& p_name)
{
	for (s32 i = 0; i < CullFrontOrder_Count; ++i)
	{
		CullFrontOrder asEnum = static_cast<CullFrontOrder>(i);
		if (p_name == getCullFrontOrderName(asEnum))
		{
			return asEnum;
		}
	}
	
	return CullFrontOrder_Invalid;
}


// CullMode

const char* getCullModeName(CullMode p_enum)
{
	switch (p_enum)
	{
	case CullMode_Front: return "front";
	case CullMode_Back:  return "back";
		
	default:
		TT_PANIC("Invalid CullMode value: %d", p_enum);
		return "";
	}
}

CullMode getCullModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < CullMode_Count; ++i)
	{
		CullMode asEnum = static_cast<CullMode>(i);
		if (p_name == getCullModeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return CullMode_Invalid;
}


// BlendOp

const char* getBlendOpName(BlendOp p_enum)
{
	switch (p_enum)
	{
	case BlendOp_Modulate:   return "modulate";
	case BlendOp_Modulate2X: return "modulate2x";
	case BlendOp_Decal:      return "decal";
	case BlendOp_Add:        return "add";
		
	default:
		TT_PANIC("Invalid BlendOp value: %d", p_enum);
		return "";
	}
}

BlendOp getBlendOpFromName(const std::string& p_name)
{
	for (s32 i = 0; i < BlendOp_Count; ++i)
	{
		BlendOp asEnum = static_cast<BlendOp>(i);
		if (p_name == getBlendOpName(asEnum))
		{
			return asEnum;
		}
	}
	
	return BlendOp_Invalid;
}


// BlendFactor

const char* getBlendFactorName(BlendFactor p_enum)
{
	switch (p_enum)
	{
	case BlendFactor_Zero:        return "zero";
	case BlendFactor_One:         return "one";
	case BlendFactor_SrcColor:    return "src_color";
	case BlendFactor_InvSrcColor: return "inv_src_color";
	case BlendFactor_SrcAlpha:    return "src_alpha";
	case BlendFactor_InvSrcAlpha: return "inv_src_alpha";
	case BlendFactor_DstAlpha:    return "dst_alpha";
	case BlendFactor_InvDstAlpha: return "inv_dst_alpha";
	case BlendFactor_DstColor:    return "dst_color";
	case BlendFactor_InvDstColor: return "inv_dst_color";
		
	default:
		TT_PANIC("Invalid BlendFactor value: %d", p_enum);
		return "";
	}
}

BlendFactor getBlendFactorFromName(const std::string& p_name)
{
	for (s32 i = 0; i < BlendFactor_Count; ++i)
	{
		BlendFactor asEnum = static_cast<BlendFactor>(i);
		if (p_name == getBlendFactorName(asEnum))
		{
			return asEnum;
		}
	}
	
	return BlendFactor_Invalid;
}


// AddressMode

const char* getAddressModeName(AddressMode p_enum)
{
	switch (p_enum)
	{
	case AddressMode_Clamp:  return "clamp";
	case AddressMode_Mirror: return "mirror";
	case AddressMode_Wrap:   return "wrap";
		
	default:
		TT_PANIC("Invalid AddressMode value: %d", p_enum);
		return "";
	}
}

AddressMode getAddressModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < AddressMode_Count; ++i)
	{
		AddressMode asEnum = static_cast<AddressMode>(i);
		if (p_name == getAddressModeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return AddressMode_Invalid;
}


// FilterMode

const char* getFilterModeName(FilterMode p_enum)
{
	switch (p_enum)
	{
	case FilterMode_None:        return "none";
	case FilterMode_Point:       return "point";
	case FilterMode_Linear:      return "linear";
	case FilterMode_Anisotropic: return "anisotropic";
		
	default:
		TT_PANIC("Invalid FilterMode value: %d", p_enum);
		return "";
	}
}

FilterMode getFilterModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < FilterMode_Count; ++i)
	{
		FilterMode asEnum = static_cast<FilterMode>(i);
		if (p_name == getFilterModeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return FilterMode_Invalid;
}


// FogSetting

const char* getFogSettingName(FogSetting p_enum)
{
	switch (p_enum)
	{
	case FogSetting_Start:        return "start";
	case FogSetting_End:          return "end";
	case FogSetting_Density:      return "density";
		
	default:
		TT_PANIC("Invalid FogSetting value: %d", p_enum);
		return "";
	}
}


FogSetting getFogSettingFromName(const std::string& p_name)
{
	for (s32 i = 0; i < FogSetting_Count; ++i)
	{
		FogSetting asEnum = static_cast<FogSetting>(i);
		if (p_name == getFogSettingName(asEnum))
		{
			return asEnum;
		}
	}
	
	return FogSetting_Invalid;
}


// FogMode

const char* getFogModeName(FogMode p_enum)
{
	switch (p_enum)
	{
	case FogMode_Linear:             return "linear";
	case FogMode_Exponential:        return "exponential";
	case FogMode_ExponentialSquared: return "exponential_squared";
		
	default:
		TT_PANIC("Invalid FogMode value: %d", p_enum);
		return "";
	}
}


FogMode getFogModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < FogMode_Count; ++i)
	{
		FogMode asEnum = static_cast<FogMode>(i);
		if (p_name == getFogModeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return FogMode_Invalid;
}


// Layout

const char* getLayoutName(Layout p_enum)
{
	switch (p_enum)
	{
	case Layout_Standard:  return "standard";
	case Layout_LeftRight: return "left_right";
	case Layout_TopBottom: return "top_bottom";
		
	default:
		TT_PANIC("Invalid Layout value: %d", p_enum);
		return "";
	}
}

Layout getLayoutFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Layout_Count; ++i)
	{
		Layout asEnum = static_cast<Layout>(i);
		if (p_name == getLayoutName(asEnum))
		{
			return asEnum;
		}
	}
	
	return Layout_Invalid;
}


// ViewPortID

const char* getViewPortIDName(ViewPortID p_enum)
{
	switch (p_enum)
	{
	case ViewPortID_1: return "1";
	case ViewPortID_2: return "2";
	case ViewPortID_3: return "3";
	case ViewPortID_4: return "4";
		
	default:
		TT_PANIC("Invalid ViewPortID value: %d", p_enum);
		return "";
	}
}

ViewPortID getViewPortIDFromName(const std::string& p_name)
{
	// Special-case: support viewport alias names
	if (p_name == "main"   ||
	    p_name == "bottom" ||
	    p_name == "left")
	{
		return ViewPortID_1;
	}
	else if (p_name == "top" ||
	         p_name == "right")
	{
		return ViewPortID_2;
	}
	
	for (s32 i = 0; i < ViewPortID_Count; ++i)
	{
		ViewPortID asEnum = static_cast<ViewPortID>(i);
		if (p_name == getViewPortIDName(asEnum))
		{
			return asEnum;
		}
	}
	
	return ViewPortID_Invalid;
}


const char* getPrimitiveTypeName(PrimitiveType p_value)
{
	switch (p_value)
	{
	case PrimitiveType_PointList      : return "point_list";
	case PrimitiveType_LineList       : return "line_list";
	case PrimitiveType_LineStrip      : return "line_strip";
	case PrimitiveType_TriangleList   : return "triangle_list";
	case PrimitiveType_TriangleStrip  : return "triangle_strip";

	case PrimitiveType_LineLoop       : return "line_loop";
	case PrimitiveType_TriangleFan    : return "triangle_fan";
	case PrimitiveType_QuadList       : return "quad_list";

	default:
		TT_PANIC("Invalid ViewPortID value: %d", p_value);
		return "";
	}
}


PrimitiveType getPrimitiveTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < PrimitiveType_Count; ++i)
	{
		PrimitiveType asEnum = static_cast<PrimitiveType>(i);
		if (p_name == getPrimitiveTypeName(asEnum))
		{
			return asEnum;
		}
	}
	
	return PrimitiveType_Invalid;
}


// Namespace end
}
}
}
