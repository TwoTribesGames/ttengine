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

#if defined(TT_PLATFORM_WIN)

#ifndef H_SPK_DX9BUFFERHANDLER
#define H_SPK_DX9BUFFERHANDLER

#include <spark/RenderingAPIs/Shared/SPK_Shared_DEF.h>
#include <spark/RenderingAPIs/Shared/SPK_SharedInfo.h>

#if SPARK_USE_BUFFERHANDLER


namespace SPK
{
namespace Shared
{
	enum {
		DX9_NONE               = 0, // invalid
		DX9_VERTEX_BUFFER_KEY  = 1 << 0,
		DX9_COLOR_BUFFER_KEY   = 1 << 1,
		DX9_INDEX_BUFFER_KEY   = 1 << 2,
		DX9_TEXTURE_BUFFER_KEY = 1 << 3
	};

	class TTBufferHandler
	{
	public :
		virtual ~TTBufferHandler() {}

		virtual bool DX9CreateBuffers(const Group& /*group*/) {return true;};

		virtual bool DX9DestroyBuffers(const Group& /*group*/) {return true;};

		inline bool DX9DestroyAllBuffers();

	protected :

		// The constructor is private so that the class is not instanciable
		TTBufferHandler() {}

		bool DX9PrepareBuffers(const Group& group);

		bool DX9Bind(const Group& group, int key, void** to);

		bool DX9Release(const Group& group, int key);

		bool DX9Create();

		virtual bool DX9CheckBuffers(const Group& group);

		std::map<std::pair<const Group *, int>, IDirect3DResource9 *> DX9Buffers;
		std::map<std::pair<const Group *, int>, IDirect3DResource9 *>::iterator DX9BuffersIt;
		std::pair<const Group *, int> DX9BuffersKey;

	};

	inline bool TTBufferHandler::DX9CheckBuffers(const Group& /*group*/)
	{
		return true;
	}
}}

#endif // SPARK_USE_BUFFERHANDLER

#endif

#endif // #if defined(TT_PLATFORM_WIN)
