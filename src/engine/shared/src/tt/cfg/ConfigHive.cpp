#include <algorithm>
#include <cstdlib>

#include <tt/cfg/ConfigHive.h>
#include <tt/cfg/ConfigRegistry.h>
#include <tt/code/helpers.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/streams/BIFStream.h>


namespace tt {
namespace cfg {

// IMPORTANT! INCREMENT THIS NUMBER WHEN CHANGING THE DATA FORMAT!
// (version must match the version of the converter used to create the binary file)
static const u16 DATAFORMAT_VERSION = 1;


/*! \brief Internal option type IDs. This list must match that of the converter exactly. */
enum OptionType
{
	OptionType_String,
	OptionType_Real,
	OptionType_Integer,
	OptionType_Bool,
	
	OptionType_Count
};



//--------------------------------------------------------------------------------------------------
// Local helper functions

const char* getOptionTypeName(u16 p_type);
bool loadString(streams::BIStream& p_stream, char*& p_result, std::size_t& p_resultLength);
inline bool equal(const char* p_a, const char* p_b, std::size_t p_lenA, std::size_t p_lenB);


/*! \brief For debugging only! Returns a human-readable name for an option type ID. */
const char* getOptionTypeName(u16 p_type)
{
	switch (p_type)
	{
	case OptionType_String:  return "string";
	case OptionType_Real:    return "real";
	case OptionType_Integer: return "integer";
	case OptionType_Bool:    return "bool";
	default: return "!!INVALID!!";
	}
}


/*! \brief Loads an ANSI string (null-terminated) in length-prefixed format from a binary stream.
    \param p_stream Stream to load from.
    \param p_result Receives a pointer to the string that was loaded. Ownership transfers to caller.
    \param p_resultLength Receives the length of the string that was read (in characters, without null terminator).
    \return True if loading string succeeded, false if it did not. */
bool loadString(streams::BIStream& p_stream, char*& p_result, std::size_t& p_resultLength)
{
	if (p_stream.hasFailed()) return false;
	
	// Get the string length
	u16 charLen = 0;
	p_stream >> charLen;
	if (p_stream.hasFailed()) return false;
	
	// Prepare storage for the string
	char* rawString = new char[charLen + 1];
	std::memset(rawString, 0, static_cast<std::size_t>(charLen + 1));
	
	// Read the string in one go
	p_stream.read(reinterpret_cast<u8*>(rawString), charLen);
	if (p_stream.hasFailed())
	{
		delete[] rawString;
		return false;
	}
	
	// Pass the string on to calling code
	p_result       = rawString;
	p_resultLength = static_cast<std::size_t>(charLen);
	
	return true;
}


/*! \brief Indicates whether two raw strings are equal. */
inline bool equal(const char* p_a, const char* p_b, std::size_t p_lenA, std::size_t p_lenB)
{
	if (p_lenA != p_lenB) return false;
	return str::equal(p_a, p_b, p_lenA);
}



//--------------------------------------------------------------------------------------------------
// Internal helper structs

ConfigHive::Option::Option()
:
name(0),
nameLength(0),
type(0),
arrayIndex(0)
{
}


ConfigHive::Option::Option(const ConfigHive::Option& p_rhs)
:
name(0),
nameLength(p_rhs.nameLength),
type(p_rhs.type),
arrayIndex(p_rhs.arrayIndex)
{
	if (p_rhs.nameLength > 0 && p_rhs.name != 0)
	{
		name = new char[p_rhs.nameLength + 1];
		mem::copy8(name, p_rhs.name, static_cast<mem::size_type>(p_rhs.nameLength + 1));
	}
}


ConfigHive::Option::~Option()
{
	delete[] name;
}


ConfigHive::Option& ConfigHive::Option::operator=(const ConfigHive::Option& p_rhs)
{
	// Prevent self-assignment (avoid needless recreation of resources)
	if (&p_rhs == this)
	{
		return *this;
	}
	
	type       = p_rhs.type;
	arrayIndex = p_rhs.arrayIndex;
	
	if (nameLength != p_rhs.nameLength)
	{
		nameLength = p_rhs.nameLength;
		code::helpers::safeDeleteArray(name);
		if (p_rhs.nameLength > 0 && p_rhs.name != 0)
		{
			name = new char[p_rhs.nameLength + 1];
		}
	}
	
	if (p_rhs.nameLength > 0 && p_rhs.name != 0)
	{
		mem::copy8(name, p_rhs.name, static_cast<mem::size_type>(p_rhs.nameLength + 1));
	}
	
	return *this;
}


ConfigHive::Namespace::Namespace()
:
name(0),
nameLength(0),
childCount(0),
children(0),
optionCount(0),
options(0)
{
}


ConfigHive::Namespace::Namespace(const ConfigHive::Namespace& p_rhs)
:
name(0),
nameLength(p_rhs.nameLength),
childCount(p_rhs.childCount),
children(0),
optionCount(p_rhs.optionCount),
options(0)
{
	if (p_rhs.nameLength > 0 && p_rhs.name != 0)
	{
		name = new char[p_rhs.nameLength + 1];
		mem::copy8(name, p_rhs.name, static_cast<mem::size_type>(p_rhs.nameLength + 1));
	}
	
	if (p_rhs.childCount > 0 && p_rhs.children != 0)
	{
		children = new Namespace[p_rhs.childCount];
		for (u16 i = 0; i < p_rhs.childCount; ++i)
		{
			children[i] = p_rhs.children[i];
		}
	}
	
	if (p_rhs.optionCount > 0 && p_rhs.options != 0)
	{
		options = new Option[p_rhs.optionCount];
		for (u16 i = 0; i < p_rhs.optionCount; ++i)
		{
			options[i] = p_rhs.options[i];
		}
	}
}


ConfigHive::Namespace::~Namespace()
{
	delete[] name;
	delete[] children;
	delete[] options;
}


ConfigHive::Namespace& ConfigHive::Namespace::operator=(const ConfigHive::Namespace& p_rhs)
{
	// Prevent self-assignment (avoid needless recreation of resources)
	if (&p_rhs == this)
	{
		return *this;
	}
	
	if (nameLength != p_rhs.nameLength)
	{
		nameLength = p_rhs.nameLength;
		code::helpers::safeDeleteArray(name);
		if (p_rhs.nameLength > 0 && p_rhs.name != 0)
		{
			name = new char[p_rhs.nameLength + 1];
		}
	}
	
	if (p_rhs.nameLength > 0 && p_rhs.name != 0)
	{
		mem::copy8(name, p_rhs.name, static_cast<mem::size_type>(p_rhs.nameLength + 1));
	}
	
	
	if (childCount != p_rhs.childCount)
	{
		childCount = p_rhs.childCount;
		code::helpers::safeDeleteArray(children);
		if (p_rhs.childCount > 0 && p_rhs.children != 0)
		{
			children = new Namespace[p_rhs.childCount];
		}
	}
	
	if (p_rhs.childCount > 0 && p_rhs.children != 0)
	{
		for (u16 i = 0; i < p_rhs.childCount; ++i)
		{
			children[i] = p_rhs.children[i];
		}
	}
	
	
	if (optionCount != p_rhs.optionCount)
	{
		optionCount = p_rhs.optionCount;
		code::helpers::safeDeleteArray(options);
		if (p_rhs.optionCount > 0 && p_rhs.options != 0)
		{
			options = new Option[p_rhs.optionCount];
		}
	}
	
	if (p_rhs.optionCount > 0 && p_rhs.options != 0)
	{
		for (u16 i = 0; i < p_rhs.optionCount; ++i)
		{
			options[i] = p_rhs.options[i];
		}
	}
	
	return *this;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

ConfigHivePtr ConfigHive::load(const std::string& p_filename)
{
	// Attempt to open the file
	streams::BIFStream input(p_filename);
	if (input.hasFailed())
	{
		TT_PANIC("Opening file '%s' failed.", p_filename.c_str());
		return ConfigHivePtr();
	}
	
	// Binary config hives are in little endian
	input.useLittleEndian();
	
	// Read and verify the file signature
	{
		u8 signature[5];
		input.read(signature, 5);
		if (input.hasFailed() ||
		    signature[0] != 'B' ||
		    signature[1] != 'C' ||
		    signature[2] != 'F' ||
		    signature[3] != 'G' ||
		    signature[4] != 0)
		{
			TT_PANIC("File '%s': Not a valid binary config hive (signature does not match).",
			         p_filename.c_str());
			return ConfigHivePtr();
		}
	}
	
	// Read and verify the file version
	{
		u16 fileVersion = 0;
		input >> fileVersion;
		if (input.hasFailed() || fileVersion != DATAFORMAT_VERSION)
		{
			TT_PANIC("File '%s': Binary config hive was made with an incompatible version of "
			         "the converter. File version is %u, this library requires version %u.",
			         p_filename.c_str(), fileVersion, DATAFORMAT_VERSION);
			return ConfigHivePtr();
		}
	}
	
	// Get the number of types specified in this file
	u16 typeCount = 0;
	input >> typeCount;
	if (input.hasFailed() || typeCount > OptionType_Count)
	{
		TT_PANIC("File '%s': Invalid number of option types (%u). File corrupt?",
		         p_filename.c_str(), typeCount);
		return ConfigHivePtr();
	}
	
	ConfigHivePtr hive(new ConfigHive);
	
	// Load the option values for each type in the file
	for (u16 typeIndex = 0; typeIndex < typeCount; ++typeIndex)
	{
		// Get the option type
		u16 typeID = 0;
		input >> typeID;
		if (input.hasFailed() || typeID >= OptionType_Count)
		{
			TT_PANIC("File '%s': Invalid option type ID (%u). File corrupt?",
			         p_filename.c_str(), typeID);
			return ConfigHivePtr();
		}
		
		// Get the number of values for this type
		u16 valueCount = 0;
		input >> valueCount;
		if (input.hasFailed())
		{
			TT_PANIC("File '%s': Reading value count for type '%s' failed. File corrupt?",
			         p_filename.c_str(), getOptionTypeName(typeID));
			return ConfigHivePtr();
		}
		
		// Load values based on the option type
		// FIXME: This code simply begs to be generalized...
		switch (typeID)
		{
		case OptionType_String:
			if (hive->m_valuesString != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return ConfigHivePtr();
			}
			
			hive->m_valueCountString = valueCount;
			hive->m_valuesString     = new char*[valueCount];
			
			for (u16 valueIndex = 0; valueIndex < valueCount; ++valueIndex)
			{
				u16 strLen = 0;
				input >> strLen;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading string length failed. File corrupt?",
					         p_filename.c_str());
					return ConfigHivePtr();
				}
				
				hive->m_valuesString[valueIndex] = new char[strLen + 1];
				std::memset(hive->m_valuesString[valueIndex], 0, static_cast<std::size_t>(strLen + 1));
				input.read(reinterpret_cast<u8*>(hive->m_valuesString[valueIndex]), strLen);
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading %u string characters failed. File corrupt?",
					         p_filename.c_str(), strLen);
					return ConfigHivePtr();
				}
			}
			break;
			
		case OptionType_Real:
			if (hive->m_valuesReal != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return ConfigHivePtr();
			}
			
			hive->m_valueCountReal = valueCount;
			hive->m_valuesReal     = new real[valueCount];
			
			for (u16 valueIndex = 0; valueIndex < valueCount; ++valueIndex)
			{
				float rawValue;
				input >> rawValue;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading real %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return ConfigHivePtr();
				}
				
				hive->m_valuesReal[valueIndex] = static_cast<real>(rawValue);
			}
			break;
			
		case OptionType_Integer:
			if (hive->m_valuesInteger != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return ConfigHivePtr();
			}
			
			hive->m_valueCountInteger = valueCount;
			hive->m_valuesInteger     = new s32[valueCount];
			
			for (u16 valueIndex = 0; valueIndex < valueCount; ++valueIndex)
			{
				input >> hive->m_valuesInteger[valueIndex];
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading integer %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return ConfigHivePtr();
				}
			}
			break;
			
		case OptionType_Bool:
			if (hive->m_valuesBool != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return ConfigHivePtr();
			}
			
			hive->m_valueCountBool = valueCount;
			hive->m_valuesBool     = new bool[valueCount];
			
			for (u16 valueIndex = 0; valueIndex < valueCount; ++valueIndex)
			{
				u8 rawValue;
				input >> rawValue;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading bool %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return ConfigHivePtr();
				}
				
				hive->m_valuesBool[valueIndex] = (rawValue != 0);
			}
			break;
			
		default:
			TT_PANIC("File '%s': Invalid option type: %u. File corrupt?",
			         p_filename.c_str(), typeID);
			return ConfigHivePtr();
		}
	}
	
	// Load the name -> index mapping
	if (loadNamespace(input, p_filename, hive->m_rootNamespace) == false)
	{
		TT_PANIC("File '%s': Loading name hierarchy failed. File corrupt?", p_filename.c_str());
		return ConfigHivePtr();
	}
	
#if !defined(TT_BUILD_FINAL)
	// Save the filename, so that we can reload the hive later if requested
	TT_ASSERT(hive->m_filenames.empty());
	hive->m_filenames.push_back(p_filename);
#endif
	
	return hive;
}


ConfigHive::~ConfigHive()
{
	ConfigRegistry::unregisterHive(this);
	
	for (u16 i = 0; i < m_valueCountString; ++i)
	{
		delete[] m_valuesString[i];
	}
	delete[] m_valuesString;
	delete[] m_valuesReal;
	delete[] m_valuesInteger;
	delete[] m_valuesBool;
}


#if !defined(TT_BUILD_FINAL)
void ConfigHive::reload()
{
	TT_ASSERT(m_filenames.empty() == false);
	if (m_filenames.empty()) return;
	
	ConfigHivePtr newHive(load(m_filenames[0]));
	if (newHive == 0)
	{
		// Reloading failed, but keep existing data
		return;
	}
	
	// And append all other hives
	for (size_t i = 1; i < m_filenames.size(); ++i)
	{
		newHive->appendHive(m_filenames[i]);
	}
	
	// Copy the newly loaded data
	*this = *newHive;
}
#endif  // !defined(TT_BUILD_FINAL)


bool ConfigHive::appendHive(const std::string& p_filename)
{
	// FIXME: MR REMOVE DUPLICATE CODE WITH LOADING
	// Didn't do that because appending should be completely isolated for now
	
	// Attempt to open the file
	streams::BIFStream input(p_filename);
	if (input.hasFailed())
	{
		TT_PANIC("Opening file '%s' failed.", p_filename.c_str());
		return false;
	}
	
	// Binary config hives are in little endian
	input.useLittleEndian();
	
	// Read and verify the file signature
	{
		u8 signature[5];
		input.read(signature, 5);
		if (input.hasFailed() ||
		    signature[0] != 'B' ||
		    signature[1] != 'C' ||
		    signature[2] != 'F' ||
		    signature[3] != 'G' ||
		    signature[4] != 0)
		{
			TT_PANIC("File '%s': Not a valid binary config hive (signature does not match).",
			         p_filename.c_str());
			return false;
		}
	}
	
	// Read and verify the file version
	{
		u16 fileVersion = 0;
		input >> fileVersion;
		if (input.hasFailed() || fileVersion != DATAFORMAT_VERSION)
		{
			TT_PANIC("File '%s': Binary config hive was made with an incompatible version of "
			         "the converter. File version is %u, this library requires version %u.",
			         p_filename.c_str(), fileVersion, DATAFORMAT_VERSION);
			return false;
		}
	}
	
	// Get the number of types specified in this file
	u16 typeCount = 0;
	input >> typeCount;
	if (input.hasFailed() || typeCount > OptionType_Count)
	{
		TT_PANIC("File '%s': Invalid number of option types (%u). File corrupt?",
		         p_filename.c_str(), typeCount);
		return false;
	}
	
	// Load the option values for each type in the file
	u16 newCountString = m_valueCountString;
	u16 newCountReal = m_valueCountReal;
	u16 newCountInteger = m_valueCountInteger;
	u16 newCountBool = m_valueCountBool;
	
	for (u16 typeIndex = 0; typeIndex < typeCount; ++typeIndex)
	{
		// Get the option type
		u16 typeID = 0;
		input >> typeID;
		if (input.hasFailed() || typeID >= OptionType_Count)
		{
			TT_PANIC("File '%s': Invalid option type ID (%u). File corrupt?",
			         p_filename.c_str(), typeID);
			return false;
		}
		
		// Get the number of values for this type
		u16 valueCount = 0;
		input >> valueCount;
		if (input.hasFailed())
		{
			TT_PANIC("File '%s': Reading value count for type '%s' failed. File corrupt?",
			         p_filename.c_str(), getOptionTypeName(typeID));
			return false;
		}
		
		// Load values based on the option type
		// FIXME: This code simply begs to be generalized...
		switch (typeID)
		{
		case OptionType_String:
		{
			if (m_valueCountString == 0 && m_valuesString != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once in same file. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return false;
			}
			
			const u16 oldSize = m_valueCountString;
			const u16 newSize = oldSize + valueCount;
			char** values = new char*[newSize];
			if (oldSize > 0)
			{
				TT_ASSERT(m_valuesString != 0);
				std::memcpy(values, m_valuesString, oldSize * sizeof(char*));
				delete [] m_valuesString;
			}
			
			newCountString = newSize;
			m_valuesString = values;
			
			for (u16 valueIndex = oldSize; valueIndex < newSize; ++valueIndex)
			{
				u16 strLen = 0;
				input >> strLen;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading string length failed. File corrupt?",
					         p_filename.c_str());
					return false;
				}
				
				m_valuesString[valueIndex] = new char[strLen + 1];
				std::memset(m_valuesString[valueIndex], 0, static_cast<std::size_t>(strLen + 1));
				input.read(reinterpret_cast<u8*>(m_valuesString[valueIndex]), strLen);
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading %u string characters failed. File corrupt?",
					         p_filename.c_str(), strLen);
					return false;
				}
			}
			break;
		}
		
		case OptionType_Real:
		{
			if (m_valueCountReal == 0 && m_valuesReal != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once in same file. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return false;
			}
			
			const u16 oldSize = m_valueCountReal;
			const u16 newSize = oldSize + valueCount;
			real* values = new real[newSize];
			if (oldSize > 0)
			{
				TT_ASSERT(m_valuesReal != 0);
				std::memcpy(values, m_valuesReal, oldSize * sizeof(real));
				delete [] m_valuesReal;
			}
			
			newCountReal = newSize;
			m_valuesReal = values;
			
			for (u16 valueIndex = oldSize; valueIndex < newSize; ++valueIndex)
			{
				float rawValue;
				input >> rawValue;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading real %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return false;
				}
				
				m_valuesReal[valueIndex] = static_cast<real>(rawValue);
			}
			break;
		}
		
		case OptionType_Integer:
		{
			if (m_valueCountInteger == 0 && m_valuesInteger != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once in same file. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return false;
			}
			
			const u16 oldSize = m_valueCountInteger;
			const u16 newSize = oldSize + valueCount;
			s32* values = new s32[newSize];
			if (oldSize > 0)
			{
				TT_ASSERT(m_valuesInteger != 0);
				std::memcpy(values, m_valuesInteger, oldSize * sizeof(s32));
				delete [] m_valuesInteger;
			}
			
			newCountInteger = newSize;
			m_valuesInteger     = values;
			
			for (u16 valueIndex = oldSize; valueIndex < newSize; ++valueIndex)
			{
				input >> m_valuesInteger[valueIndex];
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading integer %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return false;
				}
			}
			break;
		}
		
		case OptionType_Bool:
		{
			if (m_valueCountBool == 0 && m_valuesBool != 0)
			{
				TT_PANIC("File '%s': Option type '%s' specified more than once in same file. File corrupt?",
				         p_filename.c_str(), getOptionTypeName(typeID));
				return false;
			}
			
			const u16 oldSize = m_valueCountBool;
			const u16 newSize = oldSize + valueCount;
			bool* values = new bool[newSize];
			if (oldSize > 0)
			{
				TT_ASSERT(m_valuesBool != 0);
				std::memcpy(values, m_valuesBool, oldSize * sizeof(bool));
				delete [] m_valuesBool;
			}
			
			newCountBool = newSize;
			m_valuesBool = values;
			
			for (u16 valueIndex = oldSize; valueIndex < newSize; ++valueIndex)
			{
				u8 rawValue;
				input >> rawValue;
				if (input.hasFailed())
				{
					TT_PANIC("File '%s': Reading bool %u failed. File corrupt?",
					         p_filename.c_str(), valueIndex);
					return false;
				}
				
				m_valuesBool[valueIndex] = (rawValue != 0);
			}
			break;
		}
		
		default:
			TT_PANIC("File '%s': Invalid option type: %u. File corrupt?",
			         p_filename.c_str(), typeID);
			return false;
		}
	}
	
	// Load the name -> index mapping
	ConfigHive::Namespace ns;
	if (loadNamespace(input, p_filename, ns) == false)
	{
		TT_PANIC("File '%s': Loading name hierarchy failed. File corrupt?", p_filename.c_str());
		return false;
	}
	
	if (appendNamespace(ns) == false)
	{
		TT_PANIC("File '%s': Appending name hierarchy failed. File corrupt?", p_filename.c_str());
		return false;
	}
	
	// Finally set the correct values
	m_valueCountString = newCountString;
	m_valueCountReal = newCountReal;
	m_valueCountInteger = newCountInteger;
	m_valueCountBool = newCountBool;
#if !defined(TT_BUILD_FINAL)
	m_filenames.push_back(p_filename);
#endif
	
	return true;
}


bool ConfigHive::hasOption(const std::string& p_option) const
{
	if (p_option.empty())
	{
		return false;
	}
	
	const char* rawOption = p_option.c_str();
	std::string::size_type nameStart = 0;
	std::string::size_type nameEnd   = p_option.find('.');
	
	if (nameEnd == std::string::npos)
	{
		// At least two parts are needed: namespace name and option name
		// (options cannot exist outside of a namespace)
		TT_WARN("Malformed option string: '%s'", p_option.c_str());
		return false;
	}
	
	if (equal(rawOption, m_rootNamespace.name,
	          nameEnd - nameStart, m_rootNamespace.nameLength) == false)
	{
		return false;
	}
	
	// Descend the namespace tree to find the 'leaf' namespace referenced by the option string
	const Namespace* ns = &m_rootNamespace;
	std::string::size_type optionNameStart  = p_option.rfind('.') + 1;
	std::string::size_type optionNameLength = p_option.length() - optionNameStart;
	while (nameEnd < optionNameStart)
	{
		// Find the next namespace
		nameStart = nameEnd + 1;
		nameEnd   = p_option.find('.', nameStart);
		if (nameEnd == std::string::npos || nameEnd >= optionNameStart)
		{
			// Reached the option part of the name
			break;
		}
		
		bool found = false;
		for (u16 idx = 0; idx < ns->childCount; ++idx)
		{
			if (equal(rawOption + nameStart, ns->children[idx].name,
			          nameEnd - nameStart, ns->children[idx].nameLength))
			{
				found = true;
				ns = &(ns->children[idx]);
				break;
			}
		}
		
		if (found == false)
		{
			// Sub namespace not found
			/*
			TT_Printf("ConfigHive::getArrayIndex: Namespace '%s' does not have child '%s'.\n",
			          ns->name, p_option.substr(nameStart, nameEnd - nameStart).c_str());
			// */
			return false;
		}
	}
	
	// Find the option that was requested
	for (u16 idx = 0; idx < ns->optionCount; ++idx)
	{
		if (equal(rawOption + optionNameStart, ns->options[idx].name,
		          optionNameLength, ns->options[idx].nameLength))
		{
			return true;
		}
	}
	
	// Option wasn't found
	return false;
}


HandleString ConfigHive::getHandleString(const std::string& p_option) const
{
	u16 arrayIndex;
	if (getArrayIndex(p_option, OptionType_String, &arrayIndex) == false)
	{
		return HandleString(p_option);
	}
	
	return createHandle<HandleString::value_type>(p_option, arrayIndex);
}


HandleReal ConfigHive::getHandleReal(const std::string& p_option) const
{
	u16 arrayIndex;
	if (getArrayIndex(p_option, OptionType_Real, &arrayIndex) == false)
	{
		return HandleReal(p_option);
	}
	
	return createHandle<HandleReal::value_type>(p_option, arrayIndex);
}


HandleInteger ConfigHive::getHandleInteger(const std::string& p_option) const
{
	u16 arrayIndex;
	if (getArrayIndex(p_option, OptionType_Integer, &arrayIndex) == false)
	{
		return HandleInteger(p_option);
	}
	
	return createHandle<HandleInteger::value_type>(p_option, arrayIndex);
}


HandleBool ConfigHive::getHandleBool(const std::string& p_option) const
{
	u16 arrayIndex;
	if (getArrayIndex(p_option, OptionType_Bool, &arrayIndex) == false)
	{
		return HandleBool(p_option);
	}
	
	return createHandle<HandleBool::value_type>(p_option, arrayIndex);
}


HandleString::value_type ConfigHive::get(const HandleString& p_handle) const
{
	if (validateHandle(p_handle) == false ||
	    p_handle.index >= static_cast<s32>(m_valueCountString))
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Config option '%s' does not exist, is not a String or the option handle "
		         "belongs to a different ConfigHive.", p_handle.optionName.c_str());
#else
		TT_PANIC("Invalid String handle passed. Option may not exist.");
#endif
		// Special case: return an empty string if option doesn't exist (prevent crashes on null pointers)
		return "";
		//return HandleString::value_type();
	}
	return m_valuesString[p_handle.index];
}


HandleReal::value_type ConfigHive::get(const HandleReal& p_handle) const
{
	if (validateHandle(p_handle) == false ||
	    p_handle.index >= static_cast<s32>(m_valueCountReal))
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Config option '%s' does not exist, is not a Real or the option handle "
		         "belongs to a different ConfigHive.", p_handle.optionName.c_str());
#else
		TT_PANIC("Invalid Real handle passed. Option may not exist.");
#endif
		return HandleReal::value_type();
	}
	return m_valuesReal[p_handle.index];
}


HandleInteger::value_type ConfigHive::get(const HandleInteger& p_handle) const
{
	if (validateHandle(p_handle) == false ||
	    p_handle.index >= static_cast<s32>(m_valueCountInteger))
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Config option '%s' does not exist, is not an Integer or the option handle "
		         "belongs to a different ConfigHive.", p_handle.optionName.c_str());
#else
		TT_PANIC("Invalid Integer handle passed. Option may not exist.");
#endif
		return HandleInteger::value_type();
	}
	return m_valuesInteger[p_handle.index];
}


HandleBool::value_type ConfigHive::get(const HandleBool& p_handle) const
{
	if (validateHandle(p_handle) == false ||
	    p_handle.index >= static_cast<s32>(m_valueCountBool))
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Config option '%s' does not exist, is not a Bool or the option handle "
		         "belongs to a different ConfigHive.", p_handle.optionName.c_str());
#else
		TT_PANIC("Invalid Bool handle passed. Option may not exist.");
#endif
		return HandleBool::value_type();
	}
	return m_valuesBool[p_handle.index];
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ConfigHive::ConfigHive()
:
#if !defined(TT_BUILD_FINAL)
m_filenames(),
#endif
m_valueCountString(0),
m_valuesString(0),
m_valueCountReal(0),
m_valuesReal(0),
m_valueCountInteger(0),
m_valuesInteger(0),
m_valueCountBool(0),
m_valuesBool(0),
m_rootNamespace()
{
	ConfigRegistry::registerHive(this);
}


#if !defined(TT_BUILD_FINAL)
ConfigHive& ConfigHive::operator=(const ConfigHive& p_rhs)
{
	// Prevent self-assignment (avoid needless recreation of resources)
	if (&p_rhs == this)
	{
		return *this;
	}
	
	// NOTE: All the copy operations are implemented in such a way to keep memory
	//       reallocations to a minimum (prevent unnecessary memory fragmentation)
	
	m_filenames = p_rhs.m_filenames;
	
	// Copy the string values
	if (m_valueCountString != p_rhs.m_valueCountString)
	{
		for (u16 i = 0; i < m_valueCountString; ++i)
		{
			delete[] m_valuesString[i];
		}
		
		code::helpers::safeDeleteArray(m_valuesString);
		m_valueCountString = p_rhs.m_valueCountString;
		m_valuesString     = new char*[m_valueCountString];
		
		if (m_valueCountString > 0)
		{
			mem::zero8(m_valuesString, static_cast<mem::size_type>(m_valueCountString * sizeof(char*)));
		}
	}
	
	for (u16 i = 0; i < m_valueCountString && p_rhs.m_valuesString != 0; ++i)
	{
		std::size_t rhsLen = std::strlen(p_rhs.m_valuesString[i]);
		if (m_valuesString[i] == 0 || std::strlen(m_valuesString[i]) != rhsLen)
		{
			code::helpers::safeDeleteArray(m_valuesString[i]);
			m_valuesString[i] = new char[rhsLen + 1];
		}
		
		mem::copy8(m_valuesString[i], p_rhs.m_valuesString[i], static_cast<mem::size_type>(rhsLen + 1));
	}
	
	
	// Copy the real values
	if (m_valueCountReal != p_rhs.m_valueCountReal)
	{
		code::helpers::safeDeleteArray(m_valuesReal);
		m_valueCountReal = p_rhs.m_valueCountReal;
		m_valuesReal     = new real[m_valueCountReal];
	}
	
	if (p_rhs.m_valuesReal != 0)
	{
		mem::copy8(m_valuesReal, p_rhs.m_valuesReal,
		           static_cast<mem::size_type>(m_valueCountReal * sizeof(real)));
	}
	
	// Copy the integer values
	if (m_valueCountInteger != p_rhs.m_valueCountInteger)
	{
		code::helpers::safeDeleteArray(m_valuesInteger);
		m_valueCountInteger = p_rhs.m_valueCountInteger;
		m_valuesInteger     = new s32[m_valueCountInteger];
	}
	
	if (p_rhs.m_valuesInteger != 0)
	{
		mem::copy32(m_valuesInteger, p_rhs.m_valuesInteger,
		            static_cast<mem::size_type>(m_valueCountInteger * sizeof(s32)));
	}
	
	// Copy the Boolean values
	if (m_valueCountBool != p_rhs.m_valueCountBool)
	{
		code::helpers::safeDeleteArray(m_valuesBool);
		m_valueCountBool = p_rhs.m_valueCountBool;
		m_valuesBool     = new bool[m_valueCountBool];
	}
	
	if (p_rhs.m_valuesBool != 0)
	{
		mem::copy8(m_valuesBool, p_rhs.m_valuesBool,
		           static_cast<mem::size_type>(m_valueCountBool * sizeof(bool)));
	}
	
	m_rootNamespace = p_rhs.m_rootNamespace;
	
	return *this;
}
#endif


bool ConfigHive::getArrayIndex(const std::string& p_option, u16 p_type, u16* p_arrayIndex) const
{
	if (p_arrayIndex == 0 || p_option.empty())
	{
		return false;
	}
	
	const char* rawOption = p_option.c_str();
	std::string::size_type nameStart = 0;
	std::string::size_type nameEnd   = p_option.find('.');
	
	if (nameEnd == std::string::npos)
	{
		// At least two parts are needed: namespace name and option name
		// (options cannot exist outside of a namespace)
		TT_WARN("Malformed option string: '%s'", p_option.c_str());
		return false;
	}
	
	if (equal(rawOption, m_rootNamespace.name,
	          nameEnd - nameStart, m_rootNamespace.nameLength) == false)
	{
		// First namespace name does not match root namespace: option not found
		/*
		TT_Printf("ConfigHive::getArrayIndex: Root namespace name mismatch: '%s' != '%s'\n",
		          p_option.substr(nameStart, nameEnd - nameStart).c_str(), m_rootNamespace.name);
		// */
		return false;
	}
	
	// Descend the namespace tree to find the 'leaf' namespace referenced by the option string
	const Namespace* ns = &m_rootNamespace;
	std::string::size_type optionNameStart  = p_option.rfind('.') + 1;
	std::string::size_type optionNameLength = p_option.length() - optionNameStart;
	while (nameEnd < optionNameStart)
	{
		// Find the next namespace
		nameStart = nameEnd + 1;
		nameEnd   = p_option.find('.', nameStart);
		if (nameEnd == std::string::npos || nameEnd >= optionNameStart)
		{
			// Reached the option part of the name
			break;
		}
		
		bool found = false;
		for (u16 idx = 0; idx < ns->childCount; ++idx)
		{
			//if (equal((*it).c_str(), ns->children[idx].name,
			//          (*it).length(), ns->children[idx].nameLength))
			if (equal(rawOption + nameStart, ns->children[idx].name,
			          nameEnd - nameStart, ns->children[idx].nameLength))
			{
				found = true;
				ns = &(ns->children[idx]);
				break;
			}
		}
		
		if (found == false)
		{
			// Sub namespace not found
			/*
			TT_Printf("ConfigHive::getArrayIndex: Namespace '%s' does not have child '%s'.\n",
			          ns->name, p_option.substr(nameStart, nameEnd - nameStart).c_str());
			// */
			return false;
		}
	}
	
	// Find the option that was requested
	for (u16 idx = 0; idx < ns->optionCount; ++idx)
	{
		if (ns->options[idx].type == p_type &&
		    equal(rawOption + optionNameStart, ns->options[idx].name,
		          optionNameLength, ns->options[idx].nameLength))
		{
			*p_arrayIndex = ns->options[idx].arrayIndex;
			return true;
		}
	}
	
	// Option wasn't found
	/*
	TT_Printf("ConfigHive::getArrayIndex: Namespace '%s' does not have option '%s' of type '%s'.\n",
	          ns->name, rawOption + optionNameStart, getOptionTypeName(p_type));
	// */
	return false;
}


bool ConfigHive::loadNamespace(streams::BIStream&     p_stream,
                               const std::string&     p_filename,
                               ConfigHive::Namespace& p_ns)
{
	(void)p_filename;
	
	// Load the namespace name
	if (loadString(p_stream, p_ns.name, p_ns.nameLength) == false)
	{
		TT_PANIC("File '%s': Reading namespace name failed. File corrupt?",
		         p_filename.c_str());
		return false;
	}
	
	// Load the child count
	p_stream >> p_ns.childCount;
	if (p_stream.hasFailed())
	{
		TT_PANIC("File '%s': Reading namespace '%s' child count failed. File corrupt?",
		         p_filename.c_str(), p_ns.name);
		return false;
	}
	
	// Load the children
	if (p_ns.childCount > 0)
	{
		p_ns.children = new Namespace[p_ns.childCount];
		for (u16 i = 0; i < p_ns.childCount; ++i)
		{
			if (loadNamespace(p_stream, p_filename, p_ns.children[i]) == false)
			{
				return false;
			}
		}
	}
	
	// Load the option count
	p_stream >> p_ns.optionCount;
	if (p_stream.hasFailed())
	{
		TT_PANIC("File '%s': Reading namespace '%s' option count failed. File corrupt?",
		         p_filename.c_str(), p_ns.name);
		return false;
	}
	
	// Load the options
	if (p_ns.optionCount > 0)
	{
		p_ns.options = new Option[p_ns.optionCount];
		for (u16 i = 0; i < p_ns.optionCount; ++i)
		{
			if (loadString(p_stream, p_ns.options[i].name, p_ns.options[i].nameLength) == false)
			{
				TT_PANIC("File '%s': Reading name for option %u in namespace '%s' failed. "
				         "File corrupt?", p_filename.c_str(), i, p_ns.name);
				return false;
			}
			
			p_stream >> p_ns.options[i].type;
			if (p_stream.hasFailed())
			{
				TT_PANIC("File '%s': Reading type for option '%s' in namespace '%s' failed. "
				         "File corrupt?", p_filename.c_str(), p_ns.options[i].name, p_ns.name);
				return false;
			}
			
			p_stream >> p_ns.options[i].arrayIndex;
			if (p_stream.hasFailed())
			{
				TT_PANIC("File '%s': Reading array index for option '%s' in namespace '%s' failed. "
				         "File corrupt?", p_filename.c_str(), p_ns.options[i].name, p_ns.name);
				return false;
			}
		}
	}
	
	return true;
}


bool ConfigHive::appendNamespace(const Namespace& p_rhs)
{
	if (p_rhs.name == 0)
	{
		TT_PANIC("Rhs has no root name");
		return false;
	}
	
	if (m_rootNamespace.name == 0)
	{
		m_rootNamespace = p_rhs;
		return true;
	}
	
	if (strcmp(m_rootNamespace.name, p_rhs.name) != 0)
	{
		TT_PANIC("Rootname '%s' mismatches with appended hive rootname: '%s'",
			m_rootNamespace.name, p_rhs.name);
		return false;
	}
	
	if (p_rhs.childCount > 0)
	{
		const u16 newSize = m_rootNamespace.childCount + p_rhs.childCount;
		Namespace* newChildren = new Namespace[newSize];
		
		// Copy all children
		for (u16 i = 0; i < m_rootNamespace.childCount; ++i)
		{
			newChildren[i] = m_rootNamespace.children[i];
		}
		for (u16 i = 0; i < p_rhs.childCount; ++i)
		{
			int newIdx(m_rootNamespace.childCount + i);
			newChildren[newIdx] = p_rhs.children[i];
			offsetArrayIndicesForNode(newChildren[newIdx]);
		}
		code::helpers::safeDeleteArray(m_rootNamespace.children);
		m_rootNamespace.children = newChildren;
		m_rootNamespace.childCount = newSize;
	}
	
	if (p_rhs.optionCount > 0)
	{
		const u16 newSize = m_rootNamespace.optionCount + p_rhs.optionCount;
		Option* newOptions = new Option[newSize];
		
		// Copy all options
		for (u16 i = 0; i < m_rootNamespace.optionCount; ++i)
		{
			newOptions[i] = m_rootNamespace.options[i];
		}
		for (u16 i = 0; i < p_rhs.optionCount; ++i)
		{
			int newIdx(m_rootNamespace.optionCount + i);
			newOptions[newIdx] = p_rhs.options[i];
			offsetArrayIndicesForOption(newOptions[newIdx]);
		}
		code::helpers::safeDeleteArray(m_rootNamespace.options);
		m_rootNamespace.options = newOptions;
		m_rootNamespace.optionCount = newSize;
	}
	
	return true;
}


void ConfigHive::offsetArrayIndicesForNode(Namespace& p_node)
{
	for (int i = 0; i < p_node.optionCount; ++i)
	{
		offsetArrayIndicesForOption(p_node.options[i]);
	}
	
	for (int i = 0; i < p_node.childCount; ++i)
	{
		offsetArrayIndicesForNode(p_node.children[i]);
	}
}


void ConfigHive::offsetArrayIndicesForOption(Option& p_option)
{
	switch(p_option.type)
	{
	case OptionType_String:  p_option.arrayIndex += m_valueCountString; break;
	case OptionType_Real:    p_option.arrayIndex += m_valueCountReal; break;
	case OptionType_Integer: p_option.arrayIndex += m_valueCountInteger; break;
	case OptionType_Bool:    p_option.arrayIndex += m_valueCountBool; break;
	default:
		TT_PANIC("Unhandled type '%d'", p_option.type);
	}
}


// Namespace end
}
}
