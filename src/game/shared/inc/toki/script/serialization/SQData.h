#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQDATA_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_SQDATA_H

#include <vector>

#include <tt/code/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <toki/script/serialization/ProcessedObject.h>
#include <toki/script/serialization/UserType.h>

namespace toki {
namespace script {
namespace serialization {

struct SQInstanceData
{
	typedef std::pair<ProcessedObject, ProcessedObject> Member;
	typedef std::vector<Member> Members;
	
	inline void add(const ProcessedObject& p_key, const ProcessedObject& p_value)
	{
		members.emplace_back(p_key, p_value);
	}
	
	inline void setClassID(s32 p_classID) { classID = p_classID; };
	inline void setUserPointer(SQUserPointer p_userPtr) { userPtr = p_userPtr; };
	inline void setUserType(UserType::Type p_userType) { userType = p_userType; };
	
	s32            classID;
	SQUserPointer  userPtr;
	UserType::Type userType;
	Members        members;
};

struct SQTableData
{
	typedef std::pair<ProcessedObject, ProcessedObject> Entry;
	typedef std::vector<Entry> Entries;
	
	virtual inline void add(const ProcessedObject& p_key, const ProcessedObject& p_value)
	{
		entries.emplace_back(p_key, p_value);
	}
	inline void setClassID(s32) { TT_PANIC("'setClassID' not implemented for SQTableData"); }
	inline void setUserPointer(SQUserPointer) { TT_PANIC("'setUserPointer' not implemented for SQTableData"); }
	inline void setUserType(UserType::Type) { TT_PANIC("'setUserType' not implemented for SQTableData"); };
	
	Entries entries;
};

struct SQArrayData
{
	typedef std::vector<ProcessedObject> Entries;
	
	inline void add(const ProcessedObject&, const ProcessedObject& p_value)
	{
		entries.emplace_back(p_value);
	}
	inline void setClassID(s32) { TT_PANIC("'setClassID' not implemented for SQArrayData"); }
	inline void setUserPointer(SQUserPointer) { TT_PANIC("'setUserPointer' not implemented for SQArrayData"); }
	inline void setUserType(UserType::Type) { TT_PANIC("'setUserType' not implemented for SQArrayData"); };
	
	Entries entries;
};


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQDATA_H)
