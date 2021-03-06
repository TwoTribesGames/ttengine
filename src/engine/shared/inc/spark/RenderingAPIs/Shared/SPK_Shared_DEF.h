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

#ifndef H_SPK_DX9_DEF
#define H_SPK_DX9_DEF

#ifdef _MSC_VER
#pragma warning(disable : 4275) // disables the warning about exporting DLL classes children of non DLL classes
#endif

// 1.02.02 Compatibility with older versions
#ifdef SPK_DLL
#define SPK_DX9_IMPORT
#endif

#ifndef SPK_NO_DX9_INC

#if (defined(WIN32) || defined(_WIN32)) && !defined(_XBOX)
#define NOMINMAX
#include <windows.h>
#else
#include <xtl.h>
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include <spark/Core/SPK_Vector3D.h>

// trace allocations under windows
/*
#if defined(DEBUG) | defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_CLIENT_BLOCK,__FILE__,__LINE__)
#endif
// */

inline DWORD FtoDW(FLOAT f) { return *((DWORD*)&f); }
inline FLOAT DWtoF(DWORD dw) { return *((FLOAT*)&dw); }

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
/*
#ifndef WSAFE_RELEASE
#define WSAFE_RELEASE(p)      { while ( (p)->Release() ); (p)=NULL; }
#endif
*/

inline void Assign( D3DXVECTOR3& Destination, const SPK::Vector3D& Source )
{
	Destination.x = Source.x;
	Destination.y = Source.y;
	Destination.z = Source.z;
}
inline void Assign( SPK::Vector3D& Destination, const D3DXVECTOR3& Source )
{
	Destination.x = Source.x;
	Destination.y = Source.y;
	Destination.z = Source.z;
}

D3DXINLINE D3DXVECTOR3* D3DXVec3ProjectNormal( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXVECTOR3 *pN )
{
#ifdef D3DX_DEBUG
	if(!pOut || !pV1 || !pV2)
		return NULL;
#endif

	D3DXVec3Cross(pOut, pV, pN);
	D3DXVec3Cross(pOut, pN, pOut);
	return D3DXVec3Normalize(pOut, pOut);
}

#endif

// Defines the APIENTRY if not already done
#ifndef APIENTRY
#define APIENTRY
#endif

/**
* @namespace SPK::DX9
* @brief the namespace for DirectX9 dependent SPARK code
* @since nc
*/

#endif

#endif // #if defined(TT_PLATFORM_WIN)
