#if !defined(INC_TOKITORI_PRES_PRESENTATIONOBJECT_H)
#define INC_TOKITORI_PRES_PRESENTATIONOBJECT_H

#include <string>
#include <vector>
#include <utility>

#include <tt/code/fwd.h>
#include <tt/pres/fwd.h>
#include <tt/pres/PresentationObject.h>
#include <tt/str/str.h>

#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/types.h>
#include <toki/pres/fwd.h>
#include <toki/serialization/utils.h>

namespace toki {
namespace pres {

class PresentationStartSettings
{
public:
	PresentationStartSettings()
	:
	m_enableEndPos(false),
	m_endPos(tt::math::Vector2::zero),
	m_endCallbackName()
	{
	}
	
	inline void setEndPos(const tt::math::Vector2& p_endPos)
	{
		m_enableEndPos = true;
		m_endPos = p_endPos;
	}
	
	inline void setEndCallbackName(const std::string& p_name) { m_endCallbackName = p_name; }
	
	bool                     isEndPosEnabled()    const { return m_enableEndPos;    }
	const tt::math::Vector2& getEndPos()          const { return m_endPos;          }
	const std::string&       getEndCallbackName() const { return m_endCallbackName; }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	bool              m_enableEndPos;
	tt::math::Vector2 m_endPos;
	std::string       m_endCallbackName;
};


class PresentationObject
{
public:
	class CallbackTrigger : public tt::pres::CallbackTriggerInterface
	{
	public:
		inline CallbackTrigger(const PresentationObjectHandle& p_presentation)
		:
		m_presentation(p_presentation)
		{ }
		
		virtual ~CallbackTrigger() { }
		
		// Presentation callbacks
		virtual void callback(const std::string& p_data, const tt::pres::PresentationObjectPtr& p_object);
		
	private:
		PresentationObjectHandle m_presentation;
	};
	
	struct CreationParams
	{
		inline CreationParams(const game::entity::EntityHandle& p_parent,
		                      const serialization::PresentationRestoreInfo& p_restoreInfo)
		:
		parent(p_parent),
		restoreInfo(p_restoreInfo)
		{ }
		
		game::entity::EntityHandle             parent;
		serialization::PresentationRestoreInfo restoreInfo;
	};
	
	struct TextLabel
	{
		inline TextLabel()
		:
		handle(),
		offset(tt::math::Vector2::zero)
		{ }
		
		inline TextLabel(const game::entity::graphics::TextLabelHandle& p_handle,
		                 const tt::math::Vector2&                       p_offset)
		:
		handle(p_handle),
		offset(p_offset)
		{ }
		
		game::entity::graphics::TextLabelHandle handle;
		tt::math::Vector2                       offset;
	};
	typedef const CreationParams& ConstructorParamType;
	typedef std::vector<TextLabel> TextLabels;
	
	PresentationObject(const CreationParams& p_creationParams, const PresentationObjectHandle& p_ownHandle);
	~PresentationObject();
	
	void start(const std::string& p_name, const tt::pres::Tags& p_tagsToStart = tt::pres::Tags(),
	           bool p_hideAtEnd = true, StartType p_startedFrom = StartType_Normal, s32 priority = 0);
	void startEx(const std::string& p_name, const tt::pres::Tags& p_tagsToStart,
	             bool p_hideAtEnd, StartType p_startedFrom, s32 p_priority,
	             const PresentationStartSettings& p_settings);
	void stop();
	void checkAnimationEnded();
	
	void addTag(const std::string& p_tagName);
	bool hasTag(const std::string& p_tagName) const;
	void removeTag(const std::string& p_tagName);
	void clearTags();
	
	void addTextLabel(const game::entity::graphics::TextLabelHandle& p_handle,
	                  const tt::math::Vector2&                       p_offset);
	
	void removeTextLabel(const game::entity::graphics::TextLabelHandle& p_handle);
	
	inline const std::string& getCurrentAnimationName() const { return m_pres->getCurrentActiveName(); }
	
	void update(real p_deltaTime);
	
	inline s32 getPriority() const { return m_priority; }
	
	inline void setAffectedByMovement(bool p_affected) { m_isAffectedByMovement = p_affected; }
	inline bool isAffectedByMovement() const { return m_isAffectedByMovement; }
	
	void setAffectedByOrientation(bool p_affected);
	inline bool isAffectedByOrientation() const { return m_isAffectedByOrientation; }
	
	void setAffectedByFloorDirection(bool p_affected);
	inline bool isAffectedByFloorDirection() const { return m_isAffectedByFloorDirection; }
	
	inline void setIsFollowingParent(bool p_isFollowingParent) { m_isFollowingParent = p_isFollowingParent; }
	inline bool isFollowingParent() const { return m_isFollowingParent; }
	
	void addCustomValue(const std::string& p_valueName, real p_value);
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	void unserializeState(tt::code::BufferReadContext* p_context);
	void serializeState(tt::code::BufferWriteContext* p_context) const;
	
	inline const PresentationObjectHandle& getHandle() const { return m_handle; }
	static PresentationObject* getPointerFromHandle(const PresentationObjectHandle& p_handle);
	void invalidateTempCopy() {}
	
	bool hasPresentationObject() const { return m_pres != 0; }
	tt::pres::ConstPresentationObjectPtr getPresentationObject() const { return m_pres; }
	
	void updatePosition(const tt::math::Vector2 p_pos);
	void updateOrientation(tt::pres::PresentationObject::FlipMask p_flipMask,
	                       const tt::math::Quaternion&            p_rotation);
	
	void setRotateToEntity(const game::entity::EntityHandle& p_entity) { m_rotateTo = p_entity; }
	
	inline void setCustomTranslation(const tt::math::Vector3& p_translation) { m_useCustomTransform = true; m_customTranslation = p_translation;                }
	inline void setCustomScale      (const tt::math::Vector3& p_scale      ) { m_useCustomTransform = true; m_customScale       = p_scale;                      }
	inline void setCustomUniformScale(real p_scale)                          { m_useCustomTransform = true; m_customScale.setValues(p_scale, p_scale, p_scale); }
	inline void setCustomRotation(    real p_rotation)                       { m_useCustomTransform = true; m_customRotation    = p_rotation;                   }
	void resetCustomTransformation();
	
	u32  getFlipMask() const;
	void setFlipMask(u32 p_mask);
	
	inline const tt::math::Vector3& getCustomTranslation() const { return m_customTranslation; }
	inline const tt::math::Vector3& getCustomScale      () const { return m_customScale;       }
	inline real                     getCustomRotation   () const { return m_customRotation;    }
	
	void setIsCulled(bool p_isCulled);
	bool isCulled() const;
	
	void setVisible(bool p_visible);
	bool isVisible() const;
	
#if !defined(TT_BUILD_FINAL)
	std::string getParentType() const;
#else
	inline std::string getParentType() const { return std::string(); }
#endif
	
	// FIXME: It's a bit weird that this info comes from the restore struct
	inline game::ParticleLayer getLayer() const { return m_restoreInfo.layer; }
	
private:
	void restartDefaultAnimation();
	
#if !defined(TT_BUILD_FINAL)
	// Debug helper to easily get the names of the specified set of tags, sorted alphabetically.
	// (only available in non-final, because the original string of a Hash is only available in non-final)
	static tt::str::Strings getSortedTagNames(const tt::pres::Tags& p_tags);
#else
	static inline tt::str::Strings getSortedTagNames(const tt::pres::Tags&) { return tt::str::Strings(); }
#endif
	
	
	PresentationObjectHandle               m_handle;
	game::entity::EntityHandle             m_parent;
	tt::pres::PresentationObjectPtr        m_pres;
	real                                   m_presTime;
	serialization::PresentationRestoreInfo m_restoreInfo;
	tt::pres::Tags                         m_tags;
	StartType                              m_startedFrom;
	s32                                    m_priority;
	std::string                            m_endCallbackName;
	bool                                   m_isAffectedByMovement;
	bool                                   m_isAffectedByOrientation;
	bool                                   m_isAffectedByFloorDirection;
	bool                                   m_isFollowingParent;
	game::entity::EntityHandle             m_rotateTo;
	
	bool                                   m_useCustomTransform;
	tt::math::Vector3                      m_customTranslation;
	tt::math::Vector3                      m_customScale;
	real                                   m_customRotation;
	
	TextLabels                             m_textLabels;
};


// Namespace end
}
}

#endif // !defined(INC_TOKITORI_PRES_PRESENTATIONOBJECT_H)
