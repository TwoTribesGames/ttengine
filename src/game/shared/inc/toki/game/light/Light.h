#if !defined(INC_TOKI_GAME_LIGHT_LIGHT_H)
#define INC_TOKI_GAME_LIGHT_LIGHT_H


#include <tt/code/fwd.h>
#include <tt/math/TimedLinearInterpolation.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/light/LightRayTracer.h>
#include <toki/game/light/LightShape.h>


namespace toki {
namespace game {
namespace light {

class Light
{
public:
	struct CreationParams
	{
		inline CreationParams(const entity::EntityHandle& p_source,
		                      const tt::math::Vector2&    p_offset,
		                      real                        p_radius,
		                      real                        p_strength)
		:
		source(p_source),
		offset(p_offset),
		radius(p_radius),
		strength(p_strength)
		{ }
		
		entity::EntityHandle source;
		tt::math::Vector2    offset;
		real                 radius;
		real                 strength;
	};
	typedef const CreationParams& ConstructorParamType;
	
	
	Light(const CreationParams& p_creationParams, const LightHandle& p_ownHandle);
	
	void update(real p_deltaTime);
	void updateLightShape(const Polygons& p_occluders);
	void render(real p_currentTime = 0.0f) const;
	void renderGlow() const;
	void debugRender();
	
	inline bool isEnabled() const          { return m_enabled;      }
	inline void setEnabled(bool p_enabled) { m_enabled = p_enabled; }
	
	inline void setDirection(real p_angle) { m_lightShape.setDirection(p_angle); }
	
	inline bool affectsEntities() const { return m_affectsEntities; }
	inline void setAffectsEntities(bool p_enabled) { m_affectsEntities = p_enabled; }
	
	inline void setTexture(const std::string& p_textureName) { m_lightShape.setTexture(p_textureName); }
	
	inline void setTextureRotationSpeed(real p_textureRotationSpeed) { m_textureRotationSpeed = p_textureRotationSpeed; }
	
	inline bool isActive() const
	{
		return m_blockedAtSource == false &&
		       isEnabled() &&
		       m_radius.getValue()   > 0.0f &&
		       m_strength.getValue() > 0.0f;
	}
	
	inline const tt::math::Vector2& getOffset()        const { return m_offset.getEndValue(); }
	inline const tt::math::Vector2& getCurrentOffset() const { return m_offset.getValue();    }
	inline void setOffset(const tt::math::Vector2& p_offset, real p_duration)
	{
		m_offset.startNewInterpolation(p_offset, p_duration);
	}
	
	inline real getStrength()        const { return m_strength.getEndValue(); }
	inline real getCurrentStrength() const { return m_strength.getValue();    }
	inline void setStrength(real p_strength, real p_duration)
	{
		TT_ASSERT(p_strength >= 0.0f && p_strength <= 1.0f);
		m_strength.startNewInterpolation(p_strength, p_duration);
	}
	
	inline void setColor(tt::engine::renderer::ColorRGBA p_color)
	{
		p_color.premultiply();
		m_lightShape.setColor(p_color.rgb());
		m_colorAlpha = p_color.a;
	}
	
	inline real getRadius()        const { return m_radius.getEndValue(); }
	inline real getCurrentRadius() const { return m_radius.getValue();    }
	inline void setRadius(real p_radius, real p_duration)
	{
		TT_ASSERT(p_radius >= 0.0f);
		m_radius.startNewInterpolation(p_radius, p_duration);
	}
	
	inline real getSpread()        const { return m_spread.getEndValue(); }
	inline real getCurrentSpread() const { return m_spread.getValue();    }
	inline void setSpread(real p_spread, real p_duration)
	{
		m_spread.startNewInterpolation(p_spread, p_duration);
	}
	
	void getAffectedEntities(entity::EntityHandleSet&        p_entities,
	                         const level::AttributeLayerPtr& p_attribs);
	
	bool isInLight(const tt::math::Vector2& p_worldPosition) const;
	bool isInLight(const entity::Entity& p_targetEntity) const;
	
	void createGlow(const std::string& p_imageName, real p_scale,
	                real p_minRadius, real p_maxRadius, real p_fadeRadius);
	
	void setJitterEffect(const JitterEffectPtr& p_jitterEffect) { m_jitterEffect = p_jitterEffect; }
	
	inline const LightHandle& getHandle() const { return m_ownHandle; }
	
	void         serialize  (tt::code::BufferWriteContext* p_context) const;
	static Light unserialize(tt::code::BufferReadContext*  p_context);
	
	static Light* getPointerFromHandle(const LightHandle& p_handle);
	void invalidateTempCopy() {}
	
	inline const tt::math::Vector2& getWorldPosition() const { return m_worldPos; }
	
private:
	inline void setRadiusImpl(real p_radius)
	{
		m_lightShape.setRadius(p_radius);
		m_radiusSquared = p_radius * p_radius;
	}
	void updateWorldPosition();
	
	LightHandle                                           m_ownHandle;
	entity::EntityHandle                                  m_source;
	tt::math::Vector2                                     m_worldPos;
	tt::math::TimedLinearInterpolation<tt::math::Vector2> m_offset;
	tt::math::TimedLinearInterpolation<real>              m_strength;
	tt::math::TimedLinearInterpolation<real>              m_radius;
	real                                                  m_radiusSquared;
	tt::math::TimedLinearInterpolation<real>              m_spread;
	bool                                                  m_enabled;
	bool                                                  m_blockedAtSource;
	bool                                                  m_affectsEntities;
	
	LightShape                                            m_lightShape;
	u8                                                    m_colorAlpha;
	real                                                  m_textureRotationSpeed; // in rad per second.
	
	GlowPtr                                               m_glow;
	
	JitterEffectPtr                                       m_jitterEffect;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_LIGHT_H)
