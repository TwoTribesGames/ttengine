#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_TILESENSORWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_TILESENSORWRAPPER_H


#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/script/wrappers/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'TileSensor' in Squirrel. */
class TileSensorWrapper
{
public:
	inline TileSensorWrapper() { }
	explicit TileSensorWrapper(const entity::sensor::TileSensorHandle& p_sensorHandle);
	
	// bindings
	
	/*! \brief Returnes whether or not this sensor is enabled/disabled */
	bool isEnabled() const;
	
	/*! \brief Enables/disables this sensor */
	void setEnabled(bool p_enabled);
	
	/*! \brief Returns whether or not this sensor is suspended. */
	bool isSuspended() const;
	
	/*! \brief Suspend this sensor. */
	void setSuspended(bool p_suspend);
	
	/*! \brief Sets the shape of this sensor
	    \param p_shape a shape, can be null */
	void setShape(const ShapeWrapper* p_shape);
	
	/*! \brief Sets this sensor on a world space position. TileSensor is now in "world space" mode. */
	void setWorldPosition(const tt::math::Vector2& p_position);
	
	/*! \brief Gets the world position of this sensor */
	tt::math::Vector2 getWorldPosition() const;
	
	/*! \brief Sets the offset of this sensor in local space. TileSensor is now in "local space" mode. */
	void setOffset(const tt::math::Vector2& p_offset);
	
	/*! \brief Returns the flag to ignore own collision tiles.*/
	bool hasIgnoreOwnCollision() const;
	
	/*! \brief Sets the flag to ignore own collision tiles. Default is false.
	    If the entity doesn't have collision tiles, don't set this flag for optimization purposes
	    \param p_ignoreOwnCollision ignore own collision tiles or not */
	void setIgnoreOwnCollision(bool p_ignoreOwnCollision);
	
	/*! \brief Tests whether this TileSensor is equal to another
	    \param p_other the other sensor */
	inline bool equals(const TileSensorWrapper* p_other) const
	{
		return p_other != 0 && m_sensorHandle == p_other->m_sensorHandle;
	}
	
	/*! \brief Returns the handle value of this sensor */
	inline s32 getHandleValue() const { return m_sensorHandle.getValue(); }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline       entity::sensor::TileSensorHandle& getHandle()       { return m_sensorHandle; }
	inline const entity::sensor::TileSensorHandle& getHandle() const { return m_sensorHandle; }
	
private:
	entity::sensor::TileSensorHandle m_sensorHandle;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_TILESENSORWRAPPER_H)
