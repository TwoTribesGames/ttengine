#include <tt/com/ComponentBase.h>
#include <tt/com/Message.h>
#include <tt/com/ComponentManager.h>


namespace tt {
namespace com {

//------------------------------------------------------------------------------
// Public member functions

ComponentBase::ComponentBase()
:
m_objectID()
#if !defined(TT_BUILD_FINAL)
,
m_componentName()
#endif
{
}


ComponentBase::~ComponentBase()
{
}


bool ComponentBase::handleAddToObject(ObjectID /* p_objectID */)
{
	return true;
}


void ComponentBase::handleRemoveFromObject()
{
}


MessageResult ComponentBase::handleMessage(const Message& /* p_message */)
{
	return MessageResult_Unknown;
}


//------------------------------------------------------------------------------
// Private member functions

bool ComponentBase::removeSelf(ComponentManager& p_manager)
{
	return p_manager.removeComponent(this);
}

// Namespace end
}
}
