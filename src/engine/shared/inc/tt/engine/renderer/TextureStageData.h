#if !defined(INC_TT_ENGINE_RENDERER_TEXTURESTAGEDATA_H)
#define INC_TT_ENGINE_RENDERER_TEXTURESTAGEDATA_H

#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector2.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Used to feed the MultiTexture with texture stage data
           which is used in the fixed function pipeline in the underlaying graphics API
    \note  Each texture stage has a transformation matrix */
class TextureStageData
{
public:
	
	TextureStageData();
	~TextureStageData() { }
	
	inline void setTexture(const TexturePtr& p_texture) { m_texture = p_texture; }
	inline const TexturePtr& getTexture() const { return m_texture; }
	
	inline bool isEnabled() const
	{
		return getColorBlendOperation() != TextureBlendOperation_Disable &&
		       getAlphaBlendOperation() != TextureBlendOperation_Disable;
	}

	/*! \brief Set/get the operation performed on all sources for this texture stage */
	void setColorBlendOperation(TextureBlendOperation p_blendOperation);
	inline TextureBlendOperation getColorBlendOperation() const { return m_colorBlendOp; }
	void setAlphaBlendOperation(TextureBlendOperation p_blendOperation);
	inline TextureBlendOperation getAlphaBlendOperation() const { return m_alphaBlendOp; }
	
	/*! \brief Set/get the color/alpha sources for this texture stage */
	void setColorSource1(TextureBlendSource p_blendValue);
	inline TextureBlendSource getColorSource1() const { return m_colorSource1; }
	void setColorSource2(TextureBlendSource p_blendValue);
	inline TextureBlendSource getColorSource2() const { return m_colorSource2; }
	void setAlphaSource1(TextureBlendSource p_blendValue);
	inline TextureBlendSource getAlphaSource1() const { return m_alphaSource1; }
	void setAlphaSource2(TextureBlendSource p_blendValue);
	inline TextureBlendSource getAlphaSource2() const { return m_alphaSource2; }
	
	/*! \brief Set/get the constant for this texture stage */
	inline void setConstant(const ColorRGBA& p_color) { m_constant = p_color; }
	inline const ColorRGBA& getConstant() const { return m_constant; }
	
	/*! \brief Set/get the texture matrix for this texture stage */
	inline void setMatrix(const math::Matrix44& p_matrix) { m_matrix = p_matrix; }
	inline const math::Matrix44& getMatrix() const { return m_matrix; }
	inline math::Matrix44& modifyMatrix() { return m_matrix; }
	
	/*! \brief Set/get the texture coordinate set index for this texture stage */
	void setTexCoordIndex(u32 p_index);
	inline u32 getTexCoordIndex() const { return m_texCoordsIndex; }
	
private:
	math::Matrix44        m_matrix;
	TexturePtr            m_texture;
	TextureBlendOperation m_colorBlendOp;
	TextureBlendOperation m_alphaBlendOp;
	TextureBlendSource    m_colorSource1;
	TextureBlendSource    m_colorSource2;
	TextureBlendSource    m_alphaSource1;
	TextureBlendSource    m_alphaSource2;
	ColorRGBA             m_constant;
	u32                   m_texCoordsIndex;
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_TEXTURESTAGEDATA_H
