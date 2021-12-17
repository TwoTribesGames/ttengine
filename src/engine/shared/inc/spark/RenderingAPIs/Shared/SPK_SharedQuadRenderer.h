//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
//                           matthieu foulon - stardeath@wanadoo.fr				//
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


#ifndef H_SPK_DX9QUADRENDERER
#define H_SPK_DX9QUADRENDERER

#ifdef min
	#undef min
#endif

#ifdef max
	#undef max
#endif

#include <spark/RenderingAPIs/Shared/SPK_SharedInfo.h>

#if SPARK_USE_BUFFERHANDLER
#	include <tt/engine/renderer/Texture.h>
#else
#	include <tt/engine/renderer/QuadBuffer.h>
#endif

#include <spark/RenderingAPIs/Shared/SPK_SharedRenderer.h>
#include <spark/RenderingAPIs/Shared/SPK_TTQuadBuffer.h>
#include <spark/Extensions/Renderers/SPK_QuadRendererInterface.h>
#include <spark/Extensions/Renderers/SPK_Oriented3DRendererInterface.h>
#include <spark/Core/SPK_Vector3D.h>
#include <spark/Core/SPK_Particle.h>
#include <spark/Core/SPK_Model.h>


#define SPARK_USE_DX9QUAD 1


#define SPARK_USE_TT_MATRIX 1

#if SPARK_USE_DX9QUAD

namespace SPK
{
namespace Shared
{
#if SPARK_USE_BUFFERHANDLER
	// déclaration du vertex descriptor sans coordonnées de texture
	const D3DVERTEXELEMENT9 QuadVertexDecl[3] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};

	// déclaration du vertex descriptor avec coordonnées de textures 2d
	const D3DVERTEXELEMENT9 QuadVertexDecl2D[4] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	// déclaration du vertex descriptor avec coordonnées de textures 3d
	const D3DVERTEXELEMENT9 QuadVertexDecl3D[4] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
#endif // SPARK_USE_BUFFERHANDLER
	
	class SharedQuadRenderer : public TTRenderer, public QuadRendererInterface, public Oriented3DRendererInterface
	{
		SPK_IMPLEMENT_REGISTERABLE(SharedQuadRenderer)

	public :

		//////////////////
		// Constructors //
		//////////////////

		SharedQuadRenderer(float scaleX = 1.0f, float scaleY = 1.0f);

		virtual ~SharedQuadRenderer();

		static SharedQuadRenderer* create(float scaleX = 1.0f,float scaleY = 1.0f);
		
		/////////////
		// Setters //
		/////////////

		virtual bool setTexturingMode(TexturingMode mode);

		void setTexture(const tt::engine::renderer::TexturePtr& textureIndex);


		/////////////
		// Getters //
		/////////////

		const tt::engine::renderer::TexturePtr& getTexture() const;

		///////////////
		// Interface //
		///////////////

		virtual void createBuffers(const Group& group);
		virtual void destroyBuffers(const Group& group);

#if SPARK_USE_BUFFERHANDLER
		virtual bool DX9CreateBuffers(const Group& group);
		virtual bool DX9DestroyBuffers(const Group& group);
		
		static void DX9ReleaseVertexDeclarations();
#endif
		
		virtual void render(const Group& group);
		
	protected:
		
		virtual bool checkBuffers(const Group& group);
#if SPARK_USE_BUFFERHANDLER
		virtual bool DX9CheckBuffers(const Group& group);
#endif

	private :

#if SPARK_USE_TT_MATRIX == 0
		mutable float modelView[16];
		mutable float invModelView[16];
#endif

		tt::engine::renderer::TexturePtr textureIndex;

#if SPARK_USE_BUFFERHANDLER
		// vertex buffers and iterators
		static D3DXVECTOR3* vertexBuffer;
		static D3DXVECTOR3* vertexIterator;
		static DWORD* colorBuffer;
		static DWORD* colorIterator;
		static float* textureBuffer;
		static float* textureIterator;
		static short* indexBuffer;
		static short* indexIterator;

		static LPDIRECT3DVERTEXBUFFER9 DX9VertexBuffer;
		static LPDIRECT3DVERTEXBUFFER9 DX9ColorBuffer;
		static LPDIRECT3DVERTEXBUFFER9 DX9TextureBuffer;
		static LPDIRECT3DINDEXBUFFER9 DX9IndexBuffer;

		// vertex declaration
		static LPDIRECT3DVERTEXDECLARATION9 pVertexDecl;
		static LPDIRECT3DVERTEXDECLARATION9 pVertexDecl2D;
		static LPDIRECT3DVERTEXDECLARATION9 pVertexDecl3D;

		// buffers names
		static const std::string VERTEX_BUFFER_NAME;
		static const std::string COLOR_BUFFER_NAME;
		static const std::string TEXTURE_BUFFER_NAME;
		static const std::string INDEX_BUFFER_NAME;
		
		float* createTextureBuffer(const Group& group) const;
#else
		static const std::string                         QUAD_BUFFER_NAME;
		static TTQuadBuffer*                             ms_quadBufferForActiveGroup;
		static tt::engine::renderer::BatchQuadCollection ms_quadCollection;
#endif // SPARK_USE_BUFFERHANDLER
		
		void DX9CallColorAndVertex(const Particle& particle) const; // DX9 calls for color and position
		void DX9CallTexture2DAtlas(const Particle& particle) const; // DX9 calls for 2D atlastexturing 
		void DX9CallTexture3D(     const Particle& particle) const; // DX9 calls for 3D texturing
		void nextQuad() const;
		
		static void (SharedQuadRenderer::*renderParticle)(const Particle&)  const;	// pointer to the right render method
		
		void render2D(        const Particle& particle) const; // Rendering for particles with texture 2D or no texture
		void render2DRot(     const Particle& particle) const; // Rendering for particles with texture 2D or no texture and rotation
		void render3D(        const Particle& particle) const; // Rendering for particles with texture 3D
		void render3DRot(     const Particle& particle) const; // Rendering for particles with texture 3D and rotation
		void render2DAtlas(   const Particle& particle) const; // Rendering for particles with texture 2D atlas
		void render2DAtlasRot(const Particle& particle) const; // Rendering for particles with texture 2D atlas and rotation
	};
	
	inline SharedQuadRenderer* SharedQuadRenderer::create(float scaleX,float scaleY)
	{
		SharedQuadRenderer* obj = new SharedQuadRenderer(scaleX,scaleY);
		registerObject(obj);
#if SPARK_USE_BUFFERHANDLER
		DX9Info::DX9RegisterRenderer(obj);
#endif
		return obj;
	}

	inline void SharedQuadRenderer::setTexture(const tt::engine::renderer::TexturePtr& p_textureIndex)
	{
		this->textureIndex = p_textureIndex;
	}

	inline const tt::engine::renderer::TexturePtr& SharedQuadRenderer::getTexture() const
	{
		return textureIndex;
	}

	inline void SharedQuadRenderer::DX9CallColorAndVertex(const Particle& particle) const
	{
		float x = particle.position().x;
		float y = particle.position().y;
		float z = particle.position().z;

#if SPARK_USE_BUFFERHANDLER

		// top left vertex
		(vertexIterator)->x = x + quadSide().x + quadUp().x;
		(vertexIterator)->y = y + quadSide().y + quadUp().y;
		(vertexIterator++)->z = z + quadSide().z + quadUp().z;

		// top right vertex
		(vertexIterator)->x = x - quadSide().x + quadUp().x;
		(vertexIterator)->y = y - quadSide().y + quadUp().y;
		(vertexIterator++)->z = z - quadSide().z + quadUp().z;

		// bottom right vertex
		(vertexIterator)->x = x - quadSide().x - quadUp().x;
		(vertexIterator)->y = y - quadSide().y - quadUp().y;
		(vertexIterator++)->z = z - quadSide().z - quadUp().z;

		// bottom left vertex
		(vertexIterator)->x = x + quadSide().x - quadUp().x;
		(vertexIterator)->y = y + quadSide().y - quadUp().y;
		(vertexIterator++)->z = z + quadSide().z - quadUp().z;

		// TODO : avoid duplication of color information
		DWORD color = D3DCOLOR_COLORVALUE(particle.getR(), particle.getG(), particle.getB(), particle.getParamCurrentValue(PARAM_ALPHA));
		*(colorIterator++) = color;
		*(colorIterator++) = color;
		*(colorIterator++) = color;
		*(colorIterator++) = color;
#else // SPARK_USE_BUFFERHANDLER
		
		tt::engine::renderer::BatchQuad& quad = ms_quadCollection.front();
		using tt::engine::renderer::ColorRGBA;
		const ColorRGBA color(ColorRGBA::createFromNormalized(tt::math::Vector4(
				particle.getR(), particle.getG(), particle.getB(), particle.getParamCurrentValue(PARAM_ALPHA))));
		
		// top left vertex
		quad.topLeft.setPositionX(x + quadSide().x + quadUp().x);
		quad.topLeft.setPositionY(y + quadSide().y + quadUp().y);
		quad.topLeft.setPositionZ(z + quadSide().z + quadUp().z);
		quad.topLeft.setColor(color);
		
		// top right vertex
		quad.topRight.setPositionX(x - quadSide().x + quadUp().x);
		quad.topRight.setPositionY(y - quadSide().y + quadUp().y);
		quad.topRight.setPositionZ(z - quadSide().z + quadUp().z);
		quad.topRight.setColor(color);
		
		// bottom right vertex
		quad.bottomRight.setPositionX(x - quadSide().x - quadUp().x);
		quad.bottomRight.setPositionY(y - quadSide().y - quadUp().y);
		quad.bottomRight.setPositionZ(z - quadSide().z - quadUp().z);
		quad.bottomRight.setColor(color);
		
		// bottom left vertex
		quad.bottomLeft.setPositionX(x + quadSide().x - quadUp().x);
		quad.bottomLeft.setPositionY(y + quadSide().y - quadUp().y);
		quad.bottomLeft.setPositionZ(z + quadSide().z - quadUp().z);
		quad.bottomLeft.setColor(color);
		
#endif // else SPARK_USE_BUFFERHANDLER
	}

	inline void SharedQuadRenderer::DX9CallTexture2DAtlas(const Particle& particle) const
	{
		computeAtlasCoordinates(particle);

#if SPARK_USE_BUFFERHANDLER

		*(textureIterator++) = textureAtlasU0();
		*(textureIterator++) = textureAtlasV0();

		*(textureIterator++) = textureAtlasU1();
		*(textureIterator++) = textureAtlasV0();

		*(textureIterator++) = textureAtlasU1();
		*(textureIterator++) = textureAtlasV1();

		*(textureIterator++) = textureAtlasU0();
		*(textureIterator++) = textureAtlasV1();

#else // SPARK_USE_BUFFERHANDLER
		tt::engine::renderer::BatchQuad& quad = ms_quadCollection.front();
		
		quad.topLeft    .setTexCoord(textureAtlasU0(), textureAtlasV0());
		quad.topRight   .setTexCoord(textureAtlasU1(), textureAtlasV0());
		quad.bottomRight.setTexCoord(textureAtlasU1(), textureAtlasV1());
		quad.bottomLeft .setTexCoord(textureAtlasU0(), textureAtlasV1());
#endif // else SPARK_USE_BUFFERHANDLER
	}

	inline void SharedQuadRenderer::DX9CallTexture3D(const Particle& particle) const
	{
		float localTextureIndex = particle.getParamCurrentValue(PARAM_TEXTURE_INDEX);

#if SPARK_USE_BUFFERHANDLER
		*(textureIterator + 2) = localTextureIndex;
		*(textureIterator + 5) = localTextureIndex;
		*(textureIterator + 8) = localTextureIndex;
		*(textureIterator + 11) = localTextureIndex;
		textureIterator += 12;
#else // SPARK_USE_BUFFERHANDLER
		(void)localTextureIndex;
		TT_PANIC("3D Texture unsupported!");
#endif // else SPARK_USE_BUFFERHANDLER
	}
	
	inline void SharedQuadRenderer::nextQuad() const
	{
#if SPARK_USE_BUFFERHANDLER
		// Nothing
#else
		ms_quadBufferForActiveGroup->addCollection(ms_quadCollection);
		
		// Reset quad to defaults.
		tt::engine::renderer::BatchQuad& quad = ms_quadCollection.front();
		
		quad.topLeft    .setTexCoord(0.0f, 0.0f);
		quad.topRight   .setTexCoord(1.0f, 0.0f);
		quad.bottomRight.setTexCoord(1.0f, 1.0f);
		quad.bottomLeft .setTexCoord(0.0f, 1.0f);
#endif
	}
}}

#endif // SPARK_USE_DX9

#endif
