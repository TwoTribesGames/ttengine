#if !defined(INC_TT_ENGINE_EFFECT_ZONEUNIONWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_ZONEUNIONWRAPPER_H

#include <squirrel/squirrel.h>
#include <spark/Extensions/Zones/SPK_ZoneUnion.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ZoneUnionWrapper : public ZoneWrapper
{
public:
	static const std::size_t defaultZoneCount = 8;

	ZoneUnionWrapper()
	{
		m_zone = SPK::ZoneUnion::create(defaultZoneCount);
	}

	~ZoneUnionWrapper()
	{
		SPK_Destroy(m_zone);
	}


	inline void addZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::ZoneUnion*>(m_zone)->addZone(p_zone.getZone());
	}

	inline void removeZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::ZoneUnion*>(m_zone)->removeZone(p_zone.getZone());
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ZoneUnionWrapper>::init(p_vm->getVM(), _SC("ZoneUnion"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "addZone",    &ZoneUnionWrapper::addZone);
		sqbind_method(p_vm->getVM(), "removeZone", &ZoneUnionWrapper::removeZone);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_ZONEUNIONWRAPPER_H
