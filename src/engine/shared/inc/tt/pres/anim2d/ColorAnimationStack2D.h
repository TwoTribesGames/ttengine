#if !defined(INC_TT_PRES_ANIM2D_COLORANIMATIONSTACK2D_H)
#define INC_TT_PRES_ANIM2D_COLORANIMATIONSTACK2D_H

#include <vector>

#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/anim2d/ColorAnimation2D.h>
#include <tt/pres/anim2d/StackBase.h>
#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace pres {
namespace anim2d {

class ColorAnimationStack2D : public StackBase<ColorAnimation2D>
{
public:
	ColorAnimationStack2D();
	
	void push_back(const ColorAnimation2DPtr& p_animation);
	
	engine::renderer::ColorRGBA getColor(const engine::renderer::ColorRGBA& p_source) const;

	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, 
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);
	
	/*! \brief Save the animation stack to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief adds two stacks together. .*/
	void appendStack(ColorAnimationStack2D& p_other);
	
	ColorAnimationStack2D(const ColorAnimationStack2D& p_rhs);
	
private:
	bool loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	ColorAnimationStack2D& operator=(const ColorAnimationStack2D&); // disable assignment
};


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_COLORANIMATIONSTACK2D_H)
