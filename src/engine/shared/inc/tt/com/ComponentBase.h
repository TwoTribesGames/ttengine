#if !defined(INC_TT_COM_COMPONENTBASE_H)
#define INC_TT_COM_COMPONENTBASE_H


#include <tt/com/types.h>
#include <tt/com/Message.h>


namespace tt {
namespace com {

class ComponentManager;


class ComponentBase
{
public:
	ComponentBase();
	virtual ~ComponentBase();
	
	virtual bool handleAddToObject(ObjectID p_objectID);
	virtual void handleRemoveFromObject();
	
	inline ObjectID getObjectID() const { return m_objectID; }
	
	virtual MessageResult handleMessage(const Message& p_message);
	virtual ComponentID getComponentID() const = 0;
	
	/*! \brief DEBUG FUNCTION! Returns the class name (of the concrete
	           derived class) of this component, to make providing
	           sensible debug output (for a profiler, for example) easier. */
#ifndef TT_BUILD_FINAL
	inline const std::string& getComponentName()
	{
		// HACK: if baseclass name is empty, set componentName
		if (m_componentName.empty())
		{
			setComponentName(typeid(*this).name());
		}
		
		return m_componentName;
	}
	inline void setComponentName(const std::string& p_name) { m_componentName = p_name; }
#else
	inline void setComponentName(const std::string& /*p_name*/) { }
#endif
	
protected:
	/*! \brief Removes this component from the object it was added to. */
	bool removeSelf(ComponentManager& p_manager);
	
private:
	inline void setObjectID(ObjectID p_objectID) { m_objectID = p_objectID; }
	
	// Disallow copying
	ComponentBase(const ComponentBase&);
	const ComponentBase& operator=(const ComponentBase&);
	
	
	ObjectID m_objectID;
	
#ifndef TT_BUILD_FINAL
	std::string m_componentName;
#endif
	
	friend class ComponentManager;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_COMPONENTBASE_H)
