#if !defined(INC_TT_PRES_ANIM2D_COLORANIMATION2D_H)
#define INC_TT_PRES_ANIM2D_COLORANIMATION2D_H

#include <tt/pres/anim2d/Animation2D.h>
#include <tt/math/Vector4.h>
#include <tt/pres/PresentationValue.h>


namespace tt {
namespace pres {
namespace anim2d {

class ColorAnimation2D : public Animation2D
{
public:
	ColorAnimation2D();
	virtual ~ColorAnimation2D();
	
	/*! \brief Returns the sorting weight, used by autosorting.
	    \return The sorting weight.*/
	virtual int getSortWeight() const;
	
	/*! \brief Returns the normalized color for the current state of the animation.
	    \return The normalized color.*/
	virtual math::Vector4 getColor() const;
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_beginR The begin r value.
	    \param p_beginG The begin g value.
	    \param p_beginB The begin b value.
	    \param p_beginA The begin a value.
	    \param p_endR The end r value.
	    \param p_endG The end g value.
	    \param p_endB The end b value.
	    \param p_endA The end a value.*/
	void setBeginAndEndRange(const pres::PresentationValue& p_beginR,
	                         const pres::PresentationValue& p_beginG,
	                         const pres::PresentationValue& p_beginB,
	                         const pres::PresentationValue& p_beginA,
	                         const pres::PresentationValue& p_endR,
	                         const pres::PresentationValue& p_endG,
	                         const pres::PresentationValue& p_endB,
	                         const pres::PresentationValue& p_endA);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this animation.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node, 
	                  const DataTags& p_applyTags, 
	                  const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this animation.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
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
	
	/*! \brief Returns a clone of the color animation 2D object.*/
	inline virtual ColorAnimation2D* clone() const { return new ColorAnimation2D(*this); }
	
private:
	virtual void setRanges(PresentationObject* p_presObj);
	
	
	math::Vector4 m_delta;
	math::Vector4 m_begin;
	
	pres::PresentationValue m_beginR;
	pres::PresentationValue m_beginG;
	pres::PresentationValue m_beginB;
	pres::PresentationValue m_beginA;
	pres::PresentationValue m_endR;
	pres::PresentationValue m_endG;
	pres::PresentationValue m_endB;
	pres::PresentationValue m_endA;
	
	
	//Enable copy / disable assignment
	ColorAnimation2D(const ColorAnimation2D& p_rhs);
	ColorAnimation2D& operator=(const ColorAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_COLORANIMATION2D_H)
