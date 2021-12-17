#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_SENSOR_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_SENSOR_H

#include <vector>

#include <tt/code/fwd.h>
#include <tt/code/BitMask.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/thread/Mutex.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/sensor/RayTracer.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/script/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class Sensor
{
public:
	struct CreationParams
	{
		inline CreationParams(SensorType          p_type,
		                      const EntityHandle& p_source,
		                      const ShapePtr&     p_shape,
		                      const EntityHandle& p_target)
		:
		type(p_type),
		source(p_source),
		shape(p_shape),
		target(p_target)
		{ }
		
		SensorType   type;
		EntityHandle source;
		ShapePtr     shape;
		EntityHandle target;
	};
	typedef const CreationParams& ConstructorParamType;
	
	
	Sensor(const CreationParams& p_creationParams, const SensorHandle& p_ownHandle);
	~Sensor();
	
	void update();
	void updateCallbacks(real64 p_gameTime);
	
	void renderDebug() const;
	
	inline bool isInLocalSpace() const { return m_inLocalSpace; } //!< false means worldspace.
	void setWorldPosition(const tt::math::Vector2& p_position);
	tt::math::Vector2 getWorldPosition() const;
	void setOffset(const tt::math::Vector2& p_offset);
	
	tt::math::Vector2 getRayTracePosition() const;
	void setRayTraceOffset(const tt::math::Vector2& p_offset);
	
	inline bool hasIgnoreActiveCollision() const { return m_ignoreActiveCollision; }
	void setIgnoreActiveCollision(bool p_ignoreActiveCollision);
	
	inline bool hasIgnoreOwnCollision() const { return m_ignoreOwnCollision; }
	void setIgnoreOwnCollision(bool p_ignoreOwnCollision);
	
	inline const SensorHandle& getHandle() const { return m_handle; }
	inline const EntityHandle& getSource() const { return m_source; }
	
	inline bool isEnabledInDarkness() const { return m_enabledInDarkness; }
	inline void setEnabledInDarkness(bool p_enable) { m_enabledInDarkness = p_enable; }
	
	void setShape(const ShapePtr& p_shape);
	
	inline const EntityHandle& getTarget() const { return m_target; }
	void setTarget(const EntityHandle& p_target);
	
	inline const EntityHandles& getSensedEntities() const { return m_sensedEntities; }
	
	inline bool isEnabled() const { return m_enabled; }
	void setEnabled(bool p_enabled);
	
	inline bool isSuspended() const { return m_suspended; }
	inline void setSuspended(bool p_suspended) { m_suspended = p_suspended; }
	
	inline real getDelay() const { return m_delayInSeconds; }
	inline void setDelay(real p_delayInSeconds) { m_delayInSeconds = p_delayInSeconds; }
	
	inline void setEnterCallback(const std::string& p_callbackName) { m_enterCallback = p_callbackName; }
	inline const std::string& getEnterCallback() const              { return m_enterCallback;           }
	
	inline void setExitCallback(const std::string& p_callbackName) { m_exitCallback = p_callbackName; }
	inline const std::string& getExitCallback() const              { return m_exitCallback;           }
	
	inline void setFilterCallback(const std::string& p_callbackName) { m_filterCallback = p_callbackName; }
	inline const std::string& getFilterCallback() const              { return m_filterCallback;           }
	
	inline bool hasDistanceSort() const { return m_distanceSort; }
	inline void setDistanceSort(bool p_distanceSort) { m_distanceSort = p_distanceSort; }
	
	inline SensorType getType() const { return m_type; }
	
	inline bool hasDirtyFlag() const { return m_isDirty; }
	
	inline       RayTracer& getRayTracer()       { return m_rayTracer; }
	inline const RayTracer& getRayTracer() const { return m_rayTracer; }
	
	// FIXME: Debug functionality
	tt::engine::renderer::ColorRGBA getDebugColor() const;
	
	static bool validate(SensorType p_type, const ShapePtr& p_shape, const EntityHandle& p_target);
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	void removeEntityFilterWithoutUnregister(const entity::EntityHandle& p_entityHandle);
	
	static Sensor* getPointerFromHandle(const SensorHandle& p_handle);
	void invalidateTempCopy();
	
	void removeAllSensedEntities(bool p_handleOnExitCallbacks, bool p_queueCallbacks);
	
private:
	struct FilterResult
	{
	public:
		enum Flag
		{
			Flag_result,
			Flag_used,
			
			Flag_Count
		};
		typedef tt::code::BitMask<Flag, Flag_Count> Flags;
		
		EntityHandle handle;
		Flags        flags;
		
		FilterResult(const EntityHandle& p_handle, bool p_filterResult)
		:
		handle(p_handle),
		flags(Flag_used)
		{
			if (p_filterResult) { flags.setFlag(Flag_result); }
		}
		
		inline bool getResult()  const { return flags.checkFlag(Flag_result); }
		inline bool wasUsed() const { return flags.checkFlag(Flag_used); }
		inline void makeUsed()      {        flags.setFlag(  Flag_used); }
		inline void resetUsed()     {        flags.resetFlag(Flag_used); }
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
		static FilterResult createByUnserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<FilterResult> FilterResults;
	struct DelayedEntity
	{
	public:
		EntityHandle handle;
		real64       time;
		
		DelayedEntity(const EntityHandle& p_handle, real64 p_time)
		:
		handle(p_handle),
		time(p_time)
		{
		}
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
		static DelayedEntity createByUnserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<DelayedEntity> DelayedEntities;
	
	void distanceSort(EntityHandles& p_handles) const;
	void getSensedEntities(Entity* p_source, EntityHandles* p_result_OUT);
	void filterSensedEntities(Entity* p_source, EntityHandles* p_result_OUT);
	bool isTargetInRange(Entity* p_source, Entity* p_target) const;
	bool isActive(const Entity* p_source) const;
	bool isTargetVisible(const Entity* p_target, tt::math::Vector2* p_visiblePoint = 0) const;
	
	void doSensedEntityDelay(real64         p_gameTime,
	                         EntityHandles* p_currentySensedEntities,
	                         EntityHandles* p_difference);
	
	void handleOnEnter(const script::EntityBase& p_source, const EntityHandle& p_target);
	void handleOnExit( const script::EntityBase& p_source, const EntityHandle& p_target,
	                   bool p_queueCallback = true);
	bool doFilterCallback(const script::EntityBase& p_script, const EntityHandle& p_targetEntity) const;
	const FilterResult& getFilterResult(const EntityHandle& p_entityHandle);
	void removeUnusedFilters();
	void removeAllFilters();
	
	bool registerFilterWithEntity(  const EntityHandle& p_entityHandle) const;
	void unregisterFilterWithEntity(const EntityHandle& p_entityHandle) const;
	
	Entity* getSourceEntityForUpdate() const;
	
	// FIXME: Remove hardcoded value
	static const s32 reserveCount = 50;
	
	// --- Handle: ---
	SensorHandle      m_handle;
	
	// --- CreationParams: ---
	SensorType        m_type;
	EntityHandle      m_source;
	ShapePtr          m_shape;
	EntityHandle      m_target;
	
	// --- Other members: ---
	bool              m_inLocalSpace;
	tt::math::Vector2 m_translation;
	
	bool              m_enabled;
	bool              m_suspended;
	real              m_delayInSeconds;
	
	RayTracer         m_rayTracer;
	tt::math::Vector2 m_rayTraceOffset;
	
	EntityHandles     m_sensedEntities;
	FilterResults     m_filteredEntities; // Sensed entities but rejected by the fitler.
	DelayedEntities   m_delayedEntities;
	
	EntityHandles     m_currentySensedEntitiesCache; // Only used in update. Don't serialize. It's a member so we don't create and destroy it so much.
	EntityHandles     m_differenceCache;             // Only used in update. Don't serialize. It's a member so we don't create and destroy it so much.
	
	std::string       m_enterCallback;
	std::string       m_exitCallback;
	std::string       m_filterCallback;
	
	bool              m_enabledInDarkness;
	bool              m_ignoreOwnCollision;
	bool              m_ignoreActiveCollision;
	bool              m_distanceSort;
	
	bool              m_isDirty;
	
	// Threading
	static tt::thread::Mutex ms_sensorMutex;
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_SENSOR_H)
