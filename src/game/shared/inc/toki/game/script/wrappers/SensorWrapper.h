#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SENSORWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_SENSORWRAPPER_H

#include <vector>

#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/script/fwd.h>
#include <toki/game/script/wrappers/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


typedef std::vector<SensorWrapper> SensorWrappers;


/*! \brief 'Sensor' in Squirrel. */
class SensorWrapper
{
public:
	inline SensorWrapper() { }
	explicit SensorWrapper(const entity::sensor::SensorHandle& p_sensorHandle);
	
	// bindings
	
	/*! \brief Returns whether or not this sensor is enabled/disabled */
	bool isEnabled() const;
	
	/*! \brief Enables/disables this sensor */
	void setEnabled(bool p_enabled);
	
	/*! \brief Returns whether or not this sensor is suspended. */
	bool isSuspended() const;
	
	/*! \brief Suspend this sensor. */
	void setSuspended(bool p_suspend);
	
	/*! \brief Get this sensors delay in seconds. (How long it has to 'see/feel' an entity before it response. */
	real getDelay() const;
	
	/*! \brief Set this sensors delay in seconds. (How long it has to 'see/feel' an entity before it response. */
	void setDelay(real p_delayInSeconds);
	
	/*! \brief Sets the shape of this sensor
	    \param p_shape a shape, can be null */
	void setShape(const ShapeWrapper* p_shape);
	
	/*! \brief Sets the enter and exit callback to the default.
	           For Enter: "onTouchEnter" for touch sensor, and "onSightEnter" for sight sensor.
	           For Exit: "onTouchExit" for touch sensor, and "onSightExit" for sight sensor. */
	void setDefaultEnterAndExitCallback();
	
	/*! \brief Set the name for the enter callback function.*/
	void setEnterCallback(const std::string& p_callbackName);
	
	/*! \brief Get the name for the enter callback function.*/
	const std::string& getEnterCallback() const;
	
	/*! \brief Set the name for the exit callback function.*/
	void setExitCallback(const std::string& p_callbackName);
	
	/*! \brief Get the name for the exit callback function.*/
	const std::string& getExitCallback() const;
	
	/*! \brief Set the name for the filter callback function.*/
	void setFilterCallback(const std::string& p_callbackName);
	
	/*! \brief Get the name for the filter callback function.*/
	const std::string& getFilterCallback() const;
	
	/*! \brief Returns the target of this Sensor */
	EntityBase* getTarget() const;
	
	/*! \brief Sets the target of this sensor
	    \param p_target a target, can be null */
	void setTarget(const EntityBase* p_target);
	
	/*! \brief Returns all entities sensed by this sensor. */
	EntityBaseCollection getSensedEntities() const;
	
	/*! \brief Removes all entities currently being sensed by this sensor. No callbacks will be triggered. */
	void removeAllSensedEntities();
	
	/*! \brief Removes all entities currently being sensed by this sensor. onExit callbacks will be triggered for every sensed entity. */
	void removeAllSensedEntitiesWithCallbacks();
	
	/*! \brief Sets this sensor on a world space position. Sensor is now in "world space" mode. */
	void setWorldPosition(const tt::math::Vector2& p_position);
	
	/*! \brief Gets the world position of this sensor */
	tt::math::Vector2 getWorldPosition() const;
	
	/*! \brief Sets the offset of this sensor in local space. Sensor is now in "local space" mode. */
	void setOffset(const tt::math::Vector2& p_offset);
	
	/*! \brief Sets the offset of the raytracer of this sensor in local space. Only valid for sight sensors! */
	void setRayTraceOffset(const tt::math::Vector2& p_offset);
	
	/*! \brief Returns the flag to ignore active collision tiles.*/
	bool hasIgnoreActiveCollision() const;
	
	/*! \brief Sets the flag to ignore active collision tiles. Default is false.
	    \param p_ignoreOwnCollision ignore active collision tiles or not */
	void setIgnoreActiveCollision(bool p_ignoreActiveCollision);
	
	/*! \brief Returns the flag to ignore own collision tiles.*/
	bool hasIgnoreOwnCollision() const;
	
	/*! \brief Sets the flag to ignore own collision tiles. Default is false.
	    If the entity doesn't have collision tiles, don't set this flag for optimization purposes
	    \param p_ignoreOwnCollision ignore own collision tiles or not */
	void setIgnoreOwnCollision(bool p_ignoreOwnCollision);
	
	/*! \brief Returns flag to ignore darkness. */
	bool isEnabledInDarkness() const;
	
	/*! \brief Sets the flag to enable this sensor in darkness.
	    Default is true for touch sensors, false for all other sensors
	    \param p_enable enable or not */
	void setEnabledInDarkness(bool p_enable);
	
	/*! \brief Returns flag if distance sort is set.*/
	bool hasDistanceSort() const;
	
	/*! \brief Sets the flag to sort entities based on the distance to this sensor, before calling the callbacks (default is false)
	    NOTE: Don't use this by default, as it will have a performance impact.*/
	void setDistanceSort(bool p_distanceSort);
	
	/*! \brief Tests whether this Sensor is equal to another
	    \param p_other the other sensor */
	inline bool equals(const SensorWrapper* p_other) const
	{
		return p_other != 0 && m_sensorHandle == p_other->m_sensorHandle;
	}
	
	/*! \brief Returns the handle value of this sensor */
	inline s32 getHandleValue() const { return m_sensorHandle.getValue(); }
	
	/*! \brief Set whether ray tracing should stop when it encounters empty tiles
	           ('air': no collision or fluids, meaning this sensor will or will not see through empty tiles).<br/>
	           Default setting: false */
	void setStopOnEmpty(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters solid tiles
	           (meaning this sensor will or will not see through solid tiles).<br/>
	           Default setting: true */
	void setStopOnSolid(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters crystal tiles
	           (meaning this sensor will or will not see through crystal tiles).<br/>
	           Default setting: true */
	void setStopOnCrystal(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters a water pool
	           (meaning this sensor will or will not see through water pools).<br/>
	           Default setting: false */
	void setStopOnWaterPool(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters a waterfall
	           (meaning this sensor will or will not see through waterfalls).<br/>
	           Default setting: false */
	void setStopOnWaterFall(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters a lava pool
	           (meaning this sensor will or will not see through lava pools).<br/>
	           Default setting: false */
	void setStopOnLavaPool(bool p_stop);
	
	/*! \brief Set whether ray tracing should stop when it encounters a lava fall
	           (meaning this sensor will or will not see through lava falls).<br/>
	           Default setting: false */
	void setStopOnLavaFall(bool p_stop);
	
	/*! \brief Get whether ray tracing should stop when it encounters empty tiles
	           ('air': no collision or fluids, meaning this sensor will or will not see through empty tiles). */
	bool getStopOnEmpty() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters solid tiles
	           (meaning this sensor will or will not see through solid tiles). */
	bool getStopOnSolid() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters crystal tiles
	           (meaning this sensor will or will not see through crystal tiles). */
	bool getStopOnCrystal() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters a water pool
	           (meaning this sensor will or will not see through water pools). */
	bool getStopOnWaterPool() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters a waterfall
	           (meaning this sensor will or will not see through waterfalls). */
	bool getStopOnWaterFall() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters a lava pool
	           (meaning this sensor will or will not see through lava pools). */
	bool getStopOnLavaPool() const;
	
	/*! \brief Get whether ray tracing should stop when it encounters a lava fall
	           (meaning this sensor will or will not see through lava falls). */
	bool getStopOnLavaFall() const;
	
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline       entity::sensor::SensorHandle& getHandle()       { return m_sensorHandle; }
	inline const entity::sensor::SensorHandle& getHandle() const { return m_sensorHandle; }
	
private:
	entity::sensor::SensorHandle m_sensorHandle;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SENSORWRAPPER_H)
