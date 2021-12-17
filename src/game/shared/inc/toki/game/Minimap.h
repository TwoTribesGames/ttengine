#if !defined(INC_TOKI_GAME_MINIMAP_H)
#define INC_TOKI_GAME_MINIMAP_H

#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>

#include <toki/serialization/fwd.h>

namespace toki {
namespace game {

class Minimap
{
public:
	Minimap();
	
	inline bool isEnabled()            const { return m_enabled;           }
	inline bool isHidden()             const { return m_hidden;            }
	inline real getSidesBorder()       const { return m_sidesBorder;       }
	inline real getYOffset()           const { return m_yOffset;           }
	inline s32  getBiggestLevelWidth() const { return m_biggestLevelWidth; }
	inline real getGraphicsSize()      const { return m_graphicsSize;      }
	inline real getExtraWidth()        const { return m_extraWidth;        }
	inline real getSideFadeWidth()     const { return m_sideFadeWidth;     }
	tt::math::Vector2 getPositionFromWorld(const tt::math::Vector2& p_pos) const;
	
	inline void setEnabled(bool p_enabled)        { m_enabled           = p_enabled; updateGraphics(); }
	inline void setHidden( bool p_hidden)         { m_hidden            = p_hidden;  updateGraphics(); }
	inline void setSidesBorder( real p_size)      { m_sidesBorder       = p_size;    updateGraphics(); }
	inline void setYOffset(real p_size)           { m_yOffset           = p_size;    updateGraphics(); }
	inline void setBiggestLevelWidth(s32 p_width) { m_biggestLevelWidth = p_width;   updateGraphics(); }
	inline void setGraphicsSize(real p_size)      { m_graphicsSize      = p_size;    updateGraphics(); }
	inline void setExtraWidth(real p_size)        { m_extraWidth        = p_size;    updateGraphics(); }
	inline void setSideFadeWidth(real p_size)     { m_sideFadeWidth     = p_size;    updateGraphics(); }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	void render(const tt::pres::PresentationMgrPtr& p_presMgr);
	
private:
	void updateGraphics();
	
	bool                                m_enabled;
	bool                                m_hidden;
	real                                m_sidesBorder;
	real                                m_yOffset;
	s32                                 m_biggestLevelWidth;
	real                                m_graphicsSize;
	real                                m_extraWidth;
	real                                m_sideFadeWidth;
	tt::engine::renderer::QuadBufferPtr m_graphics;
	bool                                m_needsUpdate;
};


// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_MINIMAP_H)
