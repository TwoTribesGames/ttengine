#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECTMGR_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECTMGR_H


#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/effect/fwd.h>
#include <toki/game/entity/effect/types.h>
#include <toki/game/entity/effect/EffectRect.h>
#include <toki/game/entity/effect/EffectMgr.h>
#include <toki/game/Camera.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class EffectRectMgr
{
public:
	explicit EffectRectMgr(s32 p_reserveCount);
	
	EffectRectHandle createEffectRect(EffectRectTarget            p_targetType,
	                                  const entity::EntityHandle& p_owner);
	
	void destroyEffectRect(EffectRectHandle& p_handle);
	inline EffectRect* getEffectRect(const EffectRectHandle& p_handle)
	{
		return m_effectRects.get(p_handle);
	}
	
	void update(real p_elapsedTime, const Camera& p_camera);
	void renderDebug() const;
	
	inline void reset() { m_effectRects.reset(); m_effectMgr.reset();}
	
	EffectMgr& getEffectMgr() { return m_effectMgr; }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<EffectRect> EffectRects;
	
	EffectRects                   m_effectRects;
	EffectRect::EffectRectContext m_latestContext;
	EffectMgr                     m_effectMgr;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTRECTMGR_H)
