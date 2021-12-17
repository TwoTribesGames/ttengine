#if !defined(INC_TT_PRES_FRAMEANIMATIONSTACK_H)
#define INC_TT_PRES_FRAMEANIMATIONSTACK_H
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/pres/anim2d/StackBase.h>
#include <tt/pres/fwd.h>
#include <tt/pres/FrameAnimation.h>
#include <tt/str/str_types.h>


namespace tt {
namespace pres {

class FrameAnimationStack : public anim2d::StackBase<FrameAnimation>
{
public:
	FrameAnimationStack() { }
	virtual ~FrameAnimationStack() { }
	
	/*! \brief Creates the Quads for rendering the frame animations. Sets the texturecoordinates and texture. */ 
	void createQuads() const;
	
	/*! \brief Updates the colors of the frame animation quads with the given ColorAnimationStack2D. */ 
	void updateColors(const anim2d::ColorAnimationStack2D& p_coloranim);
	
	/*! \brief Renders the visible Frame animation quads in the order they appear in the presentation file */ 
	void render(const math::Matrix44& p_transform) const;
	
	/*! \brief Renders the visible Frame animation quads with the specified render pass */ 
	void renderPass(const std::string& p_passName, const math::Matrix44& p_transform) const;
	
	/*! \brief Loads all the texures for the frameanims in this stack. And returns them.
	        used for precaching the textures. */ 
	engine::renderer::TextureContainer getAndLoadAllUsedTextures() const;
	
	/*! \brief Returns whether the quad should be rendered */ 
	bool isVisible()const;
	
	/*! \brief Returns whether the given pass is visible */ 
	bool isPassVisible(const std::string& p_passName)const;
	
	/*! \brief Loads the frame animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags tags that all loaded animations should be applied to
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, const DataTags& p_applyTags, const Tags& p_acceptedTags, 
	          code::ErrorStatus* p_errStatus);
	
	virtual bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const;
	
	/*! \brief Loads the frame animation stack from a buffer.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags tags that all loaded animations should be applied to
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	           const DataTags& p_applyTags, const Tags& p_acceptedTags, 
	           code::ErrorStatus* p_errStatus);

	/*! \brief adds two stacks together. .*/
	void appendStack(FrameAnimationStack& p_other);
	
	/*! \brief Returns all the render pass names used in this stack. .*/
	str::StringSet getRenderPassNames() const;
	
	/*! \brief Gets the asset dependencies for this stack.*/
	Dependencies getDependencies()const;

	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Clears this stack and makes it a default one.*/
	void makeDefault();
	
	FrameAnimationStack(const FrameAnimationStack& p_rhs);
	
private:
	FrameAnimationStack& operator=(const FrameAnimationStack&); // disable assignment
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_FRAMEANIMATIONSTACK_H)
