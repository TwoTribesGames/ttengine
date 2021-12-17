#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_POWERBEAMGRAPHICWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_POWERBEAMGRAPHICWRAPPER_H


#include <tt/code/fwd.h>

#include <toki/game/entity/graphics/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'PowerBeamGraphic' in Squirrel. */
class PowerBeamGraphicWrapper
{
public:
	PowerBeamGraphicWrapper();
	PowerBeamGraphicWrapper(const entity::graphics::PowerBeamGraphicHandle& p_powerBeamGraphic);
	
	
	// Squirrel bindings:
	
	// ... none yet
	
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline const entity::graphics::PowerBeamGraphicHandle& getHandle() const { return m_powerBeamGraphic; }
	inline       entity::graphics::PowerBeamGraphicHandle& getHandle()       { return m_powerBeamGraphic; }
	
private:
	entity::graphics::PowerBeamGraphicHandle m_powerBeamGraphic;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_POWERBEAMGRAPHICWRAPPER_H)
