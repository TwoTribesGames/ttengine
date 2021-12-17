#include <tt/engine/renderer/TextureStageData.h>
#include <tt/engine/renderer/MultiTexture.h> // needed for constant MultiTexture::Constants_maxStages


namespace tt {
namespace engine {
namespace renderer {


/*! \brief The default constructor uses the default values of the DirectX 9c API texture stage states.
    \note  The default values are used to reset a texture stage */
TextureStageData::TextureStageData()
:
m_matrix(math::Matrix44::identity),
m_texture(),
m_colorBlendOp(TextureBlendOperation_Disable),
m_alphaBlendOp(TextureBlendOperation_SelectArg1),
m_colorSource1(TextureBlendSource_Texture),
m_colorSource2(TextureBlendSource_Previous),
m_alphaSource1(TextureBlendSource_Texture),
m_alphaSource2(TextureBlendSource_Previous),
m_constant(ColorRGB::white),
m_texCoordsIndex(0)
{
}


void TextureStageData::setColorBlendOperation(TextureBlendOperation p_blendOperation)
{
	if (isValidTextureBlendOperation(p_blendOperation) == false)
	{
		TT_PANIC("invalid TextureBlendOperation: %d", p_blendOperation);
		p_blendOperation = TextureBlendOperation_Modulate;
	}
	m_colorBlendOp = p_blendOperation;
}


void TextureStageData::setAlphaBlendOperation(TextureBlendOperation p_blendOperation)
{
	if (isValidTextureBlendOperation(p_blendOperation) == false)
	{
		TT_PANIC("invalid TextureBlendOperation: %d", p_blendOperation);
		p_blendOperation = TextureBlendOperation_Modulate;
	}
	m_alphaBlendOp = p_blendOperation;
}


void TextureStageData::setColorSource1(TextureBlendSource p_blendValue)
{
	if (isValidTextureBlendSource(p_blendValue) == false)
	{
		TT_PANIC("invalid TextureBlendValue: %d", p_blendValue);
		p_blendValue = TextureBlendSource_Texture;
	}
	m_colorSource1 = p_blendValue;
}


void TextureStageData::setColorSource2(TextureBlendSource p_blendValue)
{
	if (isValidTextureBlendSource(p_blendValue) == false)
	{
		TT_PANIC("invalid TextureBlendValue: %d", p_blendValue);
		p_blendValue = TextureBlendSource_Previous;
	}
	m_colorSource2 = p_blendValue;
}


void TextureStageData::setAlphaSource1(TextureBlendSource p_blendValue)
{
	if (isValidTextureBlendSource(p_blendValue) == false)
	{
		TT_PANIC("invalid TextureBlendValue: %d", p_blendValue);
		p_blendValue = TextureBlendSource_Texture;
	}
	m_alphaSource1 = p_blendValue;
}


void TextureStageData::setAlphaSource2(TextureBlendSource p_blendValue)
{
	if (isValidTextureBlendSource(p_blendValue) == false)
	{
		TT_PANIC("invalid TextureBlendValue: %d", p_blendValue);
		p_blendValue = TextureBlendSource_Previous;
	}
	m_alphaSource2 = p_blendValue;
}


void TextureStageData::setTexCoordIndex(u32 p_index)
{
	if (p_index >= MultiTexture::getChannelCount())
	{
		TT_PANIC("Can not assign texcoord set index (%u) beyond max (%u) textures",
		         p_index, MultiTexture::getChannelCount());
		p_index = 0;
	}
	m_texCoordsIndex = p_index;
}


// Namespace end
}
}
}
