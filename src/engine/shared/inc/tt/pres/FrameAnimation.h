#if !defined(INC_TT_PRES_FRAMEANIMATION_H)
#define INC_TT_PRES_FRAMEANIMATION_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/Animation2D.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>
#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/fwd.h>
#include <tt/pres/PresentationValue.h>


namespace tt {
namespace pres {

enum LightMaskType
{
	LightMaskType_None,               // No light mask.
	LightMaskType_UseAlphaChannel,    // Use alpha from (normal) texture as lightmask.
	LightMaskType_UseLightmaskTexture // Use separate lightmask texture. (Find it by prefixing "lightmask_" to texture name.)
};


class FrameAnimation : public anim2d::Animation2D
{
public:
	FrameAnimation();
	virtual ~FrameAnimation() { }
	
	/*! \brief Creates a Quad for rendering. Sets the texturecoordinates and texture. */ 
	void createQuad();	
	
	/*! \brief Updates the quad withe the given ColorAnimationStack2D. */ 
	void updateColor(const anim2d::ColorAnimationStack2D& p_coloranim);
	
	virtual void update(real p_delta);
	
	/*! \brief Renders the quad. */ 
	void render(const math::Matrix44& p_transform) const;
	
	inline void renderWithLightPass(const math::Matrix44& p_transform) const
	{
		if (getRenderPass().empty())
		{
			render(p_transform);
		}
	}
	
	/*! \brief Creates and returns the texture used by this frameAnim.*/
	void getAndLoadAllUsedTextures(engine::renderer::TextureContainer& p_textures) const;
	
	/*! \brief Returns the texture matrix for the frame animation.
	    \return texture matrix */ 
	math::Matrix44 getTextureMatrix() const;
	
	/*! \brief Returns whether the quad should be rendered */ 
	bool isVisible() const;
	
	virtual bool isActive() const;
	virtual bool isLooping() const;
	
	bool loadXml(const xml::XmlNode* p_node, const DataTags& p_applyTags, const Tags& p_acceptedTags, 
	          code::ErrorStatus* p_errStatus);
	
	bool parseSpriteStripDirectory(const std::string& p_directory, code::ErrorStatus* p_errStatus);
	
	bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads the frame anim from a buffer.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this timer.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT,
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus);
	
	using Animation2D::save; // prevent hiding
	using Animation2D::load; // prevent hiding
	
	inline const std::string& getSpriteDirectory() const { return m_spriteDirectory; }
	inline const std::string& getRenderPass()      const { return m_passName; }

	inline void setRenderPass     (const std::string& p_passName)        { m_passName = p_passName; }
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const ;
	
	virtual int getSortWeight() const { TT_PANIC("Sorting Not implemented for Frame Animations"); return 0; }
	
	/*! \brief Returns a clone of the frame animation object.*/
	virtual FrameAnimation* clone() const { return new FrameAnimation(*this); }
	
	/*! \brief Clears this stack and makes it a default one.*/
	void makeDefault();
	
	inline void setLightMaskType(LightMaskType p_type) { m_lightMaskType = p_type; }
	inline LightMaskType getLightMaskType() const { return m_lightMaskType; }
	
	static void enableHudScale(bool p_enable) { ms_doHudScale = p_enable; }
	
private:
	std::string parseOptionalString(const xml::XmlNode* p_node, const std::string& p_attribute, 
	                               const std::string& p_default, code::ErrorStatus* p_errStatus);
	virtual void setRanges(PresentationObject* p_presObj);
	void updateTextureAndQuadWithPresentationValues();
	
	inline math::Vector2 getFrameSize() const { return m_frameSize; }
	inline math::Vector2 getQuadSize()  const { return math::Vector2(m_quadSizeX,  m_quadSizeY);  }
	inline s32 getBeginFrame()          const { return static_cast<s32>(m_beginFrame.get());      }
	inline s32 getEndFrame()            const { return static_cast<s32>(m_endFrame.get());        }
	inline s32 getFrameDifference()     const { return m_frameDifference;                         }
	
	inline math::Vector3 getTranslation() const { return math::Vector3(m_translationX, m_translationY, m_translationZ); }
	inline math::Vector2 getScale()       const { return math::Vector2(m_scaleX, m_scaleY); }
	inline real          getRotation()    const { return math::degToRad(m_rotation); }
	
	enum Flip // flip mask
	{
		Flip_None        = 0,
		Flip_Horizontal  = 1,
		Flip_Vertical    = 2
	};
	
	enum Flag
	{
		Flag_IgnoreFog,
		
		Flag_Count
	};
	typedef tt::code::BitMask<Flag, Flag_Count> Flags;
	
	PresentationValue m_beginFrame;
	PresentationValue m_endFrame;
	s32               m_frameDifference;
	s32               m_frameBegin;
	s32               m_totalFrames;
	math::Point2      m_frameCount;
	math::Vector2     m_frameSize;
	PresentationValue m_frameSizeX;
	PresentationValue m_frameSizeY;
	PresentationValue m_quadSizeX;
	PresentationValue m_quadSizeY;
	PresentationValue m_fps;
	bool              m_holdFirstFrame;
	bool              m_holdLastFrame;
	u32               m_flip;
	engine::renderer::FilterMode m_minFilter;
	engine::renderer::FilterMode m_magFilter;
	engine::renderer::FilterMode m_mipFilter;
	engine::renderer::BlendMode  m_blendMode;
	Flags                        m_flags;
	
	// FIXME: Use enum instead of string
	std::string m_passName;
	
	engine::EngineID m_spritestripID;
	engine::EngineID m_lightmaskID;
	
	std::string                  m_spriteDirectory;
	bool                         m_usingDirectory;
	engine::renderer::TexturePtr m_texture;
	bool                         m_isTextureForVisualBoy;
	engine::renderer::TexturePtr m_lightmask;
	PresentationQuadPtr          m_quad;
	
	PresentationValue m_translationX;
	PresentationValue m_translationY;
	PresentationValue m_translationZ;
	PresentationValue m_scaleX;
	PresentationValue m_scaleY;
	bool              m_isUniformScale;
	PresentationValue m_rotation;
	PresentationValue m_texAnimU;
	PresentationValue m_texAnimV;
	
	bool m_loaded;
	
	bool              m_usingDuration;
	PresentationValue m_frameDuration;
	real              m_durationTimeLeft;

	math::Vector2     m_animationUV;
	math::Vector2     m_offsetUV;
	math::Vector2     m_cameraSpaceScale;

	LightMaskType     m_lightMaskType;
	
	static bool       ms_doHudScale;
	
	//Enable copy / disable assignment
	FrameAnimation(const FrameAnimation& p_rhs);
	FrameAnimation& operator=(const FrameAnimation&);
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_FRAMEANIMATION_H)
