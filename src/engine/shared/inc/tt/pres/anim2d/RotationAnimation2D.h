#if !defined(INC_TT_PRES_ANIM2D_ROTATIONANIMATION2D_H)
#define INC_TT_PRES_ANIM2D_ROTATIONANIMATION2D_H

#include <tt/pres/anim2d/PositionAnimation2D.h>
#include <tt/pres/PresentationValue.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace pres {
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
	void setBeginAndEndRange(const pres::PresentationValue& p_begin, const pres::PresentationValue& p_end);
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setBeginAndEnd(real p_begin, real p_end);
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBeginRange(const pres::PresentationValue& p_begin) { setBeginAndEndRange(p_begin, m_end); }
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBegin(real p_begin) { setBeginAndEnd(p_begin, m_end); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEndRange(const pres::PresentationValue& p_end) { setBeginAndEndRange(m_begin, p_end); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEnd(real p_end) { setBeginAndEnd(m_begin, p_end); }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline const pres::PresentationValue& getBeginRange() const { return m_begin; }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline real getBegin() const { return m_begin; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline const pres::PresentationValue& getEndRange() const { return m_end; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline real getEnd() const { return m_end; }
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node, 
	                  const DataTags& p_applyTags, 
	                  const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads a Static animation. For offsets that do not animate.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this animation.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool loadStatic(const xml::XmlNode* p_node,
	                        const DataTags& p_applyTags,
	                        const Tags& p_acceptedTags,
	                        code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	                  const DataTags& p_applyTags, 
	                  const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);
	
	/*! \brief Save the animation to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
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
	virtual void setRanges(PresentationObject* p_presObj);
	
	
	pres::PresentationValue m_begin;
	pres::PresentationValue m_end;
	real m_delta;
	real m_beginAngle;
	
	
	//Enable copy / disable assignment
	RotationAnimation2D(const RotationAnimation2D& p_rhs);
	RotationAnimation2D& operator=(const RotationAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_ROTATIONANIMATION2D_H)
