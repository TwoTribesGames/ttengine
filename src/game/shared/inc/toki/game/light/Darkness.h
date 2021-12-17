#if !defined(INC_TOKI_GAME_LIGHT_DARKNESS_H)
#define INC_TOKI_GAME_LIGHT_DARKNESS_H


#include <toki/game/entity/fwd.h>
#include <toki/game/light/fwd.h>


namespace toki {
namespace game {
namespace light {

class Darkness
{
public:
	struct CreationParams
	{
		inline CreationParams(const entity::EntityHandle& p_source,
		                      real                        p_width,
		                      real                        p_height)
		:
		source(p_source),
		width(p_width),
		height(p_height)
		{ }
		
		entity::EntityHandle source;
		real                 width;
		real                 height;
	};
	typedef const CreationParams& ConstructorParamType;
	
	Darkness(const CreationParams& p_creationParams, const DarknessHandle& p_ownHandle);
	
	tt::math::VectorRect getRect() const;
	void render() const;
	
	inline real getWidth() const           { return m_width;        }
	//inline void setWidth(real p_width)     { m_width = p_width;     }
	inline real getHeight() const          { return m_height;       }
	//inline void setHeight(real p_height)   { m_height = p_height;   }
	
	inline bool isEnabled() const          { return m_enabled;      }
	inline void setEnabled(bool p_enabled) { m_enabled = p_enabled; }
	
	inline const DarknessHandle& getHandle() const { return m_ownHandle; }
	
	void setAmbient(u8 p_ambient);
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static Darkness* getPointerFromHandle(const DarknessHandle& p_handle);
	void invalidateTempCopy() {}
	
private:
	tt::math::Vector2 getWorldPosition() const;
	
	DarknessHandle       m_ownHandle;
	entity::EntityHandle m_source;
	real                 m_width;
	real                 m_height;
	bool                 m_enabled;
	PolygonPtr           m_poly;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_DARKNESS_H)
