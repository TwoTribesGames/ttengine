#if !defined(INC_TT_ENGINE_RENDERER_D3DRESOURCEREGISTRY_H)
#define INC_TT_ENGINE_RENDERER_D3DRESOURCEREGISTRY_H

#include <list>
#include <tt/thread/Mutex.h>

namespace tt {
namespace engine {
namespace renderer {

class D3DResource;

class D3DResourceRegistry
{
public:
	static void registerResource(D3DResource* p_resource);
	static void unregisterResource(D3DResource* p_resource);

	static void onDestroyDevice();
	static void onCreateDevice();
	static void onLostDevice();
	static void onResetDevice();

private:
	typedef std::list<D3DResource*> ResourceList;
	static ResourceList  ms_resources;
	static bool          ms_insideCallback;
	static thread::Mutex ms_mutex;
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_D3DRESOURCEREGISTRY_H
