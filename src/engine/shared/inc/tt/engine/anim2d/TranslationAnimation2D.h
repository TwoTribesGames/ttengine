#if !defined(INC_TT_ENGINE_ANIM2D_TRANSLATIONANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_TRANSLATIONANIMATION2D_H

#include <tt/engine/anim2d/PositionAnimation2D.h>
#include <tt/math/Vector3.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace engine {
namespace anim2d {

class TranslationAnimation2D : public PositionAnimation2D
{
public:
	explicit TranslationAnimation2D(bool p_isGameTranslation = false);
	virtual ~TranslationAnimation2D() { }
	
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
	    \param p_endX The end x value.
		\param p_endY The end y value.*/
	void setBeginAndEndRange(const math::Range& p_beginX, const math::Range& p_beginY, const math::Range& p_beginZ,
	                         const math::Range& p_endX,   const math::Range& p_endY,   const math::Range& p_endZ);
	
	/*! \brief Sets the begin and end values for the animation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end);
	
	/*! \brief Sets the begin range for the animation.
	    \param p_beginX The begin x range.
	    \param p_beginY The begin y range.*/
	inline void setBeginRange(const math::Range& p_beginX, const math::Range& p_beginY, const math::Range& p_beginZ)
	{ setBeginAndEndRange(p_beginX, p_beginY, p_beginZ, m_endXRange, m_endYRange, m_endZRange); }
	
	/*! \brief Sets the begin value for the animation.
	    \param p_begin The begin value.*/
	inline void setBegin(const math::Vector3& p_begin) { setBeginAndEnd(p_begin, m_end); }
	
	/*! \brief Sets the end range for the animation.
	    \param p_endX The end x range.
	    \param p_endY The end y range.
	    \param p_endZ The end z range.*/
	inline void setEndRange(const math::Range& p_endX, const math::Range& p_endY, const math::Range& p_endZ)
	{ setBeginAndEndRange(m_beginXRange, m_beginYRange, m_beginZRange, p_endX, p_endY, p_endZ); }
	
	/*! \brief Sets the end value for the animation.
	    \param p_end The end value.*/
	inline void setEnd(const math::Vector3& p_end) { setBeginAndEnd(m_begin, p_end); }
	
	/*! \brief Gets the begin x range for the animation.
	    \return The begin x range.*/
	inline math::Range getBeginXRange() const { return m_beginXRange; }
	
	/*! \brief Gets the begin y range for the animation.
	    \return The begin y range.*/
	inline math::Range getBeginYRange() const { return m_beginYRange; }
	
	/*! \brief Gets the begin z range for the animation.
	    \return The begin z range.*/
	inline math::Range getBeginZRange() const { return m_beginZRange; }
	
	/*! \brief Gets the begin value for the animation.
	    \return The begin value.*/
	inline math::Vector3 getBegin() const { return m_begin; }
	
	/*! \brief Gets the end x range for the animation.
	    \return The end x range.*/
	inline math::Range getEndXRange() const { return m_endXRange; }
	
	/*! \brief Gets the end y range for the animation.
	    \return The end y range.*/
	inline math::Range getEndYRange() const { return m_endYRange; }
	
	/*! \brief Gets the end z range for the animation.
	    \return The end z range.*/
	inline math::Range getEndZRange() const { return m_endZRange; }
	
	/*! \brief Gets the end value for the animation.
	    \return The end value.*/
	inline math::Vector3 getEnd() const { return m_end; }
	
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
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
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
	virtual AnimationType getAnimationType() const;
	
	/*! \brief Returns a clone of the position animation object.*/
	inline virtual TranslationAnimation2D* clone() const { return new TranslationAnimation2D(*this); }
	
	// HACK: Set whether to invert the Y axis during loads.
	//       This isn't passed as a parameter to load() because these functions are virtual,
	//       signatures dictated by the base class
	inline void setInvertYDuringLoad(bool p_invertY) { m_invertYDuringLoad = p_invertY; }
	
private:
	virtual void setRanges();
	
	math::Vector3 m_begin;
	math::Vector3 m_end;
	math::Vector3 m_delta;
	
	math::Range m_beginXRange;
	math::Range m_beginYRange;
	math::Range m_beginZRange;
	math::Range m_endXRange;
	math::Range m_endYRange;
	math::Range m_endZRange;
	
	bool m_invertYDuringLoad;
	
	bool m_rangesSet;
	bool m_isGameTranslation;
	
	
	//Enable copy / disable assignment
	TranslationAnimation2D(const TranslationAnimation2D& p_rhs);
	TranslationAnimation2D& operator=(const TranslationAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_TRANSLATIONANIMATION2D_H)
