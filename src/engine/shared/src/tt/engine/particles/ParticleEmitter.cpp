#include <algorithm>
#include <tt/code/helpers.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/particles/ParticleEmitter.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/fs/utils/utils.h>
#include <tt/math/math.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Rect.h>
#include <tt/str/str.h>


namespace tt {
namespace engine {
namespace particles {

bool ParticleEmitter::ms_useFileTextureCache = false;


ParticleEmitter::ParticleEmitter(TriggerID p_triggerID)
:
m_active(false),
m_pregenerating(false),
m_visible(true),
m_useValidRect(false),
m_position(math::Vector2::zero),
m_zDepth(0.0f),
m_quads(),
m_texture(),
m_triggerID(p_triggerID),
m_accumulated_time(0.0f),
m_time_alive(0.0f),
m_delay(0.0f),
m_settings(),
m_settingsScale(1.0f),
m_hasExternalImpulse(false),
m_externalImpulse(math::Vector2::zero),
m_isCulled(false),
m_boundingBox(),
m_validRect(),
m_particles()
{
}


ParticleEmitter::~ParticleEmitter()
{
	kill();
}


bool ParticleEmitter::initialize(const EmitterSettings& p_settings)
{
	// Store settings
	m_settings = p_settings;

	// FIXME: Determine during conversion
	m_settings.vertex_color_interpolation =
		m_settings.vertex_color_enabled &&
		(m_settings.particle_creation.start_color.low  != m_settings.particle_creation.start_color.high ||
		 m_settings.particle_creation.start_color.high != m_settings.particle_creation.end_color.low    ||
		 m_settings.particle_creation.end_color.low    != m_settings.particle_creation.end_color.high);

	m_particles.reserve(m_settings.max_particles);

	return initTextureSettings();
}


bool ParticleEmitter::initTextureSettings()
{
	if (ms_useFileTextureCache)
	{
		m_texture = cache::FileTextureCache::get("textures/" + m_settings.texture_filename);
	}
	else
	{
		m_texture = renderer::TextureCache::get(m_settings.assetID, true);
	}
	
	if(m_texture == 0)
	{
#ifndef TT_BUILD_FINAL
		TT_PANIC("ParticleEmitter: Failed to load texture '%s', AssetID '%s.%s'.\n", 
			m_settings.texture_filename.c_str(), 
			m_settings.assetID.getName().c_str(),
			m_settings.assetID.getNamespace().c_str());
#endif
		return false;
	}

	//Set addres mode to clamp
	using namespace engine::renderer;
	m_texture->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);

	// Set m_texture filtering
	m_texture->setMinificationFilter (renderer::FilterMode_Linear);
	m_texture->setMagnificationFilter(renderer::FilterMode_Linear);

	// Create buffer to hold quads
	m_quads = renderer::QuadBufferPtr(new renderer::QuadBuffer(m_settings.max_particles, m_texture, 
		(tt::engine::renderer::BatchFlagQuad)(renderer::BatchFlagQuad_EmbedGpuBuffer | (m_settings.vertex_color_enabled ? renderer::BatchFlagQuad_UseVertexColor : renderer::BatchFlagQuad_None))));

	return initAnimationSettings();
}


bool ParticleEmitter::initAnimationSettings()
{
	s32 texWidth(m_texture->getWidth());
	s32 texHeight(m_texture->getHeight());

	// Get cell properties
	if(m_settings.frame_info.cell_size <= 0 || m_settings.frame_info.cell_size >= texWidth)
	{
		// No animation in texture 
		m_settings.frame_info.cell_size = static_cast<s16>(texWidth);
		m_settings.animation_enabled = false;
	}
	else
	{
		m_settings.animation_enabled = true;
	}

	// Compute nr of cells in both directions
	m_settings.frame_info.cells_x = static_cast<s16>(texWidth  / m_settings.frame_info.cell_size);
	m_settings.frame_info.cells_y = static_cast<s16>(texHeight / m_settings.frame_info.cell_size);

	if(m_settings.frame_info.cells_x <= 1 && m_settings.frame_info.cells_y <= 1)
	{
		m_settings.animation_enabled = false;
	}

	// HACK: support for non square textures but not for animations
	m_settings.heightScale =
		m_settings.animation_enabled ? real(1) : static_cast<real>(texHeight) / texWidth;

	if(m_settings.animation_enabled == false)
	{
		// (no need to setup animation variables)
		return true;
	}

	// Compute frame size in texture coordinate space
	m_settings.frame_info.tex_size.x = static_cast<real>(m_settings.frame_info.cell_size) / texWidth;
	m_settings.frame_info.tex_size.y = static_cast<real>(m_settings.frame_info.cell_size) / texHeight;
	
	// Check animation settings -- TODO: this could be optimized by moving sanity checks to
	// loading code!!!
	for(ParticleAnimationContainer::iterator iter = m_settings.animations.begin();
		iter != m_settings.animations.end(); ++iter)
	{
		if ((*iter).end_frame >= (m_settings.frame_info.cells_x * m_settings.frame_info.cells_y))
		{
			// HACK: warn instead of panic
			TT_WARN("Attribute 'end' in <Animation> exceeds or is equal to "
			         "the total number of frames in texture. Texture name: '%s'", 
			         m_settings.texture_filename.c_str());
			
			(*iter).end_frame = static_cast<s16>((m_settings.frame_info.cells_x *
								 m_settings.frame_info.cells_y) - 1);
		}
	}
	
	return true;
}


ParticleEmitter* ParticleEmitter::clone() const
{
	return new ParticleEmitter(*this);
}


void ParticleEmitter::update(real p_delta_time)
{
	if (m_delay > 0.0f)
	{
		m_delay -= p_delta_time;
		
		if(m_delay > 0.0f) return;
		
		// Finished delay -> pregenerate if needed
		if (m_settings.emission.pregeneration_time > 0.0f)
		{
			pregenerateParticles();
		}
	}
	
	if (m_isCulled)
	{
		return;
	}
	
	// If this emitter is active it might need to create new particles
	if (m_active)
	{
		// Compute accumulated time
		m_accumulated_time += p_delta_time;
		
		switch(m_settings.emission.type)
		{
			case EmissionBehavior::EmissionType_Burst:
			{
				// Emit all particles at once
				emit(static_cast<s32>(m_settings.emission.particles));
				
				// Stop emitting
				m_active = false;
				break;
			}
			
			case EmissionBehavior::EmissionType_Timed:
			{
				m_time_alive += p_delta_time;
				
				if(m_time_alive > m_settings.emission.lifetime)
				{
					// Stop emitting
					m_active = false;
					break;
				}
			}
			
			// NOTE: intentional fall-through if timed emitter is active
			
			case EmissionBehavior::EmissionType_Continuous:
			{
				// Handle per-frame emissions
				if(m_settings.emission.particles > 0.0f)
				{
					// Compute nr of particles to generate
					s32 particles = static_cast<s32>(m_settings.emission.particles * m_accumulated_time);
					
					// Update remaining time
					m_accumulated_time -= (1.0f / m_settings.emission.particles) * particles;
					
					// Emit new particles
					emit(particles);
				}
				break;
			}
			
			default:
				TT_PANIC("Invalid emission type");
		}
	}
	
	s32 deadParticles(0);
	
	// Update particles
	math::VectorRect validRect(m_validRect);
	if(m_useValidRect && m_settings.inWorldSpace)
	{
		validRect.translate(m_position);
	}
	
	// NOTE: std::max() returns incorrect results if using std::numeric_limits here
	math::Vector2 minPosition( 1000000.0f,  1000000.0f);
	math::Vector2 maxPosition(-1000000.0f, -1000000.0f);
	
	for (Particles::iterator it = m_particles.begin(); it != m_particles.end(); )
	{
		// Compute position in lifetime
		real life = 1.0f - (it->energy / it->lifetime);
		
		// Update particle size
		it->size = interpolate(it->start_size, it->end_size, life);
		
		// Compute new scale
		it->scale.x = it->size;
		it->scale.y = it->size * m_settings.heightScale;
		
		// Calculate current weight
		it->weight = interpolate(it->start_weight, it->end_weight, life);
		
		// Compute external force for this particle
		math::Vector2 externalForce(
			getRandom(m_settings.external_force_x), getRandom(m_settings.external_force_y));
		
		const real deltaTimeScale = (60 * p_delta_time);
		externalForce *= m_settingsScale * deltaTimeScale;
		
		// Calculate new velocity
		it->velocity.x += externalForce.x * it->weight;
		it->velocity.y += externalForce.y * it->weight;
		it->velocity.x -= it->velocity_friction * it->velocity.x * deltaTimeScale;
		it->velocity.y -= it->velocity_friction * it->velocity.y * deltaTimeScale;
		
		// Update the particle offset
		it->position.x += (it->velocity.x * p_delta_time);
		it->position.y += (it->velocity.y * p_delta_time);
		
		// Update bounding box
		const real scaleFactor = std::max(it->scale.x, it->scale.y) * 0.5f;
		math::Vector2 curMinPosition(it->position.x - scaleFactor, it->position.y - scaleFactor);
		math::Vector2 curMaxPosition(it->position.x + scaleFactor, it->position.y + scaleFactor);
		
		minPosition.x = std::min(curMinPosition.x, minPosition.x);
		minPosition.y = std::min(curMinPosition.y, minPosition.y);
		maxPosition.x = std::max(curMaxPosition.x, maxPosition.x);
		maxPosition.y = std::max(curMaxPosition.y, maxPosition.y);
		
		// Check if still in valid zone
		if(m_useValidRect && validRect.intersects(math::VectorRect(curMinPosition, curMaxPosition)) == false)
		{
			it->energy = 0.0f;
		}
		
		// Update rotation
		if (m_settings.orientation == ParticleOrientation_ToRotation)
		{
			it->rotation_speed += (it->rotation_force * it->weight) * deltaTimeScale;
			it->rotation_speed -= it->rotation_friction * it->rotation_speed * deltaTimeScale;
			it->rotation += it->rotation_speed * p_delta_time;
		}
		else if(m_settings.orientation == ParticleOrientation_Sway)
		{
			// FIXME: Ideally we would use a dedicated random offset to start the rotation at
			//        This value could be computed once, but we would have to add another member to Particle
			const Range<real>& rotationRange = m_settings.particle_creation.start_rotation;
			real offset = it->start_rotation - rotationRange.low / rotationRange.high - rotationRange.low;
			
			it->rotation = math::sin(it->energy * it->rotation_speed + (offset * math::twoPi)) * it->start_rotation;
		}
		else
		{
			math::Vector2 direction(it->velocity);
			math::Vector2 origin(m_settings.origin);
			if (m_settings.inWorldSpace)
			{
				origin += m_position;
			}
			
			switch(m_settings.orientation)
			{
			case ParticleOrientation_ToDirection:
				// Already set direction to velocity
				break;
			
			case ParticleOrientation_ToOrigin:
				direction = origin - it->position;
				break;
			
			case ParticleOrientation_FromOrigin:
				direction = it->position - origin;
				break;
			
			default:
				TT_PANIC("Invalid particle orientation");
			}
			
			math::clamp(direction.normalize().y, -1.0f, 1.0f);
			it->rotation = math::acos(direction.y);
			if(direction.x > 0) it->rotation = -it->rotation;
		}
		
		// Update animation 
		if(m_settings.animation_enabled)
		{
			// Update animation time
			it->animation_time += p_delta_time;
			
			// Update current frame
			ParticleAnimation& anim(m_settings.animations[it->anim_idx]);
			if(anim.type == ParticleAnimation::AnimationType_Stretch)
			{
				it->frame = static_cast<s32>(
					(it->animation_time / it->lifetime) * (anim.end_frame - anim.start_frame + 1));
			}
			else
			{
				if (anim.spf > 0.0f)
				{
					while(it->animation_time > anim.spf)
					{
						if(anim.type == ParticleAnimation::AnimationType_PingPong && it->frame == anim.start_frame)
						{
							++(it->frame);
							anim.forward = true;
						}
						else if(it->frame == anim.end_frame)
						{
							if(anim.type == ParticleAnimation::AnimationType_Loop)
							{
								it->frame = anim.start_frame;
							}
							else if(anim.type == ParticleAnimation::AnimationType_PingPong)
							{
								TT_ASSERT(anim.end_frame != anim.start_frame);
								--(it->frame);
								anim.forward = false;
							}
						}
						else
						{
							anim.forward ? ++(it->frame) : --(it->frame);
						}
						
						it->animation_time -= anim.spf;
					}
				}
			}
			
			// Update texture transform
			const FrameInfo& frame(m_settings.frame_info); 
			it->tex_transform.x = (it->frame % frame.cells_x) * frame.tex_size.x;
			it->tex_transform.y = (it->frame / frame.cells_x) * frame.tex_size.y;
		}
		
		// Decrease energy
		it->energy -= p_delta_time;
		
		// Check for dead particles
		if (it->energy <= 0.0f)
		{
			// Remove particle
			it = code::helpers::unorderedErase(m_particles, it);
			++deadParticles;
		}
		else
		{
			// Go to next particle
			++it;
		}
	}
	
	if(deadParticles > 0)
	{
		ParticleMgr::getInstance()->releaseParticles(m_triggerID, deadParticles);
	}
	
	// Set bounding box
	if (m_particles.empty() == false)
	{
		m_boundingBox = math::VectorRect(minPosition, maxPosition);
		if (m_settings.inWorldSpace)
		{
			m_boundingBox.translate(-m_position);
		}
	}
	
	if (m_pregenerating == false)
	{
		createQuadBatch();
	}
}


void ParticleEmitter::updateForRender(const math::VectorRect&)
{
}


void ParticleEmitter::render()
{
	if(m_visible == false || m_particles.empty() || m_isCulled) return;
	
	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	
	// Fill buffer
	{
		TT_ASSERT(m_quadBatch.size() >= m_particles.size());
		renderer::BatchQuadCollection::iterator quadIt = m_quadBatch.begin() + m_particles.size();
		m_quads->fillBuffer(m_quadBatch.begin(), quadIt);
	}
	
	renderer->setBlendMode(m_settings.blend_mode);
	
	// Remember blend mode alpha so we can restore it.
	using engine::renderer::BlendFactor;
	const bool        prevSeparateAlphaBlendEnabled = renderer->hasSeparateAlphaBlendEnabled();
	const BlendFactor prevCustomBlendModeAlphaSrc   = renderer->getCustomBlendModeAlphaSrc();
	const BlendFactor prevCustomBlendModeAlphaDst   = renderer->getCustomBlendModeAlphaDst();
	// Check if we need to override blend mode alpha.
	if (isValidBlendModeAlpha(m_settings.blend_mode_alpha))
	{
		renderer->setBlendModeAlpha(m_settings.blend_mode_alpha);
	}
	
	// Save fog enabled setting
	const bool fogEnabled = renderer->isFogEnabled();
	
	// Enable/disable fog based on blend mode, only if fog is enabled in the renderer
	if (fogEnabled)
	{
		renderer->setFogEnabled(m_settings.ignore_fog == false);
	}
	
	
	m_quads->render();
	
	// Restore fog enabled setting
	renderer->setFogEnabled(fogEnabled);
	
	// Restore blend mode alpha.
	if (prevSeparateAlphaBlendEnabled)
	{
		renderer->setCustomBlendModeAlpha(prevCustomBlendModeAlphaSrc, prevCustomBlendModeAlphaDst);
	}
	else
	{
		renderer->resetCustomBlendModeAlpha();
	}
}


void ParticleEmitter::setInitialExternalImpulse(real p_angle, real p_power)
{
	const real weight = getRandom(m_settings.external_impulse_weight);
	m_hasExternalImpulse = p_power > 0.0f && weight > 0.0f;
	
	if (m_hasExternalImpulse)
	{
		// Offset with 90 so angle 0 produces an upwards motion
		m_externalImpulse.x = math::cos(math::degToRad(p_angle + 90.0f)) * p_power * weight;
		m_externalImpulse.y = math::sin(math::degToRad(p_angle + 90.0f)) * p_power * weight;
		
		if (m_settings.directionFromSpawnLocation == false)
		{
			// Update x 
			{
				const real curMidpoint = (m_settings.velocity_x.low + m_settings.velocity_x.high) / 2.0f;
				m_settings.velocity_x.low  += m_externalImpulse.x - curMidpoint;
				m_settings.velocity_x.high += m_externalImpulse.x - curMidpoint;
			}
			
			// Update y (offset with 90 so angle 0 produces an upwards motion)
			{
				const real curMidpoint = (m_settings.velocity_y.low + m_settings.velocity_y.high) / 2.0f;
				m_settings.velocity_y.low  += m_externalImpulse.y - curMidpoint;
				m_settings.velocity_y.high += m_externalImpulse.y - curMidpoint;
			}
		}
	}
}


void ParticleEmitter::activate()
{
	// Reset timers
	m_accumulated_time = 0.0f;
	m_time_alive = 0.0f;
	
	// FIXME: Need a better handling of trigger wide delay
	m_delay = std::max(m_delay, m_settings.emitter_delay);
	
	// Activate
	m_active = true;
	
	// Pregenerate particles if needed. If this emitter is delayed, don't pre-generate now
	if(m_settings.emission.pregeneration_time > 0.0f && math::realEqual(m_delay,0.0f))
	{
		pregenerateParticles();
	}
}


void ParticleEmitter::kill()
{
	// Stop emitting
	m_active = false;
	
	// Kill all active particles
	ParticleMgr::getInstance()->releaseParticles(m_triggerID, static_cast<s32>(m_particles.size()));
	m_particles.clear();
}


engine::renderer::TexturePtr ParticleEmitter::getAndLoadAllUsedTextures() const
{
	return m_texture;
}


void ParticleEmitter::changeMaxParticles(s32 p_maxParticles)
{
	// FIXME: Change max particles in emitter settings to s32
	if (p_maxParticles <= 0)
	{
		p_maxParticles = 1;
	}
	
	if(p_maxParticles > m_settings.max_particles)
	{
		m_quads.reset(new renderer::QuadBuffer(p_maxParticles, m_texture,
		m_settings.vertex_color_enabled ? renderer::BatchFlagQuad_UseVertexColor : renderer::BatchFlagQuad_None));
	}
	m_settings.max_particles = static_cast<s16>(p_maxParticles);

	if (m_particles.size() > static_cast<u32>(m_settings.max_particles))
	{
		m_particles.clear();
	}
}


void ParticleEmitter::renderEmissionArea(const renderer::ColorRGBA& p_color) const
{
	if(m_visible == false) return;

	const debug::DebugRendererPtr dbg(renderer::Renderer::getInstance()->getDebug());
	const math::Vector2 emitterPosition(m_settings.origin.x, m_settings.origin.y);

	switch(m_settings.emission.area_type)
	{
	case EmissionBehavior::AreaType_Circle:
	{
		dbg->renderCircle(p_color, emitterPosition, m_settings.emission.radius);
		dbg->renderCircle(p_color, emitterPosition, m_settings.emission.innerRadius);
		break;
	}

	case EmissionBehavior::AreaType_Rectangle:
	{
		math::VectorRect emitterRect;
		emitterRect.setWidth(m_settings.emission.rect_width.high   - m_settings.emission.rect_width.low);
		emitterRect.setHeight(m_settings.emission.rect_height.high - m_settings.emission.rect_height.low);
		emitterRect.setCenterPosition(emitterPosition);

		dbg->renderRect(p_color, emitterRect);
		break;
	}

	default:
		TT_PANIC("Invalid area type: %s", getAreaTypeName(m_settings.emission.area_type));
	}
}


void ParticleEmitter::renderBoundingBox(const renderer::ColorRGBA& p_color) const
{
	if(m_visible == false || m_particles.empty()) return;

	const debug::DebugRendererPtr dbg(renderer::Renderer::getInstance()->getDebug());
	dbg->renderRect(p_color, getBoundingBox());
}


void ParticleEmitter::setValidRect(const math::VectorRect* p_rect)
{
	if(p_rect == 0)
	{
		m_useValidRect = false;
	}
	else
	{
		m_validRect = *p_rect;
		m_useValidRect = true;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void ParticleEmitter::emit(s32 p_particleCount)
{
	s32 particlesToEmit = ParticleMgr::getInstance()->requestParticles(m_triggerID, p_particleCount);
	
	while (particlesToEmit > 0 && 
	       m_particles.size() < static_cast<u32>(m_settings.max_particles))
	{
		// Initialize new particle
		initializeParticle();
		
		// Decrease nr of particles to create
		--particlesToEmit;
	}
	
	// release the unused particles if there are any
	if(particlesToEmit > 0)
	{
		ParticleMgr::getInstance()->releaseParticles(m_triggerID, particlesToEmit);
	}
}


void ParticleEmitter::initializeParticle()
{
	const EmitterSettings& settings(m_settings);
	const ParticleCreationProperties& pc(settings.particle_creation);

	// Set initial lifetime
	real lifetime = getRandom(pc.lifetime);
	if(lifetime <= 0.0f) return;
	
	// Create new particle
	Particle particle;
	
	particle.lifetime = lifetime;
	particle.energy   = particle.lifetime;
	
	// Determine starting position
	particle.position = settings.origin;
	if(settings.inWorldSpace)
	{
		particle.position += m_position;
	}
	
	// Adjust position based on emitter area
	switch(settings.emission.area_type)
	{
		case EmissionBehavior::AreaType_Circle:
		{
			// we're going to get a random point from a trapezoid, then bend it out into a donut.
			// circles are handled by having a zero-length side on the trapezoid.
			// this allows us to (seemingly, at least) evenly distribute points across the surface...
			// ...the old implementation bunched them towards the middle.
			
			// if the spawner is a single point, we get divide-by-zero errors. this SHOULD mitigate that.
			if (settings.emission.radius != 0.0f || settings.emission.innerRadius != 0.0f)
			{
				const real radMin = std::min<real>(settings.emission.radius, settings.emission.innerRadius);
				const real radMax = std::max<real>(settings.emission.radius, settings.emission.innerRadius);
				
				const real circOuter = math::twoPi;
				const real circInner = math::twoPi * (radMin / radMax);
				
				// random point within a trapezoid, height 1, width ranging from circInner to circOuter.
				// we make a rectangle from two trapezoids:
				// any points in the right-hand one get folded back into the left-hand one.
				
				//  circOuter       circInner
				// .--------------/----------.
				// |  e          /           |
				// |            /            | 1
				// |           /          f  |
				// '----------/--------------'
				//  circInner       circOuter
				//
				// in the above diagram, f will get folded over onto the location of e.
				
				real x = (circInner + circOuter) * getRand();
				real y = getRand();
				if (x - circInner - (y * (math::twoPi - circInner)) > 0)
				{
					x = (circInner + circOuter) - x;
					y = 1 - y;
				}
				
				// bend the trapezoid into a donut
				const real donutRadius = radMax - radMin;
				const real radius = (y * donutRadius) + radMin;
				const real angle  = x / (radius / radMax);
				
				particle.position.x += math::cos(angle) * radius;
				particle.position.y += math::sin(angle) * radius;
			}
			
			break;
		}
		
		case EmissionBehavior::AreaType_Rectangle:
		{
			particle.position.x += getRandom(settings.emission.rect_width);
			particle.position.y += getRandom(settings.emission.rect_height);
			break;
		}
		
		default:
			TT_PANIC("Unknown area type specified: %s (%d)\n",
					 getAreaTypeName(settings.emission.area_type), settings.emission.area_type);
	}
	particle.position *= m_settingsScale;
	
	// Set initial velocity
	particle.velocity.x = getRandom(settings.velocity_x);
	particle.velocity.y = getRandom(settings.velocity_y);
	particle.velocity *= m_settingsScale;
	
	particle.velocity_friction = getRandom(settings.velocity_friction);
	
	if (settings.directionFromSpawnLocation)
	{
		math::Vector2 spawnLocation(m_settings.origin);
		
		if(settings.inWorldSpace)
		{
			spawnLocation += m_position;
		}
		
		// Set velocity based on spawn position
		math::Vector2 direction(particle.position - spawnLocation);
		direction.normalize();
		
		particle.velocity = particle.velocity.length() * direction;
		if (m_hasExternalImpulse)
		{
			particle.velocity += m_externalImpulse;
		}
	}
	
	// Set start & end weight
	particle.start_weight = getRandom(pc.start_weight);
	particle.end_weight   = getRandom(pc.end_weight);
	particle.weight = particle.start_weight;
	
	// Set rotation parameters - convert to radians here
	particle.rotation          = math::degToRad(getRandom(pc.start_rotation));
	particle.rotation_speed    = math::degToRad(getRandom(pc.rotation_speed));
	particle.rotation_force    = getRandom(pc.rotation_force);
	particle.rotation_friction = getRandom(pc.rotation_friction);
	particle.start_rotation    = particle.rotation;
	
	// Set start & end size
	particle.start_size = getRandom(pc.start_size) * m_settingsScale;
	
	if(pc.use_end_size)
	{
		particle.end_size = getRandom(pc.end_size) * m_settingsScale;
	}
	else
	{
		particle.end_size = particle.start_size;
	}
	particle.size = particle.start_size;
	particle.scale.x = particle.size;
	particle.scale.y = particle.size * settings.heightScale;
	
	// Compute Fade
	if(pc.use_fade_as_time)
	{
		// Convert time to percentage
		particle.fade_in  = pc.fade_in  / particle.lifetime;
		particle.fade_out = pc.fade_out / particle.lifetime;
	}
	else
	{
		particle.fade_in  = pc.fade_in;
		particle.fade_out = pc.fade_out;
	}
	
	// Set start & end color
	if(settings.vertex_color_enabled)
	{
		particle.start_color = getRandom(pc.start_color);
		particle.end_color   = getRandom(pc.end_color);
	}
	
	// Set initial values
	particle.anim_idx       = 0;
	particle.frame          = 0;
	particle.animation_time = 0.0f;
	
	// Animation
	if(settings.animation_enabled)
	{
		// Pick animation if needed
		if(settings.animations.size() > 1)
		{
			// Get random animation index
			u32 random = math::Random::getEffects().getNext(std::numeric_limits<u32>::max());
			particle.anim_idx = random % settings.animations.size();
		}
		else
		{
			particle.anim_idx = 0;
		}
		
		TT_ASSERT(settings.animations.size() > particle.anim_idx);
		ParticleAnimation& anim(m_settings.animations[particle.anim_idx]);
		
		// Determine start frame
		u32 startFrameOffset = math::Random::getEffects().getNext(
		    static_cast<u32>(pc.start_frame.low),
		    static_cast<u32>(pc.start_frame.high + 1));
		
		particle.frame = anim.start_frame + startFrameOffset;
		if(particle.frame > anim.end_frame) particle.frame = anim.end_frame;
		
		// Stretch spf if needed, always start at first frame
		if(settings.animations[particle.anim_idx].type == ParticleAnimation::AnimationType_Stretch)
		{
			particle.frame = anim.start_frame;
		}
		
		// Setup texture transform
		particle.tex_transform.x = (particle.frame % settings.frame_info.cells_x) * 
								    settings.frame_info.tex_size.x;
		particle.tex_transform.y = (particle.frame / settings.frame_info.cells_x) * 
								    settings.frame_info.tex_size.y;
		particle.tex_transform.z = 0.0f;
	}
	else
	{
		particle.tex_transform = math::Vector3::zero;
	}
	
	m_particles.push_back(particle);
}


renderer::ColorRGBA ParticleEmitter::interpolate(const renderer::ColorRGBA& p_start,
	                                             const renderer::ColorRGBA& p_end,
	                                             real p_life) const
{
	renderer::ColorRGBA result;
	
	// Interpolate each color channel
	result.r = static_cast<u8>(interpolate(static_cast<real>(p_start.r), static_cast<real>(p_end.r), p_life));
	result.g = static_cast<u8>(interpolate(static_cast<real>(p_start.g), static_cast<real>(p_end.g), p_life));
	result.b = static_cast<u8>(interpolate(static_cast<real>(p_start.b), static_cast<real>(p_end.b), p_life));
	result.a = static_cast<u8>(interpolate(static_cast<real>(p_start.a), static_cast<real>(p_end.a), p_life));
	
	return result;
}


void ParticleEmitter::pregenerateParticles()
{
	static const s32 maxPregenerateUpdates = 250;
	
	real time_left = m_settings.emission.pregeneration_time;
	 
	m_pregenerating = true;
	
	/*TT_Printf("[PREGENERATE] Simulating %f seconds, using update step (%f)\n",
		m_settings.emission.pregeneration_time, m_settings.emission.pregeneration_step);*/
	
	if(m_settings.emission.pregeneration_time / m_settings.emission.pregeneration_step > maxPregenerateUpdates)
	{
		//TT_WARN("Excessive pregeneration specified: adjusting pregeneration step.");
		m_settings.emission.pregeneration_step =
			m_settings.emission.pregeneration_time / maxPregenerateUpdates;
	}
	
	while(time_left > 0.0f)
	{
		update(m_settings.emission.pregeneration_step);
		time_left -= m_settings.emission.pregeneration_step;
	}
	
	createQuadBatch();
	
	m_pregenerating = false;
}


void ParticleEmitter::createQuadBatch()
{
	if(m_quadBatch.size() < m_particles.size())
	{
		m_quadBatch.resize(m_particles.size());
	}
	
	renderer::BatchQuadCollection::iterator quadIt = m_quadBatch.begin();
	
	for (Particles::reverse_iterator it = m_particles.rbegin(); it != m_particles.rend(); ++it)
	{
		// Generate batch quad (could be optimized by creating a quad with default settings)
		renderer::BatchQuad& quad = *quadIt;
		
		math::Matrix44 transform = math::Matrix44::getTranslation(math::Vector3(it->position.x, it->position.y, m_zDepth));
		if(m_settings.inWorldSpace == false)
		{
			transform.translate(m_position);
		}
		
		if(math::realEqual(it->rotation, 0.0f) == false)
		{
			transform.rotateZ(it->rotation);
		}
		transform.scale(it->scale.x, it->scale.y);
		
		static const math::Vector3 topLeft    (-0.5f,  0.5f, 0.0f);
		static const math::Vector3 topRight   ( 0.5f,  0.5f, 0.0f);
		static const math::Vector3 bottomLeft (-0.5f, -0.5f, 0.0f);
		static const math::Vector3 bottomRight( 0.5f, -0.5f, 0.0f);
		
		quad.topLeft.setPosition    (topLeft     * transform);
		quad.topRight.setPosition   (topRight    * transform);
		quad.bottomLeft.setPosition (bottomLeft  * transform);
		quad.bottomRight.setPosition(bottomRight * transform);
		
		// Compute texture coordinates
		real leftU   = 0;
		real rightU  = 1;
		real topV    = 0;
		real bottomV = 1;
		
		if(m_settings.animation_enabled)
		{
			leftU   = (it->tex_transform.x);
			rightU  = (it->tex_transform.x) + m_settings.frame_info.tex_size.x;
			topV    = (it->tex_transform.y);
			bottomV = (it->tex_transform.y) + m_settings.frame_info.tex_size.y;
		}
		
		if((m_settings.flipTexture & FlipAxis_X) == FlipAxis_X)
		{
			std::swap(leftU, rightU);
		}
		if((m_settings.flipTexture & FlipAxis_Y) == FlipAxis_Y)
		{
			std::swap(topV, bottomV);
		}
		
		quad.topLeft.setTexCoord    (leftU,  topV);
		quad.topRight.setTexCoord   (rightU, topV);
		quad.bottomLeft.setTexCoord (leftU,  bottomV);
		quad.bottomRight.setTexCoord(rightU, bottomV);
		
		if(m_settings.vertex_color_enabled)
		{
			renderer::ColorRGBA vertexColor(it->start_color);
			
			const real life = 1.0f - (it->energy / it->lifetime);
			if(m_settings.vertex_color_interpolation)
			{
				vertexColor = interpolate(it->start_color, it->end_color, life);
			}
			
			// Handle fade in/out
			if(life < it->fade_in)
			{
				vertexColor.a = static_cast<u8>((life / it->fade_in) * vertexColor.a);
			}
			else if(life > (1 - it->fade_out))
			{
				real fade((1.0f - life) / it->fade_out);
				vertexColor.a = static_cast<u8>(fade * vertexColor.a);
			}
			
			if(m_texture != 0 && m_texture->isPremultiplied() && m_settings.blend_mode != renderer::BlendMode_Modulate)
			{
				vertexColor.premultiply();
			}
			quad.topLeft.    setColor(vertexColor);
			quad.topRight.   setColor(vertexColor);
			quad.bottomLeft. setColor(vertexColor);
			quad.bottomRight.setColor(vertexColor);
		}
		
		++quadIt;
	}
}


ParticleEmitter::ParticleEmitter(const ParticleEmitter& p_emitter)
:
m_active(p_emitter.m_active),
m_pregenerating(p_emitter.m_pregenerating),
m_visible(p_emitter.m_visible),
m_useValidRect(p_emitter.m_useValidRect),
m_position(p_emitter.m_position),
m_quads(),
m_texture(p_emitter.m_texture),
m_triggerID(p_emitter.m_triggerID),
m_accumulated_time(p_emitter.m_accumulated_time),
m_time_alive(p_emitter.m_time_alive),
m_delay(p_emitter.m_delay),
m_settings(p_emitter.m_settings),
m_settingsScale(p_emitter.m_settingsScale),
m_hasExternalImpulse(p_emitter.m_hasExternalImpulse),
m_externalImpulse(p_emitter.m_externalImpulse),
m_isCulled(p_emitter.m_isCulled),
m_boundingBox(p_emitter.m_boundingBox),
m_validRect(p_emitter.m_validRect),
m_particles() // Gets filled below with the number of particles that are allowed.
{
	const s32 allowedCount = ParticleMgr::getInstance()->requestParticles(
			m_triggerID,
			static_cast<s32>(p_emitter.m_particles.size()));
	
	if (allowedCount > 0)
	{
		Particles::const_iterator beginIt = p_emitter.m_particles.begin();
		Particles::const_iterator endIt = beginIt;
		std::advance(endIt, allowedCount);
		m_particles.assign(beginIt, endIt);
	}
	
	// Create buffer to hold quads
	m_quads = renderer::QuadBufferPtr(new renderer::QuadBuffer(m_settings.max_particles, 
		p_emitter.m_quads->getTexture(), 
		m_settings.vertex_color_enabled ? renderer::BatchFlagQuad_UseVertexColor : renderer::BatchFlagQuad_None));
	
	// update here so that m_quads gets filled up with the particles
	update(0);
}

//--------------------------------------------------------------------------------------------------
// Enum helpers

const char* getAnimationTypeName(ParticleAnimation::AnimationType p_type)
{
	switch(p_type)
	{
	case ParticleAnimation::AnimationType_Loop    : return "loop";
	case ParticleAnimation::AnimationType_Stretch : return "stretch";
	case ParticleAnimation::AnimationType_Once    : return "once";
	case ParticleAnimation::AnimationType_PingPong: return "pingpong";
	default:
		TT_PANIC("Invalid enumeration value for animation type (%d)", p_type);
		return "";
	}
}


ParticleAnimation::AnimationType getAnimationTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < ParticleAnimation::AnimationType_Count; ++i)
	{
		ParticleAnimation::AnimationType asEnum = static_cast<ParticleAnimation::AnimationType>(i);
		if (p_name == getAnimationTypeName(asEnum))
		{
			return asEnum;
		}
	}
	return ParticleAnimation::AnimationType_Invalid;
}


const char* getEmissionTypeName(EmissionBehavior::EmissionType p_type)
{
	switch(p_type)
	{
	case EmissionBehavior::EmissionType_Burst      : return "burst";
	case EmissionBehavior::EmissionType_Continuous : return "continuous";
	case EmissionBehavior::EmissionType_Timed      : return "timed";
	default:
		TT_PANIC("Invalid enumeration value for emission type (%d)", p_type);
		return "";
	}
}


EmissionBehavior::EmissionType getEmissionTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < EmissionBehavior::EmissionType_Count; ++i)
	{
		EmissionBehavior::EmissionType asEnum = static_cast<EmissionBehavior::EmissionType>(i);
		if (p_name == getEmissionTypeName(asEnum))
		{
			return asEnum;
		}
	}
	return EmissionBehavior::EmissionType_Invalid;
}

const char* getAreaTypeName(EmissionBehavior::AreaType p_type)
{
	switch(p_type)
	{
	case EmissionBehavior::AreaType_Circle   :    return "circle";
	case EmissionBehavior::AreaType_Rectangle:    return "rectangle";
	default:
		TT_PANIC("Invalid enumeration value for area type (%d)", p_type);
		return "";
	}
}


EmissionBehavior::AreaType getAreaTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < EmissionBehavior::AreaType_Count; ++i)
	{
		EmissionBehavior::AreaType asEnum = static_cast<EmissionBehavior::AreaType>(i);
		if (p_name == getAreaTypeName(asEnum))
		{
			return asEnum;
		}
	}
	return EmissionBehavior::AreaType_Invalid;
}


const char* getParticleOrientationName(ParticleOrientation p_enum)
{
	switch(p_enum)
	{
	case ParticleOrientation_ToRotation : return "rotation";
	case ParticleOrientation_ToDirection: return "direction";
	case ParticleOrientation_ToOrigin   : return "to_origin";
	case ParticleOrientation_FromOrigin : return "from_origin";
	case ParticleOrientation_Sway       : return "sway";
	default:
		TT_PANIC("Invalid enumeration value for particle orientation (%d)", p_enum);
		return "";
	}
}


ParticleOrientation getParticleOrientationFromName(const std::string& p_name)
{
	for (s32 i = 0; i < ParticleOrientation_Count; ++i)
	{
		ParticleOrientation asEnum = static_cast<ParticleOrientation>(i);
		if (p_name == getParticleOrientationName(asEnum))
		{
			return asEnum;
		}
	}
	return ParticleOrientation_Invalid;
}


void scaleEmitterSettings(EmitterSettings& p_settings, real p_scale)
{
	p_settings.origin.x *= p_scale;
	p_settings.origin.y *= p_scale;
	
	p_settings.velocity_x.low  *= p_scale;
	p_settings.velocity_x.high *= p_scale;
	p_settings.velocity_y.low  *= p_scale;
	p_settings.velocity_y.high *= p_scale;
	
	p_settings.particle_creation.start_size.low  *= p_scale;
	p_settings.particle_creation.start_size.high *= p_scale;
	p_settings.particle_creation.end_size.low    *= p_scale;
	p_settings.particle_creation.end_size.high   *= p_scale;
	
	p_settings.external_force_x.low  *= p_scale;
	p_settings.external_force_x.high *= p_scale;
	p_settings.external_force_y.low  *= p_scale;
	p_settings.external_force_y.high *= p_scale;
	
	p_settings.emission.rect_width.high *= p_scale;
	p_settings.emission.rect_width.low  *= p_scale;
	
	p_settings.emission.rect_height.high *= p_scale;
	p_settings.emission.rect_height.low  *= p_scale;
	p_settings.emission.radius           *= p_scale;
	p_settings.emission.innerRadius      *= p_scale;
}


void flipEmitterSettings(EmitterSettings& p_settings, u32 p_flip)
{
	real x = ((p_flip & FlipAxis_X) == 0) ? 1.0f : -1.0f;
	real y = ((p_flip & FlipAxis_Y) == 0) ? 1.0f : -1.0f;
	
	p_settings.flipTexture ^= p_flip; // This is to get the same behavior as above. We're not setting the flip value, we're flipping each time the function is called.
	
	{
		ParticleCreationProperties& pc(p_settings.particle_creation);
		pc.start_rotation.high *= x;
		pc.start_rotation.low  *= x;
		
		pc.rotation_speed.high *= x;
		pc.rotation_speed.low  *= x;
		
		pc.rotation_force.high *= x;
		pc.rotation_force.low  *= x;
		
		// FIXME: Y flip doesn't change rotation settings.
		//        Though fliping the rotation here might not be the best thing either.
		//        It might be better to keep all these settings and think of them as local space.
		//        Then during rendering convert local space to global based on the flip settings.
	}
	
	p_settings.origin.x *= x;
	p_settings.origin.y *= y;
	
	p_settings.velocity_x.low  *= x;
	p_settings.velocity_x.high *= x;
	p_settings.velocity_y.low  *= y;
	p_settings.velocity_y.high *= y;
	
	p_settings.external_force_x.low  *= x;
	p_settings.external_force_x.high *= x;
	p_settings.external_force_y.low  *= y;
	p_settings.external_force_y.high *= y;
}


bool convertFilenameToEngineID(EmitterSettings& p_settings)
{
	const std::string::value_type dirSeparator = fs::getDirSeparator();
	
	std::string filename(p_settings.texture_filename);
	if(filename.empty()) return false;
			
	TT_ASSERTMSG(str::endsWith(filename, ".png"),
		"Texturename '%s' doesn't end with '.png'. Currently only PNG is supported", filename.c_str());
	
	filename = fs::utils::compactPath("textures/" + filename, "\\/");
	if (filename.empty())
	{
		TT_WARN("Invalid filename %s.", p_settings.texture_filename.c_str());
		return false;
	}
	
	// nameSpace output example: "textures.menu"
	std::string assetNamespace = filename.substr(0, filename.rfind(dirSeparator));
	std::replace(assetNamespace.begin(), assetNamespace.end(), dirSeparator, '.');
	
	std::string::size_type min = filename.rfind(dirSeparator);
	min += 1; // min will never be std::string::npos
	
	std::string::size_type max = filename.rfind('.');
	std::string::size_type length = std::string::npos;
	if (max >= min)
	{
		length = max - min;
	}
	
	// output example: "menucursor"
	std::string assetName = filename.substr(min, length);
	
	p_settings.assetID = EngineID(assetName, assetNamespace);
	
#if defined(TT_BUILD_FINAL)
		p_settings.texture_filename.clear();
#endif
	
	return true;
}



// Namespace end
}
}
}
