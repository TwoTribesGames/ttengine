#if !defined(INC_TT_ENGINE_ANIM2D_ROTATIONANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_ROTATIONANIMATION2D_H

#include <tt/engine/anim2d/PositionAnimation2D.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace engine {
namespace anim2d {

class RotationAnimation2D : public PositionAnimation2D
{
public:
	RotationAnimation2D();
	virtual ~RotationAnimation2D() { }
	
	virtual void applyTransform(Transform* p_transform) const;
	
	/*! \brief Returns whether or not the animation has a Z animation.
	    \return Whether or not the animation has a Z animation.*/
	virtual bool hasZAnimation() const;
	
	/*! \brief Returns whether or not the animation has a Rotation animation.
	    \return Whether or not the animation has a Rotation animation.*/
	virtual inline bool hasRotationAnimation() const { return true; }
	
	/*! \brief Returns the sorting weight.
	    \return The sorting weight.*/
	virtual int getSortWeight() const;
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setBeginAndEndRange(const math::Range& p_begin, const math::Range& p_end);
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setBeginAndEnd(real p_begin, real p_end);
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBeginRange(const math::Range& p_begin) { setBeginAndEndRange(p_begin, m_endRange); }
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBegin(real p_begin) { setBeginAndEnd(p_begin, m_end); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEndRange(const math::Range& p_end) { setBeginAndEndRange(m_beginRange, p_end); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEnd(real p_end) { setBeginAndEnd(m_begin, p_end); }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline math::Range getBeginRange() const { return m_beginRange; }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline real getBegin() const { return m_begin; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline math::Range getEndRange() const { return m_endRange; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline real getEnd() const { return m_end; }
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node, 
	                  const Tags& p_applyTags, 
	                  const Tags& p_acceptedTags);
	
	/*! \brief Stores the animation to an xml node.
	    \param p_node The xml node to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(xml::XmlNode* p_node) const;
	
	/*! \brief Loads the animation from a file.
	    \param p_file The file to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const fs::FilePtr& p_file);
	
	/*! \brief Stores the animation to a file.
	    \param p_file The file to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(const fs::FilePtr& p_file) const;
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	                  const Tags& p_applyTags, 
	                  const Tags& p_acceptedTags);
	
	/*! \brief Save the animation to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns the type of the animation
	    \return AnimationType */ 
	virtual AnimationType getAnimationType() const 
	{ 
		return PositionAnimation2D::AnimationType_Rotation; 
	}
	
	/*! \brief Returns a clone of the position animation object.*/
	inline virtual RotationAnimation2D* clone() const { return new RotationAnimation2D(*this); }
	
private:
	virtual void setRanges();
	
	
	real m_begin;
	real m_end;
	real m_delta;
	
	math::Range m_beginRange;
	math::Range m_endRange;
	
	
	//Enable copy / disable assignment
	RotationAnimation2D(const RotationAnimation2D& p_rhs);
	RotationAnimation2D& operator=(const RotationAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_ROTATIONANIMATION2D_H)
