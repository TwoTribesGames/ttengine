#if !defined(INC_TT_ENGINE_PARTICLES_PARTICLEEMITTER)
#define INC_TT_ENGINE_PARTICLES_PARTICLEEMITTER


#include <string>
#include <list>
#include <vector>
#include <limits>

#include <tt/engine/particles/fwd.h>
#include <tt/engine/particles/Particle.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/Random.h>


namespace tt {
namespace engine {
namespace particles {

// Structure for storing a range
template <typename T> struct Range 
{
	T low;
	T high;
	
	inline Range()
	:
	low(),
	high()
	{ }
};

// Structure defining an animation
struct ParticleAnimation
{
	enum AnimationType
	{
		AnimationType_Loop,
		AnimationType_Stretch,
		AnimationType_Once,
		AnimationType_PingPong,
		
		AnimationType_Count,
		AnimationType_Invalid
	};
	
	s16           start_frame;
	s16           end_frame;
	real          spf; // seconds per frame
	AnimationType type;
	bool          forward;
	
	inline ParticleAnimation()
	:
	start_frame(0),
	end_frame(0),
	spf(0.0f),
	type(AnimationType_Once),
	forward(true)
	{ }
};

// Container for holding particle animations
typedef std::vector<ParticleAnimation> ParticleAnimationContainer;

// Behaviour of emitting particles
struct EmissionBehavior
{
	enum EmissionType
	{
		EmissionType_Burst,
		EmissionType_Continuous,
		EmissionType_Timed,
		
		EmissionType_Count,
		EmissionType_Invalid
	};
	
	EmissionType type;
	real particles;
	real lifetime;
	
	// Emitter area
	enum AreaType
	{
		AreaType_Circle,
		AreaType_Rectangle,
	
		AreaType_Count,
		AreaType_Invalid
	};
	
	AreaType area_type;
	real radius;
	real innerRadius;
	Range<real> rect_width;
	Range<real> rect_height;
	
	real pregeneration_time;
	real pregeneration_step;
	
	inline EmissionBehavior()
	:
	type(EmissionType_Burst),
	particles(0.0f),
	lifetime(0.0f),
	area_type(AreaType_Rectangle),
	radius(0.0f),
	innerRadius(0.0f),
	rect_width(),
	rect_height(),
	pregeneration_time(0.0f),
	pregeneration_step(0.0f)
	{ }
};

// Behavior of creating particles
struct ParticleCreationProperties
{
	Range<real> lifetime;
	Range<real> start_size;
	Range<real> end_size;
	Range<real> start_weight;
	Range<real> end_weight;
	Range<real> start_rotation;
	Range<real> rotation_speed;
	Range<real> rotation_force;
	Range<real> rotation_friction;
	Range<s16>  start_frame;
	Range<renderer::ColorRGBA> start_color;
	Range<renderer::ColorRGBA> end_color;
	real fade_in;
	real fade_out;
	bool use_end_size;
	bool use_fade_as_time;
	
	inline ParticleCreationProperties()
	:
	lifetime(),
	start_size(),
	end_size(),
	start_weight(),
	end_weight(),
	start_rotation(),
	rotation_speed(),
	rotation_force(),
	rotation_friction(),
	start_frame(),
	start_color(),
	end_color(),
	fade_in(0.0f),
	fade_out(0.0f),
	use_end_size(true),
	use_fade_as_time(false)
	{ }
};

// Information for animations in a texture
struct FrameInfo
{
	s16 cell_size;
	s16 cells_x;
	s16 cells_y;
	
	math::Vector2 tex_size;
	
	inline FrameInfo()
	:
	cell_size(0),
	cells_x(0),
	cells_y(0),
	tex_size(0.0f, 0.0f)
	{ }
};

enum ParticleOrientation
{
	ParticleOrientation_ToRotation,
	ParticleOrientation_ToDirection,
	ParticleOrientation_ToOrigin,
	ParticleOrientation_FromOrigin,
	ParticleOrientation_Sway,
	
	ParticleOrientation_Count,
	ParticleOrientation_Invalid
};

enum FlipAxis
{
	FlipAxis_None = 0,
	FlipAxis_X = 0x1,
	FlipAxis_Y = 0x2
};


struct EmitterSettings
{
	// Global Emitter Properties
	s32 max_particles;
	s32 render_group;
	math::Vector2 origin;
	real heightScale;
	
	std::string texture_filename;
	EngineID    assetID;
	bool vertex_color_enabled;
	bool vertex_color_interpolation;
	
	bool animation_enabled;
	FrameInfo frame_info;
	ParticleAnimationContainer animations;
	
	real emitter_delay;
	
	bool        orientate_to_external_impulse;
	Range<real> external_impulse_weight;
	Range<real> external_force_x;
	Range<real> external_force_y;
	Range<real> velocity_x;
	Range<real> velocity_y;
	Range<real> velocity_friction;
	
	renderer::BlendMode      blend_mode;
	renderer::BlendModeAlpha blend_mode_alpha;
	bool                     ignore_fog;
	
	EmissionBehavior emission;
	
	// Particle Creation
	ParticleCreationProperties particle_creation;
	
	ParticleOrientation orientation;
	bool                inWorldSpace;
	bool                directionFromSpawnLocation;
	u32                 flipTexture;
	
	inline EmitterSettings()
	:
	max_particles(0),
	render_group(-1),
	origin(math::Vector2::zero),
	heightScale(0.0f),
	texture_filename(),
	assetID(0,0),
	vertex_color_enabled(false),
	vertex_color_interpolation(false),
	animation_enabled(false),
	frame_info(),
	animations(),
	emitter_delay(0.0f),
	external_impulse_weight(),
	external_force_x(),
	external_force_y(),
	velocity_x(),
	velocity_y(),
	velocity_friction(),
	blend_mode(renderer::BlendMode_Blend),
	blend_mode_alpha(renderer::BlendModeAlpha_NoOverride),
	ignore_fog(false),
	emission(),
	particle_creation(),
	orientation(ParticleOrientation_ToRotation),
	inWorldSpace(true),
	directionFromSpawnLocation(false),
	flipTexture(FlipAxis_None)
	{ }
};
typedef std::vector<EmitterSettings> EmitterSettingsCollection;

void scaleEmitterSettings(EmitterSettings& p_settings, real p_scale);


void flipEmitterSettings(EmitterSettings& p_settings, u32 p_flip);

bool convertFilenameToEngineID(EmitterSettings& p_settings);


class ParticleEmitter
{
public:
	/*! \brief Create a particle emitter with the supplied settings */
	explicit ParticleEmitter(TriggerID p_triggerID);
	
	~ParticleEmitter();
	
	bool initialize(const EmitterSettings& p_settings);
	
	bool initTextureSettings();
	
	bool initAnimationSettings();
	
	/*! \brief Returns exact copy of original */
	ParticleEmitter* clone() const;
	
	/*! \brief Run simulation for particles */
	void update(real p_delta_time);
	
	/*! \brief Prepare render resources for particles */
	void updateForRender(const tt::math::VectorRect& p_visibilityRect);
	
	/*! \brief Render the particles */
	void render();
	
	inline void setPosition(const math::Vector2& p_position) { m_position = p_position; }
	inline const math::Vector2& getPosition() const { return m_position; }
	
	/*! \brief Applies an impulse to the emitter based on angle and power and external_impulse_weight */
	void setInitialExternalImpulse(real p_angle, real p_power);
	
	inline void setIsCulled(bool p_isCulled) { m_isCulled = p_isCulled; }
	inline bool isCulled() const { return m_isCulled; }
	
	inline void setDelay(real p_delay) { m_delay = p_delay; }
	inline void addDelay(real p_delay) { m_delay += p_delay; }
	inline real getDelay() const { return m_delay; }
	
	/*! \brief Activate this emitter */
	void activate();
	
	/*! \brief De-activate this emitter */
	inline void deactivate() { m_active = false; }
	
	/*! \brief Kill all particles in this emitter */
	void kill();
	
	/*! \brief Returns the status of this emitter */
	inline bool isActive() const { return m_active; }
	
	/*! \brief Retrieve number of particles in the system */
	inline u32 getParticleCount() const { return static_cast<u32>(m_particles.size()); }
	
	/*! \brief Retrieve the render group id this effect belongs to */
	inline s32 getGroup() const { return m_settings.render_group; }
	
	// Toki iPhone HACK (TODO: Remove this and all related code after Toki is done.)
	static void setUseFileTextureCache(bool p_enable = true) { ms_useFileTextureCache = p_enable; }
	
	/*! \brief Creates and returns the texture used by this emitter.*/
	engine::renderer::TexturePtr getAndLoadAllUsedTextures() const;
	
	/*! \brief Access emitter settings */
	EmitterSettings& getSettings() { return m_settings; }
	const EmitterSettings& getSettings() const { return m_settings; }
	
	/*! \brief Set the visibility state of this emitter - if not visible particle will not be rendered */
	void setVisible(bool p_visible) { m_visible = p_visible; }
	
	/*! \brief Change the maximum number of particles 
		\param p_maxParticles New maximum amount of particles for this emitter */
	void changeMaxParticles(s32 p_maxParticles);
	
	/*! \brief Show the emission area */
	void renderEmissionArea(const renderer::ColorRGBA& p_color = renderer::ColorRGB::red) const;
	
	/*! \brief Show the bounding box */
	void renderBoundingBox(const renderer::ColorRGBA& p_color = renderer::ColorRGB::red) const;
	
	/*! \brief Scale the particle system values */
	inline void setScale(real p_scale) { m_settingsScale = p_scale; }
	
	/*! \brief Set an area in which particles are valid (they are killed outside this area)
		\param p_rect Pointer to rectangle of valid area, if 0 particles are always valid */
	void setValidRect(const math::VectorRect* p_rect);
	
	/*! \brief Get the area in which particles are valid (returns 0 if they are always valid) */
	inline const math::VectorRect* getValidRect() const { return m_useValidRect ? &m_validRect : 0; }
	
	/*! \brief Get the area occupied by this emitter.*/
	inline math::VectorRect getBoundingBox() const { return math::VectorRect(m_boundingBox).translate(m_position); }
	
	inline void setDepth(real p_depth) { m_zDepth = p_depth; }
	inline real getDepth() const { return m_zDepth; }
	
private:
	static bool ms_useFileTextureCache;
	
	// Interpolation for several types
	inline real interpolate(real p_start, real p_end, real p_life) const
	{
		return p_start + p_life * (p_end - p_start);
	}
	renderer::ColorRGBA interpolate(const renderer::ColorRGBA& p_start, const renderer::ColorRGBA& p_end, real p_life) const;
	
	// Get a random floating point value between 0 and 1
	inline real getRand() const
	{
		return tt::math::Random::getEffects().getNormalizedNext();
	}
	
	// Pick a random value from specified range
	template <typename T>
	inline T getRandom(const Range<T>& p_range) const
	{
		return interpolate(p_range.low, p_range.high, getRand());
	}
	
	// Emit specified nr of particles
	void emit(s32 p_particle_count);
	
	// Initialize a new particle
	void initializeParticle();
	
	void pregenerateParticles();
	
	void createQuadBatch();
	
	//Enable Copy / disable assignment
	ParticleEmitter(const ParticleEmitter& p_emitter);
	ParticleEmitter& operator=(const ParticleEmitter&);
	
	
	// Member variables
	bool m_active;
	bool m_pregenerating;
	bool m_visible;
	bool m_useValidRect;
	math::Vector2 m_position;
	real          m_zDepth;
	
	renderer::QuadBufferPtr m_quads;
	renderer::TexturePtr m_texture;
	
	TriggerID m_triggerID;
	
	// Timing
	real m_accumulated_time;
	real m_time_alive;
	real m_delay;
	
	// Settings
	EmitterSettings m_settings;
	real            m_settingsScale;
	
	// External impulse
	bool             m_hasExternalImpulse;
	math::Vector2    m_externalImpulse;
	
	// Culling
	bool             m_isCulled;
	math::VectorRect m_boundingBox;
	
	// 'Kills' particles outside this rect
	math::VectorRect m_validRect;
	
	// List containing all particles in the system
	typedef std::vector<Particle> Particles;
	Particles m_particles;
	
	renderer::BatchQuadCollection m_quadBatch;
};

// Enum functions
const char* getAnimationTypeName(ParticleAnimation::AnimationType p_type);
ParticleAnimation::AnimationType getAnimationTypeFromName(const std::string& p_name);

const char* getEmissionTypeName(EmissionBehavior::EmissionType p_type);
EmissionBehavior::EmissionType getEmissionTypeFromName(const std::string& p_name);

const char* getAreaTypeName(EmissionBehavior::AreaType p_type);
EmissionBehavior::AreaType getAreaTypeFromName(const std::string& p_name);

const char* getParticleOrientationName(ParticleOrientation p_enum);
ParticleOrientation getParticleOrientationFromName(const std::string& p_name);

// Namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_PARTICLES_PARTICLEEMITTER)
