#if !defined(INC_TT_ENGINE_EFFECT_LINEWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_LINEWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class LineWrapper : public ZoneWrapper
{
public:
	LineWrapper()
	{
		m_zone = SPK::Line::create();
	}

	~LineWrapper()
	{
		SPK_Destroy(m_zone);
	}

	inline void setBounds(float p_x1, float p_y1, float p_z1, float p_x2, float p_y2, float p_z2)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::Line*>(m_zone)->setBounds(
			SPK::Vector3D(p_x1, p_y1, p_z1), SPK::Vector3D(p_x2, p_y2, p_z2));
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<LineWrapper>::init(p_vm->getVM(), _SC("Line"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "setBounds", &LineWrapper::setBounds);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_LINEWRAPPER_H
