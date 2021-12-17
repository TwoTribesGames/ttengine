#include <tt/code/bufferutils.h>
#include <tt/platform/tt_printf.h>

#include <toki/AppGlobal.h>
#include <toki/game/script/Registry.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/savedata/utils.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace script {


Registry& getRegistry()
{
	return getRegistry(AppGlobal::getCurrentProgressType());
}


Registry& getRegistry(ProgressType p_progressType)
{
	static Registry registry[ProgressType_Count];
	if (isValidProgressType(p_progressType) == false)
	{
		TT_PANIC("Incorrect ProgressType: %d!", p_progressType);
		return registry[ProgressType_Main];
	}
	return registry[p_progressType];
}


Registry* getRegistryPtr()
{
	return &getRegistry();
}


Registry* getRegistryExPtr(ProgressType p_progressType)
{
	return &getRegistry(p_progressType);
}

//--------------------------------------------------------------------------------------------------
// Public member functions

int Registry::get(HSQUIRRELVM p_vm)
{
	const SQInteger argc = sq_gettop(p_vm) - 1; // Stack has arguments + context.
	
	if (argc != 1)
	{
		TT_PANIC("Registry::get(string) has %d argument(s), expected 1", argc);
		return 0;
	}
	
	std::string key;
	if (getKeyFromStack(p_vm, -1, key) == false)
	{
		return 0;
	}
	
	RegistryValues::const_iterator it = m_registry.find(key);
	if (it != m_registry.end())
	{
		(*it).second.pushOnStack(p_vm);
	}
	else
	{
		sq_pushnull(p_vm);
	}
	return 1;
}


int Registry::getPersistent(HSQUIRRELVM p_vm)
{
	const SQInteger argc = sq_gettop(p_vm) - 1; // Stack has arguments + context.
	
	if (argc != 1)
	{
		TT_PANIC("Registry::getPersistent(string) has %d argument(s), expected 1", argc);
		return 0;
	}
	
	std::string key;
	if (getKeyFromStack(p_vm, -1, key) == false)
	{
		return 0;
	}
	
	RegistryValues::const_iterator it = m_persistentRegistry.find(key);
	if (it != m_persistentRegistry.end())
	{
		(*it).second.pushOnStack(p_vm);
	}
	else
	{
		sq_pushnull(p_vm);
	}
	return 1;
}


void Registry::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING(Registry); // Make sure script doesn't make copies.
	TT_SQBIND_SQUIRREL_METHOD(Registry, get);
	TT_SQBIND_SQUIRREL_METHOD(Registry, set);
	TT_SQBIND_METHOD(Registry, getKeyAt);
	TT_SQBIND_METHOD(Registry, getKeyPersistentAt);
	TT_SQBIND_METHOD(Registry, exists);
	TT_SQBIND_METHOD(Registry, erase);
	TT_SQBIND_METHOD(Registry, clear);
	TT_SQBIND_METHOD(Registry, size);
	TT_SQBIND_SQUIRREL_METHOD(Registry, getPersistent);
	TT_SQBIND_SQUIRREL_METHOD(Registry, setPersistent);
	TT_SQBIND_METHOD(Registry, existsPersistent);
	TT_SQBIND_METHOD(Registry, erasePersistent);
	TT_SQBIND_METHOD(Registry, clearPersistent);
	TT_SQBIND_METHOD(Registry, sizePersistent);
	
	// Bind global functions
	TT_SQBIND_FUNCTION_NAME(getRegistryPtr,   "getRegistry"  );
	TT_SQBIND_FUNCTION_NAME(getRegistryExPtr, "getRegistryEx");
}


void Registry::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_Registry);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Registry data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	serialize(m_registry, &context);
	
	context.flush();
}


void Registry::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_Registry);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Registry data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	unserialize(m_registry, &context);
}


void Registry::serializePersistent(tt::code::BufferWriteContext* p_context) const
{
	serialize(m_persistentRegistry, p_context);
}


bool Registry::unserializePersistent(tt::code::BufferReadContext* p_context)
{
	return unserialize(m_persistentRegistry, p_context);
}




//--------------------------------------------------------------------------------------------------
// Private member functions


bool Registry::save(const RegistryValues& p_registry, tt::fs::FilePtr& p_file)
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(256, 128));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	serialize(p_registry, &context);
	
	context.flush();
	
	// Save the data's total (used/written) size
	bool saveOk = tt::fs::writeInteger(p_file, static_cast<u32>(buffer->getUsedSize()));
	
	// Save all data blocks
	const s32 blockCount = buffer->getBlockCount();
	for (s32 i = 0; i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(buffer->getBlockSize(i));
		saveOk = saveOk && (p_file->write(buffer->getBlock(i), blockSize) == blockSize);
	}
	
	return saveOk;
}


bool Registry::load(RegistryValues& p_registry, const tt::fs::FilePtr& p_file)
{
	u32 dataSize = 0;
	if (tt::fs::readInteger(p_file, &dataSize) == false)
	{
		TT_PANIC("Loading section size from serialization data failed.");
		return false;
	}
	
	if (dataSize == 0)
	{
		// No further data to read: just return a default-created object
		// (no initial data, default starting size)
		return true;
	}
	
	u8* data = new u8[dataSize];
	
	const tt::fs::size_type readSize = static_cast<tt::fs::size_type>(dataSize);
	if (p_file->read(data, readSize) != readSize)
	{
		TT_PANIC("Loading section data (%u bytes) from serialization data failed.", dataSize);
		delete[] data;
		return false;
	}
	
	tt::code::AutoGrowBufferPtr buffer = tt::code::AutoGrowBuffer::createPrePopulated(
			data,
			static_cast<tt::code::Buffer::size_type>(dataSize),
			128);
	delete[] data;
	data = 0;
	
	if (buffer == 0)
	{
		TT_PANIC("Could not create new serializer section (of %u bytes) from serialization data.", dataSize);
		return false;
	}
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	unserialize(p_registry, &context);
	
	return true;
}


bool Registry::findValue(const tt::str::Strings&  p_keys,
                         const RegistryValues&    p_registry,
                         tt::script::ScriptValue* p_value)
{
	TT_NULL_ASSERT(p_value);
	
	if (p_keys.empty())
	{
		return false;
	}
	
	RegistryValues::const_iterator it = p_registry.find(p_keys.front());
	if (it != p_registry.end())
	{
		const tt::script::ScriptValue& rootValue = (*it).second;
		if (p_keys.size() == 1)
		{
			(*p_value) = rootValue;
			return true;
		}
		else
		{
			return findValue(p_keys, 1, rootValue, p_value);
		}
	}
	return false;
}


bool Registry::findValue(const tt::str::Strings&        p_keys,
                         tt::str::Strings::size_type    p_keysIndex,
                         const tt::script::ScriptValue& p_rootValue,
                         tt::script::ScriptValue*       p_value)
{
	TT_NULL_ASSERT(p_value);
	
	if (p_keys.size() <= p_keysIndex)
	{
		TT_PANIC("Invalid index %u for keys.", p_keysIndex);
		return false;
	}
	
	const tt::script::ScriptValue& foundValue = p_rootValue.findInTable(p_keys[p_keysIndex]);
	
	if (foundValue.isNull() == false)
	{
		const tt::str::Strings::size_type nextIndex = p_keysIndex + 1;
		if (p_keys.size() <= nextIndex)
		{
			(*p_value) = foundValue;
			return true;
		}
		return findValue(p_keys, nextIndex, foundValue, p_value);
	}
	return false;
}


bool Registry::setValue(const tt::str::Strings&        p_keys,
                        tt::str::Strings::size_type    p_keysIndex,
                        tt::script::ScriptValue&       p_rootValue,
                        const tt::script::ScriptValue& p_value)
{
	if (p_keys.size() <= p_keysIndex)
	{
		TT_PANIC("Invalid index %u for keys.", p_keysIndex);
		return false;
	}
	
	const tt::str::Strings::size_type nextIndex = p_keysIndex + 1;
	if (p_keys.size() <= nextIndex) // At final key
	{
		return p_rootValue.setInTable(p_keys[p_keysIndex], p_value);
	}
	tt::script::ScriptValue& foundValue = p_rootValue.findInTable(p_keys[p_keysIndex]);
	
	if (foundValue.isNull() == false)
	{
		return setValue(p_keys, nextIndex, foundValue, p_value);
	}
	
	// Key doesn't exist yet. Create empty table.
	tt::script::ScriptValue newTable = tt::script::ScriptValue::createTable();
	setValue(p_keys, nextIndex, newTable, p_value);
	p_rootValue.setInTable(p_keys[p_keysIndex], newTable);
	
	return false;
}


bool Registry::getKeyFromStack(HSQUIRRELVM p_vm, s32 p_idx, std::string& p_key_OUT)
{
	const SQChar* strPtr = 0;
	if (SQ_FAILED(sq_getstring(p_vm, p_idx, &strPtr)))
	{
		TT_PANIC("Registry::getKeyFromStack() argument should be of type string");
		return false;
	}
	
	p_key_OUT = std::string(strPtr);
	return true;
}


int Registry::set(HSQUIRRELVM p_vm, RegistryValues& p_registry)
{
	const SQInteger argc = sq_gettop(p_vm) - 1; // Stack has arguments + context.
	
	if (argc != 2)
	{
		TT_PANIC("Registry::set(string, value) has %d argument(s), expected 2", argc);
		return 0;
	}
	
	std::string key;
	if (getKeyFromStack(p_vm, -2, key) == false)
	{
		return 0;
	}
	
	tt::str::Strings keys = tt::str::explode(key, ".");
	
	if (keys.empty())
	{
		return 0;
	}
	
	const tt::script::ScriptValue newValue = tt::script::ScriptValue::create(p_vm, -1);
	
	const std::string& firstKey = keys.front();
	RegistryValues::iterator it = p_registry.find(firstKey);
	if (it != p_registry.end())
	{
		tt::script::ScriptValue& rootValue = (*it).second;
		if (keys.size() == 1)
		{
			rootValue = newValue;
			return 0;
		}
		setValue(keys, 1, rootValue, newValue);
	}
	else
	{
		if (keys.size() == 1)
		{
			p_registry[firstKey] = newValue;
		}
		else
		{
			tt::script::ScriptValue newTable =  tt::script::ScriptValue::createTable();
			setValue(keys, 1, newTable, newValue);
			p_registry[firstKey] = newTable;
		}
	}
	
	return 0;
}


void Registry::serialize(const RegistryValues& p_registry, tt::code::BufferWriteContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	const u32 count = static_cast<u32>(p_registry.size());
	bu::put(count, p_context);
	
	for (RegistryValues::const_iterator it = p_registry.begin();
	     it != p_registry.end(); ++it)
	{
		// Serialize key
		bu::put((*it).first, p_context);	// std::string
		
		// Serialize value
		(*it).second.serialize(p_context);
	}
}


bool Registry::unserialize(RegistryValues& p_registry, tt::code::BufferReadContext* p_context)
{
	// Clear the registry first
	p_registry.clear();
	
	namespace bu = tt::code::bufferutils;
	
	const u32 count = bu::get<u32>(p_context);
	for (u32 i = 0; i < count; ++i)
	{
		std::string key = bu::get<std::string>(p_context);
		using tt::script::ScriptValue;
		ScriptValue value = ScriptValue::unserialize(p_context);
		p_registry[key] = value;
	}
	
	if (p_context->statusCode != 0)
	{
		p_registry.clear();
		return false;
	}
	
	return true;
}

// Namespace end
}
}
}
