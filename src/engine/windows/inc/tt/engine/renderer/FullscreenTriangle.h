#if !defined(INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H)
#define INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H


struct IDirect3DVertexBuffer9;


namespace tt {
namespace engine {
namespace renderer {


class FullscreenTriangle
{
public:
	/*! \brief Draws a fullscreen triangle to the screen */
	static void draw();

	// DirectX specific, must be called from Renderer
	static void deviceCreated();
	static void deviceDestroyed();

private:
	static IDirect3DVertexBuffer9* ms_vertexBuffer;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H
