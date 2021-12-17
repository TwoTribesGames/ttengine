#if !defined(INC_TT_ENGINE_ANIM2D_ANIMATIONSTACK2D_H)
#define INC_TT_ENGINE_ANIM2D_ANIMATIONSTACK2D_H

#include <vector>

#include <tt/engine/anim2d/fwd.h>
#include <tt/engine/anim2d/PositionAnimation2D.h>
#include <tt/engine/anim2d/StackBase.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace anim2d {

class AnimationStack2D : public StackBase<PositionAnimation2DPtr>
{
public:
	AnimationStack2D();
	
	void push_back(const PositionAnimation2DPtr& p_animation);
	virtual void clear();
	
	inline math::Matrix44 getTransform() const { math::Matrix44 mtx; updateTransform(&mtx); return mtx; }
	void updateTransform(math::Matrix44* p_mtx) const;
	
	bool hasZAnimation() const;
	bool hasRotationAnimation() const;
	
	void setAutoSort(bool p_enabled);
	inline bool usesAutoSort() const { return m_autoSort; }
	
	void setInitiallySuspended(bool p_suspended);
	inline bool isInitiallySuspended() const { return m_initiallySuspended; }
	
	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, bool p_invertTranslationAnimationY);
	
	/*! \brief Loads the animation stack from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node,
	          bool                p_invertTranslationAnimationY,
	          const Tags&         p_applyTags,
	          const Tags&         p_acceptedTags);
	
	/*! \brief Stores the animation stack to an xml node.
	    \param p_node The xml node to save to.
	    \return Whether storing succeeded or not.*/
	bool save(xml::XmlNode* p_node) const;
	
	/*! \brief Loads the animation stack from a file.
	    \param p_file The file to load from.
	    \return Whether loading succeeded or not.*/
	bool load(const fs::FilePtr& p_file, bool p_invertTranslationAnimationY);
	
	/*! \brief Stores the animation stack to a file.
	    \param p_file The file to save to.
	    \return Whether storing succeeded or not.*/
	bool save(const fs::FilePtr& p_file) const;
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, bool p_invertTranslationAnimationY);
	
	/*! \brief Loads the animation stack from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this stacks animations.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, bool p_invertTranslationAnimationY,
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
	
	/*! \brief returns the first occurence of the Animation of the given type
	    \param p_animationType  type of animation to return as used in the factory
	    \return the first animation of the given type */ 
	//PositionAnimation2DPtr getAnimationByType(const PositionAnimation2D::AnimationType& p_animationType);
	
	/*! \brief Sets the begin and end values for the gameTranslation.
	    \param p_begin The begin value.
	    \param p_end The end value.*/
	void setGameTranslationBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end);
	
	/*! \brief adds two stacks together. .*/
	void appendStack(const AnimationStack2DPtr& p_other);
	
	/*! \brief Returns a clone of the animation stack 2D object.*/
	AnimationStack2DPtr clone() const;
	
	/*! \brief Clears this stack and makes it a default one.*/
	void makeDefault();
	
	void setAnimationDirectionType(Animation2D::DirectionType p_type);
	void setAnimationTimeType     (Animation2D::TimeType      p_type);
	void setAnimationTweenType    (TweenType                  p_type);
	
private:
	bool loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	//Animations m_animations;
	bool m_autoSort;
	bool m_initiallySuspended;
	
	typedef std::vector<TranslationAnimation2DPtr> Translations;
	Translations m_gameTranslations;
	
	AnimationStack2D(const AnimationStack2D& p_rhs);      // Enable copy 
	AnimationStack2D& operator=(const AnimationStack2D&); // Disable assignment
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_ANIMATIONSTACK2D_H)
