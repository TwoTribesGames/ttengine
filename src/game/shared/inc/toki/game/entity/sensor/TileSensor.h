#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_TILESENSOR_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_TILESENSOR_H


#include <tt/code/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/game/script/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class TileSensor
{
public:
	struct CreationParams
	{
		inline CreationParams(const EntityHandle& p_source,
		                      const ShapePtr&     p_shape)
		:
		source(p_source),
		shape(p_shape)
		{ }
		
		EntityHandle source;
		ShapePtr     shape;
	};
	typedef const CreationParams& ConstructorParamType;
	
	TileSensor(const CreationParams& p_creationParams, const TileSensorHandle& p_ownHandle);
	~TileSensor();
	
	void update();
	void renderDebug() const;
	
	void setWorldPosition(const tt::math::Vector2& p_position);
	tt::math::Vector2 getWorldPosition() const;
	void setOffset(const tt::math::Vector2& p_offset);
	
	inline bool hasIgnoreOwnCollision() const { return m_ignoreOwnCollision; }
	void setIgnoreOwnCollision(bool p_ignoreOwnCollision);
	
	inline const TileSensorHandle& getHandle() const { return m_ownHandle; }
	inline const EntityHandle& getSource() const { return m_parent; }
	
	void setShape(const ShapePtr& p_shape);
	
	inline bool isEnabled() const { return m_enabled; }
	void setEnabled(bool p_enabled);
	
	inline bool isSuspended() const { return m_suspended; }
	inline void setSuspended(bool p_suspended) { m_suspended = p_suspended; }
	
	inline bool hasDirtyFlag() const { return m_isDirty; }
	
	tt::engine::renderer::ColorRGBA getDebugColor() const;
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	
	static TileSensor* getPointerFromHandle(const TileSensorHandle& p_handle);
	void invalidateTempCopy() {}
	
private:
	void doCallbacks(const level::CollisionTypes& p_newCollisionTouching,
	                 const fluid::FluidTypes&     p_newFluidTouching,
	                 const fluid::FluidTypes&     p_newFallTouching);
	
	bool isActive(const Entity* p_parent) const;
	
	static const s32 reserveCount = 50;
	
	bool              m_inLocalSpace;
	tt::math::Vector2 m_translation;
	
	bool             m_enabled;
	bool             m_suspended;
	TileSensorHandle m_ownHandle;
	EntityHandle     m_parent;
	ShapePtr         m_shape;
	
	bool              m_ignoreOwnCollision;
	
	bool              m_isDirty;
	
	level::CollisionTypes m_collisionTouching;
	fluid::FluidTypes     m_fluidTouching;
	fluid::FluidTypes     m_fallTouching;
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_TILESENSOR_H)
