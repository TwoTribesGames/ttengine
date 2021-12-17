#if !defined(INC_TT_ENGINE_EFFECT_ZONEWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_ZONEWRAPPER_H

#include <spark/Core/SPK_Zone.h>
#include <squirrel/squirrel.h>

#include <tt/script/VirtualMachine.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ZoneWrapper
{
public:
	inline ZoneWrapper()
	:
	m_zone(0)
	{ }
	
	inline ~ZoneWrapper() { }
	
	inline void setPosition(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_zone);
		m_zone->setPosition(SPK::Vector3D(p_x, p_y, p_z));
	}
	
	
	static inline void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ZoneWrapper>::init(p_vm->getVM(), _SC("Zone"), (HSQOBJECT*)0, false);
		sqbind_method(p_vm->getVM(), "setPosition", &ZoneWrapper::setPosition);
	}
	
	inline SPK::Zone* getZone() const { return m_zone; }
	
	// FIXME: Should copying be enabled for this class?
	inline ZoneWrapper(const ZoneWrapper& p_rhs)
	:
	m_zone(p_rhs.m_zone)
	{ }
	
	inline ZoneWrapper& operator=(const ZoneWrapper& p_rhs)
	{
		m_zone = p_rhs.m_zone;
		return *this;
	}
	
protected:
	SPK::Zone* m_zone;
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_ZONEWRAPPER_H
