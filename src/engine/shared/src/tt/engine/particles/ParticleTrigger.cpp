#include <algorithm>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/engine/particles/WorldObject.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene2d/SceneInterface.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/utils/utils.h>
#include <tt/xml/XmlFileReader.h>
#include <tt/str/common.h>
#include <tt/streams/BIFStream.h>


namespace tt {
namespace engine {
namespace particles {

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
// for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_signatureLength = 9;
const u8     g_signature[g_signatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'P', 'T', 'R', 'G', // The actual signature bytes for a particle trigger
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};
const u32    g_currentVersion = 11;


//--------------------------------------------------------------------------------------------------
// Public member functions

ParticleTrigger::ParticleTrigger(const WorldObject* p_object, u32 p_category)
:
m_active(false),
m_isCullingEnabled(true),
m_haveReferenceObjectOwnership(false),
m_triggerID(),
m_scene(0),
m_emitters(),
m_referenceObject(p_object),
m_scale(1.0f),
m_category(p_category),
m_particleLimit(-1),
m_name()
#ifndef TT_BUILD_FINAL
,m_fileName()
#endif
{
}


ParticleTrigger::ParticleTrigger(tt::engine::scene2d::SceneInterface* p_scene,
                                 u32                                  p_category)
:
m_active(false),
m_isCullingEnabled(true),
m_haveReferenceObjectOwnership(false),
m_triggerID(),
m_scene(p_scene),
m_emitters(),
m_referenceObject(0),
m_scale(1.0f),
m_category(p_category),
m_particleLimit(-1),
m_boundingBox(),
m_name()
#ifndef TT_BUILD_FINAL
,m_fileName()
#endif
{
}


ParticleTrigger::~ParticleTrigger()
{
	destroyEmitters();
	
	if (m_haveReferenceObjectOwnership)
	{
		delete m_referenceObject;
	}
	
	// Check if we need to remove us from a scene
	if (m_scene != 0)
	{
		m_scene->remove(this);
	}
	
}


ParticleTrigger* ParticleTrigger::clone(bool p_keepReferenceObject)
{
	ParticleTrigger* tempTrigger = new ParticleTrigger(*this);
	if (p_keepReferenceObject == false)
	{
		tempTrigger->m_referenceObject = 0;
	}
	else if (m_haveReferenceObjectOwnership)
	{
		// HACK: Transfer ownership of the reference object to the cloned trigger
		// (what should happen is that the reference object is copied, but there is no interface to do this)
		m_referenceObject = 0;
	}
	ParticleMgr::getInstance()->addClonedTrigger(tempTrigger, this);
	return tempTrigger;
}


void ParticleTrigger::trigger(TriggerType p_type)
{
	// First determine action for trigger class
	switch (p_type)
	{
	case TriggerType_Start:
		// Trigger becomes active
		m_active = true;
		if (m_scene != 0)
		{
			m_scene->insert(this);
		}
		break;
		
	case TriggerType_Stop:
		// Nothing to be done for trigger
		break;
		
	case TriggerType_Kill:
		// Trigger deactivated
		m_active = false;
		break;
		
	case TriggerType_Reload:
		TT_PANIC("TriggerType_Reload not supported");
		/*
		// Recreate emitters from xml file
		// FIXME: doesn't work with binary particles
		destroyEmitters();
		loadFromXML(m_xml_filename);
		
		// If this was an active trigger, restart emitters
		if (m_active)
		{
			p_type = ParticleTrigger::TriggerType_Start;
			break;
		}
		*/
		
		return;
		
	default:
		// Not interested in this type
		return;
	}
	
	// Pass message along to emitters
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		switch (p_type)
		{
		case TriggerType_Start:
			// Start emitters
			(*it)->activate();
			break;
			
		case TriggerType_Stop:
			// Stop emitters
			(*it)->deactivate();
			break;
			
		case TriggerType_Kill:
			// Kill all active particles
			(*it)->kill();
			break;
			
		default:
			break;
		}
	}
}


bool ParticleTrigger::load(const std::string& p_filename, real p_scale, s32 p_renderGroup)
{
	m_scale = p_scale;
	
	TT_ASSERTMSG(str::endsWith(p_filename, ".xml") == false,
	             "Trying to load particle file '%s' which is an XML file. XML files are unsupported, "
	             "please convert to binary trigger file", p_filename.c_str());
	
	// Not an XML file so try to load as binary
	return loadFromBinary(p_filename, p_renderGroup);
}


void ParticleTrigger::addDelay(real p_delay)
{
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		// Update emitter delay
		(*it)->addDelay(p_delay);
	}
}


void ParticleTrigger::setOrigin(real p_x, real p_y)
{
	setOrigin(math::Vector2(p_x, p_y));
}


void ParticleTrigger::setOrigin(const tt::math::Vector2& p_origin)
{
	TT_ASSERTMSG(m_referenceObject == 0, "setOrigin() is not useful when trigger has parent.");
	
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		// Update emitter position
		(*it)->setPosition(p_origin);
	}
}


void ParticleTrigger::setFollowObject(const WorldObject* p_object)
{
	if (m_haveReferenceObjectOwnership)
	{
		delete m_referenceObject;
	}
	
	m_referenceObject = p_object;
	
	if (m_referenceObject == 0)
	{
		return;
	}
	
	// Set correct position of emitters
	const math::Vector3& pos3D = m_referenceObject->getPosition();
	const math::Vector2& pos2D = math::Vector2(pos3D.x, pos3D.y);
	
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		(*it)->setPosition(pos2D);
	}
	
	setDepth(pos3D.z);
}


void ParticleTrigger::setTriggerHasFollowObjectOwnership(bool p_haveOwnership)
{
	m_haveReferenceObjectOwnership = p_haveOwnership;
}


void ParticleTrigger::update(real p_deltaTime)
{
	// Always update position
	math::Vector3 pos3D;
	math::Vector2 pos2D;
	if (m_referenceObject != 0)
	{
		pos3D = m_referenceObject->getPosition();
		pos2D = math::Vector2(pos3D.x, pos3D.y);
		setDepth(pos3D.z);
		
		const real scale = m_referenceObject->getScaleForParticles();
		// Only set scale if reference object has a positive scale
		if (scale > 0.0f)
		{
			setScale(scale);
		}
	}
	
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		if (m_referenceObject != 0)
		{
			(*it)->setPosition(pos2D);
		}
	}
	
	if (m_isCulled)
	{
		return;
	}
	
	bool active = false;
	const real zDepth = getDepth();
	
	// Check for fixed deltatime
	ParticleMgr* pm = ParticleMgr::getInstance();
	if (pm == 0)
	{
		return;
	}
	
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		ParticleEmitter* emitter(*it);
		emitter->setDepth(zDepth);
		
		// Update particles
		real timestep = pm->getFixedTimestepForRenderGroup(emitter->getGroup());
		if (timestep == 0.0)
		{
			// No fixed timestep; use normal deltatime
			timestep = p_deltaTime;
		}
		
		emitter->update(timestep);
		
		if (active == false && (emitter->isActive() || emitter->getParticleCount() > 0))
		{
			active = true;
		}
	}
	
	m_active = active;
}


void ParticleTrigger::updateForRender(const tt::math::VectorRect* p_visibilityRect)
{
	m_isCulled = false;
	
	if (m_emitters.empty())
	{
		m_isCulled = true;
		return;
	}
	
	// Check if particle culling is disabled
	if (m_isCullingEnabled == false || ParticleMgr::getInstance()->isParticleCullingEnabled() == false)
	{
		// Set isCulled for all emitters
		for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
		{
			(*it)->setIsCulled(m_isCulled);
		}
		
		return;
	}
	
	const scene::CameraPtr& cam(renderer::Renderer::getInstance()->getActiveCamera());
	
	// Cull if reference object is culled or if trigger is behind near plane
	if ((m_referenceObject != 0 && m_referenceObject->isCulled()) || 
		cam->getPosition().z <= getDepth())
	{
		m_isCulled = true;
	}
	else if (p_visibilityRect == 0) 
	{
		m_isCulled = false;
	}
	else
	{
		// Calculate combined boundingbox
		math::VectorRect boundingBox;
		bool foundRect = false;
		for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
		{
			if ((*it)->getParticleCount() > 0)
			{
				if (foundRect == false)
				{
					boundingBox = (*it)->getBoundingBox();
					foundRect = true;
				}
				else
				{
					// Already found a rect, merge it with this one
					boundingBox = math::merge(boundingBox, (*it)->getBoundingBox());
				}
			}
		}
		
		if (foundRect)
		{
			const bool validBox = cam->convert3Dto2D(boundingBox, getDepth());
			m_boundingBox = boundingBox;
			m_isCulled = validBox && p_visibilityRect->intersects(m_boundingBox) == false;
		}
		else
		{
			m_isCulled = false;
		}
	}
	
	// Set isCulled for all emitters
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		(*it)->setIsCulled(m_isCulled);
	}
}


// FIXME: Add constness to Scene2D::render
void ParticleTrigger::render()
{
	// Only used by Scene2D::render()
	TT_ASSERT(isSceneNode());
	if (m_isCulled)
	{
		return;
	}
	
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		(*it)->render();
	}
}


void ParticleTrigger::renderAllGroups() const
{
	// Should not be used by Scene2D::render()
	TT_ASSERT(isSceneNode() == false);
	if (m_isCulled)
	{
		return;
	}
	
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		(*it)->render();
	}
}


void ParticleTrigger::renderGroup(s32 p_group) const
{
	// Should not be used by Scene2D::render()
	TT_ASSERT(isSceneNode() == false);
	if (m_isCulled)
	{
		return;
	}
	
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		if (p_group == (*it)->getGroup())
		{
			(*it)->render();
		}
	}
}


void ParticleTrigger::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	const debug::DebugRendererPtr dbg(renderer::Renderer::getInstance()->getDebug());
	// Render cullRect
	if (m_isCulled)
	{
		dbg->renderRect(engine::renderer::ColorRGBA(32, 64, 255, 255), m_boundingBox);
	}
	else if (m_isCullingEnabled)
	{
		dbg->renderRect(engine::renderer::ColorRGBA(64, 192, 255, 255), m_boundingBox);
	}
#endif
}


real ParticleTrigger::getHeight() const
{
	return m_boundingBox.getHeight();
}


real ParticleTrigger::getWidth() const
{
	return m_boundingBox.getWidth();
}


renderer::TextureContainer ParticleTrigger::getAndLoadAllUsedTextures() const
{
	renderer::TextureContainer textures;
	
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		textures.push_back((*it)->getAndLoadAllUsedTextures());
	}
	
	return textures;
}


void ParticleTrigger::removeEmitter(ParticleEmitter* p_emitter)
{
	m_emitters.erase(std::remove(m_emitters.begin(), m_emitters.end(), p_emitter), m_emitters.end());
	delete p_emitter;
}


void ParticleTrigger::removeEmitterByIndex(s32 p_emitterIndex)
{
	TT_ASSERT(p_emitterIndex < static_cast<s32>(m_emitters.size()));

	removeEmitter(m_emitters[p_emitterIndex]);
}


void ParticleTrigger::moveEmitterUp(s32 p_emitterIndex)
{
	TT_ASSERT(p_emitterIndex -1 >= 0);
	TT_ASSERT(p_emitterIndex < static_cast<s32>(m_emitters.size()));

	std::iter_swap(
		m_emitters.begin() + p_emitterIndex - 1,
		m_emitters.begin() + p_emitterIndex
	);
}


void ParticleTrigger::moveEmitterDown(s32 p_emitterIndex)
{
	TT_ASSERT(p_emitterIndex >= 0);
	TT_ASSERT(p_emitterIndex + 1 < static_cast<s32>(m_emitters.size()));

	std::iter_swap(
		m_emitters.begin() + p_emitterIndex,
		m_emitters.begin() + p_emitterIndex + 1
	);
}


void ParticleTrigger::flip(u32 p_flipMask)
{
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		flipEmitterSettings((*it)->getSettings(), p_flipMask);
	}
}


void ParticleTrigger::setScale(real p_scale)
{
	TT_ASSERT(p_scale > 0.0f);
	
	// scaleEmitterSettings() multiplies emitter settings so account for this
	const real scaleFactor = (p_scale / m_scale);
	
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		scaleEmitterSettings((*it)->getSettings(), scaleFactor);
	}
	
	m_scale = p_scale;
}


void ParticleTrigger::setInitialExternalImpulse(real p_angle, real p_power)
{
	for (EmitterCollection::const_iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		(*it)->setInitialExternalImpulse(p_angle, p_power);
	}
}


void ParticleTrigger::setIsCulled(bool p_isCulled)
{
	TT_ASSERTMSG(m_referenceObject == 0,
		"Manual setIsCulled only works if trigger doesn't have a reference object");
	m_isCulled = p_isCulled;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ParticleTrigger::ParticleTrigger(const ParticleTrigger& p_rhs)
:
m_active(p_rhs.m_active),
m_isCullingEnabled(p_rhs.m_isCullingEnabled),
m_haveReferenceObjectOwnership(p_rhs.m_haveReferenceObjectOwnership),
m_triggerID(p_rhs.m_triggerID),
m_scene(p_rhs.m_scene),
m_emitters(),
m_referenceObject(p_rhs.m_referenceObject),
m_scale(p_rhs.m_scale),
m_category(p_rhs.m_category),
m_particleLimit(p_rhs.m_particleLimit),
m_name(p_rhs.m_name)
#ifndef TT_BUILD_FINAL
,m_fileName(p_rhs.m_fileName)
#endif
{
	m_isCulled = p_rhs.m_isCulled;
	// Clone all emitters
	for (EmitterCollection::const_iterator it = p_rhs.m_emitters.begin();
	     it != p_rhs.m_emitters.end(); ++it)
	{
		m_emitters.push_back((*it)->clone());
	}
}


bool ParticleTrigger::loadFromBinary(const std::string& p_filename, s32 p_renderGroup)
{
	ParticleMgr* pm = ParticleMgr::getInstance();
	
#ifndef TT_BUILD_FINAL
	m_fileName = tt::fs::utils::getFileTitle(p_filename);
#endif
	
	if (pm->isTriggerCachingEnabled())
	{
		TriggerSettings* settings = pm->getTriggerSettingsFromCache(p_filename);
		if (settings != 0)
		{
			TriggerSettings TriggerSettingsCopy(*settings);
			
			// Apply the scale to copy of the cached settings.
			for (EmitterSettingsCollection::iterator it = TriggerSettingsCopy.emitSettings.begin();
			     it != TriggerSettingsCopy.emitSettings.end(); ++it)
			{
				// Apply scaling
				scaleEmitterSettings(*it, m_scale);
			}
			
			return setupTrigger(TriggerSettingsCopy, p_renderGroup);
		}
	}
	
	TriggerSettings settings;
	
	// Open file as binary stream
	streams::BIFStream file(p_filename);
	
	// Check status
	if (file.hasFailed())
	{
		TT_PANIC("Failed to open ParticleTrigger '%s'", p_filename.c_str());
		return false;
	}
	
	// Files are in big endian
	file.useBigEndian();
	
	// Read and verify the file signature
	u8 signature[g_signatureLength] = { 0 };
	file.read(signature, g_signatureLength);
	for (size_t i = 0; i < g_signatureLength; ++i)
	{
		if (signature[i] != g_signature[i])
		{
			// Does not appear to be a valid particle trigger file
			TT_Printf("ParticleTrigger::loadFromBinary: File signature byte %d (0x%02X) does not match expected byte 0x%02X.",
			          s32(i), u32(signature[i]), u32(g_signature[i]));
			return false;
		}
	}
	
	// Read and check the file version
	u32 fileVersion = 0;
	file >> fileVersion;
	if (fileVersion != g_currentVersion)
	{
		TT_PANIC("Unsupported particle format version: %u. Expected version %u. "
		         "Cannot load the file. Tip: Update SDK and then build!\nFilename: %s",
		         fileVersion, g_currentVersion, p_filename.c_str());
		return false;
	}
	
	// Read length of trigger name
	s16 nameLength = 0;
	file >> nameLength;
	
	// Read trigger name
	if (nameLength > 0)
	{
		std::vector<char> name(nameLength);
		file.read(reinterpret_cast<u8*>(&name[0]), nameLength);
		settings.name.assign(name.begin(), name.end());
	}
	
	settings.id = TriggerID(settings.name);
	
	// Read max particles
	settings.maxParticles = -1;
	file >> settings.maxParticles;
	m_particleLimit = settings.maxParticles;
	
	// Read particle culling enabled
	settings.isParticleCullingEnabled = true;
	file >> settings.isParticleCullingEnabled;
	m_isCullingEnabled = settings.isParticleCullingEnabled;
	
	// Read number of emitters contained in the trigger
	s16 emitterCount = 0;
	file >> emitterCount;
	
	// Read all emitters
	settings.emitSettings.resize(emitterCount);
	
	for (EmitterSettingsCollection::iterator iter = settings.emitSettings.begin();
	     iter != settings.emitSettings.end(); ++iter)
	{
		// Read max number of particles
		file >> (*iter).max_particles;
		
		// Read delay
		file >> (*iter).emitter_delay;
		
		// Read Origin (X,Y)
		file >> (*iter).origin.x;
		file >> (*iter).origin.y;

		// Read space
		file >> (*iter).inWorldSpace;
		
		// Read Render Group ID
		file >> (*iter).render_group;
		
		// Read texture name
		s16 texNameLength = 0;
		file >> texNameLength;
		
		std::vector<char> texture(texNameLength);
		file.read(reinterpret_cast<u8*>(&texture[0]), texNameLength);
		(*iter).texture_filename.assign(texture.begin(), texture.end());

		// Read flip settings
		file >> (*iter).flipTexture;
		
		// Read frame size
		file >> (*iter).frame_info.cell_size;
		
		// Read animation count
		s16 animationCount = 0;
		file >> animationCount;
		
		(*iter).animations.resize(animationCount);
		
		for (ParticleAnimationContainer::iterator animIter = (*iter).animations.begin();
		     animIter != (*iter).animations.end(); ++animIter)
		{
			// Read start & end frame
			file >> (*animIter).start_frame;
			file >> (*animIter).end_frame;
			
			// Read seconds per frame
			file >> (*animIter).spf;
			
			// Read animation type
			u8 animationType = 0;
			file >> animationType;
			(*animIter).type = static_cast<ParticleAnimation::AnimationType>(animationType);
		}
		
		// Read area type
		u8 areaType = 0;
		file >> areaType;
		(*iter).emission.area_type = static_cast<EmissionBehavior::AreaType>(areaType);
		
		// Read area properties based on type
		if ((*iter).emission.area_type == EmissionBehavior::AreaType_Rectangle)
		{
			// Read rectangle
			file >> (*iter).emission.rect_width.high;
			(*iter).emission.rect_width.low = -(*iter).emission.rect_width.high;
			
			file >> (*iter).emission.rect_height.high;
			(*iter).emission.rect_height.low = -(*iter).emission.rect_height.high;
		}
		else
		{
			// Read circle radius
			file >> (*iter).emission.radius;
			file >> (*iter).emission.innerRadius;
		}
		
		// Read external force
		file >> (*iter).external_force_x.low;
		file >> (*iter).external_force_x.high;
		file >> (*iter).external_force_y.low;
		file >> (*iter).external_force_y.high;
		
		// Read blend mode
		u8 blendMode = 0;
		file >> blendMode;
		
		(*iter).blend_mode = static_cast<renderer::BlendMode>(blendMode);
		TT_ASSERTMSG(renderer::isValidBlendMode((*iter).blend_mode),
		             "ParticleTrigger '%s' contains an emitter with an invalid blend mode (mode value %d).",
		             p_filename.c_str(), (*iter).blend_mode);
		
		u8 blendModeAlpha = 0;
		file >> blendModeAlpha;
		(*iter).blend_mode_alpha = static_cast<renderer::BlendModeAlpha>(blendModeAlpha);
		TT_ASSERTMSG(renderer::isValidBlendModeAlpha((*iter).blend_mode_alpha),
		             "ParticleTrigger '%s' contains an emitter with an invalid alpha blend mode (mode value %d).",
		             p_filename.c_str(), (*iter).blend_mode_alpha);
		
		// Read ignore fog
		file >> (*iter).ignore_fog;
		
		// Read pre-generation
		file >> (*iter).emission.pregeneration_time;
		file >> (*iter).emission.pregeneration_step;
		
		// Read particles amount
		file >> (*iter).emission.particles;
		
		// Read emitter type
		u8 type = 0;
		file >> type;
		(*iter).emission.type = static_cast<EmissionBehavior::EmissionType>(type);
		
		// Read lifetime of emitter
		file >> (*iter).emission.lifetime;

		// Read orientation
		u8 orientation(0);
		file >> orientation;
		(*iter).orientation = static_cast<ParticleOrientation>(orientation);
		
		// Read velocity
		file >> (*iter).velocity_x.low;
		file >> (*iter).velocity_x.high;
		file >> (*iter).velocity_y.low;
		file >> (*iter).velocity_y.high;
		file >> (*iter).velocity_friction.low;
		file >> (*iter).velocity_friction.high;
		
		// Read external impulse weight
		file >> (*iter).external_impulse_weight.low;
		file >> (*iter).external_impulse_weight.high;
		
		// Read particle lifetime
		file >> (*iter).particle_creation.lifetime.low;
		file >> (*iter).particle_creation.lifetime.high;
		
		// Read particle size
		file >> (*iter).particle_creation.start_size.low;
		file >> (*iter).particle_creation.start_size.high;
		file >> (*iter).particle_creation.end_size.low;
		file >> (*iter).particle_creation.end_size.high;
		file >> (*iter).particle_creation.use_end_size;
		
		// Read particle weight
		file >> (*iter).particle_creation.start_weight.low;
		file >> (*iter).particle_creation.start_weight.high;
		file >> (*iter).particle_creation.end_weight.low;
		file >> (*iter).particle_creation.end_weight.high;
		
		// Read particle rotation
		file >> (*iter).particle_creation.start_rotation.low;
		file >> (*iter).particle_creation.start_rotation.high;
		file >> (*iter).particle_creation.rotation_speed.low;
		file >> (*iter).particle_creation.rotation_speed.high;
		
		// Read rotation force
		file >> (*iter).particle_creation.rotation_force.low;
		file >> (*iter).particle_creation.rotation_force.high;
		
		// Read rotation friction
		file >> (*iter).particle_creation.rotation_friction.low;
		file >> (*iter).particle_creation.rotation_friction.high;
		
		// Read frame offset range
		file >> (*iter).particle_creation.start_frame.low;
		file >> (*iter).particle_creation.start_frame.high;

		// Read direction from spawn
		file >> (*iter).directionFromSpawnLocation;
		
		// Read vertex color enabled boolean
		file >> (*iter).vertex_color_enabled;
		
		// If enabled, read color settings
		if ((*iter).vertex_color_enabled)
		{
			// Read start color
			file.read(reinterpret_cast<u8*>(&(*iter).particle_creation.start_color.low),
			          sizeof(renderer::ColorRGBA));
			file.read(reinterpret_cast<u8*>(&(*iter).particle_creation.start_color.high),
			          sizeof(renderer::ColorRGBA));
			
			// Read end color
			file.read(reinterpret_cast<u8*>(&(*iter).particle_creation.end_color.low),
			          sizeof(renderer::ColorRGBA));
			file.read(reinterpret_cast<u8*>(&(*iter).particle_creation.end_color.high),
			          sizeof(renderer::ColorRGBA));
			
			// Read fade settings
			file >> (*iter).particle_creation.fade_in;
			file >> (*iter).particle_creation.fade_out;
			file >> (*iter).particle_creation.use_fade_as_time;
		}

		convertFilenameToEngineID(*iter);
	}
	
	// Add to cache before applying scale
	if (pm->isTriggerCachingEnabled())
	{
		pm->addTriggerSettingsToCache(p_filename, settings);
	}
	
	for (EmitterSettingsCollection::iterator it = settings.emitSettings.begin();
	     it != settings.emitSettings.end(); ++it)
	{
		// Apply scaling
		scaleEmitterSettings(*it, m_scale);
	}
	
	return setupTrigger(settings, p_renderGroup);
}


bool ParticleTrigger::setupTrigger(TriggerSettings& p_settings, s32 p_renderGroup)
{
	m_name = p_settings.name;
	m_triggerID = p_settings.id;
	
	// Clear previous emitters
	destroyEmitters();
	
	// Update emitter position
	math::Vector3 pos3D;
	math::Vector2 pos2D;
	if (m_referenceObject != 0)
	{
		pos3D = m_referenceObject->getPosition();
		pos2D = math::Vector2(pos3D.x, pos3D.y);
		
		setDepth(pos3D.z);
	}
	
	// Create particle emit_settings
	for (EmitterSettingsCollection::iterator it = p_settings.emitSettings.begin();
	     it != p_settings.emitSettings.end(); ++it)
	{
		if (p_renderGroup != -1) it->render_group = p_renderGroup;
		
		if (it->max_particles > 0)
		{
			ParticleEmitter* emitter = new ParticleEmitter(m_triggerID);
			
			if (emitter->initialize(*it) == false)
			{
				delete emitter;
				return false;
			}
			
			if (m_referenceObject != 0)
			{
				emitter->setPosition(pos2D);
			}
			
			m_emitters.push_back(emitter);
		}
	}
	
	m_particleLimit    = p_settings.maxParticles;
	m_isCullingEnabled = p_settings.isParticleCullingEnabled;
	
	return true;
}


void ParticleTrigger::destroyEmitters()
{
	// Destroy all emitters, releasing memory
	for (EmitterCollection::iterator it = m_emitters.begin(); it != m_emitters.end(); ++it)
	{
		ParticleEmitter* emitter = *it;
		if (emitter != 0)
		{
			// Delete emitter
			delete emitter;
		}
	}
	
	// Clear all entries
	m_emitters.clear();
}


// Namespace end
}
}
}
