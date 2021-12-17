#if !defined(INC_TT_ENGINE_EFFECT_GROUPWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_GROUPWRAPPER_H

#include <squirrel/squirrel.h>

#include <spark/Core/SPK_Group.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>
#include <tt/engine/effect/wrapper/ModelWrapper.h>
#include <tt/engine/effect/wrapper/ModifierWrapper.h>
#include <tt/engine/effect/wrapper/RendererWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class Modifier;


class GroupWrapper
{
public:
	GroupWrapper()
	:
	m_group(0)
	{
	}

	~GroupWrapper()
	{
		if(m_group != 0)
		{
			SPK_Destroy(m_group);
		}
	}


	/*! \brief Construct a group from a Model */
	// TODO: Make custom constructor for this!
	inline void construct(const ModelWrapper& p_model, int p_capacity)
	{
		m_group = SPK::Group::create(p_model.getModel(), p_capacity);
	}


	/*! \brief Set the gravity for this group */
	inline void setFriction(float p_friction)
	{
		TT_NULL_ASSERT(m_group);
		m_group->setFriction(p_friction);
	}


	/*! \brief Set the gravity for this group */
	inline void setGravity(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_group);
		m_group->setGravity(SPK::Vector3D(p_x, p_y, p_z));
	}

	inline void enableSorting(bool p_sort)
	{
		TT_NULL_ASSERT(m_group);
		m_group->enableSorting(p_sort);
	}


	inline void setRenderer(const RendererWrapper& p_renderer)
	{
		m_group->setRenderer(p_renderer.getRenderer());
	}


	/*! \brief Add emitter to this group */
	void addEmitter(const EmitterWrapper& p_emitter)
	{
		TT_NULL_ASSERT(m_group);
		m_group->addEmitter(p_emitter.getEmitter());
	}


	/*! \brief Add modifier to this group */
	void addModifier(const ModifierWrapper& p_modifier)
	{
		TT_NULL_ASSERT(m_group);
		m_group->addModifier(p_modifier.getModifier());
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<GroupWrapper>::init(p_vm->getVM(), _SC("Group"));
		sqbind_method<GroupWrapper>(p_vm->getVM(), "construct",     &GroupWrapper::construct);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "setFriction",   &GroupWrapper::setFriction);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "setGravity",    &GroupWrapper::setGravity);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "enableSorting", &GroupWrapper::enableSorting);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "setRenderer", &GroupWrapper::setRenderer);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "addModifier",   &GroupWrapper::addModifier);
		sqbind_method<GroupWrapper>(p_vm->getVM(), "addEmitter",    &GroupWrapper::addEmitter);
	}

private:
	friend class SystemWrapper;
	inline SPK::Group* getGroup() const {return m_group;}
	
	SPK::Group* m_group;
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_GROUPWRAPPER_H
