#if !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHIC_H)
#define INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHIC_H


#include <tt/code/fwd.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector3.h>

#include <toki/audio/SoundCueWithPosition.h>
#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/sensor/fwd.h>
#include <toki/utils/types.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

class PowerBeamGraphic
{
public:
	struct CreationParams
	{
		inline CreationParams(PowerBeamType               p_type,
		                      const sensor::SensorHandle& p_source)
		:
		type(p_type),
		source(p_source)
		{ }
		
		PowerBeamType        type;
		sensor::SensorHandle source;
	};
	typedef const CreationParams& ConstructorParamType;
	
	
	PowerBeamGraphic(const CreationParams&         p_creationParams,
	                 const PowerBeamGraphicHandle& p_ownHandle);
	
	~PowerBeamGraphic();
	
	void update(real p_elapsedTime);
	void updateForRender(const tt::math::VectorRect& p_visibilityRect);
	void render(const tt::math::VectorRect& p_visibilityRect) const;
	
	inline const PowerBeamGraphicHandle& getHandle() const { return m_ownHandle; }
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static PowerBeamGraphic* getPointerFromHandle(const PowerBeamGraphicHandle& p_handle);
	void invalidateTempCopy() {}
	
	static void getNeededTextureIDs(utils::StringPairs* p_textureIDs_OUT);
	static void reloadConfig();
	
private:
	tt::engine::particles::ParticleEffectPtr spawnParticleEffect(const std::string& p_filename) const;
	void updateParticlePosition(const tt::engine::particles::ParticleEffectPtr& p_effect,
	                            const audio::SoundCueWithPositionPtr&           p_soundEffect,
	                            const tt::math::Vector2&                        p_basePos,
	                            const tt::math::Vector2&                        p_beamNormal,
	                            real                                            p_lineLength,
	                            real                                            p_offset);
	
	
	PowerBeamGraphicHandle m_ownHandle;
	
	PowerBeamType        m_type;
	sensor::SensorHandle m_source;
	
	bool m_firstFrame;
	bool m_beamIsBlocked;
	bool m_isVisible;
	bool m_isCulled;
	
	real m_animationTime;
	
	struct BeamQuad
	{
		tt::engine::renderer::Quad2DPtr  quad;
		tt::engine::renderer::TexturePtr texture;
		tt::math::Matrix44               matrix;
		tt::math::Vector2                uv;
	};
	
	BeamQuad m_beamLine;
	BeamQuad m_beamRider;
	BeamQuad m_targetSeeker;
	real     m_seekerPosition;
	s32      m_seekerRepeatCount;
	real     m_seekerDelayTimer;
	
	tt::engine::renderer::QuadSpritePtr m_beamStartQuad;
	tt::engine::renderer::QuadSpritePtr m_beamEndQuad;
	
	tt::engine::particles::ParticleEffectPtr m_particleStart;
	audio::SoundCueWithPositionPtr           m_particleStartSound;
	tt::engine::particles::ParticleEffectPtr m_particleEnd;  // conditional effect: depends on whether beam blocked
	audio::SoundCueWithPositionPtr           m_particleEndSound;
	
	tt::math::VectorRect m_areaOccupiedByGraphic;  // for culling; calculated in update()
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHIC_H)
