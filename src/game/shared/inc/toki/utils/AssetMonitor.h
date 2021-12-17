#if !defined(INC_TOKI_UTILS_ASSETMONITOR_H)
#define INC_TOKI_UTILS_ASSETMONITOR_H

#include <tt/code/fwd.h>
#include <tt/thread/types.h>
#include <tt/platform/tt_types.h>

namespace toki {
namespace utils {

class AssetMonitor;
typedef tt_ptr<AssetMonitor>::shared AssetMonitorPtr;


class AssetMonitor
{
public:
	enum AssetType
	{
		AssetType_Texture,
		AssetType_Presentation,
		
		AssetType_Count
	};
	
	AssetMonitor();
	~AssetMonitor();
	
	void lockForReload();
	void unlockFromReload();
	void signalAssetReloadCompleted(AssetType p_type);
	bool shouldReloadAssets(AssetType p_type) const;
	
	void start();
	void stop();
	
	void run();
	
private:

	typedef tt::code::BitMask<AssetType, AssetType_Count> ChangedBitMask;
	
	tt::thread::handle m_thread;
	tt::thread::Mutex  m_mutex;
	ChangedBitMask     m_changedBitMask;
	bool               m_threadShouldExit;
	bool               m_isLockedForReload;
};


// Namespace end
}
}

#endif  // !defined(INC_TOKI_UTILS_ASSETMONITOR_H)
