#if !defined(INC_TT_ENGINE_EFFECT_QUADRENDERERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_QUADRENDERERWRAPPER_H

#include <spark/RenderingAPIs/Shared/SPK_SharedQuadRenderer.h>
#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/RendererWrapper.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/cache/FileTextureCache.h>

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class QuadRendererWrapper : public RendererWrapper
{
public:
	QuadRendererWrapper()
	{
#if SPARK_USE_DX9QUAD
		m_renderer = SPK::Shared::SharedQuadRenderer::create(SPK::POINT_SPRITE);
		
		// Default setup
		setTexturingMode(SPK::TEXTURE_2D);
		//static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->setTextureBlending();
		
		setBlending(SPK::BLENDING_ALPHA);
		enableRenderingHint(SPK::DEPTH_WRITE, false);
		enableRenderingHint(SPK::DEPTH_TEST, true);
#endif
	}
	
	~QuadRendererWrapper()
	{
#if SPARK_USE_DX9QUAD
		SPK_Destroy(m_renderer);
#endif
	}
	
	
	inline void setScale(float p_scaleX, float p_scaleY)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->setScale(p_scaleX, p_scaleY);
#else
		(void) p_scaleX;
		(void) p_scaleY;
#endif
	}


	inline void setAtlasDimensions(s32 p_x, s32 p_y)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->
			setAtlasDimensions(static_cast<std::size_t>(p_x), static_cast<std::size_t>(p_y));
#else
		(void) p_x;
		(void) p_y;
#endif
	}


	inline void setTexturingMode(SPK::TexturingMode p_mode)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->setTexturingMode(p_mode);
#else
		(void) p_mode;
#endif
	}


	inline void setTexture(const std::string& p_name, const std::string& p_namespace)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->
			setTexture(renderer::TextureCache::get(p_name, p_namespace, true));
#else
		(void) p_name;
		(void) p_namespace;
#endif
	}


	// NOTE: For projects that cannot use the Asset Tool
	inline void setFileTexture(const std::string& p_filename)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->
			setTexture(cache::FileTextureCache::get(p_filename));
#else
		(void) p_filename;
#endif
	}
	
	
	inline void setOrientationEx(SPK::LookOrientation p_lookOrientation,
	                             SPK::UpOrientation   p_upOrientation,
	                             SPK::LockedAxis      p_lockedAxis)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->
			setOrientation(p_lookOrientation, p_upOrientation, p_lockedAxis);
#else
		(void) p_lookOrientation;
		(void) p_upOrientation;
		(void) p_lockedAxis;
#endif
	}
	
	
	inline void setOrientation(SPK::OrientationPreset p_orientation)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->setOrientation(p_orientation);
#else
		(void) p_orientation;
#endif
	}
	
	
	inline void setLookVector(float p_x, float p_y, float p_z)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->lookVector = SPK::Vector3D(p_x, p_y, p_z);
#else
		(void) p_x;
		(void) p_y;
		(void) p_z;
#endif
	}

	
	inline SPK::Vector3D getLookVectorX() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->lookVector.x;
#else
		return SPK::Vector3D();
#endif
	}
	
	
	inline SPK::Vector3D getLookVectorY() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->lookVector.y;
#else
		return SPK::Vector3D();
#endif
	}
	

	inline SPK::Vector3D getLookVectorZ() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->lookVector.z;
#else
		return SPK::Vector3D();
#endif
	}


	
	inline void setUpVector(float p_x, float p_y, float p_z)
	{
#if SPARK_USE_DX9QUAD
		static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->upVector = SPK::Vector3D(p_x, p_y, p_z);
#else
		(void) p_x;
		(void) p_y;
		(void) p_z;
#endif
	}

	
	inline SPK::Vector3D getUpVectorX() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->upVector.x;
#else
		return SPK::Vector3D();
#endif
	}
	
	
	inline SPK::Vector3D getUpVectorY() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->upVector.y;
#else
		return SPK::Vector3D();
#endif
	}
	
	
	inline SPK::Vector3D getUpVectorZ() const
	{
#if SPARK_USE_DX9QUAD
		return static_cast<SPK::Shared::SharedQuadRenderer*>(m_renderer)->upVector.z;
#else
		return SPK::Vector3D();
#endif
	}
	
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<QuadRendererWrapper>::init(p_vm->getVM(), _SC("QuadRenderer"), _SC("Renderer"));
		sqbind_method(p_vm->getVM(), "setScale",           &QuadRendererWrapper::setScale);
		sqbind_method(p_vm->getVM(), "setAtlasDimensions", &QuadRendererWrapper::setAtlasDimensions);
		sqbind_method(p_vm->getVM(), "setTexturingMode",   &QuadRendererWrapper::setTexturingMode);
		sqbind_method(p_vm->getVM(), "setTexture",         &QuadRendererWrapper::setTexture);
		sqbind_method(p_vm->getVM(), "setFileTexture",     &QuadRendererWrapper::setFileTexture);
		
		// HACK-ish Actually from Oriented3DRendererInterface.
		sqbind_method(p_vm->getVM(), "setOrientationEx",   &QuadRendererWrapper::setOrientationEx);
		sqbind_method(p_vm->getVM(), "setOrientation",     &QuadRendererWrapper::setOrientation);
		
		sqbind_method(p_vm->getVM(), "setLookVector",      &QuadRendererWrapper::setLookVector);
		sqbind_method(p_vm->getVM(), "getLookVectorX",     &QuadRendererWrapper::getLookVectorX);
		sqbind_method(p_vm->getVM(), "getLookVectorY",     &QuadRendererWrapper::getLookVectorY);
		sqbind_method(p_vm->getVM(), "getLookVectorZ",     &QuadRendererWrapper::getLookVectorZ);
		
		sqbind_method(p_vm->getVM(), "setUpVector",      &QuadRendererWrapper::setUpVector);
		sqbind_method(p_vm->getVM(), "getUpVectorX",     &QuadRendererWrapper::getUpVectorX);
		sqbind_method(p_vm->getVM(), "getUpVectorY",     &QuadRendererWrapper::getUpVectorY);
		sqbind_method(p_vm->getVM(), "getUpVectorZ",     &QuadRendererWrapper::getUpVectorZ);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_QUADRENDERERWRAPPER_H
