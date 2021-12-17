#if !defined(INC_TOKI_GAME_CHECKPOINTMGR_H)
#define INC_TOKI_GAME_CHECKPOINTMGR_H

#include <map>
#include <string>

#include <tt/code/fwd.h>
#include <tt/fs/types.h>
#include <tt/str/str.h>

#include <toki/constants.h>
#include <toki/game/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {

class CheckPointMgr
{
public:
	static CheckPointMgrPtr create(ProgressType p_progressType); //!< Use ProgressType to decide size limit.
	static CheckPointMgrPtr create(u32 p_sizeLimitCheckpoints);
	
	void setCheckPoint(const serialization::SerializationMgrPtr& p_checkPointData,
	                   const std::string&                        p_id);
	serialization::SerializationMgrPtr getCheckPoint(const std::string& p_id) const;
	
	tt::str::Strings getAllCheckPointIDs() const;
	bool hasCheckPoint(const std::string& p_id) const;
	bool renameCheckPoint(const std::string& p_oldID, const std::string& p_newID);
	void resetCheckPoint(const std::string& p_id);
	void resetAllCheckPoints();
	
	bool save(tt::fs::FilePtr& p_file) const;
	bool load(const tt::fs::FilePtr& p_file);
	
	inline bool canStoreProgress() const
	{
		return m_totalSize < m_sizeLimitCheckpoints;
	}
	
	inline u32 getNumberOfCheckpoints() const { return static_cast<u32>(m_checkPoints.size()); }
	
	CheckPointMgrPtr clone() const;
	
private:
	CheckPointMgr(u32 p_sizeLimitCheckpoints);
	
	typedef std::map<std::string, tt::code::BufferPtr> CheckPoints;
	
	// Copy constructor used for clone
	CheckPointMgr(const CheckPointMgr& p_rhs);
	
	// No assignment
	CheckPointMgr& operator=(const CheckPointMgr&);
	
	
	CheckPoints m_checkPoints;
	u32         m_totalSize;
	const u32   m_sizeLimitCheckpoints;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_CHECKPOINTMGR_H)
