//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
//                           foulon matthieu - stardeath@wanadoo.fr				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#ifndef H_SPK_DX9RENDERER
#define H_SPK_DX9RENDERER

#include <spark/RenderingAPIs/Shared/SPK_Shared_DEF.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedInfo.h>
#include <spark/Core/SPK_Renderer.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedBufferHandler.h>

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Renderer.h>

namespace SPK
{
namespace Shared
{
	class TTRenderer : public Renderer
#if SPARK_USE_BUFFERHANDLER
		, public TTBufferHandler
#endif
	{
	public :

		TTRenderer();
		
		virtual ~TTRenderer();
		
		virtual void setBlending(BlendingMode blendMode);

	protected:
		void initBlending() const;
		
		void initRenderingHints() const;
		
	private :
		
		tt::engine::renderer::BlendMode m_blendMode;
	};
	
	
	inline void TTRenderer::initRenderingHints() const
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		
		// alpha test
		if (isRenderingHintEnabled(ALPHA_TEST))
		{
			renderer->setAlphaTestEnabled(true);
			renderer->setAlphaTestFunction(tt::engine::renderer::AlphaTestFunction_GreaterEqual);
			renderer->setAlphaTestValue(static_cast<u8>(getAlphaTestThreshold() * 255)); // TODO: Am I using the right scale here?
			//DX9Info::getDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, true);
			//DX9Info::getDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			//DX9Info::getDevice()->SetRenderState(D3DRS_ALPHAREF, FtoDW(getAlphaTestThreshold()));
		}
		else
		{
			renderer->setAlphaTestEnabled(false);
			//DX9Info::getDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		}
		
		// depth test
		renderer->setZBufferEnabled(isRenderingHintEnabled(DEPTH_TEST));
		//DX9Info::getDevice()->SetRenderState(D3DRS_ZENABLE, isRenderingHintEnabled(DEPTH_TEST));
		
		// depth write
		renderer->setDepthWriteEnabled(isRenderingHintEnabled(DEPTH_WRITE));
		//DX9Info::getDevice()->SetRenderState(D3DRS_ZWRITEENABLE, isRenderingHintEnabled(DEPTH_WRITE));
	}
}}

#endif
