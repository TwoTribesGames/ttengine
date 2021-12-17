#include <limits>

#include <tt/code/bufferutils.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

// Config handles for the various beam settings

struct BeamConfig
{
	std::string startTexture;
	std::string startTextureNS;
	real        startOffset;
	real        startWidth;
	real        startHeight;
	
	std::string endTexture;
	std::string endTextureNS;
	real        endOffset;
	real        endWidth;
	real        endHeight;
	
	std::string lineTexture;
	std::string lineTextureNS;
	bool        lineRenderAdditive;
	real        lineBeginOffset;
	real        lineEndOffset;
	real        lineEndBlockedOffset;
	real        lineThicknessBase;
	real        lineThicknessSineAFrequency;
	real        lineThicknessSineAAmplitude;
	real        lineThicknessSineBFrequency;
	real        lineThicknessSineBAmplitude;
	
	std::string riderTexture;
	std::string riderTextureNS;
	real        riderTextureLength;
	bool        riderRenderAdditive;
	real        riderBeginOffset;
	real        riderEndOffset;
	real        riderEndBlockedOffset;
	real        riderThickness;
	real        riderAnimU;
	real        riderAnimV;

	std::string seekerTexture;
	std::string seekerTextureNS;
	real        seekerTextureLength;
	bool        seekerRenderAdditive;
	real        seekerBeginOffset;
	real        seekerEndOffset;
	real        seekerWidth;
	real        seekerHeight;
	real        seekerSpeed;
	bool        seekerFixedUV;
	s32         seekerRepeatCount;
	real        seekerRepeatDelay;
	
	std::string particleStartFilename;
	real        particleStartOffset;
	std::string particleStartSoundEffectFilename;
	std::string particleEndFilename;
	real        particleEndOffset;
	std::string particleEndSoundEffectFilename;
	std::string particleEndBlockedFilename;
	real        particleEndBlockedOffset;
	std::string particleEndBlockedSoundEffectFilename;
	
	bool        showIfBlocked;
	bool        breakOnEntityHit;
	
	
	inline BeamConfig()
	:
	startTexture(),
	startTextureNS(),
	startOffset(0.0f),
	startWidth(0.0f),
	startHeight(0.0f),
	endTexture(),
	endTextureNS(),
	endOffset(0.0f),
	endWidth(0.0f),
	endHeight(0.0f),
	lineTexture(),
	lineTextureNS(),
	lineRenderAdditive(false),
	lineBeginOffset(0.0f),
	lineEndOffset(0.0f),
	lineEndBlockedOffset(0.0f),
	lineThicknessBase(0.0f),
	lineThicknessSineAFrequency(0.0f),
	lineThicknessSineAAmplitude(0.0f),
	lineThicknessSineBFrequency(0.0f),
	lineThicknessSineBAmplitude(0.0f),
	riderTexture(),
	riderTextureNS(),
	riderTextureLength(0.0f),
	riderRenderAdditive(true),
	riderBeginOffset(0.0f),
	riderEndOffset(0.0f),
	riderEndBlockedOffset(0.0f),
	riderThickness(0.0f),
	riderAnimU(0.0f),
	riderAnimV(0.0f),
	seekerTexture(),
	seekerTextureNS(),
	seekerTextureLength(0),
	seekerRenderAdditive(false),
	seekerBeginOffset(0.0f),
	seekerEndOffset(0.0f),
	seekerWidth(0.0f),
	seekerHeight(0.0f),
	seekerSpeed(0.0f),
	seekerFixedUV(false),
	seekerRepeatCount(0),
	seekerRepeatDelay(0.0f),
	particleStartFilename(),
	particleStartOffset(0.0f),
	particleStartSoundEffectFilename(),
	particleEndFilename(),
	particleEndOffset(0.0f),
	particleEndSoundEffectFilename(),
	particleEndBlockedFilename(),
	particleEndBlockedOffset(0.0f),
	particleEndBlockedSoundEffectFilename(),
	showIfBlocked(true),
	breakOnEntityHit(true)
	{ }
};

static BeamConfig g_beamConfig[PowerBeamType_Count];
static bool       g_beamConfigInitialized = false;


static void initializeBeamConfig()
{
	TT_ASSERT(g_beamConfigInitialized == false);
	
	const tt::cfg::ConfigHivePtr& cfgPtr(cfg());
	
	for (s32 i = 0; i < PowerBeamType_Count; ++i)
	{
		const PowerBeamType type = static_cast<PowerBeamType>(i);
		const std::string   typeName(getPowerBeamTypeName(type));
		const std::string   root("toki." + typeName + ".");
		
		BeamConfig& c(g_beamConfig[type]);

		tt::cfg::HandleString handle = cfgPtr->getHandleString(root + "start.texture_name");
		if(handle.isValid())
		{
			c.startTexture   = cfgPtr->getStringDirect(root + "start.texture_name");
			c.startTextureNS = cfgPtr->getStringDirect(root + "start.texture_namespace");
			c.startOffset    = cfgPtr->getRealDirect  (root + "start.offset");
			c.startWidth     = cfgPtr->getRealDirect  (root + "start.width");
			c.startHeight    = cfgPtr->getRealDirect  (root + "start.height");
		}
		
		handle = cfgPtr->getHandleString(root + "end.texture_name");
		if(handle.isValid())
		{
			c.endTexture     = cfgPtr->getStringDirect(root + "end.texture_name");
			c.endTextureNS   = cfgPtr->getStringDirect(root + "end.texture_namespace");
			c.endOffset      = cfgPtr->getRealDirect  (root + "end.offset");
			c.endWidth       = cfgPtr->getRealDirect  (root + "end.width");
			c.endHeight      = cfgPtr->getRealDirect  (root + "end.height");
		}
		
		handle = cfgPtr->getHandleString(root + "line.texture_name");
		if(handle.isValid())
		{
			c.lineTexture                 = cfgPtr->getStringDirect(root + "line.texture_name");
			c.lineTextureNS               = cfgPtr->getStringDirect(root + "line.texture_namespace");
			c.lineRenderAdditive          = cfgPtr->getBoolDirect  (root + "line.render_additive");
			c.lineBeginOffset             = cfgPtr->getRealDirect  (root + "line.begin_offset");
			c.lineEndOffset               = cfgPtr->getRealDirect  (root + "line.end_offset");
			c.lineEndBlockedOffset        = cfgPtr->getRealDirect  (root + "line.end_offset_blocked");
			c.lineThicknessBase           = cfgPtr->getRealDirect  (root + "line.thickness.base_value");
			c.lineThicknessSineAFrequency = cfgPtr->getRealDirect  (root + "line.thickness.sine_a.frequency");
			c.lineThicknessSineAAmplitude = cfgPtr->getRealDirect  (root + "line.thickness.sine_a.amplitude");
			c.lineThicknessSineBFrequency = cfgPtr->getRealDirect  (root + "line.thickness.sine_b.frequency");
			c.lineThicknessSineBAmplitude = cfgPtr->getRealDirect  (root + "line.thickness.sine_b.amplitude");
		}
		
		handle = cfgPtr->getHandleString(root + "rider.texture_name");
		if(handle.isValid())
		{
			c.riderTexture          = cfgPtr->getStringDirect(root + "rider.texture_name");
			c.riderTextureNS        = cfgPtr->getStringDirect(root + "rider.texture_namespace");
			c.riderTextureLength    = cfgPtr->getRealDirect  (root + "rider.texture_length");
			c.riderRenderAdditive   = cfgPtr->getBoolDirect  (root + "rider.render_additive");
			c.riderBeginOffset      = cfgPtr->getRealDirect  (root + "rider.begin_offset");
			c.riderEndOffset        = cfgPtr->getRealDirect  (root + "rider.end_offset");
			c.riderEndBlockedOffset = cfgPtr->getRealDirect  (root + "rider.end_offset_blocked");
			c.riderThickness        = cfgPtr->getRealDirect  (root + "rider.thickness");
			c.riderAnimU            = cfgPtr->getRealDirect  (root + "rider.uv_animation.u");
			c.riderAnimV            = cfgPtr->getRealDirect  (root + "rider.uv_animation.v");
		}
		
		handle = cfgPtr->getHandleString(root + "seeker.texture_name");
		if(handle.isValid())
		{
			c.seekerTexture          = cfgPtr->getStringDirect (root + "seeker.texture_name");
			c.seekerTextureNS        = cfgPtr->getStringDirect (root + "seeker.texture_namespace");
			c.seekerTextureLength    = cfgPtr->getRealDirect   (root + "seeker.texture_length");
			c.seekerRenderAdditive   = cfgPtr->getBoolDirect   (root + "seeker.render_additive");
			c.seekerBeginOffset      = cfgPtr->getRealDirect   (root + "seeker.begin_offset");
			c.seekerEndOffset        = cfgPtr->getRealDirect   (root + "seeker.end_offset");
			c.seekerWidth            = cfgPtr->getRealDirect   (root + "seeker.width");
			c.seekerHeight           = cfgPtr->getRealDirect   (root + "seeker.height");
			c.seekerSpeed            = cfgPtr->getRealDirect   (root + "seeker.speed");
			c.seekerFixedUV          = cfgPtr->getBoolDirect   (root + "seeker.fixed_uv");
			c.seekerRepeatCount      = cfgPtr->getIntegerDirect(root + "seeker.repeat_count");
			c.seekerRepeatDelay      = cfgPtr->getRealDirect   (root + "seeker.repeat_delay");
		}

		handle = cfgPtr->getHandleString(root + "particles.start.filename");
		if(handle.isValid())
		{
			c.particleStartFilename                 = cfgPtr->getStringDirect(root + "particles.start.filename");
			c.particleStartOffset                   = cfgPtr->getRealDirect  (root + "particles.start.offset");
			c.particleStartSoundEffectFilename      = cfgPtr->getStringDirect(root + "particles.start.sound");
			c.particleEndFilename                   = cfgPtr->getStringDirect(root + "particles.end.filename");
			c.particleEndOffset                     = cfgPtr->getRealDirect  (root + "particles.end.offset");
			c.particleEndSoundEffectFilename        = cfgPtr->getStringDirect(root + "particles.end.sound");
			c.particleEndBlockedFilename            = cfgPtr->getStringDirect(root + "particles.end_blocked.filename");
			c.particleEndBlockedOffset              = cfgPtr->getRealDirect  (root + "particles.end_blocked.offset");
			c.particleEndBlockedSoundEffectFilename = cfgPtr->getStringDirect(root + "particles.end_blocked.sound");
		}
		
		c.showIfBlocked    = cfgPtr->getBoolDirect(root + "show_if_blocked");
		c.breakOnEntityHit = cfgPtr->getBoolDirect(root + "break_on_entityhit");
	}
	
	g_beamConfigInitialized = true;
}


inline void addTextureIDToContainer(const std::string&  p_assetID,
                                    const std::string&  p_assetNamespace,
                                    utils::StringPairs* p_textureIDs_OUT)
{
	if (p_assetID.empty() == false)
	{
		p_textureIDs_OUT->push_back(utils::StringPair(p_assetID, p_assetNamespace));
	}
}


static inline void updateExtents(tt::math::Vector2& p_minExtent,
                                 tt::math::Vector2& p_maxExtent,
                                 real               p_centerX,
                                 real               p_centerY,
                                 real               p_width,
                                 real               p_height)
{
	const real halfOfBiggestDimension = std::max(p_width, p_height) * 0.5f;
	p_minExtent.x = std::min(p_minExtent.x, p_centerX - halfOfBiggestDimension);
	p_minExtent.y = std::min(p_minExtent.y, p_centerY - halfOfBiggestDimension);
	p_maxExtent.x = std::max(p_maxExtent.x, p_centerX + halfOfBiggestDimension);
	p_maxExtent.y = std::max(p_maxExtent.y, p_centerY + halfOfBiggestDimension);
}


static inline void updateExtents(tt::math::Vector2&                         p_minExtent,
                                 tt::math::Vector2&                         p_maxExtent,
                                 const tt::engine::renderer::QuadSpritePtr& p_quad)
{
	TT_NULL_ASSERT(p_quad);
	updateExtents(p_minExtent,              p_maxExtent,
	              p_quad->getPosition().x, -p_quad->getPosition().y,
	              p_quad->getWidth(),       p_quad->getHeight());
}


//--------------------------------------------------------------------------------------------------
// Public member functions

PowerBeamGraphic::PowerBeamGraphic(const CreationParams&         p_creationParams,
                                   const PowerBeamGraphicHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_type(p_creationParams.type),
m_source(p_creationParams.source),
m_firstFrame(true),
m_beamIsBlocked(false),
m_isVisible(true),
m_isCulled(false),
m_animationTime(0.0f),
m_beamLine(),
m_beamRider(),
m_targetSeeker(),
m_seekerPosition(0),
m_seekerRepeatCount(0),
m_seekerDelayTimer(0.0f),
m_beamStartQuad(),
m_beamEndQuad(),
m_particleStart(),
m_particleStartSound(),
m_particleEnd(),
m_particleEndSound(),
m_areaOccupiedByGraphic()
{
	// Extra sanity check: PowerBeamGraphicMgr should already have refused to create a graphic if the type is invalid
	TT_ASSERT(isValidPowerBeamType(m_type));
	
	if (g_beamConfigInitialized == false) initializeBeamConfig();
	
	const BeamConfig& c(g_beamConfig[m_type]);
	
	using namespace tt::engine::renderer;

	if (c.lineTexture.empty() == false)
	{
		// Texture for the line between start and end point
		m_beamLine.texture = TextureCache::get(c.lineTexture, c.lineTextureNS, true);
		if (m_beamLine.texture != 0)
		{
			m_beamLine.texture->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);
		}
		
		m_beamLine.quad.reset(new Quad2D(VertexBuffer::Property_Texture0));
	}
	
	if (c.riderTexture.empty() == false)
	{
		// Texture for the texture animation on top of the line between start and end point
		m_beamRider.texture = TextureCache::get(c.riderTexture, c.riderTextureNS, true);
		if (m_beamRider.texture != 0)
		{
			m_beamRider.texture->setAddressMode(AddressMode_Wrap, AddressMode_Wrap);
		}
		
		m_beamRider.quad.reset(new Quad2D(VertexBuffer::Property_Texture0));
	}
	
	if (c.seekerTexture.empty() == false)
	{
		// Texture for the texture animation on top of the line between start and end point
		m_targetSeeker.texture = TextureCache::get(c.seekerTexture, c.seekerTextureNS, true);
		if (m_targetSeeker.texture != 0)
		{
			m_targetSeeker.texture->setAddressMode(AddressMode_Wrap, AddressMode_Wrap);
		}
		
		m_targetSeeker.quad.reset(new Quad2D(VertexBuffer::Property_Texture0|VertexBuffer::Property_Diffuse));
	}
	
	// Quad for the start point
	if (c.startTexture.empty() == false)
	{
		m_beamStartQuad = QuadSprite::createQuad(TextureCache::get(c.startTexture, c.startTextureNS, true));
		m_beamStartQuad->setWidth (c.startWidth );
		m_beamStartQuad->setHeight(c.startHeight);
	}
	
	// Quad for the end point
	if (c.endTexture.empty() == false)
	{
		m_beamEndQuad = QuadSprite::createQuad(TextureCache::get(c.endTexture, c.endTextureNS, true));
		m_beamEndQuad->setWidth (c.endWidth );
		m_beamEndQuad->setHeight(c.endHeight);
	}
	
	// Randomize the starting points between 0 and 10 seconds
	const real t = tt::math::Random::getEffects().getNextReal(0.0f, 10.0f);
	m_beamRider.uv.x += c.riderAnimU * t;
	m_beamRider.uv.y += c.riderAnimV * t;
	tt::math::wrap(m_beamRider.uv.x, 0.0f, 1.0f);
	tt::math::wrap(m_beamRider.uv.y, 0.0f, 1.0f);
}


PowerBeamGraphic::~PowerBeamGraphic()
{
	if (m_particleEnd != 0) m_particleEnd->stop(true);
}


void PowerBeamGraphic::update(real p_elapsedTime)
{
	const sensor::Sensor* sensor = m_source.getPtr();
	if (sensor == 0 || sensor->isEnabled() == false)
	{
		m_isVisible = false;
		return;
	}
	
	const sensor::RayTracer& rayTracer = sensor->getRayTracer();
	if (rayTracer.hasHitlocation() == false)
	{
		// The sensor's raytracer did not trace yet. Don't update the powerbeam
		m_isVisible = false;
		return;
	}
	
	m_isVisible = true;
	
	const BeamConfig& c(g_beamConfig[m_type]);
	
	static const real quadSizeScale = 1.0f / (tt::engine::renderer::Quad2D::quadSize * 2.0f);
	
	m_animationTime += p_elapsedTime;
	
	using tt::math::Vector2;
	using tt::math::Vector3;
	
	// These variables track the extents of the area taken by this power beam graphic
	// (used for culling)
	static const real maxRealValue = std::numeric_limits<real>::max();
	Vector2 minExtent( maxRealValue,  maxRealValue);
	Vector2 maxExtent(-maxRealValue, -maxRealValue);
	
	const Vector2  source(sensor->getRayTracePosition());
	// FIXME: This code assumes that the ray tracer in the sensor holds the correct hitlocation.
	//        This can only be true if there has been only 1 raycheck in that sensor.
	Vector2 target(rayTracer.getHitLocation());
	real    totalLength = (target - source).length();
	
	// Check if we need to resize the beam if it is hit by an entity
	if (c.breakOnEntityHit && sensor->getSensedEntities().size() > 0)
	{
		entity::Entity* entity = (sensor->getSensedEntities().front().getPtr());
		if (entity != 0)
		{
			totalLength = (entity->getCenterPosition() - source).length();
			const Vector2 norm((target - source).normalize());
			target = source + (norm * totalLength);
		}
	}
	
	const Vector2  beamNormal((target - source).getNormalized());
	const real     beamAngle        = -beamNormal.getAngle(Vector2::unitX);
	const real     beamReverseAngle = beamAngle + tt::math::pi;
	
	const real twoPiTime = m_animationTime * tt::math::twoPi;
	
	const bool beamWasBlocked = m_beamIsBlocked;
	m_beamIsBlocked = rayTracer.hasCollision();
	
	// Scale and position the beam line
	if (m_beamLine.quad != 0)
	{
		const real    cfgBeginOffset = c.lineBeginOffset;
		const real    cfgEndOffset   = m_beamIsBlocked ? c.lineEndBlockedOffset : c.lineEndOffset;
		const Vector2 lineStart(source + (beamNormal * std::min(cfgBeginOffset, totalLength)));
		const Vector2 lineEnd  (target - (beamNormal * std::min(cfgEndOffset, totalLength - std::min(cfgBeginOffset, totalLength))));
		
		const Vector2 diff(lineEnd - lineStart);
		const Vector2 midway(lineStart + (diff * 0.5f));
		
		const real thickness = c.lineThicknessBase +
				(tt::math::sin(twoPiTime * c.lineThicknessSineAFrequency) * c.lineThicknessSineAAmplitude) +
				(tt::math::sin(twoPiTime * c.lineThicknessSineBFrequency) * c.lineThicknessSineBAmplitude);
		
		m_beamLine.matrix = tt::math::Matrix44::getTranslation(Vector3(midway.x, midway.y, 0.0f));
		m_beamLine.matrix.rotateZ(beamAngle);
		m_beamLine.matrix.scale(diff.length() * quadSizeScale, thickness * quadSizeScale);
		
		updateExtents(minExtent, maxExtent, lineStart.x, lineStart.y, thickness, thickness);
		updateExtents(minExtent, maxExtent, lineEnd.x,   lineEnd.y,   thickness, thickness);
	}
	
	// Scale and position the beam rider
	if (m_beamRider.quad != 0)
	{
		const real    cfgBeginOffset = c.riderBeginOffset;
		const real    cfgEndOffset   = m_beamIsBlocked ? c.riderEndBlockedOffset : c.riderEndOffset;
		const Vector2 riderStart(source + (beamNormal * std::min(cfgBeginOffset, totalLength)));
		const Vector2 riderEnd  (target - (beamNormal * std::min(cfgEndOffset,   totalLength - std::min(cfgBeginOffset, totalLength))));
		
		const Vector2 diff(riderEnd - riderStart);
		const real    length = diff.length();
		const Vector2 midway(riderStart + (diff * 0.5f));
		
		const real thickness = c.riderThickness;
		
		m_beamRider.matrix = tt::math::Matrix44::getTranslation(Vector3(midway.x, midway.y, 0.0f));
		m_beamRider.matrix.rotateZ(beamAngle);
		m_beamRider.matrix.scale(length * quadSizeScale, thickness * quadSizeScale);
		
		// Make the texture repeat every so many world units
		const real quadTexMaxU = length / c.riderTextureLength;
		using tt::engine::renderer::Quad2D;
		m_beamRider.quad->setTexcoord(Quad2D::Vertex_TopLeft,     Vector2(0.0f,        0.0f));
		m_beamRider.quad->setTexcoord(Quad2D::Vertex_BottomLeft,  Vector2(0.0f,        1.0f));
		m_beamRider.quad->setTexcoord(Quad2D::Vertex_TopRight,    Vector2(quadTexMaxU, 0.0f));
		m_beamRider.quad->setTexcoord(Quad2D::Vertex_BottomRight, Vector2(quadTexMaxU, 1.0f));
		
		// UV animation for the beam rider
		m_beamRider.uv.x += c.riderAnimU * p_elapsedTime;
		m_beamRider.uv.y += c.riderAnimV * p_elapsedTime;
		tt::math::wrap(m_beamRider.uv.x, 0.0f, 1.0f);
		tt::math::wrap(m_beamRider.uv.y, 0.0f, 1.0f);
		
		updateExtents(minExtent, maxExtent, riderStart.x, riderStart.y, thickness, thickness);
		updateExtents(minExtent, maxExtent, riderEnd.x,   riderEnd.y,   thickness, thickness);
	}
	
	// Scale and position the target seeker
	if (m_targetSeeker.quad != 0)
	{
		if(m_seekerDelayTimer > 0.0f)
		{
			m_seekerDelayTimer -= p_elapsedTime;
		}
		else
		{
			m_targetSeeker.quad->setAlpha(255);
			const Vector2 seekerStart(source + (beamNormal * std::min(c.seekerBeginOffset, totalLength)));
			const Vector2 seekerEnd  (target - (beamNormal * std::min(
				c.seekerEndOffset,   totalLength - std::min(c.seekerBeginOffset, totalLength))));
			const Vector2 diff(seekerEnd - seekerStart);
			const real    length = diff.length();
			const real    halfSeekerWidth(c.seekerWidth / 2);

			// Animate seeker quad along the beam
			m_seekerPosition += p_elapsedTime * c.seekerSpeed;

			// Get quad position
			const Vector2 position = seekerStart + diff * (m_seekerPosition / length);
		
			m_targetSeeker.matrix = tt::math::Matrix44::getTranslation(Vector3(position.x, position.y, 0.0f));
			m_targetSeeker.matrix.rotateZ(beamAngle);
			m_targetSeeker.matrix.scale(c.seekerWidth * quadSizeScale, c.seekerHeight * quadSizeScale);
		
			if(c.seekerFixedUV == false)
			{
				const real quadTexMaxU = length / c.seekerTextureLength;
				const real quadMinU = ((m_seekerPosition - halfSeekerWidth) / length) * quadTexMaxU;
				const real quadMaxU = ((m_seekerPosition + halfSeekerWidth) / length) * quadTexMaxU;

				using tt::engine::renderer::Quad2D;
				m_targetSeeker.quad->setTexcoord(Quad2D::Vertex_TopLeft,     Vector2(quadMinU, 0.0f));
				m_targetSeeker.quad->setTexcoord(Quad2D::Vertex_BottomLeft,  Vector2(quadMinU, 1.0f));
				m_targetSeeker.quad->setTexcoord(Quad2D::Vertex_TopRight,    Vector2(quadMaxU, 0.0f));
				m_targetSeeker.quad->setTexcoord(Quad2D::Vertex_BottomRight, Vector2(quadMaxU, 1.0f));
			}

			// Check if seeker reached target
			if(m_seekerPosition > (length - halfSeekerWidth))
			{
				m_seekerPosition = halfSeekerWidth;
				m_targetSeeker.quad->setAlpha(0);
				++m_seekerRepeatCount;
				m_seekerDelayTimer = c.seekerRepeatDelay;

				if(m_seekerRepeatCount > c.seekerRepeatCount)
				{
					// Done with the seeker quad
					m_targetSeeker.quad.reset();
				}
			}
			else
			{
				updateExtents(minExtent, maxExtent, seekerStart.x, seekerStart.y, c.seekerWidth, c.seekerHeight);
				updateExtents(minExtent, maxExtent, seekerEnd.x,   seekerEnd.y,   c.seekerWidth, c.seekerHeight);
			}
		}
	}
	
	// Scale and position the start quad
	if (m_beamStartQuad != 0)
	{
		const Vector2 pos(source + (beamNormal * std::min(c.startOffset, totalLength)));
		m_beamStartQuad->setPosition(Vector3(pos.x, -pos.y, 0.0f));
		m_beamStartQuad->setRotation(beamAngle);
		
		updateExtents(minExtent, maxExtent, m_beamStartQuad);
	}
	
	// Scale and position the end quad
	if (m_beamEndQuad != 0)
	{
		const Vector2 pos(target - (beamNormal * std::min(c.endOffset, totalLength)));
		m_beamEndQuad->setPosition(Vector3(pos.x, -pos.y, 0.0f));
		m_beamEndQuad->setRotation(beamReverseAngle);
		
		updateExtents(minExtent, maxExtent, m_beamEndQuad);
	}
	
	// Update the particle effects
	if (m_firstFrame)
	{
		m_particleStart = spawnParticleEffect(c.particleStartFilename);
		if (c.particleStartSoundEffectFilename.empty() == false)
		{
			m_particleStartSound = audio::SoundCueWithPosition::create("Effects",
				c.particleStartSoundEffectFilename, tt::math::Vector3::zero);
		}
	}
	
	if (m_firstFrame || m_beamIsBlocked != beamWasBlocked)
	{
		const std::string& particleFilename = m_beamIsBlocked ?
			c.particleEndBlockedFilename : c.particleEndFilename;
		const std::string& soundFilename = m_beamIsBlocked ?
			c.particleEndBlockedSoundEffectFilename : c.particleEndSoundEffectFilename;
		
		// Beam switched between blocked and non-blocked: spawn a new particle effect
		m_particleEnd = spawnParticleEffect(particleFilename);
		if (m_particleEndSound != 0)
		{
			m_particleEndSound.reset();
		}
		
		if (soundFilename.empty() == false)
		{
			m_particleEndSound = audio::SoundCueWithPosition::create("Effects", soundFilename, tt::math::Vector3::zero);
		}
	}
	
	// Update start
	updateParticlePosition(m_particleStart, m_particleStartSound, source, beamNormal, totalLength,
			c.particleStartOffset);
	
	// Update end
	updateParticlePosition(m_particleEnd, m_particleEndSound,  target, -beamNormal, totalLength,
			m_beamIsBlocked ? c.particleEndBlockedOffset : c.particleEndOffset);
	
	if (minExtent.x > maxExtent.x || minExtent.y > maxExtent.y)
	{
		// Nothing visible at all: set to empty rect
		m_areaOccupiedByGraphic = tt::math::VectorRect(maxExtent, 0.0f, 0.0f);
	}
	else
	{
		m_areaOccupiedByGraphic = tt::math::VectorRect(minExtent, maxExtent);
	}
	
	m_firstFrame = false;
}


void PowerBeamGraphic::updateForRender(const tt::math::VectorRect& p_visibilityRect)
{
	if (m_particleStart != 0) m_particleStart->getTrigger()->setIsCulled(m_isVisible == false);
	if (m_particleEnd != 0) m_particleEnd->getTrigger()->setIsCulled(m_isVisible == false);
	
	// Check if graphic should be culled
	m_isCulled = m_areaOccupiedByGraphic.intersects(p_visibilityRect) == false;
	
	if (m_isCulled)
	{
		return;
	}
	
	if (m_beamLine.quad     != 0) m_beamLine    .quad->update();
	if (m_beamRider.quad    != 0) m_beamRider   .quad->update();
	if (m_targetSeeker.quad != 0) m_targetSeeker.quad->update();
	if (m_beamStartQuad     != 0) m_beamStartQuad    ->update();
	if (m_beamEndQuad       != 0) m_beamEndQuad      ->update();
}


void PowerBeamGraphic::render(const tt::math::VectorRect& /*p_visibilityRect*/) const
{
	if (m_isCulled) return;
	
	const BeamConfig& c(g_beamConfig[m_type]);
	if (m_isVisible == false || (m_beamIsBlocked && c.showIfBlocked == false))
	{
		return;
	}
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack*                    stack    = MatrixStack::getInstance();
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	// Render the beam source
	if (m_beamStartQuad != 0)
	{
		m_beamStartQuad->render();
	}
	
	// Render the beam line
	if (m_beamLine.quad != 0)
	{
		renderer->setTexture(m_beamLine.texture);
		
		if (c.lineRenderAdditive)
		{
			renderer->setBlendMode(tt::engine::renderer::BlendMode_Add);
		}
		
		stack->push();
		stack->multiply44(m_beamLine.matrix);
		stack->updateWorldMatrix();
		
		m_beamLine.quad->render();
		
		if (c.lineRenderAdditive)
		{
			renderer->setBlendMode(tt::engine::renderer::BlendMode_Blend);
		}
		
		stack->pop();
	}
	
	if (c.showIfBlocked || m_beamIsBlocked == false)
	{
		if (m_beamRider.quad != 0)
		{
			// Render the beam 'rider'
			renderer->setTexture(m_beamRider.texture);
			if (c.riderRenderAdditive)
			{
				renderer->setBlendMode(tt::engine::renderer::BlendMode_Add);
			}
			
			stack->setMode(MatrixStack::Mode_Texture);
			stack->translate(tt::math::Vector3(m_beamRider.uv.x, m_beamRider.uv.y, 0));
			stack->updateTextureMatrix();
			stack->setMode(MatrixStack::Mode_Position);
			
			stack->push();
			stack->multiply44(m_beamRider.matrix);
			stack->updateWorldMatrix();
			
			m_beamRider.quad->render();
			
			if (c.riderRenderAdditive)
			{
				renderer->setBlendMode(tt::engine::renderer::BlendMode_Blend);
			}
			stack->pop();
		}

		if (m_targetSeeker.quad != 0)
		{
			// Render the beam 'target seeker'
			renderer->setTexture(m_targetSeeker.texture);
			if (c.seekerRenderAdditive)
			{
				renderer->setBlendMode(tt::engine::renderer::BlendMode_Add);
			}

			stack->setMode(MatrixStack::Mode_Texture);
			stack->resetTextureMatrix();
			stack->translate(tt::math::Vector3(m_targetSeeker.uv.x, m_targetSeeker.uv.y, 0));
			stack->updateTextureMatrix();
			stack->setMode(MatrixStack::Mode_Position);
			
			stack->push();
			stack->multiply44(m_targetSeeker.matrix);
			stack->updateWorldMatrix();
			
			m_targetSeeker.quad->render();

			if (c.seekerRenderAdditive)
			{
				renderer->setBlendMode(tt::engine::renderer::BlendMode_Blend);
			}
			stack->pop();
		}
		
		// Render the beam endpoint
		if (m_beamEndQuad != 0)
		{
			m_beamEndQuad->render();
		}
	}
	
	// Restore settings
	stack->resetTextureMatrix();
}


void PowerBeamGraphic::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(m_type,   p_context);
	bu::putHandle  (m_source, p_context);
}


PowerBeamGraphic::CreationParams PowerBeamGraphic::unserializeCreationParams(
		tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	PowerBeamType        type   = bu::getEnum  <u8, PowerBeamType>(p_context);
	sensor::SensorHandle source = bu::getHandle<sensor::Sensor   >(p_context);
	
	return CreationParams(type, source);
}


void PowerBeamGraphic::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_beamIsBlocked, p_context);
	bu::put(m_animationTime, p_context);
}


void PowerBeamGraphic::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_beamIsBlocked = bu::get<bool>(p_context);
	m_animationTime = bu::get<real>(p_context);
}


PowerBeamGraphic* PowerBeamGraphic::getPointerFromHandle(const PowerBeamGraphicHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getPowerBeamGraphicMgr().getPowerBeamGraphic(p_handle);
}


void PowerBeamGraphic::getNeededTextureIDs(utils::StringPairs* p_textureIDs_OUT)
{
	TT_NULL_ASSERT(p_textureIDs_OUT);
	
	if (g_beamConfigInitialized == false) initializeBeamConfig();
	
	for (s32 typeIndex = 0; typeIndex < PowerBeamType_Count; ++typeIndex)
	{
		const BeamConfig& c(g_beamConfig[typeIndex]);
		
		addTextureIDToContainer(c.startTexture , c.startTextureNS , p_textureIDs_OUT);
		addTextureIDToContainer(c.endTexture   , c.endTextureNS   , p_textureIDs_OUT);
		addTextureIDToContainer(c.lineTexture  , c.lineTextureNS  , p_textureIDs_OUT);
		addTextureIDToContainer(c.riderTexture , c.riderTextureNS , p_textureIDs_OUT);
		addTextureIDToContainer(c.seekerTexture, c.seekerTextureNS, p_textureIDs_OUT);
	}
}


void PowerBeamGraphic::reloadConfig()
{
	g_beamConfigInitialized = false;
	initializeBeamConfig();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

tt::engine::particles::ParticleEffectPtr PowerBeamGraphic::spawnParticleEffect( 
	const std::string& p_filename) const
{
	if (p_filename.empty())
	{
		// Empty effect filename = do not spawn a particle effect
		return tt::engine::particles::ParticleEffectPtr();
	}
	
	using namespace tt::engine::particles;
	ParticleEffectPtr effect = 
		ParticleMgr::getInstance()->createEffect(p_filename + ".trigger", tt::math::Vector3::zero);
	if (effect != 0)
	{
		effect->spawn();
	}
	
	return effect;
}


void PowerBeamGraphic::updateParticlePosition(const tt::engine::particles::ParticleEffectPtr& p_effect,
                                              const audio::SoundCueWithPositionPtr&           p_soundEffect,
                                              const tt::math::Vector2&                        p_basePos,
                                              const tt::math::Vector2&                        p_beamNormal,
                                              real                                            p_lineLength,
                                              real                                            p_offset)
{
	const tt::math::Vector2 pos(p_basePos + (p_beamNormal * std::min(p_offset, p_lineLength)));
	
	if (p_effect != 0)
	{
		p_effect->getTrigger()->setOrigin(pos);
	}
	
	if (p_soundEffect != 0)
	{
		p_soundEffect->setAudioPosition(tt::math::Vector3(pos.x, pos.y, 0.0f));
	}
}

// Namespace end
}
}
}
}
