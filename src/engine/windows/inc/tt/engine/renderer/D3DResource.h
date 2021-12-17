#if !defined(INC_TT_ENGINE_RENDERER_D3DRESOURCE_H)
#define INC_TT_ENGINE_RENDERER_D3DRESOURCE_H

namespace tt {
namespace engine {
namespace renderer {


class D3DResource
{
public:
	// Resource classes should override these functions when needed 
	// to (re-)create & release DirectX resources.
	virtual void deviceCreated()   {}
	virtual void deviceLost()      {}
	virtual void deviceReset()     {}
	virtual void deviceDestroyed() {}

protected:
	D3DResource() {}
	~D3DResource() {}
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_D3DRESOURCE_H
