#include <tt/code/bufferutils.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>
#include <tt/script/helpers.h>

#include <toki/game/entity/graphics/TextLabel.h>
#include <toki/game/movement/MoveBase.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/Game.h>
#include <toki/game/Minimap.h>
#include <toki/pres/PresentationObject.h>
#include <toki/pres/PresentationObjectMgr.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// PresentationStartSettings public member functions

void PresentationStartSettings::serialize(tt::code::BufferWriteContext* p_context) const
{
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_enableEndPos,    p_context); // bool
	bu::put(m_endPos,          p_context); // Vector2
	bu::put(m_endCallbackName, p_context); // std::string
}


void PresentationStartSettings::unserialize(tt::code::BufferReadContext* p_context)
{
	// Unserialize members
	namespace bu = tt::code::bufferutils;
	
	m_enableEndPos    = bu::get<bool             >(p_context);
	m_endPos          = bu::get<tt::math::Vector2>(p_context);
	m_endCallbackName = bu::get<std::string      >(p_context);
}


//--------------------------------------------------------------------------------------------------
// CallbackTrigger member functions

void PresentationObject::CallbackTrigger::callback(const std::string& p_data,
                                                   const tt::pres::PresentationObjectPtr& p_object)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	const PresentationObject* pres = m_presentation.getPtr();
	if (pres == 0)
	{
		return;
	}
	
	TT_ASSERTMSG(pres->getPresentationObject() == p_object,
	             "Got callback from mismatching presentation objects.");
	
	const game::entity::Entity* entity = pres->m_parent.getPtr();
	if (entity != 0)
	{
		const game::script::EntityBasePtr& script(entity->getEntityScript());
		
		if(script != 0)
		{
			script->onPresentationObjectCallback(
				game::script::wrappers::PresentationObjectWrapper(m_presentation),
				p_data);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions


PresentationObject::PresentationObject(const CreationParams& p_creationParams,
                                       const PresentationObjectHandle& p_ownHandle)
:
m_handle(p_ownHandle),
m_parent(p_creationParams.parent),
m_pres(),
m_presTime(0.0f),
m_restoreInfo(p_creationParams.restoreInfo),
m_tags(),
m_startedFrom(StartType_Normal),
m_priority(0),
m_endCallbackName(),
m_isAffectedByMovement(false),
m_isAffectedByOrientation(true),
m_isAffectedByFloorDirection(true),
m_isFollowingParent(true),
m_rotateTo(),
m_useCustomTransform(false),
m_customTranslation(0.0f, 0.0f, 0.0f),
m_customScale(1.0f, 1.0f, 1.0f),
m_customRotation(0.0f)
{
	game::Game* game = AppGlobal::getGame();
	TT_NULL_ASSERT(game);
	
	m_pres = game->getPresentationMgr(m_restoreInfo.layer)->createPresentationObject(
		m_restoreInfo.filename, m_restoreInfo.requiredTags);
	
	m_pres->setCallbackInterface(
		tt::pres::CallbackTriggerInterfacePtr(new pres::PresentationObject::CallbackTrigger(m_handle)));
	
	// FIXME: Hack for the overlay quads so that they are scaled correctly when presentation is in screenspace
	m_pres->setInScreenSpace(
		(m_restoreInfo.layer == game::ParticleLayer_Hud) ||
		(m_restoreInfo.layer == game::ParticleLayer_HudDrcOnly) ||
		(m_restoreInfo.layer == game::ParticleLayer_HudTvOnly));
	
	const game::entity::Entity* parent = m_parent.getPtr();
	TT_NULL_ASSERT(parent);
	if (parent == 0)
	{
		return;
	}
	
	// Update PRS
	const tt::math::Vector2 pos(parent->getCenterPosition());
	updatePosition(pos);
	m_pres->setRotation(game::entity::getOrientationQuaternion(parent->getOrientationDown()));
	m_pres->setFlipMask(parent->isOrientationForwardLeft() ?
		tt::pres::PresentationObject::FlipMask_Horizontal : tt::pres::PresentationObject::FlipMask_None);
	
	// Apply changes.
	m_pres->update(0.0f);
}


PresentationObject::~PresentationObject()
{
	TT_NULL_ASSERT(m_pres);
	m_pres->stop();
}


void PresentationObject::start(const std::string& p_name, const tt::pres::Tags& p_tagsToStart,
                               bool p_hideAtEnd, StartType p_startedFrom, s32 p_priority)
{
	startEx(p_name, p_tagsToStart, p_hideAtEnd, p_startedFrom, p_priority, PresentationStartSettings());
}


void PresentationObject::startEx(const std::string& p_name, const tt::pres::Tags& p_tagsToStart,
                                 bool p_hideAtEnd, StartType p_startedFrom, s32 p_priority,
                                 const PresentationStartSettings& p_settings)
{
	TT_NULL_ASSERT(m_pres);
	
	update(0.0);
	
	// HACK: Restore part of the old blocking by pass logic for script calls.
	//       In the future Hessel would like use to remove this and properly fix the scripts.
	//       Script may override a priority 1 animation, even if has priority 0.
	const bool byPassPrioCheck = (p_startedFrom == StartType_Script && m_priority == 1);
	
	const game::entity::Entity* parent = m_parent.getPtr();
#if !defined(TT_BUILD_FINAL)
	const bool doDebugPrints = (parent != 0 && parent->shouldShowPresentationTags());
#endif
	
	if (m_priority > p_priority && byPassPrioCheck == false) // Don't start new animation when current priority higher.
	{
#if !defined(TT_BUILD_FINAL)
		if (doDebugPrints)
		{
			tt::pres::Tags tags(p_tagsToStart);
			tags.insert(m_tags.begin(), m_tags.end());
			
			TT_Printf("[%u] PresentationObject::start: [0x%08X, '%s']\n"
			           "\t!NOT STARTED! current priority %d > %d !!\n"
			          "\tStarting name [%s] with tags [%s].\n"
			          "\t(hideAtEnd: %d, startedFrom: %d, priority:%d.)\n"
			          "\t(PresStartSettings - EndPosEnabled: %d, EndPos: (%f, %f),\n"
			          "\tEndCallbackName: [%s]\n",
			          AppGlobal::getUpdateFrameCount(), this, getParentType().c_str(), 
			          m_priority, p_priority,
			          p_name.c_str(),
			          tags.empty() ? "" : ("'" + tt::str::implode(getSortedTagNames(tags), "', '") + "'").c_str(),
			          p_hideAtEnd, p_startedFrom, p_priority,
			          p_settings.isEndPosEnabled(), p_settings.getEndPos().x, p_settings.getEndPos().y,
			          p_settings.getEndCallbackName().c_str());
		}
#endif
		return;
	}
	
	m_startedFrom = p_startedFrom;
	m_priority    = p_priority;
	
	if (parent != 0 && m_endCallbackName.empty() == false)
	{
		const game::script::EntityBasePtr& script(parent->getEntityScript());
		if (script == 0)
		{
			TT_PANIC("Entity script from parent is 0");
			return;
		}
		
		// cancel callback with m_endCallbackName.
		script->onPresentationObjectCanceled(
			game::script::wrappers::PresentationObjectWrapper(m_handle),
			m_endCallbackName);
	}
	m_endCallbackName = p_settings.getEndCallbackName();
	
	tt::pres::Tags tags(p_tagsToStart);
	tags.insert(m_tags.begin(), m_tags.end());
	
#if !defined(TT_BUILD_FINAL)
	if (doDebugPrints)
	{
		TT_Printf("[%u] PresentationObject::start: [0x%08X, '%s']\n"
		          "\tStarting name [%s] with tags [%s].\n"
		          "\t(hideAtEnd: %d, startedFrom: %d, priority:%d.)\n"
		          "\t(PresStartSettings - EndPosEnabled: %d, EndPos: (%f, %f),\n"
		          "\tEndCallbackName: [%s]\n",
		          AppGlobal::getUpdateFrameCount(), this, getParentType().c_str(), p_name.c_str(),
		          tags.empty() ? "" : ("'" + tt::str::implode(getSortedTagNames(tags), "', '") + "'").c_str(),
		          p_hideAtEnd, p_startedFrom, p_priority,
		          p_settings.isEndPosEnabled(), p_settings.getEndPos().x, p_settings.getEndPos().y,
		          p_settings.getEndCallbackName().c_str());
	}
#endif
	
	//bool startedNewAnimation = false;
	if (p_settings.isEndPosEnabled())
	{
		m_pres->setEndPosition(tt::math::Vector3(p_settings.getEndPos().x, p_settings.getEndPos().y, 0.0f));
		m_pres->start(tags, p_hideAtEnd, p_name);
		//startedNewAnimation = true;
		m_presTime = 0.0f;
	}
	else if ((m_pres->isActive() == false && m_pres->isLooping() == false) ||
	         m_pres->getCurrentActiveName() != p_name || 
	         m_pres->getCurrentActiveTags() != tags   || // Only start the presentation when changed.
	         m_pres->isHidingAtEnd()        != p_hideAtEnd)
	{
		m_pres->start(tags, p_hideAtEnd, p_name);
		//startedNewAnimation = true;
		m_presTime = 0.0f;
	}
	
	/*
	// Martijn: disabled this check to allow for empty presentations without asserts
	TT_ASSERTMSG((m_pres->isActive() || m_pres->isLooping()),
	             "No valid presentation animation found! %s name [%s] with tags [%s].\n"
	             "(Game update frame: %u. pres obj isActive: %d, isLooping: %d, isMissingFrame: %d.)\n"
	             "[0x%08X, '%s'] (hideAtEnd: %d, startedFrom: %d, priority: %d.)\n"
	             "(PresStartSettings - EndPosEnabled: %d, EndPos: (%f, %f), EndCallbackName: [%s]\n"
	             "From File: '%s' (Entity Type: '%s')\n",
	             ((startedNewAnimation) ? "Starting " : "Didn't start "), 
	             p_name.c_str(),
	             tags.empty() ? "" : ("'" + tt::str::implode(getSortedTagNames(tags), "', '") + "'").c_str(),
	             AppGlobal::getUpdateFrameCount(), m_pres->isActive(), m_pres->isLooping(), 
	             m_pres->isMissingFrame(), this, getParentType().c_str(),
	             p_hideAtEnd, p_startedFrom, p_priority,
	             p_settings.isEndPosEnabled(), p_settings.getEndPos().x, p_settings.getEndPos().y,
	             p_settings.getEndCallbackName().c_str(),
	             m_restoreInfo.filename.c_str(), getParentType().c_str());
	// */
	
#if !defined(TT_BUILD_FINAL)
	if (m_pres->isMissingFrame() && doDebugPrints)
	{
		TT_WARN("Presentation is not visible after starting new animation");
	}
#endif
}


void PresentationObject::stop()
{
	const game::entity::Entity* parent = m_parent.getPtr();
#if !defined(TT_BUILD_FINAL)
	if (parent != 0 && parent->shouldShowPresentationTags())
	{
		TT_Printf("[%u] PresentationObject::stopPresentationAnimation: [0x%08X, '%s']\n",
			        AppGlobal::getUpdateFrameCount(), this, getParentType().c_str());
	}
#endif
	
	TT_NULL_ASSERT(m_pres);
	
	m_priority = 0;
	m_startedFrom = StartType_Normal;
	m_pres->stop();
	
	if (parent != 0 && parent->isInitialized())
	{
		restartDefaultAnimation();
	}
}


void PresentationObject::checkAnimationEnded()
{
	// Resume movementset animation after blocking animation started from script is finished
	if (m_startedFrom == StartType_Normal)
	{
		return;
	}
	
	if (m_pres->isLooping() == false && m_pres->isActive()  == false)
	{
		const bool startedFromScript = (m_startedFrom == StartType_Script);
		
		m_startedFrom = StartType_Normal;
		m_priority    = 0;
		
		const game::entity::Entity* parent = m_parent.getPtr();
		if (parent == 0)
		{
			return;
		}
		
		if (startedFromScript)
		{
			const game::script::EntityBasePtr& script(parent->getEntityScript());
			TT_NULL_ASSERT(script);
			
			if (m_endCallbackName.empty() == false)
			{
				script->onPresentationObjectEnded(
					game::script::wrappers::PresentationObjectWrapper(m_handle),
					m_endCallbackName);
				m_endCallbackName.clear();
			}
		}
		
		if (parent->isSuspended() == false)
		{
			restartDefaultAnimation();
		}
	}
}


void PresentationObject::addTag(const std::string& p_tagName)
{
	m_tags.insert(tt::pres::Tag(p_tagName));
	
#if !defined(TT_BUILD_FINAL)
	const game::entity::Entity* parent = m_parent.getPtr();
	if (parent != 0 && parent->shouldShowPresentationTags())
	{
		TT_Printf("[%u] Entity::addPresentationAnimationTag: [0x%08X, '%s'] Added tag '%s'. Current set of entity tags: [%s]\n",
		          AppGlobal::getUpdateFrameCount(), this, getParentType().c_str(),
		          p_tagName.c_str(),
		          m_tags.empty() ? "" : ("'" + tt::str::implode(getSortedTagNames(m_tags), "', '") + "'").c_str());
	}
#endif
}


bool PresentationObject::hasTag(const std::string& p_tagName) const
{
	return m_tags.find(tt::pres::Tag(p_tagName)) != m_tags.end();
}


void PresentationObject::removeTag(const std::string& p_tagName)
{
	m_tags.erase(tt::pres::Tag(p_tagName));
	
#if !defined(TT_BUILD_FINAL)
	const game::entity::Entity* parent = m_parent.getPtr();
	if (parent != 0 && parent->shouldShowPresentationTags())
	{
		TT_Printf("[%u] PresentationObject::removePresentationAnimationTag: [0x%08X, '%s']\n"
		          "\tRemoved tag '%s'. Current set of entity tags: [%s]\n",
		          AppGlobal::getUpdateFrameCount(), this, getParentType().c_str(),
		          p_tagName.c_str(),
		          m_tags.empty() ? "" : ("'" + tt::str::implode(getSortedTagNames(m_tags), "', '") + "'").c_str());
	}
#endif
}


void PresentationObject::clearTags()
{
	m_tags.clear();
		
#if !defined(TT_BUILD_FINAL)
	const game::entity::Entity* parent = m_parent.getPtr();
	if (parent != 0 && parent->shouldShowPresentationTags())
	{
		TT_Printf("[%u] PresentationObject::clearPresentationAnimationTags: [0x%08X, '%s'] Removed all presentation tags.\n",
		          AppGlobal::getUpdateFrameCount(), this, getParentType().c_str());
	}
#endif
}


void PresentationObject::addTextLabel(const game::entity::graphics::TextLabelHandle& p_handle,
                                      const tt::math::Vector2&                       p_offset)
{
	game::entity::graphics::TextLabel* label = p_handle.getPtr();
	if (label == 0)
	{
		TT_PANIC("Can't set TextLabel as overlay because text label is null!");
		return;
	}
	
	const tt::engine::renderer::TexturePtr& ptr = label->getTexture();
	if (ptr == 0)
	{
		TT_PANIC("Can't set TextLabel as overlay because texture is null!");
		return;
	}
	if (label->hasDropShadow())
	{
		m_pres->addOverlay(ptr, tt::math::Vector3(p_offset + label->getScaledDropShadowOffset()), label->getDropShadowColor());
	}
	m_pres->addOverlay(ptr, tt::math::Vector3(p_offset), label->getColor());
	label->incRefPresentationOverlay();
	m_textLabels.emplace_back(p_handle, p_offset);
}


void PresentationObject::removeTextLabel(const game::entity::graphics::TextLabelHandle& p_handle)
{
	// First remove entry from m_textLabels
	// FIXME: Replace this with a more fancy std::find call
	for (TextLabels::iterator it = m_textLabels.begin(); it != m_textLabels.end(); ++it)
	{
		if ((*it).handle == p_handle)
		{
			m_textLabels.erase(it);
			break;
		}
	}
	
	// Now remove the overlay texture from the presentation object
	game::entity::graphics::TextLabel* label = p_handle.getPtr();
	if (label == 0)
	{
		TT_PANIC("Can't remove overlay because text label is null!");
		return;
	}
	label->decRefPresentationOverlay();
	
	const tt::engine::renderer::TexturePtr& ptr = label->getTexture();
	if (ptr == 0)
	{
		TT_PANIC("Can't remove overlay because texture is null!");
		return;
	}
	m_pres->removeOverlay(ptr);
}


void PresentationObject::update(real p_deltaTime)
{
	const real fixedTimestep = m_pres->getFixedTimestep();
	if (p_deltaTime >= 0.0f && fixedTimestep > 0.0f)
	{
		p_deltaTime = fixedTimestep;
	}
	
	m_presTime += p_deltaTime;
	
	real toEntityRotation = 0.0f;
	
	if (m_rotateTo.isEmpty() == false)
	{
		using namespace game::entity;
		using namespace tt::math;
		
		const Entity* rotateTo = m_rotateTo.getPtr();
		if (rotateTo != 0)
		{
			const Vector3& presentationPosition = m_pres->getPositionClean();
			const Vector2  rotateToPos(rotateTo->getCenterPosition());
			// Get angle between the two points.
			toEntityRotation = atan2f(rotateToPos.y - presentationPosition.y,
			                          rotateToPos.x - presentationPosition.x);
		}
		else
		{
			m_rotateTo.invalidate();
		}
	}
	
	tt::pres::anim2d::Transform& transform = m_pres->modifyCustomTransform();
	
	if (m_useCustomTransform)
	{
		transform.translation = m_customTranslation;
		transform.rotation    = m_customRotation + toEntityRotation;
		transform.scale       = m_customScale;
	}
	else
	{
		transform.rotation = toEntityRotation;
	}
}


void PresentationObject::setAffectedByOrientation(bool p_affected)
{
	if (m_isAffectedByOrientation != p_affected)
	{
		m_isAffectedByOrientation = p_affected;
		
		if (m_isAffectedByOrientation)
		{
			const game::entity::Entity* parent = m_parent.getPtr();
			TT_NULL_ASSERT(parent);
			if (parent == 0)
			{
				return;
			}
			m_pres->setFlipMask(parent->isOrientationForwardLeft() ?
				tt::pres::PresentationObject::FlipMask_Horizontal : tt::pres::PresentationObject::FlipMask_None);
		}
		else
		{
			m_pres->setFlipMask(tt::pres::PresentationObject::FlipMask_None);
		}
		m_pres->update(0.0f);
	}
}


void PresentationObject::setAffectedByFloorDirection(bool p_affected)
{
	if (m_isAffectedByFloorDirection != p_affected)
	{
		m_isAffectedByFloorDirection = p_affected;
		
		if (m_isAffectedByFloorDirection)
		{
			const game::entity::Entity* parent = m_parent.getPtr();
			TT_NULL_ASSERT(parent);
			if (parent == 0)
			{
				return;
			}
			m_pres->setRotation(game::entity::getOrientationQuaternion(parent->getOrientationDown()));
		}
		else
		{
			m_pres->setRotation(tt::math::Quaternion::identity);
		}
		m_pres->update(0.0f);
	}
}


void PresentationObject::addCustomValue(const std::string& p_valueName, real p_value)
{
	TT_NULL_ASSERT(m_pres);
	m_pres->addCustomPresentationValue(p_valueName, p_value);
}


void PresentationObject::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_parent, p_context);
	serialization::serializePresentationRestoreInfo(m_restoreInfo, p_context);
}


PresentationObject::CreationParams PresentationObject::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	game::entity::EntityHandle parent = bu::getHandle<game::entity::Entity>(p_context);
	
	serialization::PresentationRestoreInfo restoreInfo =
		serialization::unserializePresentationRestoreInfo(p_context);
	
	return CreationParams(parent, restoreInfo);
}


void PresentationObject::serialize(tt::code::BufferWriteContext* p_context) const
{
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	serialization::serializePresTags(m_tags, p_context); // tt::pres::Tags
	
	bu::putEnum<u8, StartType>(m_startedFrom,   p_context);
	bu::put(      m_priority,                   p_context); // s32
	bu::put(      m_endCallbackName,            p_context); // std::string
	bu::put(      m_isAffectedByMovement,       p_context); // bool
	bu::put(      m_isAffectedByOrientation,    p_context); // bool
	bu::put(      m_isAffectedByFloorDirection, p_context); // bool
	bu::put(      m_isFollowingParent,          p_context); // bool
	
	if (m_isFollowingParent == false)
	{
		// Not following parent; store the position
		bu::put(m_pres->getPositionClean(), p_context); // tt::math::Vector3
	}
	
	bu::putHandle(m_rotateTo,                   p_context); // EntityHandle
	
	bu::put(m_useCustomTransform, p_context);
	bu::put(m_customTranslation , p_context);
	bu::put(m_customScale       , p_context);
	bu::put(m_customRotation    , p_context);
	
	bu::put(m_pres->getFlipMask(), p_context);
	
	const s32 textLabelCount = static_cast<s32>(m_textLabels.size());
	bu::put(textLabelCount, p_context);
	for (TextLabels::const_iterator it = m_textLabels.begin(); it != m_textLabels.end(); ++it)
	{
		const TextLabel& textLabel = (*it);
		bu::putHandle  (textLabel.handle, p_context);
		bu::put        (textLabel.offset, p_context);
	}
}


void PresentationObject::unserialize(tt::code::BufferReadContext* p_context)
{
	// Unserialize members
	namespace bu = tt::code::bufferutils;
	
	m_tags                       = serialization::unserializePresTags(p_context); // tt::pres::Tags
	
	m_startedFrom                = bu::getEnum<u8, StartType>(p_context);
	m_priority                   = bu::get<s32              >(p_context);
	m_endCallbackName            = bu::get<std::string      >(p_context);
	m_isAffectedByMovement       = bu::get<bool             >(p_context);
	
	const bool isAffectedByOrientation = bu::get<bool>(p_context);
	m_isAffectedByOrientation = isAffectedByOrientation == false; // to force update
	setAffectedByOrientation(isAffectedByOrientation);
	
	const bool isAffectedByFloorDirection = bu::get<bool>(p_context);
	m_isAffectedByFloorDirection = isAffectedByFloorDirection == false; // to force update
	setAffectedByFloorDirection(isAffectedByFloorDirection);
	
	m_isFollowingParent = bu::get<bool>(p_context);
	if (m_isFollowingParent == false)
	{
		// Not following parent; store the position
		const tt::math::Vector3 pos = bu::get<tt::math::Vector3>(p_context);
		m_pres->setPosition(pos);
	}
	
	m_rotateTo                   = bu::getHandle<game::entity::Entity>(p_context);
	
	m_useCustomTransform         = bu::get<bool>             (p_context);
	m_customTranslation          = bu::get<tt::math::Vector3>(p_context);
	m_customScale                = bu::get<tt::math::Vector3>(p_context);
	m_customRotation             = bu::get<real>             (p_context);
	
	const u32 flipMask           = bu::get<u32>              (p_context);
	m_pres->setFlipMask(flipMask);
	
	const s32 textLabelCount = bu::get<s32>(p_context);
	
	m_textLabels.clear();
	m_textLabels.reserve(textLabelCount);
	for (s32 i = 0; i < textLabelCount; ++i)
	{
		TextLabel textLabel;
		textLabel.handle = bu::getHandle<game::entity::graphics::TextLabel>(p_context);
		textLabel.offset = bu::get<tt::math::Vector2>(p_context);
		
		addTextLabel(textLabel.handle, textLabel.offset);
	}
}


void PresentationObject::unserializeState(tt::code::BufferReadContext* p_context)
{
	m_presTime = tt::code::bufferutils::get<real>(p_context);
	serialization::unserializePresentationObjectState(m_pres, p_context, m_presTime);
}


void PresentationObject::serializeState(tt::code::BufferWriteContext* p_context) const
{
	tt::code::bufferutils::put(m_presTime, p_context);
	serialization::serializePresentationObjectState(m_pres, p_context);
}

PresentationObject* PresentationObject::getPointerFromHandle(const PresentationObjectHandle& p_handle)
{
	if (AppGlobal::hasGame() == false || AppGlobal::getGame()->hasPresentationObjectMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getPresentationObjectMgr().getPresentationObject(p_handle);
}


void PresentationObject::updatePosition(const tt::math::Vector2 p_pos)
{
	if (m_pres != 0)
	{
		tt::math::Vector3 position(p_pos.x, p_pos.y, 0.0f);
		if (getLayer() == game::ParticleLayer_Minimap)
		{
			game::Game* game = AppGlobal::getGame();
			
			tt::math::Vector2 minimapPos = game->getMinimap().getPositionFromWorld(p_pos);
			position.x = minimapPos.x;
			position.y = minimapPos.y;
		}
		m_pres->setPosition(position);
	}
}


void PresentationObject::updateOrientation(tt::pres::PresentationObject::FlipMask p_flipMask,
                                           const tt::math::Quaternion&            p_rotation)
{
	if (m_pres != 0)
	{
		bool isDirty = false;
		if (isAffectedByOrientation())
		{
			m_pres->setFlipMask(p_flipMask);
			isDirty = true;
		}
		
		if (isAffectedByFloorDirection())
		{
			m_pres->setRotation(p_rotation);
			isDirty = true;
		}
		
		// Apply changes.
		if (isDirty)
		{
			m_pres->update(0.0f);
		}
	}
}


void PresentationObject::resetCustomTransformation()
{
	m_useCustomTransform = false;
	m_customTranslation.setValues(0.0f, 0.0f, 0.0f);
	m_customScale      .setValues(1.0f, 1.0f, 1.0f);
	m_customRotation = 0.0f;
}


u32 PresentationObject::getFlipMask() const
{
	return m_pres->getFlipMask();
}


void PresentationObject::setFlipMask(u32 p_mask)
{
	m_pres->setFlipMask(p_mask);
}


void PresentationObject::setIsCulled(bool p_isCulled)
{
	m_pres->setIsCulled(p_isCulled);
}


bool PresentationObject::isCulled() const
{
	return m_pres->isCulled();
}


void PresentationObject::setVisible(bool p_visible)
{
	m_pres->setVisible(p_visible);
}


bool PresentationObject::isVisible() const
{
	return m_pres->isVisible();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PresentationObject::restartDefaultAnimation()
{
	using namespace game::entity;
	
	Entity* parent = m_parent.getPtr();
	if (parent == 0 || parent->isInitialized() == false)
	{
		return;
	}
	
	movementcontroller::DirectionalMovementController* controller =
		parent->getDirectionalMovementController();
	
	if (controller != 0)
	{
		controller->restartPresentationObject(*this);
	}
}


#if !defined(TT_BUILD_FINAL)
tt::str::Strings PresentationObject::getSortedTagNames(const tt::pres::Tags& p_tags)
{
	tt::str::StringSet tagNames;
	for (tt::pres::Tags::const_iterator it = p_tags.begin(); it != p_tags.end(); ++it)
	{
		tagNames.insert((*it).getName());
	}
	
	return tt::str::Strings(tagNames.begin(), tagNames.end());
}


std::string PresentationObject::getParentType() const
{
	game::entity::Entity* parent = m_parent.getPtr();
	return parent != 0 ? parent->getType() : "<No Entity>";
}
#endif

// Namespace end
}
}

