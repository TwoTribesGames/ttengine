#if !defined(INC_TOKI_GAME_ENTITY_ENTITYMGR_H)
#define INC_TOKI_GAME_ENTITY_ENTITYMGR_H


#include <string>
#include <vector>
#include <algorithm>

#include <squirrel/squirrel.h>

#include <tt/code/HandleArrayMgr.h>
#include <tt/math/Vector2.h>

#include <toki/game/entity/graphics/PowerBeamGraphicMgr.h>
#include <toki/game/entity/graphics/TextLabelMgr.h>
#include <toki/game/entity/movementcontroller/MovementControllerMgr.h>
#include <toki/game/entity/effect/EffectRectMgr.h>
#include <toki/game/entity/sensor/SensorMgr.h>
#include <toki/game/entity/sensor/TileSensorMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/Camera.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/fwd.h>
#include <toki/serialization/fwd.h>
#include <toki/utils/SectionProfiler.h>


namespace toki {
namespace game {
namespace entity {

class EntityMgr : private tt::code::HandleArrayMgr<Entity>
{
public:
	explicit EntityMgr(s32 p_reserveCount);
	~EntityMgr();
	
	void createSpawnSections(const level::entity::EntityInstances& p_instances,
	                         const std::string&                    p_missionID,
	                         bool                                  p_gameReloaded);
	
	/*! \param p_overridePositionEntityID If passed (and a non-negative number),
	                                      the position of this entity (identified by level data ID)
	                                      will be overridden during creation.
	    \param p_overridePosition The position to use instead of the position from level data,
	                              if p_overridePositionEntityID is specified. */
	void createEntities(const level::entity::EntityInstances& p_instances,
	                    const std::string&                    p_missionID                = "",
	                    s32                                   p_overridePositionEntityID = -1,
	                    const tt::math::Vector2&              p_overridePosition         = tt::math::Vector2::zero,
	                    bool                                  p_gameReloaded             = false,
	                    s32                                   p_spawnSectionID           = -1);
	EntityHandle createEntity(const std::string& p_type, s32 p_id);
	
	bool initEntity(const EntityHandle& p_handle, const tt::math::Vector2& p_position,
	                EntityProperties& p_properties);
	
	bool initEntity(const EntityHandle& p_handle, const tt::math::Vector2& p_position,
	                const HSQOBJECT& p_properties);
	
	void deinitEntity(EntityHandle p_handle);
	
	inline Entity* getEntity(const EntityHandle& p_handle) const { return get(p_handle); }
	
	void updateEntities(real p_deltaTime, const Camera& p_camera);
	void updateEntitiesChanges(real p_deltaTime);
	void updateForRender(const tt::math::VectorRect& p_visibilityRect);
	void renderEntitiesDebug();
	void renderPowerBeamGraphics(const tt::math::VectorRect& p_visibilityRect) const;
	void renderTextLabels() const;
	
	inline s32 getActiveEntitiesCount()   const { return getActiveCount(); }
	inline s32 getMaxEntities()           const { return getMax();         }
	inline       Entity* getFirstEntity()       { return getFirst();       }
	inline const Entity* getFirstEntity() const { return getFirst();       }
	inline movementcontroller::MovementControllerMgr& getMovementControllerMgr() { return m_movementControllerMgr; }
	
	inline sensor::SensorMgr&     getSensorMgr()     { return m_sensorMgr;     }
	inline sensor::TileSensorMgr& getTileSensorMgr() { return m_tileSensorMgr; }
	inline graphics::PowerBeamGraphicMgr& getPowerBeamGraphicMgr() { return m_powerBeamGraphicMgr; }
	inline graphics::TextLabelMgr&        getTextLabelMgr()        { return m_textLabelMgr;        }
	inline effect::EffectRectMgr& getEffectRectMgr() { return m_effectRectMgr; }
	
	EntityHandle getEntityHandleByID(s32 p_id) const;
	s32          getEntityIDByHandle(const EntityHandle& p_handle) const;
	void         removeEntityIDFromMapping(s32 p_id);
	
#if !defined(TT_BUILD_FINAL)
	void setDirectionForAllEntities(const tt::math::Vector2& p_dir); // FIXME: Remove this function, it's debug.
	std::string getDebugUnculledInfo() const;
	std::string getDebugCulledInfo() const;
#endif
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
	void resetAll();
	
	inline s32 renderProfiler(s32 p_x, s32 p_y)
	{
		return m_sectionProfiler.render(p_x, p_y, false);
	}
	
	void callOnValidateScriptStateOnAllEntities() const;
	void callFunctionOnAllEntities(const std::string& p_functionName) const;
	void callUpdateOnAllEntities(real p_deltaTime) const;
	
	// For debug purposes
	inline void toggleEntityCulling() { m_entityCullingEnabled = m_entityCullingEnabled == false; }
	
	static void appendMissionSpecificEntities(const level::entity::EntityInstances& p_allEntities,
	                                          const std::string& p_missionID,
	                                          level::entity::EntityInstances& p_missionEntities_OUT);
	
	void initCullingForAllEntities(const Camera& p_camera);
	
#if !defined(TT_BUILD_FINAL)
	std::string getDebugTimings() const;
#endif
	
private:
	struct DeathRowEntry
	{
		s32          framesToLive;
		EntityHandle handle;
	};
	typedef std::vector<DeathRowEntry> DeathRowEntries;
	
	void handleEntityPreSpawn(Entity& p_entity);
	void flushPostCreateSpawn();
	
	void destroyEntity(EntityHandle p_handle);
	
	void updateCullingForAllEntities(const Camera& p_camera);
	void updateIsOnScreenAllEntities(const Camera& p_camera);
	
	EntityMgr(const EntityMgr&);                  // Disabled
	const EntityMgr& operator=(const EntityMgr&); // Disabled
	
	movementcontroller::MovementControllerMgr m_movementControllerMgr;
	DeathRowEntries                           m_deathRow;
	
	graphics::PowerBeamGraphicMgr m_powerBeamGraphicMgr;
	graphics::TextLabelMgr        m_textLabelMgr;
	sensor::SensorMgr     m_sensorMgr;
	sensor::TileSensorMgr m_tileSensorMgr;
	effect::EffectRectMgr m_effectRectMgr;
	
	typedef std::map<s32, entity::EntityHandle> IDToHandleMapping;
	IDToHandleMapping m_idToHandleMapping;
	
	bool          m_isCreatingEntities;
	EntityHandles m_postCreateSpawn;
	
	bool          m_entityCullingEnabled;
	
#if !defined(TT_BUILD_FINAL)
	enum { maxTimingFrames = 60 };
	using Timings = std::map<std::string, u64[maxTimingFrames]>;
	mutable Timings m_timings;
	mutable s32 m_currentTimingFrame{0};
#endif
	
	utils::SectionProfiler<utils::EntityMgrSection, utils::EntityMgrSection_Count> m_sectionProfiler;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_ENTITYMGR_H)
