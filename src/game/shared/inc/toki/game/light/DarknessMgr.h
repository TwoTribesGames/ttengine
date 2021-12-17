#if !defined(INC_TOKI_GAME_LIGHT_DARKNESSMGR_H)
#define INC_TOKI_GAME_LIGHT_DARKNESSMGR_H

#include <tt/code/HandleArrayMgr.h>
#include <tt/math/Rect.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/light/Darkness.h>
#include <toki/game/light/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace light {

class DarknessMgr
{
public:
	DarknessMgr(s32 p_reserveCount);
	~DarknessMgr();
	
	DarknessHandle createDarkness(const entity::EntityHandle& p_source,
	                              real                        p_width,
	                              real                        p_height);
	void destroyDarkness(DarknessHandle& p_handle);
	inline Darkness* getDarkness(const DarknessHandle& p_handle) { return m_darknesses.get(p_handle); }
	
	void updateRects(Rects* p_rects, u8 p_ambient);
	void renderDarkness() const;
	
	void handleLevelResized();
	void resetLevel();
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<Darkness> Darknesses;
	Darknesses m_darknesses;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_DARKNESSMGR_H)
