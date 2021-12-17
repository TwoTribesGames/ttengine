//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2009 - foulon matthieu - stardeath@wanadoo.fr					//
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


#include <spark/RenderingAPIs/Shared/SPK_SharedRenderer.h>

#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/fwd.h>


namespace SPK
{
namespace Shared
{
	TTRenderer::TTRenderer()
	:
	Renderer(),
	m_blendMode(tt::engine::renderer::BlendMode_Invalid)
	{}
	
	TTRenderer::~TTRenderer()
	{
#if SPARK_USE_BUFFERHANDLER
		DX9Info::DX9ReleaseRenderer(this);
#endif // SPARK_USE_BUFFERHANDLER
	}

	void TTRenderer::setBlending(BlendingMode blendMode)
	{
		switch(blendMode)
		{
		case BLENDING_NONE :
			m_blendMode = tt::engine::renderer::BlendMode_Invalid;
			//srcBlending = D3DBLEND_ONE;
			//destBlending = D3DBLEND_ZERO;
			//blendingEnabled = false;
			break;

		case BLENDING_ADD :
			m_blendMode = tt::engine::renderer::BlendMode_Add;
			//srcBlending = D3DBLEND_SRCALPHA;
			//destBlending = D3DBLEND_ONE;
			//blendingEnabled = true;
			break;

		case BLENDING_ALPHA :
			m_blendMode = tt::engine::renderer::BlendMode_Blend;
			//srcBlending = D3DBLEND_SRCALPHA;
			//destBlending = D3DBLEND_INVSRCALPHA;
			//blendingEnabled = true;
			break;

		case BLENDING_MULTIPLY :
			m_blendMode = tt::engine::renderer::BlendMode_Modulate;
			//srcBlending = D3DBLEND_ZERO;
			//destBlending = D3DBLEND_SRCCOLOR;
			//blendingEnabled = true;
			break;
		}
	}
	
	
	void TTRenderer::initBlending() const
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		if (tt::engine::renderer::isValidBlendMode(m_blendMode))
		{
			renderer->setBlendMode(m_blendMode);
			//	DX9Info::getDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			//	DX9Info::getDevice()->SetRenderState(D3DRS_SRCBLEND, srcBlending);
			//	DX9Info::getDevice()->SetRenderState(D3DRS_DESTBLEND, destBlending);
		}
		else
		{
			renderer->setCustomBlendMode(tt::engine::renderer::BlendFactor_One, tt::engine::renderer::BlendFactor_Zero);
			// FIXME: Would be better if the engine had something like:
			//	DX9Info::getDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		}
	}

// Namespace
}}
