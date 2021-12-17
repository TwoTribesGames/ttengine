#if !defined(INC_TT_ENGINE_ENGINEID_H)
#define INC_TT_ENGINE_ENGINEID_H


#include <string>

#include <tt/engine/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/streams/BIStream.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace engine {

class EngineID
{
public:
	explicit EngineID(u64 p_value)
	:
	crc1(static_cast<u32>((p_value >> 32) & 0xFFFFFFFF)),
	crc2(static_cast<u32>(p_value & 0xFFFFFFFF))
	{ }
	EngineID(u32 p_crc1, u32 p_crc2);
	EngineID(const std::string& p_name, const std::string& p_namespace);
	
	bool load(const fs::FilePtr& p_file);
	
	inline bool valid() const { return (crc1 != 0 && crc2 != 0); }
	
	inline bool operator==(const EngineID& p_rhs) const
	{
		return (crc1 == p_rhs.crc1 && crc2 == p_rhs.crc2);
	}
	inline bool operator!=(const EngineID& p_rhs) const { return operator==(p_rhs) == false; }
	
	std::string toString() const;
	
#if !defined(TT_BUILD_FINAL)
	inline const std::string& getName() const { return m_name; }
	inline const std::string& getNamespace() const { return m_namespace; }
	inline void setName(const std::string& p_name) { m_name = p_name; }
	std::string toDebugString() const;
#else
	inline std::string toDebugString() const { return std::string(); }
	inline void setName(const std::string&) { }
#endif
	
	u64 getValue() const { return (u64(crc1) << 32) + crc2; }
	
	u32 crc1;
	u32 crc2;
	
#if !defined(TT_BUILD_FINAL)
private:
	std::string m_name;
	std::string m_namespace;
#endif
};


struct EngineIDLess
{
	// Types expected by standard library functions / containers
	typedef EngineID first_argument_type;
	typedef EngineID second_argument_type;
	typedef bool     result_type;
	
	inline bool operator()(const EngineID& p_a, const EngineID& p_b) const
	{
		if (p_a.crc1 != p_b.crc1)
		{
			return p_a.crc1 < p_b.crc1;
		}
		return p_a.crc2 < p_b.crc2;
	}
};

// Namespace end
}
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& p_s, tt::engine::EngineID& p_rhs)
{
	p_s >> p_rhs.crc1;
	p_s >> p_rhs.crc2;
	return p_s;
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& p_s, const tt::engine::EngineID& p_rhs)
{
	p_s << p_rhs.crc1;
	p_s << p_rhs.crc2;
	return p_s;
}


#endif  // !defined(INC_TT_ENGINE_ENGINEID_H)
