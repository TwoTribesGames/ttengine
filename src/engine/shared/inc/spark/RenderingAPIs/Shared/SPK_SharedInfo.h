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

#define SPARK_USE_BUFFERHANDLER 0 // define before the following includes.

#if defined(TT_PLATFORM_WIN)

#ifndef H_SPK_DX9INFO
#define H_SPK_DX9INFO

#include <spark/RenderingAPIs/Shared/SPK_Shared_DEF.h>
#include <spark/Core/SPK_Group.h>

#include <vector>
#include <map>

namespace SPK
{
namespace Shared
{
	class TTRenderer;

	class DX9Info
	{
	private:
		static LPDIRECT3DDEVICE9 device;

#if SPARK_USE_BUFFERHANDLER
		static std::vector<TTRenderer *> ms_renderers;
#endif // SPARK_USE_BUFFERHANDLER

	public:
		static LPDIRECT3DDEVICE9 getDevice();
		static D3DPOOL getPool();

		static void setDevice(LPDIRECT3DDEVICE9 device);
		static void setPool(D3DPOOL pool);
		
#if SPARK_USE_BUFFERHANDLER
		static void DX9RegisterRenderer(TTRenderer *renderer);
		static void DX9ReleaseRenderer(TTRenderer *renderer);
#endif // SPARK_USE_BUFFERHANDLER
		
		static void DX9DestroyAllBuffers();
	};

#if SPARK_USE_BUFFERHANDLER
	inline void DX9Info::DX9RegisterRenderer(TTRenderer *renderer)
	{
		ms_renderers.push_back(renderer);
	}

	inline void DX9Info::DX9ReleaseRenderer(TTRenderer *renderer)
	{
		for(std::vector<TTRenderer *>::iterator it = ms_renderers.begin(); it != ms_renderers.end(); )
		{
			if (*it == renderer)
				it = ms_renderers.erase(it);
			else
				++it;
		}
	}
#endif // SPARK_USE_BUFFERHANDLER

	inline LPDIRECT3DDEVICE9 DX9Info::getDevice()
	{
		return DX9Info::device;
	}

	inline void DX9Info::setDevice(LPDIRECT3DDEVICE9 p_device)
	{
		DX9Info::device = p_device;
	}
}}

#endif

#endif // #if defined(TT_PLATFORM_WIN)
