#include <tt/platform/tt_error.h>

#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/thread/CriticalSection.h>


namespace tt {
namespace engine {
namespace renderer {


D3DResourceRegistry::ResourceList D3DResourceRegistry::ms_resources;
bool                              D3DResourceRegistry::ms_insideCallback = false;
thread::Mutex                     D3DResourceRegistry::ms_mutex;


void D3DResourceRegistry::registerResource(D3DResource* p_resource)
{
	thread::CriticalSection cs(&ms_mutex);
	
	// Add resource
	TT_ASSERTMSG(ms_insideCallback == false,
		"Resource registered from inside device callback, this should not happen!");
	TT_NULL_ASSERT(p_resource);
	ms_resources.push_back(p_resource);
}


void D3DResourceRegistry::unregisterResource(D3DResource* p_resource)
{
	thread::CriticalSection cs(&ms_mutex);
	
	TT_ASSERTMSG(ms_insideCallback == false,
		"Resource unregistered from inside device callback, this should not happen!");
	TT_NULL_ASSERT(p_resource);
	if(ms_resources.empty()) return;

	ResourceList::iterator it = std::find(ms_resources.begin(), ms_resources.end(), p_resource);

	// Remove resource
	if(it != ms_resources.end())
	{
		ms_resources.remove(p_resource);
	}
}


void D3DResourceRegistry::onCreateDevice()
{
	thread::CriticalSection cs(&ms_mutex);
	ms_insideCallback = true;
	
	// Let all registered resources know that a new device is created
	// Resources are notified in creation order
	for(ResourceList::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->deviceCreated();
	}
	
	ms_insideCallback = false;
}


void D3DResourceRegistry::onResetDevice()
{
	thread::CriticalSection cs(&ms_mutex);
	ms_insideCallback = true;
	
	// Let all registered resources know the device has been reset
	// Resources are notified in creation order
	for(ResourceList::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->deviceReset();
	}
	
	ms_insideCallback = false;
}


void D3DResourceRegistry::onLostDevice()
{
	thread::CriticalSection cs(&ms_mutex);
	ms_insideCallback = true;
	
	// Let all registered resources know that the device is lost
	// Resources are notified in reverse-creation order
	for(ResourceList::reverse_iterator it = ms_resources.rbegin(); it != ms_resources.rend(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->deviceLost();
	}
	
	ms_insideCallback = false;
}


void D3DResourceRegistry::onDestroyDevice()
{
	thread::CriticalSection cs(&ms_mutex);
	ms_insideCallback = true;
	
	// Let all registered resources know that the device is destroyed
	// Resources are notified in reverse-creation order
	for(ResourceList::reverse_iterator it = ms_resources.rbegin(); it != ms_resources.rend(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->deviceDestroyed();
	}
	
	ms_insideCallback = false;
}


// Namespace end
}
}
}