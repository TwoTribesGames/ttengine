#if !defined(INC_TOKITORI_GAME_SCRIPT_REGISTRY_H)
#define INC_TOKITORI_GAME_SCRIPT_REGISTRY_H


#include <map>

#include <tt/script/helpers.h>
#include <tt/script/ScriptValue.h>
#include <tt/str/str.h>

#include <toki/constants.h>
#include <toki/script/serialization/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace script {

class Registry; // Forward declaration.

Registry& getRegistry();                            //!< \brief Get Registry instance. (Based on AppGlobal::getCurrentProgressType().)
Registry& getRegistry(ProgressType p_progressType); //!< \brief (NOTE: Called getRegistryEx in script!) Get a specific Registry instance.

Registry* getRegistryPtr();                              // (Ptr version for script binding.)
Registry* getRegistryExPtr(ProgressType p_progressType); // (Ptr version for script binding.)

/*! \brief Persistent storage (survives across levels/reloads). */
class Registry
{
public:
	/*! \brief Gets a value with key p_key from the registry or null if key doesn't exist.
	    \param  p_key the key (Optional add extra subkeys delimited by  ".". e.g. "first.second.third".)
	    \return the value with key p_key */
	int get(HSQUIRRELVM p_vm);
	
	/*! \brief Sets a key/value pair in the non-persistent registry. Valid value types are: array, bool, float, integer, null, string or table.
	    \param  p_key the key (Optional add extra subkeys delimited by  ".". e.g. "first.second.third".
	    \param  p_value the value */
	inline int set(HSQUIRRELVM p_vm)
	{
		return set(p_vm, m_registry);
	}
	
	/*! \brief Gets a value with key p_key from the persistent registry or null if key doesn't exist.
	    \param  p_key the key
	    \return the value with key p_key */
	int getPersistent(HSQUIRRELVM p_vm);
	
	/*! \brief Sets a key/value pair in the persistent registry. Valid value types are: array, bool, float, integer, null, string or table.
	    \param  p_key the key
	    \param  p_value the value */
	inline int setPersistent(HSQUIRRELVM p_vm)
	{
		return set(p_vm, m_persistentRegistry);
	}
	
	/*! \brief Gets a key with index p_index from the non-persistent registry.
	           If the key is out-of-bounds, it will panic and return an empty string.
	    \param  p_index the index of the key
	    \return the key */
	inline std::string getKeyAt(s32 p_index) const
	{
		if (p_index < 0 || p_index >= static_cast<s32>(m_registry.size()))
		{
			TT_PANIC("getKeyAt(%d) is out-of-bounds. Index should be in the range [0..%d]",
				p_index, m_registry.size()-1);
			return std::string();
		}
		RegistryValues::const_iterator it(m_registry.begin());
		std::advance(it, p_index);
		
		return it->first;
	}
	
	/*! \brief Gets a key with index p_index from the persistent registry.
	           If the key is out-of-bounds, it will panic and return an empty string.
	    \param  p_index the index of the key
	    \return the key */
	inline std::string getKeyPersistentAt(s32 p_index) const
	{
		if (p_index < 0 || p_index >= static_cast<s32>(m_persistentRegistry.size()))
		{
			TT_PANIC("getKeyPersistentAt(%d) is out-of-bounds. Index should be in the range [0..%d]",
				p_index, m_persistentRegistry.size()-1);
			return std::string();
		}
		RegistryValues::const_iterator it(m_persistentRegistry.begin());
		std::advance(it, p_index);
		
		return it->first;
	}
	
	/*! \brief Returns whether or not a key with name p_key exists in the non-persistent registry.
	    \param  p_key the key
	    \return if p_key exists in the non-persistent registry. */
	inline bool exists(const std::string& p_key) const
	{
		return m_registry.find(p_key) != m_registry.end();
	}
	
	/*! \brief Returns whether or not a key with name p_key exists in the persistent registry.
	    \param  p_key the key
	    \return if p_key exists in the persistent registry. */
	inline bool existsPersistent(const std::string& p_key) const
	{
		return m_persistentRegistry.find(p_key) != m_persistentRegistry.end();
	}
	
	/*! \brief Removes the key/value pair with key p_key from the non-persistent registry.
	    \param  p_key the key to be erased */
	inline void erase(const std::string& p_key)
	{
		RegistryValues::iterator it = m_registry.find(p_key);
		if (it != m_registry.end())
		{
			m_registry.erase(it);
		}
	}
	
	/*! \brief Removes the key/value pair with key p_key from the persistent registry.
	    \param  p_key the key to be erased */
	inline void erasePersistent(const std::string& p_key)
	{
		RegistryValues::iterator it = m_persistentRegistry.find(p_key);
		if (it != m_persistentRegistry.end())
		{
			m_persistentRegistry.erase(it);
		}
	}
	
	/*! \brief Clears the non-persistent registry */
	inline void clear()
	{
		m_registry.clear();
	}
	
	/*! \brief Clears the persistent registry */
	inline void clearPersistent()
	{
		m_persistentRegistry.clear();
	}
	
	/*! \brief Returns the size (number of entries) of the persistent registry */
	inline size_t size() const
	{
		return m_registry.size();
	}
	
	/*! \brief Returns the size (number of entries) of the non-persistent registry */
	inline size_t sizePersistent() const
	{
		return m_persistentRegistry.size();
	}
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline void copyPersistentRegistry(const Registry& p_src) { m_persistentRegistry = p_src.m_persistentRegistry; }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
	void serializePersistent  (tt::code::BufferWriteContext* p_context) const;
	bool unserializePersistent(tt::code::BufferReadContext*  p_context);
	
	inline bool saveFull(tt::fs::FilePtr& p_file) const { return save(m_registry, p_file) && save(m_persistentRegistry, p_file); }
	inline bool loadFull(const tt::fs::FilePtr& p_file) { return load(m_registry, p_file) && load(m_persistentRegistry, p_file); }
	inline bool saveNonPersistent(tt::fs::FilePtr& p_file) const { return save(m_registry, p_file); }
	inline bool loadNonPersistent(const tt::fs::FilePtr& p_file) { return load(m_registry, p_file); }
	
private:
	typedef std::map<std::string, tt::script::ScriptValue> RegistryValues;
	
	static bool save(const RegistryValues& p_registry, tt::fs::FilePtr& p_file);
	static bool load(RegistryValues& p_registry, const tt::fs::FilePtr& p_file);
	
	static bool findValue(const tt::str::Strings&  p_keys,
	                      const RegistryValues&    p_registry,
	                      tt::script::ScriptValue* p_value);
	static bool findValue(const tt::str::Strings&        p_keys,
	                      tt::str::Strings::size_type    p_keysIndex,
	                      const tt::script::ScriptValue& p_rootValue,
	                      tt::script::ScriptValue*       p_value);
	static bool setValue(const tt::str::Strings&        p_keys,
	                     tt::str::Strings::size_type    p_keysIndex,
	                     tt::script::ScriptValue&       p_rootValue,
	                     const tt::script::ScriptValue& p_value);
	
	bool getKeyFromStack(HSQUIRRELVM p_vm, s32 p_idx, std::string& p_key_OUT);
	int set(HSQUIRRELVM p_vm, RegistryValues& p_registry);
	
	static void serialize(const RegistryValues& p_registry, tt::code::BufferWriteContext* p_context);
	static bool unserialize(RegistryValues& p_registry, tt::code::BufferReadContext* p_context);
	
	
	RegistryValues m_registry;
	RegistryValues m_persistentRegistry;
};


// Namespace end
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_REGISTRY_H)
