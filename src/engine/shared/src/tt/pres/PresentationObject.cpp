#include <tt/pres/anim2d/AnimationStack2D.h>
#include <tt/pres/anim2d/ColorAnimationStack2D.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/pres/FrameAnimationStack.h>
#include <tt/pres/PresentationQuad.h>
#include <tt/pres/ParticlesStack.h>
#include <tt/pres/PresentationGroup.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/RootMatrixInterface.h>
#include <tt/pres/TimerStack.h>
#include <tt/pres/TriggerStack.h>
#include <tt/str/toStr.h>


namespace tt {
namespace pres {


//--------------------------------------------------------------------------------------------------
// Public member functions

PresentationObject::~PresentationObject()
{
	if (m_mgr != 0)
	{
		m_mgr->unregisterPresentationObject(this);
	}
}


void PresentationObject::setPosition(const math::Vector3& p_position)
{
	// Only recalculate if the value changed
	if (p_position == m_position)
	{
		return;
	}
	
	m_position = p_position;
	updatePrsMatrix();
	updateWorldPosition();
	
	// Resort if the Z position changed
	TT_NULL_ASSERT(m_group);
	if (m_position.z != p_position.z)
	{
		m_group->resort(this);
	}
}


void PresentationObject::setEndPosition(const math::Vector3& p_destination)
{
	/* for anim2d the current position is always 0,0,0. When the presentation objects getPosition
	 is called the position is added to anim2d transform. In the render the position is also added
	 after the anim2d transform. This is so that positions specified in the presentation file
	 are always offsets to the current position.*/
	/* When specifying the gametranslation we set the end position as zero. The start position we
	 set to the current position minus the end position. And set the position to the end position
	 when we start. This will make the anim2d transform move to zero. This way we can just add the
	 position and we have the transformed position. */
	
	m_anim2dStack.setGameTranslationBeginAndEnd(m_position - p_destination, math::Vector3::zero);
	
	// this changes the position but getPosition will stay the same because the anim2d transform is changed too.
	m_position = p_destination;
	updatePrsMatrix();
	updateWorldPosition();
	
	TT_NULL_ASSERT(m_group);
	// Resort if the Z position changed
	if (m_position.z != p_destination.z)
	{
		m_group->resort(this);
	}
}


void PresentationObject::setRotation(const math::Quaternion& p_rotation)
{
	// Only recalculate if the value changed
	if (p_rotation != m_rotation)
	{
		m_rotation = p_rotation;
		updatePrsMatrix();
	}
}


void PresentationObject::setScale(const math::Vector3& p_scale)
{
	// Only recalculate if the value changed
	if (p_scale != m_scale)
	{
		m_scale = p_scale;
		updatePrsMatrix();
	}
}


void PresentationObject::addOverlay(const engine::renderer::TexturePtr& p_texture,
                                    const math::Vector3&                p_positionOffset,
                                    const engine::renderer::ColorRGBA&  p_color)
{
	// FIXME: get rid of hardcoded 32 (worldspace) and 1080 (screenspace) values
	//engine::renderer::Renderer* renderer = engine::renderer::Renderer::getInstance();
	const real scale = m_isInScreenSpace ? (1.0f / 1080.0f ) : // renderer->getScreenHeight()) :
	                                       (1.0f / 32.0f);
	
	PresentationQuadPtr pres = PresentationQuadPtr(new PresentationQuad(p_texture, p_color, p_positionOffset));
	//pres->setWidth(pres->getWidth() * scale);
	//pres->setHeight(pres->getHeight()  * scale);;
	pres->setScale(math::Vector2(scale, scale));
	m_overlayQuads.push_back(pres);
}


void PresentationObject::removeOverlay(const engine::renderer::TexturePtr& p_texture)
{
	for (OverlayQuads::iterator it = m_overlayQuads.begin(); it != m_overlayQuads.end(); ++it)
	{
		if ((*it)->getTexture() == p_texture)
		{
			m_overlayQuads.erase(it);
			return;
		}
	}
	
	TT_PANIC("Cannot removeOverlay because texture wasn't found.");
	return;
}


bool PresentationObject::isActive() const
{
	if (m_timerStack.hasNameOrTagMatch())
	{
		return m_timerStack.isActive();
	}
	else
	{
		return (m_anim2dStack   .isActive() && m_anim2dStack   .isAllLooping() == false) ||
		       (m_colorAnimStack.isActive() && m_colorAnimStack.isAllLooping() == false) ||
		       (m_frameAnimStack.isActive() && m_frameAnimStack.isAllLooping() == false) ||
		       (m_particleStack .isActive() && m_particleStack .isAllLooping() == false) ||
		       (m_triggerStack  .isActive() && m_triggerStack  .isAllLooping() == false);
	}
}


bool PresentationObject::isLooping() const
{
	return m_timerStack    .isLooping() ||
	       m_anim2dStack   .isLooping() ||
	       m_colorAnimStack.isLooping() ||
	       m_frameAnimStack.isLooping() ||
	       m_particleStack .isLooping() ||
	       m_triggerStack  .isLooping();
}


bool PresentationObject::hasNameOrTagMatch() const
{
	return m_anim2dStack   .hasNameOrTagMatch() ||
	       m_colorAnimStack.hasNameOrTagMatch() ||
	       m_frameAnimStack.hasNameOrTagMatch() ||
	       m_particleStack .hasNameOrTagMatch() ||
	       m_triggerStack  .hasNameOrTagMatch();
}


DataTags PresentationObject::getTags() const
{
	DataTags tags;
	
	tags.addDataTags(m_anim2dStack   .getTags());
	tags.addDataTags(m_colorAnimStack.getTags());
	tags.addDataTags(m_frameAnimStack.getTags());
	tags.addDataTags(m_particleStack .getTags());
	tags.addDataTags(m_triggerStack  .getTags());
	tags.addDataTags(m_timerStack    .getTags());
	
	return tags;
}


str::StringSet PresentationObject::getRenderPassNames() const
{
	return m_frameAnimStack.getRenderPassNames();
}


void PresentationObject::start(const Tags& p_tags, bool p_hideEnd, const std::string& p_name)
{
	m_syncedTriggers.clear();
	m_endSyncedTriggers.clear();
	
	updatePresetCustomValues();
	
	m_anim2dStack   .start(p_tags, this, p_name);
	m_colorAnimStack.start(p_tags, this, p_name);
	m_frameAnimStack.start(p_tags, this, p_name);
	
	if (m_frameAnimStack.hasNameOrTagMatch())
	{
		m_frameAnimStack.createQuads();
	}
	
	
	updateTransform();
	
	m_particleStack.start(p_tags, this, p_name);
	m_triggerStack .start(p_tags, this, p_name);
	m_timerStack   .start(p_tags, this, p_name);
	
	m_activeTags = p_tags;
	m_activeName = p_name;
	m_hideAtEnd  = p_hideEnd;
	m_shouldRender = isActive() ? true : (m_hideAtEnd == false);
	if (m_shouldRender == false)
	{
		m_triggerStack.presentationEnded();
	}
	
	if(m_colorAnimStack.activeEmpty() == false)
	{
		m_frameAnimStack.updateColors(m_colorAnimStack);
	}
	
	for (OverlayQuads::const_iterator it = m_overlayQuads.begin(); it != m_overlayQuads.end(); ++it)
	{
		(*it)->setColor(m_colorAnimStack.getColor((*it)->getOriginalColor()));
	}
}


void PresentationObject::start(bool p_hideEnd, const std::string& p_name)
{
	Tags emptyTags;
	start(emptyTags, p_hideEnd, p_name);
}


void PresentationObject::stop()
{
	m_anim2dStack.stop();
	m_colorAnimStack.stop();
	m_frameAnimStack.stop();
	m_particleStack.stopParticles();
	m_particleStack.stop();
	m_triggerStack.stop();
	m_timerStack.stop();
	m_activeTags.clear();
	m_activeName.clear();
}


void PresentationObject::killParticles()
{
	m_particleStack.stopParticles(true);
	m_particleStack.stop();
}


void PresentationObject::pause()
{
	m_anim2dStack.pause();
	m_colorAnimStack.pause();
	m_frameAnimStack.pause();
	m_particleStack.pause();
	m_triggerStack.pause();
	m_timerStack.pause();
}


void PresentationObject::resume()
{
	m_anim2dStack.resume();
	m_colorAnimStack.resume();
	m_frameAnimStack.resume();
	m_particleStack.resume();
	m_triggerStack.resume();
	m_timerStack.resume();
}


void PresentationObject::reset()
{
	m_anim2dStack.reset();
	m_colorAnimStack.reset();
	m_frameAnimStack.reset();
	m_particleStack.reset();
	m_particleStack.stopParticles();
	m_triggerStack.reset();
	m_timerStack.reset();
}


void PresentationObject::resetAndClear()
{
	reset();
	
	m_anim2dStack.clear();
	m_colorAnimStack.clear();
	m_frameAnimStack.clear();
	m_particleStack.clear();
	m_triggerStack.clear();
	m_timerStack.clear();
}


size_t PresentationObject::getBufferSize() const
{
	size_t size = 0;
	size += m_anim2dStack.getBufferSize();
	size += m_colorAnimStack.getBufferSize();
	size += m_frameAnimStack.getBufferSize();
	size += m_particleStack.getBufferSize();
	size += m_timerStack.getBufferSize();
	size += m_triggerStack.getBufferSize();
	return size;
}


Dependencies PresentationObject::getDependencies() const
{
	return m_frameAnimStack.getDependencies();
}


bool PresentationObject::isMissingFrame() const
{
	return m_frameAnimStack.isVisible() == false && m_particleStack.isVisible() == false;
}


void PresentationObject::update(real p_deltaTime)
{
	if (m_isCulled)
	{
		return;
	}
	
	// Overwrite timestep with custom fixed timestep
	if (p_deltaTime >= 0.0f && m_fixedTimestep > 0.0f)
	{
		p_deltaTime = m_fixedTimestep;
	}
	
	const bool previousShouldRender = m_shouldRender;
	// Hold the previous active state before doing any updating
	m_shouldRender = isActive() ? true : (m_hideAtEnd == false);
	
	// When not rendering don't update either
	if (m_shouldRender == false)
	{
		if (previousShouldRender)
		{
			m_triggerStack.presentationEnded();
			m_particleStack.stopParticles();
		}
		return;
	}
	
	m_anim2dStack.update(p_deltaTime);
	
	updateTransform();
	
	m_colorAnimStack.update(p_deltaTime);
	m_frameAnimStack.update(p_deltaTime);
	
	// After updating both color and frame anim stacks update frame with current color.
	if(m_colorAnimStack.activeEmpty() == false)
	{
		m_frameAnimStack.updateColors(m_colorAnimStack);
	
		for (OverlayQuads::const_iterator it = m_overlayQuads.begin(); it != m_overlayQuads.end(); ++it)
		{
			(*it)->setColor(m_colorAnimStack.getColor((*it)->getOriginalColor()));
		}
	}
	
	m_particleStack.update(p_deltaTime);
	m_triggerStack.update(p_deltaTime);
	
	if (m_resetTriggerTime >= 0.0f)
	{
		const tt::pres::Tags activeTags(getCurrentActiveTags());
		const bool           hideAtEnd = isHidingAtEnd();
		const std::string&   activeName(getCurrentActiveName());
		const real resetToTime = m_resetTriggerTime;
		m_resetTriggerTime     = -1.0f;
		reset();
		// HACK: prevents triggers going off during a reset
		TriggerBase::setTriggersDisabled(true);
		start(activeTags, hideAtEnd, activeName);
		update(resetToTime);
		TriggerBase::setTriggersDisabled(false);
		return; // Don't continue current update.
	}
	
	m_timerStack.update(p_deltaTime);
}


void PresentationObject::render() const
{
	if (m_isCulled || m_shouldRender == false || m_visible == false) return;
	
	m_frameAnimStack.render(m_combinedMatrix);
	
	// Render overlays
	if(m_overlayQuads.empty() == false)
	{
		engine::renderer::MatrixStack* stack = engine::renderer::MatrixStack::getInstance();
		
		// TODO: Add culling for overlay quads
		stack->setMode(engine::renderer::MatrixStack::Mode_Position);
		stack->push();
		stack->multiply44(m_combinedMatrix);

		for (OverlayQuads::const_iterator it = m_overlayQuads.begin(); it != m_overlayQuads.end(); ++it)
		{
			// Texture setting and quad scaling is done in the quad.
			(*it)->render();
		}
		stack->pop();
	}
}


void PresentationObject::renderPass(const std::string& p_passName) const
{
	if (m_shouldRender && m_visible)
	{
		m_frameAnimStack.renderPass(p_passName, m_combinedMatrix);
	}
}


PresentationObjectPtr PresentationObject::clone(removeFunction p_removeFunction, PresentationMgr* p_mgr) const
{
	TT_ASSERT(p_removeFunction != 0);
	
	PresentationObjectPtr obj(new PresentationObject(*this, p_mgr), p_removeFunction);
	
	obj->m_particleStack.setFollowObject(obj);
	obj->m_triggerStack.setPresentationObject(obj);
	
	return obj;
}


engine::renderer::TextureContainer PresentationObject::getAndLoadAllUsedTextures() const
{
	using namespace engine::renderer;
	TextureContainer textures(m_frameAnimStack.getAndLoadAllUsedTextures());
	TextureContainer texturesPart(m_particleStack.getAndLoadAllUsedTextures());
	textures.insert(textures.end(), texturesPart.begin(), texturesPart.end());
	return textures;
}


void PresentationObject::addCustomPresentationValue(const std::string& p_name, real p_value)
{
	m_customValues[p_name] = p_value;
}


void PresentationObject::addPresetCustomPresentationValue(const std::string& p_name, const PresentationValue& p_value)
{
	m_presetCustomValues[p_name] = p_value;
}


bool PresentationObject::getCustomValue(const std::string& p_name, real* p_valueOut) const
{
	CustomPresentationValues::const_iterator it(m_customValues.find(p_name));
	if (it != m_customValues.end())
	{
		*p_valueOut = it->second;
		return true;
	}
	else
	{
		return false;
	}
}


void PresentationObject::addSyncedTrigger(const std::string& p_syncId, TriggerInterface* p_trigger)
{
	// check if this trigger is not already added
	std::pair<SyncedTriggers::iterator, SyncedTriggers::iterator> range(m_syncedTriggers.equal_range(p_syncId));
	
	for (SyncedTriggers::iterator it(range.first); it != range.second; ++it)
	{
		if (it->second == p_trigger) return;
	}
	
	// add the trigger
	m_syncedTriggers.insert(std::make_pair(p_syncId, p_trigger));
}


void PresentationObject::addEndSyncedTrigger(const std::string& p_syncId, TriggerInterface* p_trigger)
{
	// check if this trigger is not already added
	std::pair<SyncedTriggers::iterator, SyncedTriggers::iterator> range(m_endSyncedTriggers.equal_range(p_syncId));
	
	for (SyncedTriggers::iterator it(range.first); it != range.second; ++it)
	{
		if (it->second == p_trigger) return;
	}
	
	// add the trigger
	m_endSyncedTriggers.insert(std::make_pair(p_syncId, p_trigger));
}


void PresentationObject::triggerSync(const std::string& p_id)
{
	std::pair<SyncedTriggers::iterator, SyncedTriggers::iterator> range(m_syncedTriggers.equal_range(p_id));
	
	for (SyncedTriggers::iterator it(range.first); it != range.second; ++it)
	{
		it->second->stop();
		it->second->start(m_activeTags, this, m_activeName, true);
	}
}


void PresentationObject::triggerEndSync(const std::string& p_id)
{
	std::pair<SyncedTriggers::iterator, SyncedTriggers::iterator> range(m_endSyncedTriggers.equal_range(p_id));
	
	for (SyncedTriggers::iterator it(range.first); it != range.second; ++it)
	{
		it->second->stop();
		it->second->start(m_activeTags, this, m_activeName, true);
	}
}


void PresentationObject::setFlipMask(u32 p_mask)
{
	// Update scale of presentation and flip particle effects
	if(m_flipMask != p_mask)
	{
		m_particleStack.setFlipMask(p_mask);

		m_flipMask = p_mask;

		m_scale.x = ((m_flipMask & FlipMask_Horizontal) != 0) ? -1.0f : 1.0f;
		m_scale.y = ((m_flipMask & FlipMask_Vertical)   != 0) ? -1.0f : 1.0f;
		updatePrsMatrix();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

PresentationObject::PresentationObject(PresentationMgr* p_mgr)
:
m_rootMtx(),
m_position(math::Vector3::zero),
m_worldPosition(math::Vector3::zero),
m_rotation(math::Quaternion::identity),
m_scale(math::Vector3::allOne),
m_prsMatrix(),
m_anim2dTransform(),
m_combinedMatrix(),
m_overlayQuads(),
m_anim2dStack(),
m_colorAnimStack(),
m_frameAnimStack(),
m_particleStack(),
m_triggerStack(),
m_timerStack(),
m_customValues(),
m_presetCustomValues(),
m_activeTags(),
m_activeName(),
m_hideAtEnd(true),
m_shouldRender(false),
m_flipMask(FlipMask_None),
m_resetTriggerTime(-1.0f),
m_isPermanent(true),
m_dontPrecache(false),
m_fixedTimestep(0.0f),
m_isInScreenSpace(false),
m_mgr(p_mgr),
m_group(),
m_syncedTriggers(),
m_endSyncedTriggers(),
m_callbackInterface(),
m_isCulled(false),
m_visible(true)
{
	updatePrsMatrix();
	updateWorldPosition();
	
	if (m_mgr != 0)
	{
		m_mgr->registerPresentationObject(this);
	}
}


PresentationObject::PresentationObject(const PresentationObject& p_rhs, PresentationMgr* p_mgr)
:
m_rootMtx(p_rhs.m_rootMtx),
m_position(p_rhs.m_position),
m_worldPosition(p_rhs.m_worldPosition),
m_rotation(p_rhs.m_rotation),
m_scale(p_rhs.m_scale),
m_prsMatrix(p_rhs.m_prsMatrix),
m_anim2dTransform(p_rhs.m_anim2dTransform),
m_combinedMatrix(p_rhs.m_combinedMatrix),
m_anim2dStack(p_rhs.m_anim2dStack),
m_colorAnimStack(p_rhs.m_colorAnimStack),
m_frameAnimStack(p_rhs.m_frameAnimStack),
m_particleStack(p_rhs.m_particleStack),
m_triggerStack(p_rhs.m_triggerStack),
m_timerStack(p_rhs.m_timerStack),
m_customValues(p_rhs.m_customValues),
m_presetCustomValues(p_rhs.m_presetCustomValues),
m_activeTags(p_rhs.m_activeTags),
m_activeName(p_rhs.m_activeName),
m_hideAtEnd(p_rhs.m_hideAtEnd),
m_shouldRender(p_rhs.m_shouldRender),
m_flipMask(p_rhs.m_flipMask),
m_resetTriggerTime(p_rhs.m_resetTriggerTime),
m_isPermanent(p_rhs.m_isPermanent),
m_dontPrecache(p_rhs.m_dontPrecache),
m_fixedTimestep(p_rhs.m_fixedTimestep),
m_isInScreenSpace(p_rhs.m_isInScreenSpace),
m_mgr(p_mgr),
m_group(),
m_syncedTriggers(),
m_endSyncedTriggers(),
m_callbackInterface(p_rhs.m_callbackInterface),
m_isCulled(p_rhs.m_isCulled),
m_visible(p_rhs.m_visible)
{
	for (OverlayQuads::const_iterator it = p_rhs.m_overlayQuads.begin();
	     it != p_rhs.m_overlayQuads.end(); ++it)
	{
		m_overlayQuads.push_back((*it)->clone());
	}
	
	if (m_mgr != 0)
	{
		m_mgr->registerPresentationObject(this);
	}
}


void PresentationObject::updatePrsMatrix()
{
	// Start with position, because this will (almost) always be used
	m_prsMatrix = math::Matrix44::getTranslation(m_position);
	
	// Only add rotation matrix if it is actually used
	if(m_rotation != math::Quaternion::identity)
	{
		m_prsMatrix = m_rotation.getRotationMatrix() * m_prsMatrix;
	}
	
	// Finally apply scale if it is used
	if(m_scale != math::Vector3::allOne)
	{
		m_prsMatrix = math::Matrix44::getScale(m_scale.x, m_scale.y, m_scale.z) * m_prsMatrix;
	}
}


void PresentationObject::updateTransform()
{
	// Get the new transformation
	const math::Vector3 oldPos(getWorldPosition());
	m_anim2dStack.updateTransform(&m_anim2dTransform);
	updateWorldPosition();
	const math::Vector3& newPos(getWorldPosition());
	
	// This assert should never trigger. If it does, one of the stacks is active/visible
	// before the start is called on the object. And this can only happen if something 
	// went wrong in the code.
	// MR: Or when the code created a PresentationObject without passing a PresMgr
	TT_NULL_ASSERT(m_group);
	
	// Resort if the z position changed
	if (oldPos.z != newPos.z)
	{
		m_group->resort(this);
	}
	
	// Recalculate the combined matrix
	{
		// MARTIJN HACK: For some reason the allowed Z range for screenspace presentations is different per
		// resolution. So in windowed mode a range of [-0.50, 0.50] is allowed. But is full screen suddenly this
		// needs to be in the [-0.40, 0.40] range. To work around this, make all screenspace Z values 10 times smaller
		// so hopefully the Z value will always in the correct range.
		math::Matrix44 prs(m_prsMatrix);
		if (isInScreenSpace())
		{
			prs.scale(1.0f, 1.0f, 0.1f);
		}
		// End hack
		
		RootMatrixInterfacePtr rootMtx(m_rootMtx.lock());
		if (rootMtx == 0)
		{
			m_combinedMatrix = m_anim2dTransform * prs;
		}
		else
		{
			m_combinedMatrix = m_anim2dTransform * rootMtx->getTransform() * prs;
		}
	}
}


void PresentationObject::updatePresetCustomValues()
{
	for (PresetCustomValues::iterator it = m_presetCustomValues.begin();
	     it != m_presetCustomValues.end(); ++it)
	{
		it->second.updateValue(this);
		addCustomPresentationValue(it->first, it->second.get());
	}
}


void PresentationObject::addToGroup(const GroupInterfacePtr& p_group)
{
	TT_ASSERTMSG(m_group == 0,
	             "PresentationObject::addToGroup - PresentationObject is still in a Group");
	p_group->add(this);
	TT_ASSERT(m_group == p_group); // Should have been set after the add.
}


void PresentationObject::removeFromGroup()
{
	TT_ASSERTMSG(m_group != 0,
	             "PresentationObject::removeFromGroup - PresentationObject is not in a Group");
	if (m_group != 0)
	{
		m_group->remove(this);
		TT_ASSERT(m_group == 0); // Should have been reset after the remove.
	}
}

// Namespace end
}
}
