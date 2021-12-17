#if !defined(INC_TT_PRES_ANIM2D_ANIMATIONSTACK2D_H)
#define INC_TT_PRES_ANIM2D_ANIMATIONSTACK2D_H

#include <vector>

#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/anim2d/PositionAnimation2D.h>
#include <tt/pres/anim2d/StackBase.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace pres {
namespace anim2d {

class AnimationStack2D : public StackBase<PositionAnimation2D>
{
public:
	AnimationStack2D();
	
	void push_back(const PositionAnimation2DPtr& p_animation);
	virtual void clear();
	
	void updateTransform(math::Matrix44* p_mtx) const;
	
	bool hasZAnimation() const;
	bool hasRotationAnimation() const;
	
	void sortAll();
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_customValues Any custom values found, will be replaced by values in this container. */
	virtual void start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name);
	
	/*! \brief Stops the animation.*/
	virtual void stop();
	
	/*! \brief Pauses the animation.*/
	virtual void pause();
	
	/*! \brief Resumes the animation.*/
	virtual void resume();
	
	/*! \brief Unpauses and stops the animation, sets time to 0.*/
	virtual void reset();
	
	
	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, 
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus );
	
	/*! \brief Loads the start values from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool loadStartValues(const xml::XmlNode* p_node, 
	                     const DataTags& p_applyTags, 
	                     const Tags& p_acceptedTags,
	                     code::ErrorStatus* p_errStatus );
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus );
	
	/*! \brief Save the animation stack to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns the tags of this Stack*/
	virtual DataTags getTags() const;
	
	/*! \brief Sets the begin and end values for the gameTranslation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setGameTranslationBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end);
	
	/*! \brief adds two stacks together. .*/
	void appendStack(AnimationStack2D& p_other);
	
	/*! \brief Clears this stack and makes it a default one.*/
	void makeDefault();
	
	inline anim2d::Transform& modifyCustomTransform() { m_customTransformUsed = true; return m_customTransform; }
	inline void resetCustomTransform() { m_customTransformUsed = false; m_customTransform.reset(); }
	
	AnimationStack2D(const AnimationStack2D& p_rhs);
private:
	bool loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	typedef std::vector<TranslationAnimation2DPtr> Translations;
	Translations m_gameTranslations;
	Translations m_originTranslations;
	
	math::Matrix44 m_originOffset;           // Cached result of m_originTranslations.      Not saved. Used at runtime.
	math::Matrix44 m_originOffsetInv;        // Cached result of m_originOffset.getInverse. Not saved. Used at runtime.
	bool           m_originOffsetIsIdentity; // Cached result of m_originOffset.isIdentity. Not saved. Used at runtime.
	
	bool      m_customTransformUsed;
	Transform m_customTransform; // Custom Start Transformation.
	
	AnimationStack2D& operator=(const AnimationStack2D&); // disable assignment
};


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_ANIMATIONSTACK2D_H)
