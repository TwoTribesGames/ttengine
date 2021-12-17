#if !defined(INC_TT_ENGINE_ANIM2D_COLORANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_COLORANIMATION2D_H

#include <tt/engine/anim2d/Animation2D.h>
#include <tt/math/Vector4.h>


namespace tt {
namespace engine {
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
	void setBeginAndEndRange(const math::Range& p_beginR,
	                         const math::Range& p_beginG,
	                         const math::Range& p_beginB,
	                         const math::Range& p_beginA,
	                         const math::Range& p_endR,
	                         const math::Range& p_endG,
	                         const math::Range& p_endB,
	                         const math::Range& p_endA);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this animation.
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
	    \param p_applyTags Additional tags that should be applied to this animation.
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
	
	/*! \brief Returns a clone of the color animation 2D object.*/
	inline virtual ColorAnimation2D* clone() const { return new ColorAnimation2D(*this); }
	
private:
	virtual void setRanges();
	
	
	math::Vector4 m_begin;
	math::Vector4 m_end;
	math::Vector4 m_delta;
	
	math::Range m_beginRRange;
	math::Range m_beginGRange;
	math::Range m_beginBRange;
	math::Range m_beginARange;
	math::Range m_endRRange;
	math::Range m_endGRange;
	math::Range m_endBRange;
	math::Range m_endARange;
	
	
	//Enable copy / disable assignment
	ColorAnimation2D(const ColorAnimation2D& p_rhs);
	ColorAnimation2D& operator=(const ColorAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_COLORANIMATION2D_H)
