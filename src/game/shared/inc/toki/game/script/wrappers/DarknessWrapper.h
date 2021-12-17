#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_DARKNESSWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_DARKNESSWRAPPER_H


#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/script/wrappers/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'Darkness' in Squirrel. */
class DarknessWrapper
{
public:
	inline DarknessWrapper() { }
	explicit DarknessWrapper(const light::DarknessHandle& p_darknessHandle);
	
	// bindings
	
	/*! \brief Returnes whether or not this darkness is enabled/disabled. */
	bool isEnabled() const;
	
	/*! \brief Enables/disables this darkness. */
	void setEnabled(bool p_enabled);
	
	/*! \brief Gets the width of this darkness. Returns 0.0 if darkness doesn't exist anymore. */
	real getWidth() const;
	
	/*! \brief Gets the height of this darkness. Returns 0.0 if darkness doesn't exist anymore. */
	real getHeight() const;
	
	/*! \brief Tests whether this Darkness is equal to another.
	    \param p_other the other DarknessWrapper. */
	inline bool equals(const DarknessWrapper* p_other) const
	{
		return p_other != 0 && m_darknessHandle == p_other->m_darknessHandle;
	}
	
	/*! \brief Returns the handle value of this light. */
	inline s32 getHandleValue() const { return static_cast<s32>(m_darknessHandle.getValue()); }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline       light::DarknessHandle& getHandle()       { return m_darknessHandle; }
	inline const light::DarknessHandle& getHandle() const { return m_darknessHandle; }
	
private:
	light::DarknessHandle m_darknessHandle;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_DARKNESSWRAPPER_H)
