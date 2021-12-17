#if !defined(INC_TOKI_GAME_SHUTDOWNDATAMGR_H)
#define INC_TOKI_GAME_SHUTDOWNDATAMGR_H

#include <string>

#include <tt/code/fwd.h>
#include <tt/fs/types.h>

#include <toki/serialization/fwd.h>


namespace toki {
namespace game {

class ShutdownDataMgr
{
public:
	ShutdownDataMgr();
	
	void setData(const serialization::SerializationMgrPtr& p_data,
	             const std::string&                        p_id);
	serialization::SerializationMgrPtr getData() const;
	
	inline const std::string& getID() const { return m_id; }
	inline bool hasData() const { return m_data != 0; }
	
	void reset();
	
private:
	tt::code::BufferPtr m_data;   // Gamestate from previous run. (saved when application is quit.)
	std::string         m_id;     // progressID used for script callbacks when unserializing shutdown data.
	                              // Because we also use the shutdown data to unserialize between 
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_SHUTDOWNDATAMGR_H)
