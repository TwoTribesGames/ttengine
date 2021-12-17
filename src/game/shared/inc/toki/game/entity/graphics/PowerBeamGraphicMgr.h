#if !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHICMGR_H)
#define INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHICMGR_H


#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

class PowerBeamGraphicMgr
{
public:
	explicit PowerBeamGraphicMgr(s32 p_reserveCount);
	
	PowerBeamGraphicHandle createPowerBeamGraphic(PowerBeamType               p_type,
	                                              const sensor::SensorHandle& p_source);
	
	void destroyPowerBeamGraphic(PowerBeamGraphicHandle& p_handle);
	inline PowerBeamGraphic* getPowerBeamGraphic(const PowerBeamGraphicHandle& p_handle)
	{
		return m_powerBeamGraphics.get(p_handle);
	}
	
	void update(real p_elapsedTime);
	void updateForRender(const tt::math::VectorRect& p_visibilityRect);
	void render         (const tt::math::VectorRect& p_visibilityRect) const;
	void renderLightmask(const tt::math::VectorRect& p_visibilityRect) const;
	
	inline void reset() { m_powerBeamGraphics.reset(); }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<PowerBeamGraphic> PowerBeamGraphics;
	
	PowerBeamGraphics m_powerBeamGraphics;
	bool              m_graphicsNeedUpdate;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_POWERBEAMGRAPHICMGR_H)
