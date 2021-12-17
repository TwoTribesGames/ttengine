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

#if defined(TT_PLATFORM_WIN)

#include <spark/RenderingAPIs/Shared/SPK_SharedInfo.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedRenderer.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedQuadRenderer.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedLineTrailRenderer.h>

namespace SPK
{
namespace Shared
{
	LPDIRECT3DDEVICE9 DX9Info::device = NULL;

#if SPARK_USE_BUFFERHANDLER
	std::vector<TTRenderer*> DX9Info::ms_renderers;
#endif // SPARK_USE_BUFFERHANDLER

	void DX9Info::DX9DestroyAllBuffers()
	{
#if SPARK_USE_BUFFERHANDLER
		for(size_t i = 0; i < ms_renderers.size(); i++)
		{
			DX9Info::ms_renderers[i]->DX9DestroyAllBuffers();
		}
#endif // SPARK_USE_BUFFERHANDLER
		
#if SPARK_USE_DX9QUAD && SPARK_USE_BUFFERHANDLER
		// Release static vertex declarations
		SharedQuadRenderer::DX9ReleaseVertexDeclarations();
#endif // SPARK_USE_DX9QUAD
#if SPARK_USE_DX9LINETRAIL 
		DX9LineTrailRenderer::DX9ReleaseVertexDeclarations();
#endif // SPARK_USE_DX9LINETRAIL
	}
}}

#endif
