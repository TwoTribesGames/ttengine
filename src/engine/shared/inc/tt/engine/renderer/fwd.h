#if !defined(INC_TT_ENGINE_RENDERER_FWD_H)
#define INC_TT_ENGINE_RENDERER_FWD_H

#include <map>
#include <vector>
#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/EngineID.h>


namespace tt {
namespace engine {
namespace renderer {

struct ColorRGB;
struct ColorRGBA;
struct LightProperties;
struct MaterialProperties;

class Renderer;

class Material;
typedef tt_ptr<Material>::shared MaterialPtr;
typedef tt_ptr<Material>::weak   MaterialWeakPtr;

class QuadSprite;
typedef tt_ptr<QuadSprite>::shared QuadSpritePtr;
typedef tt_ptr<QuadSprite>::weak   QuadSpriteWeakPtr;

class Quad2D;
typedef tt_ptr<Quad2D>::shared Quad2DPtr;
typedef tt_ptr<Quad2D>::weak   Quad2DWeakPtr;

class ArcQuad;
typedef tt_ptr<ArcQuad>::shared ArcQuadPtr;
typedef tt_ptr<ArcQuad>::weak   ArcQuadWeakPtr;

class QuadBuffer;
typedef tt_ptr<QuadBuffer>::shared QuadBufferPtr;
typedef tt_ptr<QuadBuffer>::weak   QuadBufferWeakPtr;

class TriangleBuffer;
typedef tt_ptr<TriangleBuffer>::shared TriangleBufferPtr;
typedef tt_ptr<TriangleBuffer>::weak   TriangleBufferWeakPtr;

class TrianglestripBuffer;
typedef tt_ptr<TrianglestripBuffer>::shared TrianglestripBufferPtr;
typedef tt_ptr<TrianglestripBuffer>::weak   TrianglestripBufferWeakPtr;

class TextureStageData;
class TexturePainter;
class Texture;
typedef tt_ptr<Texture>::shared             TexturePtr;
typedef tt_ptr<Texture>::weak               TextureWeakPtr;
typedef std::vector<TexturePtr>             TextureContainer;
typedef std::map<u64, renderer::TexturePtr> EngineIDToTextures;

class MultiTexture;
typedef tt_ptr<MultiTexture>::shared MultiTexturePtr;
typedef tt_ptr<MultiTexture>::weak   MultiTextureWeakPtr;

class Texture3D;
typedef tt_ptr<Texture3D>::shared Texture3DPtr;
typedef tt_ptr<Texture3D>::weak   Texture3DWeakPtr;

class UpScaler;
typedef tt_ptr<UpScaler>::shared UpScalerPtr;
typedef tt_ptr<UpScaler>::weak   UpScalerWeakPtr;

class Shader;
typedef tt_ptr<Shader>::shared ShaderPtr;
typedef tt_ptr<Shader>::weak   ShaderWeakPtr;

class Sphere;
typedef tt_ptr<Sphere>::shared SpherePtr;
typedef tt_ptr<Sphere>::weak   SphereWeakPtr;

class HWLight;

class SubModel;
struct RenderContext;

class ShadowSource;
typedef tt_ptr<ShadowSource>::shared ShadowSourcePtr;
typedef tt_ptr<ShadowSource>::weak   ShadowSourceWeakPtr;

class ViewPort;
struct ViewPortSettings;
typedef std::vector<ViewPort> ViewPortContainer;

class RenderTarget;
typedef tt_ptr<RenderTarget>::shared RenderTargetPtr;
typedef tt_ptr<RenderTarget>::weak   RenderTargetWeakPtr;

}

namespace debug
{
class DebugRenderer;
typedef tt_ptr<DebugRenderer>::shared DebugRendererPtr;
}

}
}

#endif // !defined(INC_TT_ENGINE_RENDERER_FWD_H)
