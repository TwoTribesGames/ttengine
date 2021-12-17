#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECT_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECT_H

#include <algorithm>

#include <tt/code/fwd.h>

#include <toki/game/entity/effect/fwd.h>
#include <toki/game/entity/effect/types.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class EffectRect
{
public:
	struct CreationParams
	{
		inline CreationParams(EffectRectTarget            p_targetType,
		                      const entity::EntityHandle& p_owner)
		:
		targetType(p_targetType),
		owner(p_owner)
		{ }
		
		EffectRectTarget     targetType;
		entity::EntityHandle owner;
	};
	typedef const CreationParams& ConstructorParamType;
	
	struct EffectRectContext
	{
		tt::math::Vector2 cameraPos;
		tt::math::Vector2 controllingEntityPos;
		
		EffectRectContext(const tt::math::Vector2& p_cameraPos,
		                  const tt::math::Vector2& p_controllingEntityPos)
		:
		cameraPos(           p_cameraPos),
		controllingEntityPos(p_controllingEntityPos)
		{ }
	};
	
	EffectRect(const CreationParams&   p_creationParams,
	           const EffectRectHandle& p_ownHandle);
	
	void setSize(const tt::math::Vector2& p_size);
	void setBorder(const tt::math::Vector2& p_borderSize);
	inline void setBorderSize(  real p_size) { setBorder(tt::math::Vector2(p_size, p_size)); }
	inline void setLeftBorder(  real p_size) { m_borderBottomLeft.x = std::abs(p_size); }
	inline void setRightBorder( real p_size) { m_borderTopRight.x   = std::abs(p_size); }
	inline void setTopBorder(   real p_size) { m_borderTopRight.y   = std::abs(p_size); }
	inline void setBottomBorder(real p_size) { m_borderBottomLeft.y = std::abs(p_size); }
	
	inline void setOffset(const tt::math::Vector2& p_offset) { m_offset = p_offset; }
	
	inline void setBaseStrengthInstant(real p_strength)
	{
		m_baseStrengthStart = p_strength;
		m_baseStrengthEnd   = p_strength;
		m_baseStrengthTime  = 1.0f;
	}
	inline void setBaseStrength(real p_strength)
	{
		m_baseStrengthStart = getCurrentBaseStrength();
		m_baseStrengthEnd   = p_strength;
		m_baseStrengthTime  = 0.0f;
	}
	
	inline real getEffectStrength() const { return m_effectStrength; }
	
	void update(real p_elapsedTime, const EffectRectContext& p_context);
	void renderDebug() const;
	bool intersects(const EffectRect& p_rhs) const;
	
	inline const EffectRectHandle& getHandle() const { return m_ownHandle; }
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static EffectRect* getPointerFromHandle(const EffectRectHandle& p_handle);
	void invalidateTempCopy() {}
	
private:
	inline real getCurrentBaseStrength() const
	{
		return m_baseStrengthStart * (1.0f - m_baseStrengthTime) + m_baseStrengthEnd * m_baseStrengthTime;
	}
	
	EffectRectHandle m_ownHandle;
	
	EffectRectTarget     m_targetType;
	entity::EntityHandle m_owner;
	
	// The effect rect is center on owner position.
	tt::math::Vector2 m_offset;
	tt::math::Vector2 m_halfRectSize;
	tt::math::Vector2 m_borderTopRight;
	tt::math::Vector2 m_borderBottomLeft;
	real              m_baseStrengthStart;
	real              m_baseStrengthEnd;
	real              m_baseStrengthTime;
	
	// The following is caculated each frame:
	real              m_effectStrength;
	
#if !defined(TT_BUILD_FINAL)
	tt::math::Vector2 m_centerPos; // Stored for debug rendering.
#endif
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECT_H)
