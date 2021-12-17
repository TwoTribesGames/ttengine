#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONOBJECTWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONOBJECTWRAPPER_H

#include <string>

#include <tt/code/fwd.h>
#include <tt/pres/fwd.h>
#include <tt/str/str.h>

#include <toki/game/script/wrappers/fwd.h>
#include <toki/pres/PresentationObject.h>
#include <toki/serialization/utils.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'PresentationObject' in Squirrel. Used to control presentation objects */
class PresentationObjectWrapper
{
public:
	explicit PresentationObjectWrapper(const pres::PresentationObjectHandle& p_handle = pres::PresentationObjectHandle());
	
	/*! \brief Starts the presentation animation from the loaded presentation file, based on the tags to start.
	    \param p_tagsToStart Array of tag names. Will be merged with the set of tags specified using addPresentationAnimationTag. Only animations with these tags are started.
	    \param p_hideAtEnd Whether the presentation object should be hidden at the end of the animation. 
	    \param p_priority The priority for this animation. (New animation isn't started if the current animation has a high priority. Movement animation get priority 0.) */
	void start(const std::string& p_name, const tt::str::Strings& p_tagsToStart, bool p_hideAtEnd, s32 p_priority);
	
	/*! \brief Starts the presentation animation from the loaded presentation file, based on the tags to start.
	    \param p_tagsToStart Array of tag names. Will be merged with the set of tags specified using addPresentationAnimationTag. Only animations with these tags are started.
	    \param p_hideAtEnd Whether the presentation object should be hidden at the end of the animation. 
	    \param p_priority The priority for this animation. (New animation isn't started if the current animation has a high priority. Movement animation get priority 0.)
	    \param p_settings The start settings for this presentation animation */
	void startEx(const std::string& p_name, const tt::str::Strings& p_tagsToStart, bool p_hideAtEnd, s32 p_priority,
	             const PresentationStartSettingsWrapper& p_settings);
	
	/*! \brief Stops any currently playing presentation animation. */
	void stop();
	
	/*! \brief Returns whether any of the elements are active. If any of the elements is looping, this always returns true. */
	bool isActive();
	
	/*! \brief Toggles visibility of this presentation. */
	void setVisible(bool p_visible);
	
	/*! \brief Returns whether this presentation is visible or not. NOTE: It doesn't have to mean it is on screen! */
	bool isVisible();
	
	/*! \brief Adds a presentation tag to the set that will be used when playing a presentation animation
	           (either by calling start/startEx or when started from movement set).
	    \param p_tagName Name of the tag to add. */
	void addTag(const std::string& p_tagName);
	
	/*! \brief Check if a presentation tag is added.
	    \param p_tagName Name of the tag to check.
	    \returns true if found in the tags. */
	bool hasTag(const std::string& p_tagName);
	
	/*! \brief Removes a presentation tag from the set that will be used when playing a presentation animation
	           (either by calling start or when started from movement set).
	    \param p_tagName Name of the tag to remove. */
	void removeTag(const std::string& p_tagName);
	
	/*! \brief Clears the set of presentation tags that will be used when playing a presentation animation
	           (either by calling start or when started from movement set). */
	void clearTags();
	
	/*! \brief Adds a TextLabel to this presentation object */
	void addTextLabel(TextLabelWrapper* p_textLabel, const tt::math::Vector2& p_offset);
	
	/*! \brief Removes a TextLabel from this presentation object */
	void removeTextLabel(TextLabelWrapper* p_textLabel);
	
	/*! \brief Returns the current animation name*/
	std::string getCurrentAnimationName() const;
	
	/*! \brief Adds a Custom Value. Custom Values, in the presentation, with the given name will 
	           be replaced by the given value.
	    \param p_valueName Name of the custom value to add.
	    \param p_value Value of the custom value to add. */
	void addCustomValue(const std::string& p_valueName, real p_value);
	
	/*! \brief Returns the current presentation animation priority. */
	s32 getPriority() const;
	
	/*! \brief Sets whether this presentation object is affected by movement. Default is false. */
	void setAffectedByMovement(bool p_affected);
	
	/*! \brief Returns if this presentation object is affected by movement. */
	bool isAffectedByMovement() const;
	
	/*! \brief Sets whether this presentation object is affected by the orientation of the entity. Default is true.
	           NOTE: At creation of the presentation object, the object is affected by the orientation of the entity! */
	void setAffectedByOrientation(bool p_affected);
	
	/*! \brief Returns if this presentation object is affected by the orientation of the entity. */
	bool isAffectedByOrientation() const;
	
	/*! \brief Sets whether this presentation object is affected by the direction of the floor of the entity. Default is true.
	           NOTE: At creation of the presentation object, the object is affected by the direction of the floor of the entity! */
	void setAffectedByFloorDirection(bool p_affected);
	
	/*! \brief Returns if this presentation object is affected by the direction of the floor of the entity. */
	bool isAffectedByFloorDirection() const;
	
	/*! \brief Sets whether this presentation object should follow its parent entity. Default is true */
	void setIsFollowingParent(bool m_isFollowingParent);
	
	/*! \brief Returns if this presentation object is currently following its parent entity. */
	bool isFollowingParent() const;
	
	/*! \brief Set the entity the presentation should rotate towards.
	    \param p_entity Entity to rotate to. */
	void setRotateToEntity(const EntityWrapper* p_entity);
	
	/*! \brief Set translation in custom transformation. */
	void setCustomTranslation(const tt::math::Vector2& p_translation);
	/*! \brief Set custom Z depth */
	void setCustomZOffset(real p_zOffset);
	/*! \brief Set scale in custom transformation. */
	void setCustomScale      (const tt::math::Vector2& p_scale      );
	/*! \brief Set scale in custom transformation. */
	void setCustomUniformScale(real p_scale);
	/*! \brief Set rotation in custom transformation. */
	void setCustomRotation(    real p_rotation);
	/*! \brief reset custom transformation. */
	void resetCustomTransformation();
	
	/*! \brief Get translation in custom transformation. */
	tt::math::Vector2 getCustomTranslation() const;
	/*! \brief Get scale in custom transformation. */
	tt::math::Vector2 getCustomScale      () const;
	/*! \brief Get rotaiton in custom transformation. */
	real              getCustomRotation   () const;
	
	/*! \brief Flips the presentation in the X direction. */
	void              flipX();
	
	/*! \brief Flips the presentation in the Y direction. */
	void              flipY();
	
	/*! \brief Get the position of the presentation object in world coordinates */
	tt::math::Vector2 getWorldPosition() const;
	
	/*! \brief Returns the layer (particle layer) of this presentation */
	ParticleLayer getLayer() const;
	
	/*! \brief Returns the handle value of this presentationobject. */
	inline s32 getHandleValue() const { return static_cast<s32>(m_handle.getValue()); }
	
	/*! \brief Check whether two presentation objects are the same instance (use this instead of '==' or '!='). */
	inline bool equals(const PresentationObjectWrapper* p_rhs) const
	{
		return p_rhs != 0 && m_handle == p_rhs->m_handle;
	}
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	pres::PresentationObjectHandle& getHandle() { return m_handle; }
	const pres::PresentationObjectHandle& getHandle() const { return m_handle; }
	
private:
#if !defined(TT_BUILD_FINAL)
	// Debug helper to easily get the names of the specified set of tags, sorted alphabetically.
	// (only available in non-final, because the original string of a Hash is only available in non-final)
	static tt::str::Strings getSortedTagNames(const tt::pres::Tags& p_tags);
#else
	static inline tt::str::Strings getSortedTagNames(const tt::pres::Tags&) { return tt::str::Strings(); }
#endif
	
	inline const pres::PresentationObject* getPresentationObject() const
	{
		return m_handle.getPtr();
	}
	
	inline pres::PresentationObject* getModifiablePresentationObject()
	{
		return m_handle.getPtr();
	}
	
	pres::PresentationObjectHandle m_handle;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONOBJECTWRAPPER_H)
