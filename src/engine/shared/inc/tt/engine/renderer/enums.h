#if !defined(INC_TT_ENGINE_RENDERER_ENUMS_H)
#define INC_TT_ENGINE_RENDERER_ENUMS_H


#include <string>


namespace tt {
namespace engine {
namespace renderer {

/*! \brief Texture blend operation performed in the fixed function pipeline (used by texture stages)
    \note  Platform lookup tables depend on these */
enum TextureBlendOperation
{
	TextureBlendOperation_Disable,
	
	TextureBlendOperation_SelectArg1,       //<! select only argument 1 of the blend color/alpha operation
	TextureBlendOperation_SelectArg2,       //<! select only argument 2 of the blend color/alpha operation
	
	TextureBlendOperation_Modulate,         //<! arg1 * arg2
	TextureBlendOperation_Add,              //<! arg1 + arg2
	TextureBlendOperation_Subtract,         //<! arg1 - arg2
	TextureBlendOperation_Modulate2X,       //<! arg1 * arg2 * 2
	TextureBlendOperation_Decal,            //<! arg1.rgb * arg1.a + arg2.rgb * (1 - arg1.a)
	
	TextureBlendOperation_Count,
	TextureBlendOperation_Invalid
};

inline bool isValidTextureBlendOperation(TextureBlendOperation p_value)
	{ return p_value >= 0 && p_value < TextureBlendOperation_Count; }
const char*           getTextureBlendOperationName(TextureBlendOperation p_enum);
TextureBlendOperation getTextureBlendOperationFromName(const std::string& p_name);


/*! \brief Source for texture blending performed in the fixed function pipeline (used by texture stages).
           A source can provide color and/or alpha values
    \note  Platform (Renderer) lookup tables depend on these */
enum TextureBlendSource
{
	TextureBlendSource_Texture,          //<! texture
	TextureBlendSource_Diffuse,          //<! vertex
	TextureBlendSource_Previous,         //<! previous stage result
	TextureBlendSource_Constant,         //<! the texture stage's constant
	
	TextureBlendSource_OneMinusTexture,
	TextureBlendSource_OneMinusDiffuse,
	TextureBlendSource_OneMinusPrevious,
	TextureBlendSource_OneMinusConstant,
	
	TextureBlendSource_Count,
	TextureBlendSource_Invalid,
	
	TextureBlendSource_InverseOperandStart = TextureBlendSource_OneMinusTexture
};
inline bool isValidTextureBlendSource(TextureBlendSource p_value)
	{ return p_value >= 0 && p_value < TextureBlendSource_Count; }
const char*        getTextureBlendSourceName(TextureBlendSource p_enum);
TextureBlendSource getTextureBlendSourceFromName(const std::string& p_name);


/*! \brief Depends on the order of the DirectX compare function enum */
enum AlphaTestFunction
{
	AlphaTestFunction_Never,             //<! 0
	AlphaTestFunction_Less,              //<! <
	AlphaTestFunction_Equal,             //<! ==
	AlphaTestFunction_LessEqual,         //<! <=
	AlphaTestFunction_Greater,           //<! >
	AlphaTestFunction_NotEqual,          //<! !=
	AlphaTestFunction_GreaterEqual,      //<! >=
	AlphaTestFunction_Always,            //<! 1
	
	AlphaTestFunction_Count,
	AlphaTestFunction_Invalid
};
inline bool isValidAlphaTestFunction(AlphaTestFunction p_value)
	{ return p_value >= 0 && p_value < AlphaTestFunction_Count; }
const char*       getAlphaTestFunctionName(AlphaTestFunction p_enum);
AlphaTestFunction getAlphaTestFunctionFromName(const std::string& p_name);


/*! \brief Blend operation performed after fixed function pipeline */
enum BlendMode
{
	BlendMode_Blend,           //<! Alpha blending
	BlendMode_Add,             //<! Additive blending
	BlendMode_Modulate,        //<! Multiply blending
	BlendMode_Premultiplied,   //<! Premultiplied alpha
	
	BlendMode_Count,
	BlendMode_Invalid
};
inline bool isValidBlendMode(BlendMode p_value)
	{ return p_value >= 0 && p_value < BlendMode_Count; }
const char* getBlendModeName(BlendMode p_enum);
BlendMode   getBlendModeFromName(const std::string& p_name);


/*! \brief Alpha Blend operation performed after fixed function pipeline */
enum BlendModeAlpha
{
	BlendModeAlpha_NoOverride, //<! Keep the setting which was already set. (Does nothing; is used as setting in e.g. particles.)
	BlendModeAlpha_Disabled,   //<! Separate Alpha Blend Mode is disabled, use normal blend.
	BlendModeAlpha_Lightmask,  //<! Apply as lightmask    - src: One , dst: One.
	//BlendModeAlpha_OverwriteLightmask, //<! Overwrite existing light mask - src: Zero, dst: InvSrcAlpha
	
	BlendModeAlpha_Count,
	BlendModeAlpha_Invalid
};
inline bool isValidBlendModeAlpha(BlendModeAlpha p_value)
	{ return p_value >= 0 && p_value < BlendModeAlpha_Count; }
const char* getBlendModeAlphaName(BlendModeAlpha p_enum);
BlendModeAlpha   getBlendModeAlphaFromName(const std::string& p_name);


/*! \brief Determines the vertex drawing order of a front facing face */
enum CullFrontOrder
{
	CullFrontOrder_ClockWise,
	CullFrontOrder_CounterClockWise,
	
	CullFrontOrder_Count,
	CullFrontOrder_Invalid
};
inline bool isValidCullFrontOrder(CullFrontOrder p_value)
	{ return p_value >= 0 && p_value < CullFrontOrder_Count; }
const char*    getCullFrontOrderName(CullFrontOrder p_enum);
CullFrontOrder getCullFrontOrderFromName(const std::string& p_name);


// the following enum order is used on the platform lookup tables
enum CullMode
{
	CullMode_Front,
	CullMode_Back,
	
	CullMode_Count,
	CullMode_Invalid
};
inline bool isValidCullMode(CullMode p_value)
	{ return p_value >= 0 && p_value < CullMode_Count; }
const char* getCullModeName(CullMode p_enum);
CullMode    getCullModeFromName(const std::string& p_name);


enum BlendOp
{
	BlendOp_Modulate,
	BlendOp_Modulate2X,
	BlendOp_Decal,
	BlendOp_Add,
	
	BlendOp_Count,
	BlendOp_Invalid
};
inline bool isValidBlendOp(BlendOp p_value)
	{ return p_value >= 0 && p_value < BlendOp_Count; }
const char* getBlendOpName(BlendOp p_enum);
BlendOp     getBlendOpFromName(const std::string& p_name);


enum BlendFactor
{
	BlendFactor_Zero,
	BlendFactor_One,
	BlendFactor_SrcColor,
	BlendFactor_InvSrcColor,
	BlendFactor_SrcAlpha,
	BlendFactor_InvSrcAlpha,
	BlendFactor_DstAlpha,
	BlendFactor_InvDstAlpha,
	BlendFactor_DstColor,
	BlendFactor_InvDstColor,
	
	BlendFactor_Count,
	BlendFactor_Invalid
};
inline bool isValidBlendFactor(BlendFactor p_value)
	{ return p_value >= 0 && p_value < BlendFactor_Count; }
const char* getBlendFactorName(BlendFactor p_enum);
BlendFactor getBlendFactorFromName(const std::string& p_name);


enum AddressMode
{
	AddressMode_Clamp,
	AddressMode_Mirror,
	AddressMode_Wrap,
	
	AddressMode_Count,
	AddressMode_Invalid
};
inline bool isValidAddressMode(AddressMode p_value)
	{ return p_value >= 0 && p_value < AddressMode_Count; }
const char* getAddressModeName(AddressMode p_enum);
AddressMode getAddressModeFromName(const std::string& p_name);


/*! \brief Texture filtering modes */
enum FilterMode
{
	FilterMode_None,
	FilterMode_Point,
	FilterMode_Linear,
	FilterMode_Anisotropic,
	
	FilterMode_Count,
	FilterMode_Invalid
};
inline bool isValidFilterMode(FilterMode p_value)
	{ return p_value >= 0 && p_value < FilterMode_Count; }
const char* getFilterModeName(FilterMode p_enum);
FilterMode  getFilterModeFromName(const std::string& p_name);


/*! \brief Fog settings */
enum FogSetting
{
	FogSetting_Start,
	FogSetting_End,
	FogSetting_Density,
	
	FogSetting_Count,
	FogSetting_Invalid
};
inline bool isValidFogSetting(FogSetting p_value)
	{ return p_value >= 0 && p_value < FogSetting_Count; }
const char* getFogSettingName(FogSetting p_enum);
FogSetting  getFogSettingFromName(const std::string& p_name);


/*! \brief Fog modes */
enum FogMode
{
	FogMode_Linear,
	FogMode_Exponential,
	FogMode_ExponentialSquared,
	
	FogMode_Count,
	FogMode_Invalid
};
inline bool isValidFogMode(FogMode p_value)
	{ return p_value >= 0 && p_value < FogMode_Count; }
const char* getFogModeName(FogMode p_enum);
FogMode     getFogModeFromName(const std::string& p_name);


enum Layout
{
	Layout_Standard,
	Layout_LeftRight,
	Layout_TopBottom,
	
	Layout_Count,
	Layout_Invalid
};
inline bool isValidLayout(Layout p_value)
	{ return p_value >= 0 && p_value < Layout_Count; }
const char* getLayoutName(Layout p_enum);
Layout      getLayoutFromName(const std::string& p_name);


enum ViewPortID
{
	ViewPortID_1,
	ViewPortID_2,
	ViewPortID_3,
	ViewPortID_4,
	
	ViewPortID_Count,
	ViewPortID_Invalid,
	
	// Aliases
	ViewPortID_Main   = ViewPortID_1,
	
	ViewPortID_Bottom = ViewPortID_1,
	ViewPortID_Top    = ViewPortID_2,
	
	ViewPortID_Left   = ViewPortID_1,
	ViewPortID_Right  = ViewPortID_2,

	ViewPortID_TV     = ViewPortID_1,
	ViewPortID_DRC    = ViewPortID_2
};
inline bool isValidViewPortID(ViewPortID p_value)
	{ return p_value >= 0 && p_value < ViewPortID_Count; }
const char* getViewPortIDName(ViewPortID p_enum);
ViewPortID  getViewPortIDFromName(const std::string& p_name);


enum PrimitiveType
{
	PrimitiveType_PointList,
	PrimitiveType_LineList,
	PrimitiveType_LineStrip,
	PrimitiveType_TriangleList,
	PrimitiveType_TriangleStrip,

	// Not on all platforms available
	PrimitiveType_LineLoop,
	PrimitiveType_TriangleFan,
	PrimitiveType_QuadList,

	PrimitiveType_Count,
	PrimitiveType_Invalid
};

inline bool isValidPrimitiveType(PrimitiveType p_value)
{ return p_value >= 0 && p_value < PrimitiveType_Count; }
const char*   getPrimitiveTypeName(PrimitiveType p_name);
PrimitiveType getPrimitiveTypeFromName(const std::string& p_name);


enum FillMode
{
	FillMode_Solid,
	FillMode_Wireframe,
	
	FillMode_Count,
	FillMode_Invalid
};


enum ColorMask
{
	ColorMask_None  = 0x0,
	ColorMask_Red   = 0x1,
	ColorMask_Green = 0x2,
	ColorMask_Blue  = 0x4,
	ColorMask_Alpha = 0x8,
	
	ColorMask_Color = ColorMask_Red | ColorMask_Green | ColorMask_Blue,
	ColorMask_All   = ColorMask_Color | ColorMask_Alpha
};


enum StencilSide
{
	StencilSide_Front,
	StencilSide_Back
};


enum StencilTestFunction
{
	StencilTestFunction_Never,             //<! 0
	StencilTestFunction_Less,              //<! <
	StencilTestFunction_Equal,             //<! ==
	StencilTestFunction_LessEqual,         //<! <=
	StencilTestFunction_Greater,           //<! >
	StencilTestFunction_NotEqual,          //<! !=
	StencilTestFunction_GreaterEqual,      //<! >=
	StencilTestFunction_Always,            //<! 1
	
	StencilTestFunction_Count,
	StencilTestFunction_Invalid
};

inline bool isValidStencilTestFunction(StencilTestFunction p_value)
{ return p_value >= 0 && p_value < StencilTestFunction_Count; }


enum StencilOperation
{
	StencilOperation_Keep,          //<! Keep current value
	StencilOperation_Zero,          //<! Set value to 0
	StencilOperation_Replace,       //<! Set value to reference value
	StencilOperation_Increment,     //<! Increment value, clamp to max value
	StencilOperation_IncrementWrap, //<! Increment value, wrap to 0 after max value
	StencilOperation_Decrement,     //<! Decrement value, clamp to 0
	StencilOperation_DecrementWrap, //<! Decrement value, wrap to max value after 0
	StencilOperation_Invert,        //<! Bitwise invert current value
	
	StencilOperation_Count,
	StencilOperation_Invalid,
};

inline bool isValidStencilOperation(StencilOperation p_value)
{ return p_value >= 0 && p_value < StencilOperation_Count; }


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_ENUMS_H)
