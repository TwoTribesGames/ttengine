#if !defined(INC_TT_PRES_PRESENTATIONQUAD_H)
#define INC_TT_PRES_PRESENTATIONQUAD_H

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/math/Vector3.h>

#include <tt/pres/fwd.h>


namespace tt {
namespace pres {


/*! \brief Wraps Quad2D for presentations Frame animations */ 
class PresentationQuad
{
public:
	PresentationQuad(const engine::renderer::TexturePtr& p_texture,
	                 const engine::renderer::ColorRGBA&  p_quadColor, 
	                 const math::Vector3&                p_translation = math::Vector3::zero);
	~PresentationQuad(){}
	
	/*! \brief sets the texture and renders the quad. Matrix stack should be setup before calling this. */ 
	void render() const;
	void renderWithTexture(const engine::renderer::TexturePtr& p_texture) const;
	
	/*! \brief Sets the color of the quad and updates the quad if needed.
	           this is reserved for the ColorAnimationStack. Use setOriginalColor.
	    \param p_color  color to set */ 
	void setColor(const engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Returns the unmodified color of the quad that was given with the creation. */ 
	inline const engine::renderer::ColorRGBA& getOriginalColor() const { return m_originalColor; }
	
	void setWidth(real p_width);
	void setHeight(real p_height);

	inline real getWidth()  const { return m_width ; }
	inline real getHeight() const { return m_height; }
	inline real getRadius() const { return m_radius; }
	
	void setTexture(const engine::renderer::TexturePtr& p_texture);
	inline const engine::renderer::TexturePtr& getTexture() const { return m_texture; }
	
	inline void setTranslation(const math::Vector3& p_translation) { m_translation = p_translation; }
	inline const math::Vector3& getTranslation() const { return m_translation; }
	
	inline void setScale(const math::Vector2& p_scale) { m_scale = p_scale; }
	inline math::Vector2 getScale() const { return m_scale; }
	
	inline void setRotation(real p_rotation) { m_rotation = p_rotation; }
	inline real getRotation() const { return m_rotation; }
	
	inline void setBlendMode(engine::renderer::BlendMode p_mode) { m_blendMode = p_mode; }
	
	void setTexcoords(const math::Vector2& p_topLeft, const math::Vector2& p_bottomRight);
	
	inline PresentationQuadPtr clone() const { return PresentationQuadPtr(new PresentationQuad(*this)); }
	
private:
	real m_width;
	real m_height;
	real m_radius;
	math::Vector3 m_translation;
	math::Vector2 m_scale;
	real          m_rotation;
	
	math::Vector2 m_quadScale;
	
	mutable engine::renderer::Quad2D m_quad;
	engine::renderer::TexturePtr m_texture;
	engine::renderer::ColorRGBA  m_lastSetColor;
	const engine::renderer::ColorRGBA  m_originalColor;
	engine::renderer::BlendMode  m_blendMode;

	mutable bool m_quadNeedsUpdate;
	
	const PresentationQuad& operator=(const PresentationQuad&); // disable assigment
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PRESENTATIONQUAD_H)
