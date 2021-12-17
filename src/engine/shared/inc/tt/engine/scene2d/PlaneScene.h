#if !defined(INC_TT_ENGINE_SCENE2D_PLANESCENE_H)
#define INC_TT_ENGINE_SCENE2D_PLANESCENE_H

#include <tt/engine/anim2d/fwd.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/math/math.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Rect.h>
#include <tt/engine/renderer/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {

class PlaneScene : public Scene2D
{
public:
	PlaneScene(real p_width, real p_height, renderer::TexturePtr p_texture, 
	      const tt::math::Vector3& p_position = tt::math::Vector3(), real p_rotation = 0, 
	      real p_texAnimU = 0, real p_texAnimV = 0, real p_texOffsetScale = 0, s32 p_priority = 0, 
	      bool p_withVertexColors = false);
	virtual ~PlaneScene();
	
	// Update & Rendering
	virtual void update(real p_deltaTime = 0.0f);
	virtual void render();
	
	/*! \brief Retrieve the height of the node */
	inline virtual real getHeight() const { return m_height; }
	/*! \brief Retrieve the width of the node */
	inline virtual real getWidth() const { return m_width; }
	
	inline virtual real getDepth() const { return m_centerPosition.z; }
	
	void setTextureCoordinates(real p_topLeftU,      real p_topLeftV,
	                           real p_topRighttU,    real p_topRightV,
	                           real p_bottomRighttU, real p_bottomRightV,
	                           real p_bottomLeftU,   real p_bottomLeftV);
	
	void setVertexColors(const renderer::ColorRGBA& p_topLeftColor,
	                     const renderer::ColorRGBA& p_topRightColor,
	                     const renderer::ColorRGBA& p_bottomLeftColor,
	                     const renderer::ColorRGBA& p_bottomRightColor);
	
	void setOffsetAnimation(real p_offsetXMax, real p_offsetXDuration, 
	                        real p_offsetXTimeOffset, 
	                        real p_offsetYMax, real p_offsetYDuration,
	                        real p_offsetYTimeOffset);

	inline void setCameraSpaceScale(real p_scaleU, real p_scaleV)
	{ m_cameraSpaceScale.x = p_scaleU; m_cameraSpaceScale.y = p_scaleV; }
	
	void setAnimations(const anim2d::AnimationStack2DPtr& p_animations);
	//inline const anim2d::ConstAnimationStack2DPtr& getAnimations() const { return m_animations; }
	inline const anim2d::     AnimationStack2DPtr& getAnimations()       { return m_animations; }
	
	void setTextureAnimation(const anim2d::AnimationStack2DPtr& p_animation);
	//inline const anim2d::ConstAnimationStack2DPtr& getTextureAnimation() const { return m_textureAnimation; }
	inline const anim2d::     AnimationStack2DPtr& getTextureAnimation()       { return m_textureAnimation; }
	
	void setColorAnimation(const anim2d::ColorAnimationStack2DPtr& p_animation);
	//inline const anim2d::ConstColorAnimationStack2DPtr& getColorAnimation() const { return m_colorAnimation; }
	inline const anim2d::     ColorAnimationStack2DPtr& getColorAnimation()       { return m_colorAnimation; }
	
	inline real getBoundingRadius() const { return m_boundingRadius; }
	inline const math::VectorRect& getBoundingRect() const { return m_boundingRect; }
	inline bool needsUpdate() const { return m_needsUpdate; }
	
	inline const math::Matrix44& getMatrix() const 
	{
		return m_animations != 0 ? m_posAnimMatrix : m_posMatrix;
	}
	
	bool doCullCheck();
	
	virtual inline void* getMaterialID() const { return m_texture.get(); }
	
	inline const renderer::TexturePtr& getTexture() const { return m_texture; }
	inline renderer::AddressMode getAddressModeU() const { return m_addressModeU; }
	inline renderer::AddressMode getAddressModeV() const { return m_addressModeV; }
	
	void makeBatchQuad(renderer::BatchQuad& p_quad);
	
	virtual bool isSuitableForBatching() const;
	virtual bool isPlaneScene() const { return true; }
	
	
private:
	// Not implemented. (No copy or assigment)
	PlaneScene(const PlaneScene& p_copy);
	const PlaneScene& operator =(const PlaneScene& p_rhs);
	
	void updatePosition(real p_deltaTime);
	void updateBoundingRect();
	void updateNeedsUpdate();
	void setQuadColors(const renderer::ColorRGBA& p_topLeftColor,
	                   const renderer::ColorRGBA& p_topRightColor,
	                   const renderer::ColorRGBA& p_bottomLeftColor,
	                   const renderer::ColorRGBA& p_bottomRightColor);
	
	const real m_boundingRadius;     //!< Radius for a bounding sphere.
	math::VectorRect m_boundingRect; //!< AA Bounding rectangle including animation
	bool m_needsUpdate;
	
protected:
	/*! \brief Get offset for offset anim.
	           Note: Uses m_offsetTime to determine current anim time.
	    \param p_maxOffset The maximum offset with in the anim.
	    \param p_duration The time for a single sinus movement.
	    \param p_timeOffset The offset for the timing.
	    \return The offset based on current anim time (m_offsetTime) and 
	            the parameters p_maxOffset, p_duration. */
	inline real getOffset(real p_maxOffset, real p_duration, real p_timeOffset) const
	{
		TT_ASSERT(p_duration != 0.0f);
		
		return math::sin(((m_offsetTime + p_timeOffset) * math::twoPi) / p_duration ) * p_maxOffset;
	}
	virtual void onPositionChanged() { m_posMtxDirty = true; }
	
	bool m_renderQuadNeedsUpdate;
	bool m_cullCheckDone;
	bool m_hasTextureAnim;

	// Render objects
	renderer::Quad2DPtr  m_renderQuad;
	renderer::TexturePtr m_texture;
	
	// Dimensions
	const real m_width;
	const real m_height;
	
	// Rotation
	real m_rotation;
	
	// Translation offset.
	real m_offsetTime; //! < The animation time of the translation offset.
	real m_offsetXMax;
	real m_offsetYMax;
	real m_offsetXDuration;
	real m_offsetYDuration;
	real m_offsetXTimeOffset;
	real m_offsetYTimeOffset;
	math::Vector2 m_cameraSpaceScale;

	// Texture coord.
	real m_texU;
	real m_texV;
	renderer::AddressMode m_addressModeU;
	renderer::AddressMode m_addressModeV;
	
	// Texture animation
	real m_texAnimU;
	real m_texAnimV;
	
	// Influence of camera position on texture coordinates
	real m_texOffsetScale;
	
	// Animated position
	math::Vector3 m_offsetPosition;
	
	anim2d::AnimationStack2DPtr      m_animations;       //!< \brief Transform animations.
	anim2d::AnimationStack2DPtr      m_textureAnimation; //!< \brief Texture animations.
	anim2d::ColorAnimationStack2DPtr m_colorAnimation;   //!< \brief (Uniform) vertex color animation
	
	bool m_posMtxDirty;
	bool m_texMtxDirty;
	math::Matrix44 m_posMatrix;     //!< Matrix with just the position. (+ Offset.) (Does not include animations matrix!)
	math::Matrix44 m_posAnimMatrix; //!< Cached posMatrix * animation matrix; (Only valid when we have an animation!)
	math::Vector3 m_centerPosition; //!< The center position of this plane, updated when pos matrix changes. (Does not include animation!)
	mutable tt::math::Matrix44 m_texMatrix;     //!< Matrix with just the texture position. (Still needs animation matrix.)
	
	renderer::ColorRGBA m_topLeftColor;
	renderer::ColorRGBA m_topRightColor;
	renderer::ColorRGBA m_bottomLeftColor;
	renderer::ColorRGBA m_bottomRightColor;
	
	math::Vector2 m_topLeftUV;
	math::Vector2 m_topRightUV;
	math::Vector2 m_bottomLeftUV;
	math::Vector2 m_bottomRightUV;
};

//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_PLANESCENE_H)
