#if !defined(INC_TT_ENGINE_ANIM2D_COLORANIMATIONSTACK2D_H)
#define INC_TT_ENGINE_ANIM2D_COLORANIMATIONSTACK2D_H

#include <vector>

#include <tt/engine/anim2d/fwd.h>
#include <tt/engine/anim2d/ColorAnimation2D.h>
#include <tt/engine/anim2d/StackBase.h>
#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace engine {
namespace anim2d {

class ColorAnimationStack2D : public StackBase<ColorAnimation2DPtr>
{
public:
	ColorAnimationStack2D();
	
	void push_back(const ColorAnimation2DPtr& p_animation);
	
	renderer::ColorRGBA getColor(const renderer::ColorRGBA& p_source) const;
	
	void setInitiallySuspended(bool p_suspended);
	inline bool isInitiallySuspended() const { return m_initiallySuspended; }
	
	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node);
	
	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, 
	          const Tags& p_applyTags, 
	          const Tags& p_acceptedTags);
	
	/*! \brief Stores the animation stack to an xml node.
	    \param p_node The xml node to save to.
	    \return Whether storing succeeded or not.*/
	bool save(xml::XmlNode* p_node) const;
	
	/*! \brief Loads the animation stack from a file.
	    \param p_file The file to load from.
	    \return Whether loading succeeded or not.*/
	bool load(const fs::FilePtr& p_file);
	
	/*! \brief Stores the animation stack to a file.
	    \param p_file The file to save to.
	    \return Whether storing succeeded or not.*/
	bool save(const fs::FilePtr& p_file) const;
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	          const Tags& p_applyTags, 
	          const Tags& p_acceptedTags);
	
	/*! \brief Save the animation stack to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief adds two stacks together. .*/
	void appendStack(const ColorAnimationStack2DPtr& p_other);
	
	/*! \brief Returns a clone of the animation stack 2D object.*/
	ColorAnimationStack2DPtr clone() const;
	
	void setAnimationDirectionType(Animation2D::DirectionType p_type);
	void setAnimationTimeType     (Animation2D::TimeType      p_type);
	void setAnimationTweenType    (TweenType                  p_type);
	
private:
	bool loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	bool m_initiallySuspended;
	
	//Enable copy / disable assignment
	ColorAnimationStack2D(const ColorAnimationStack2D& p_rhs);
	ColorAnimationStack2D& operator=(const ColorAnimationStack2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_COLORANIMATIONSTACK2D_H)
