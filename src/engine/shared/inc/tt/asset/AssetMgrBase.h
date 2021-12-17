#ifndef INC_TT_ASSET_ASSETMGRBASE_H
#define INC_TT_ASSET_ASSETMGRBASE_H

#include <tt/platform/tt_error.h>
#include <tt/asset/asset.h>
#include <map>

// Use this define for extra debug output.
#define TT_ASSETMGR_LOG

#ifdef TT_ASSETMGR_LOG
#include <tt/platform/tt_printf.h>
#define TT_ASSMGR_Printf TT_Printf
#else
#define TT_ASSMGR_Printf(...) ((void)0)
#endif

namespace tt {
namespace asset {

template <class AssetType>
class DefaultAssetCreation
{
public:
	static AssetType* create(AssetIdType p_assetId)
	{
		return new AssetType(p_assetId);
	}
	static void destroy(AssetType* p_asset)
	{
		delete p_asset;
	}
private:
	// Static class. -> No creation, copy or assigment.
	DefaultAssetCreation();
	DefaultAssetCreation(const DefaultAssetCreation& p_copy);
	DefaultAssetCreation& operator=(const DefaultAssetCreation& p_rhs);
};


/**
 * 
 */
template
<
	class AssetType,
	template <class> class AssetCreationPolicy = DefaultAssetCreation
>
class AssetMgrBase
{
public:
	typedef AssetType ValueType;
	typedef typename tt_ptr<ValueType>::shared SharedPtr;
	typedef typename tt_ptr<ValueType>::weak WeakPtr;
	
	typedef AssetMgrBase* InstancePtr;
	
	SharedPtr getAsset(AssetIdType p_assetId)
	{
		TT_ASSMGR_Printf("AssetMgrBase::getAsset: %llu\n", p_assetId);
		
		// Search for asset in list.
		{
			WeakPtrContainer::iterator it = m_weakPointers.find(p_assetId);
			if (it != m_weakPointers.end())
			{
				SharedPtr ptr = (*it).second.lock();
				if (ptr &&
					ptr->getId() == p_assetId)
				{
					// Found resource, so return it.
					TT_ASSMGR_Printf("AssetMgrBase::getAsset: found in list.\n");
					return ptr;
				}
			}
		}
		// Resource not found, so it doesn't exist yet.
		//SharedPtr ptr(new ResourceClass(p_assetId), deallocator);
		SharedPtr ptr(AssetCreationPolicy<AssetType>::create(p_assetId),
		              deallocator);
		WeakPtr weakPtr(ptr);
		WeakPtrContainer::value_type ptrPair(p_assetId, weakPtr);
		
		m_weakPointers.insert(ptrPair);
		TT_ASSMGR_Printf("AssetMgrBase::getAsset: new pointer created.\n");
		return ptr;
	}
	
	//* //Should we hardcode singletonness here?
	static void createInstance()
	{
		if (ms_instance == 0)
		{
			ms_instance = new AssetMgrBase;
		}
	}
	
	inline static InstancePtr getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	
	inline static bool hasInstance() { return ms_instance != 0; }
	
	static void destroyInstance()
	{
		delete ms_instance;
		ms_instance = 0;
	}
	// */
	
protected:
	AssetMgrBase()
	{
	}
	
	~AssetMgrBase()
	{
	}
private:
	static void deallocator(ValueType* p_asset)
	{
		getInstance()->remove(p_asset);
	}
	
	void remove(ValueType* p_asset)
	{
		TT_NULL_ASSERT(p_asset);
		TT_ASSMGR_Printf("AssetMgrBase::remove: asset with id %llu\n", p_asset->getId());
		
		WeakPtrContainer::iterator it = m_weakPointers.find(p_asset->getId());
		if(it != m_weakPointers.end())
		{
			TT_ASSERT((*it).second.expired());
			// Remove ptr from list.
			m_weakPointers.erase(it);
			
			// Destroy asset
			AssetCreationPolicy<AssetType>::destroy(p_asset);
			return;
		}
		TT_ASSMGR_Printf("AssetMgrBase::remove: p_asset: %p with id: %llu not found!",
		                 p_asset, p_asset->getId());
	}
	
	// No Copy or assignment
	AssetMgrBase(const AssetMgrBase&);
	const AssetMgrBase& operator=(const AssetMgrBase&);
	
	typedef std::map<tt::asset::AssetIdType, WeakPtr> WeakPtrContainer;
	WeakPtrContainer m_weakPointers;
	
	static InstancePtr ms_instance;
};

template
<
	class AssetType,
	template <class> class AssetCreationPolicy
>
typename AssetMgrBase<AssetType, AssetCreationPolicy>::InstancePtr 
	AssetMgrBase<AssetType, AssetCreationPolicy>::ms_instance = 0;

// Namespace end
}
}


#endif  // !defined(INC_TT_ASSET_ASSETMGRBASE_H)
