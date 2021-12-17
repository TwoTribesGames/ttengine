#if !defined(INC_TT_ENGINE_SCENE_USERPROPERTY_H)
#define INC_TT_ENGINE_SCENE_USERPROPERTY_H

#include <string>

#include <tt/platform/tt_types.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace scene {

class UserProperty
{
public:
	enum DataType
	{
		DataType_String = 0,
		DataType_Real,
		DataType_Integer
	};
	
	UserProperty();
	~UserProperty();
	
	inline DataType           getType()          const { return m_type; }
	inline const std::string& getName()          const { return m_name; }
	inline std::string        getDataAsString()  const { return std::string(m_string); }
	inline real               getDataAsReal()    const { return m_real; }
	inline s32                getDataAsInteger() const { return m_integer; }
	
	bool load(const fs::FilePtr& p_file);
	
private:
	std::string m_name;
	DataType    m_type;
	
	union
	{
		s32   m_integer;
		char* m_string;
	};
	real m_real;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_USERPROPERTY_H
