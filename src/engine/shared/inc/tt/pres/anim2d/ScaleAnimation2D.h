#if !defined(INC_TT_PRES_ANIM2D_SCALEANIMATION2D_H)
#define INC_TT_PRES_ANIM2D_SCALEANIMATION2D_H

#include <tt/pres/anim2d/PositionAnimation2D.h>
#include <tt/math/Vector3.h>
#include <tt/pres/PresentationValue.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace pres {
namespace anim2d {

class ScaleAnimation2D : public PositionAnimation2D
{
public:
	ScaleAnimation2D();
	virtual ~ScaleAnimation2D() { }
	
	virtual void applyTransform(Transform* p_transform) const;
	
	/*! \brief Returns whether or not the animation has a Z animation.
	    \return Whether or not the animation has a Z animation.*/
	virtual bool hasZAnimation() const;
	
	/*! \brief Returns whether or not the animation has a Rotation animation.
	    \return Whether or not the animation has a Rotation animation.*/
	virtual inline bool hasRotationAnimation() const { return false; }
	
	/*! \brief Returns the sorting weight.
	    \return The sorting weight.*/
	virtual int getSortWeight() const;
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_beginX The begin x value.
	    \param p_beginY The begin y value.
	    \param p_beginZ The begin z value.
	    \param p_endX The end x value.
	    \param p_endY The end y value.
	    \param p_endZ The end z value.*/
	void setBeginAndEndRange(const pres::PresentationValue& p_beginX, const pres::PresentationValue& p_beginY, const pres::PresentationValue& p_beginZ,
	                         const pres::PresentationValue& p_endX,   const pres::PresentationValue& p_endY,   const pres::PresentationValue& p_endZ);
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end);
	
	/*! \brief Sets the begin range for the animation.
	    \param p_beginX The begin x range.
	    \param p_beginY The begin y range.
	    \param p_beginZ The begin z range.*/
	inline void setBeginRange(const pres::PresentationValue& p_beginX, const pres::PresentationValue& p_beginY, const pres::PresentationValue& p_beginZ)
	{ setBeginAndEndRange(p_beginX, p_beginY, p_beginZ, m_endX, m_endY, m_endZ); }
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBegin(const math::Vector3& p_begin) { setBeginAndEnd(p_begin, getEnd()); }
	
	/*! \brief Sets the end range for the animation.
	    \param p_endX The end x range.
	    \param p_endY The end y range.
	    \param p_endZ The end z range.*/
	inline void setEndRange(const pres::PresentationValue& p_endX, const pres::PresentationValue& p_endY, const pres::PresentationValue& p_endZ)
	{ setBeginAndEndRange(m_beginX, m_beginY, m_beginZ, p_endX, p_endY, p_endZ); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEnd(const math::Vector3& p_end) { setBeginAndEnd(getBegin(), p_end); }
	
	/*! \brief Gets the begin x range for the animation.
	    \return The begin x range.*/
	inline const pres::PresentationValue& getBeginXRange() const { return m_beginX; }
	
	/*! \brief Gets the begin y range for the animation.
	    \return The begin y range.*/
	inline const pres::PresentationValue& getBeginYRange() const { return m_beginY; }
	
	/*! \brief Gets the begin z range for the animation.
	    \return The begin z range.*/
	inline const pres::PresentationValue& getBeginZRange() const { return m_beginZ; }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline math::Vector3 getBegin() const { return math::Vector3(m_beginX, m_beginY, m_beginZ); }
	
	/*! \brief Gets the end x range for the animation.
	    \return The end x range.*/
	inline const pres::PresentationValue& getEndXRange() const { return m_endX; }
	
	/*! \brief Gets the end y range for the animation.
	    \return The end y range.*/
	inline const pres::PresentationValue& getEndYRange() const { return m_endY; }
	
	/*! \brief Gets the end z range for the animation.
	    \return The end z range.*/
	inline const pres::PresentationValue& getEndZRange() const { return m_endZ; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline math::Vector3 getEnd() const { return math::Vector3(m_endX, m_endY, m_endZ); }
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node, 
	                  const DataTags& p_applyTags, 
	                  const Tags& p_acceptedTags,
	                  code::ErrorStatus* p_errStatus);
	
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
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	                  const DataTags& p_applyTags, 
	                  const Tags& p_acceptedTags,
	                  code::ErrorStatus* p_errStatus);
	
	/*! \brief Save the animation to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT,
	                  code::ErrorStatus* p_errStatus) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns the type of the animation
	    \return AnimationType */ 
	virtual AnimationType getAnimationType() const 
	{ 
		return PositionAnimation2D::AnimationType_Scale; 
	}
	
	/*! \brief Returns a clone of the position animation object.*/
	inline virtual ScaleAnimation2D* clone() const { return new ScaleAnimation2D(*this); }
	
private:
	virtual void setRanges(PresentationObject* p_presObj);
	
	pres::PresentationValue m_beginX;
	pres::PresentationValue m_beginY;
	pres::PresentationValue m_beginZ;
	pres::PresentationValue m_endX;
	pres::PresentationValue m_endY;
	pres::PresentationValue m_endZ;
	
	math::Vector3 m_delta;
	math::Vector3 m_begin;
	bool          m_isUniformScaleBegin;
	bool          m_isUniformScaleEnd;
	
	
	//Enable copy / disable assignment
	ScaleAnimation2D(const ScaleAnimation2D& p_rhs);
	ScaleAnimation2D& operator=(const ScaleAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_SCALEANIMATION2D_H)
