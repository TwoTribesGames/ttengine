#if !defined(INC_TT_ENGINE_EFFECT_ZONEINTERSECTIONWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_ZONEINTERSECTIONWRAPPER_H

#include <squirrel/squirrel.h>
#include <spark/Extensions/Zones/SPK_ZoneIntersection.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ZoneIntersectionWrapper : public ZoneWrapper
{
public:
	static const std::size_t defaultZoneCount = 8;

	ZoneIntersectionWrapper()
	{
		m_zone = SPK::ZoneIntersection::create(defaultZoneCount);
	}

	~ZoneIntersectionWrapper()
	{
		SPK_Destroy(m_zone);
	}


	inline void addZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::ZoneIntersection*>(m_zone)->addZone(p_zone.getZone());
	}

	inline void removeZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::ZoneIntersection*>(m_zone)->removeZone(p_zone.getZone());
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ZoneIntersectionWrapper>::init(p_vm->getVM(), _SC("ZoneIntersection"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "addZone",    &ZoneIntersectionWrapper::addZone);
		sqbind_method(p_vm->getVM(), "removeZone", &ZoneIntersectionWrapper::removeZone);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_ZONEINTERSECTIONWRAPPER_H
