#include <tt/code/bufferutils.h>
#include <tt/math/Vector3.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/script/helpers.h>


#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/PresentationObjectWrapper.h>
#include <toki/game/script/wrappers/PresentationStartSettingsWrapper.h>
#include <toki/game/script/wrappers/TextLabelWrapper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/pres/PresentationObject.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

PresentationObjectWrapper::PresentationObjectWrapper(const pres::PresentationObjectHandle& p_handle)
:
m_handle(p_handle)
{
}


void PresentationObjectWrapper::start(const std::string& p_name, const tt::str::Strings& p_tagsToStart,
                                      bool p_hideAtEnd, s32 p_priority)
{
	startEx(p_name, p_tagsToStart, p_hideAtEnd, p_priority, PresentationStartSettingsWrapper());
}


void PresentationObjectWrapper::startEx(const std::string& p_name, const tt::str::Strings& p_tagsToStart,
                                        bool p_hideAtEnd, s32 p_priority,
                                        const PresentationStartSettingsWrapper& p_settings)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		tt::pres::Tags startTags;
		for (tt::str::Strings::const_iterator it = p_tagsToStart.begin(); it != p_tagsToStart.end(); ++it)
		{
			startTags.insert(tt::pres::Tag(*it));
		}
		pres->startEx(p_name, startTags, p_hideAtEnd, pres::StartType_Script, 
		              p_priority, p_settings.getSettings());
	}
}


void PresentationObjectWrapper::stop()
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->stop();
	}
}


bool PresentationObjectWrapper::isActive()
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0 && pres->hasPresentationObject())
	{
		return pres->getPresentationObject()->isActive();
	}
	return false;
}


void PresentationObjectWrapper::setVisible(bool p_visible)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setVisible(p_visible);
	}
}


bool PresentationObjectWrapper::isVisible()
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->isVisible();
	}
	return false;
}


void PresentationObjectWrapper::addTag(const std::string& p_tagName)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->addTag(p_tagName);
	}
}


bool PresentationObjectWrapper::hasTag(const std::string& p_tagName)
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->hasTag(p_tagName);
	}
	return false;
}


void PresentationObjectWrapper::removeTag(const std::string& p_tagName)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->removeTag(p_tagName);
	}
}


void PresentationObjectWrapper::clearTags()
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->clearTags();
	}
}


void PresentationObjectWrapper::addTextLabel(TextLabelWrapper* p_textLabel, const tt::math::Vector2& p_offset)
{
	if (p_textLabel == 0)
	{
		return;
	}
	
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->addTextLabel(p_textLabel->getHandle(), p_offset);
	}
}


void PresentationObjectWrapper::removeTextLabel(TextLabelWrapper* p_textLabel)
{
	if (p_textLabel == 0)
	{
		return;
	}
	
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->removeTextLabel(p_textLabel->getHandle());
	}
}


std::string PresentationObjectWrapper::getCurrentAnimationName() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->getCurrentAnimationName();
	}
	return std::string();
}


void PresentationObjectWrapper::addCustomValue(const std::string& p_valueName, real p_value)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->addCustomValue(p_valueName, p_value);
	}
}


s32 PresentationObjectWrapper::getPriority() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->getPriority();
	}
	
	return false;
}


void PresentationObjectWrapper::setAffectedByMovement(bool p_affected)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setAffectedByMovement(p_affected);
	}
}


bool PresentationObjectWrapper::isAffectedByMovement() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->isAffectedByMovement();
	}
	
	return false;
}


void PresentationObjectWrapper::setAffectedByOrientation(bool p_affected)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setAffectedByOrientation(p_affected);
	}
}


bool PresentationObjectWrapper::isAffectedByOrientation() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->isAffectedByOrientation();
	}
	
	return false;
}


void PresentationObjectWrapper::setAffectedByFloorDirection(bool p_affected)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setAffectedByFloorDirection(p_affected);
	}
}


bool PresentationObjectWrapper::isAffectedByFloorDirection() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->isAffectedByFloorDirection();
	}
	
	return false;
}


void PresentationObjectWrapper::setIsFollowingParent(bool p_isFollowingParent)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setIsFollowingParent(p_isFollowingParent);
	}
}


bool PresentationObjectWrapper::isFollowingParent() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->isFollowingParent();
	}
	
	return false;
}


void PresentationObjectWrapper::setRotateToEntity(const EntityWrapper* p_entity)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setRotateToEntity(p_entity != 0 ? p_entity->getHandle(): entity::EntityHandle());
	}
	
	return;
}


void PresentationObjectWrapper::setCustomTranslation(const tt::math::Vector2& p_translation)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		const tt::math::Vector3& currentTranslation(pres->getCustomTranslation());
		pres->setCustomTranslation(tt::math::Vector3(p_translation, currentTranslation.z));
	}
	
	return;
}


void PresentationObjectWrapper::setCustomZOffset(real p_zOffset)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		const tt::math::Vector3& currentTranslation(pres->getCustomTranslation());
		pres->setCustomTranslation(tt::math::Vector3(currentTranslation.x, currentTranslation.y, p_zOffset));
	}
	
	return;
}


void PresentationObjectWrapper::setCustomScale(const tt::math::Vector2& p_scale)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setCustomScale(tt::math::Vector3(p_scale, 1.0f));
	}
	
	return;
}


void PresentationObjectWrapper::setCustomUniformScale(real p_scale)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setCustomUniformScale(p_scale);
	}
	
	return;
}


void PresentationObjectWrapper::setCustomRotation(real p_rotation)
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->setCustomRotation(tt::math::degToRad(p_rotation));
	}
	
	return;
}


void PresentationObjectWrapper::resetCustomTransformation()
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		pres->resetCustomTransformation();
	}
	
	return;
}


tt::math::Vector2 PresentationObjectWrapper::getCustomTranslation() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->getCustomTranslation().xy();
	}
	
	return tt::math::Vector2::zero;
}


tt::math::Vector2 PresentationObjectWrapper::getCustomScale() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->getCustomScale().xy();
	}
	
	return tt::math::Vector2::allOne;
}


real PresentationObjectWrapper::getCustomRotation() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return tt::math::radToDeg(pres->getCustomRotation());
	}
	
	return 0.0f;
}


void PresentationObjectWrapper::flipX()
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		// Toggle X-flip bit
		pres->setFlipMask(pres->getFlipMask() ^ tt::pres::PresentationObject::FlipMask_Horizontal);
	}
}


void PresentationObjectWrapper::flipY()
{
	pres::PresentationObject* pres = getModifiablePresentationObject();
	if (pres != 0)
	{
		// Toggle X-flip bit
		pres->setFlipMask(pres->getFlipMask() ^ tt::pres::PresentationObject::FlipMask_Vertical);
	}
}


tt::math::Vector2 PresentationObjectWrapper::getWorldPosition() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		const tt::pres::ConstPresentationObjectPtr presObj = pres->getPresentationObject();
		
		if (presObj != 0)
		{
			return presObj->getWorldPosition().xy();
		}
	}
	
	// No valid presentation object found
	return tt::math::Vector2(-1000.0f, -1000.0f);
}


ParticleLayer PresentationObjectWrapper::getLayer() const
{
	const pres::PresentationObject* pres = getPresentationObject();
	if (pres != 0)
	{
		return pres->getLayer();
	}
	
	return ParticleLayer_Invalid;
}


void PresentationObjectWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	bool wrapperValid = (getPresentationObject() != 0);
	
	bu::put(wrapperValid, p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to save
		return;
	}
	
	bu::putHandle(m_handle, p_context); // handle
}


void PresentationObjectWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	// Unserialize members
	namespace bu = tt::code::bufferutils;
	
	const bool wrapperValid = bu::get<bool>(p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to load
		return;
	}
	
	m_handle = bu::getHandle<pres::PresentationObject>(p_context);
}


void PresentationObjectWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(PresentationObjectWrapper, "PresentationObject");
	TT_SQBIND_METHOD(PresentationObjectWrapper, start);
	TT_SQBIND_METHOD(PresentationObjectWrapper, startEx);
	TT_SQBIND_METHOD(PresentationObjectWrapper, stop);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isActive);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setVisible);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isVisible);
	TT_SQBIND_METHOD(PresentationObjectWrapper, addTag);
	TT_SQBIND_METHOD(PresentationObjectWrapper, hasTag);
	TT_SQBIND_METHOD(PresentationObjectWrapper, removeTag);
	TT_SQBIND_METHOD(PresentationObjectWrapper, clearTags);
	TT_SQBIND_METHOD(PresentationObjectWrapper, addTextLabel);
	TT_SQBIND_METHOD(PresentationObjectWrapper, removeTextLabel);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getCurrentAnimationName);
	TT_SQBIND_METHOD(PresentationObjectWrapper, addCustomValue);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getPriority);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setAffectedByMovement);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isAffectedByMovement);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setAffectedByOrientation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isAffectedByOrientation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setAffectedByFloorDirection);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isAffectedByFloorDirection);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setIsFollowingParent);
	TT_SQBIND_METHOD(PresentationObjectWrapper, isFollowingParent);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setRotateToEntity);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setCustomTranslation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setCustomZOffset);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setCustomScale);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setCustomUniformScale);
	TT_SQBIND_METHOD(PresentationObjectWrapper, setCustomRotation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, resetCustomTransformation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getCustomTranslation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getCustomScale);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getCustomRotation);
	TT_SQBIND_METHOD(PresentationObjectWrapper, flipX);
	TT_SQBIND_METHOD(PresentationObjectWrapper, flipY);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getWorldPosition);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getLayer);
	TT_SQBIND_METHOD(PresentationObjectWrapper, getHandleValue);
	TT_SQBIND_METHOD(PresentationObjectWrapper, equals);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

#if !defined(TT_BUILD_FINAL)

tt::str::Strings PresentationObjectWrapper::getSortedTagNames(const tt::pres::Tags& p_tags)
{
	tt::str::StringSet tagNames;
	for (tt::pres::Tags::const_iterator it = p_tags.begin(); it != p_tags.end(); ++it)
	{
		tagNames.insert((*it).getName());
	}
	
	return tt::str::Strings(tagNames.begin(), tagNames.end());
}

#endif

// Namespace end
}
}
}
}
