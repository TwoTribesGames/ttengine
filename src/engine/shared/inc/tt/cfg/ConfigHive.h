#if !defined(INC_TT_CFG_CONFIGHIVE_H)
#define INC_TT_CFG_CONFIGHIVE_H


#include <string>

#include <tt/cfg/Handle.h>
#include <tt/str/str.h>
#include <tt/streams/fwd.h>


namespace tt {
namespace cfg {

class ConfigHive;
typedef tt_ptr<ConfigHive>::shared ConfigHivePtr;


/*! \brief A configuration hive (stores configuration information). */
class ConfigHive
{
public:
	static ConfigHivePtr load(const std::string& p_filename);
	
	~ConfigHive();
	
#if !defined(TT_BUILD_FINAL)
	void reload();
#else
	inline void reload() { }
#endif
	
	bool          appendHive(const std::string& p_filename);
	
	bool          hasOption       (const std::string& p_option) const;
	
	HandleString  getHandleString (const std::string& p_option) const;
	HandleReal    getHandleReal   (const std::string& p_option) const;
	HandleInteger getHandleInteger(const std::string& p_option) const;
	HandleBool    getHandleBool   (const std::string& p_option) const;
	
	HandleString::value_type  get(const HandleString&  p_handle) const;
	HandleReal::value_type    get(const HandleReal&    p_handle) const;
	HandleInteger::value_type get(const HandleInteger& p_handle) const;
	HandleBool::value_type    get(const HandleBool&    p_handle) const;
	
	/*! \brief Functions for retrieving option values directly, without first requesting a handle. */
	inline HandleString::value_type getStringDirect(const std::string& p_option) const
	{ return get(getHandleString(p_option)); }
	
	inline HandleReal::value_type getRealDirect(const std::string& p_option) const
	{ return get(getHandleReal(p_option)); }
	
	inline HandleInteger::value_type getIntegerDirect(const std::string& p_option) const
	{ return get(getHandleInteger(p_option)); }
	
	inline HandleBool::value_type getBoolDirect(const std::string& p_option) const
	{ return get(getHandleBool(p_option)); }
	
private:
	struct Option
	{
		Option();
		Option(const Option& p_rhs);
		~Option();
		Option& operator=(const Option& p_rhs);
		
		char* name;
		std::size_t nameLength; //!< Length of the name string, in characters (without null terminator)
		u16   type;
		u16   arrayIndex;
	};
	
	struct Namespace
	{
		Namespace();
		Namespace(const Namespace& p_rhs);
		~Namespace();
		Namespace& operator=(const Namespace& p_rhs);
		
		char* name;
		std::size_t nameLength; //!< Length of the name string, in characters (without null terminator)
		
		u16        childCount;
		Namespace* children;
		
		u16     optionCount;
		Option* options;
	};
	
	
	ConfigHive();
	ConfigHive& operator=(const ConfigHive& p_rhs); // only implemented in non-final builds
	bool getArrayIndex(const std::string& p_option, u16 p_type, u16* p_arrayIndex) const;
	static bool loadNamespace(streams::BIStream& p_stream,
	                          const std::string& p_filename,
	                          Namespace& p_ns);
	
	
	// Append helpers
	bool appendNamespace(const Namespace& p_rhs);
	void offsetArrayIndicesForNode(Namespace& p_node);
	void offsetArrayIndicesForOption(Option& p_option);
	
	template<typename T>
	inline bool validateHandle(Handle<T> p_handle) const
	{
#if !defined(TT_BUILD_FINAL)
		return p_handle.source == this && p_handle.isValid();
#else
		return p_handle.isValid();
#endif
	}
	
	template<typename T>
	inline Handle<T> createHandle(const std::string& p_optionName, u16 p_arrayIndex) const
	{
		Handle<T> handle(p_optionName);
		handle.index  = static_cast<s32>(p_arrayIndex);
#if !defined(TT_BUILD_FINAL)
		handle.source = this;
#endif
		return handle;
	}
	
	// No copying
	ConfigHive(const ConfigHive&);
	
	
#if !defined(TT_BUILD_FINAL)
	// Filenames used to load the hive from (so that reload knows where to reload from)
	tt::str::Strings m_filenames;
#endif
	
	// Option values per type
	u16    m_valueCountString;
	char** m_valuesString;
	
	u16   m_valueCountReal;
	real* m_valuesReal;
	
	u16  m_valueCountInteger;
	s32* m_valuesInteger;
	
	u16   m_valueCountBool;
	bool* m_valuesBool;
	
	// Namespace tree
	Namespace m_rootNamespace;
};


// Not implemented general function templated (specialization following)
template <typename Type>
inline Handle<Type> getHandle(const ConfigHive& p_cfg, const std::string& p_option);

template <> inline HandleString  getHandle<const char*>(const ConfigHive& p_cfg, const std::string& p_option) { return p_cfg.getHandleString( p_option); }
template <> inline HandleReal    getHandle<real>       (const ConfigHive& p_cfg, const std::string& p_option) { return p_cfg.getHandleReal(   p_option); }
template <> inline HandleInteger getHandle<s32>        (const ConfigHive& p_cfg, const std::string& p_option) { return p_cfg.getHandleInteger(p_option); }
template <> inline HandleBool    getHandle<bool>       (const ConfigHive& p_cfg, const std::string& p_option) { return p_cfg.getHandleBool(   p_option); }

// Namespace end
}
}


#endif  // !defined(INC_TT_CFG_CONFIGHIVE_H)
