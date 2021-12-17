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


#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/Camera.h>

#include <spark/RenderingAPIs/Shared/SPK_SharedQuadRenderer.h>
#include <spark/RenderingAPIs/Shared/SPK_TTQuadBuffer.h>
#include <spark/Core/SPK_Particle.h>
#include <spark/Core/SPK_Group.h>

namespace SPK
{
namespace Shared
{
#if SPARK_USE_DX9QUAD

#if SPARK_USE_BUFFERHANDLER

	const std::string SharedQuadRenderer::VERTEX_BUFFER_NAME( "SPK_DX9QuadRenderer_Vertex");
	const std::string SharedQuadRenderer::COLOR_BUFFER_NAME(  "SPK_DX9QuadRenderer_Color");
	const std::string SharedQuadRenderer::TEXTURE_BUFFER_NAME("SPK_DX9QuadRenderer_Texture");
	const std::string SharedQuadRenderer::INDEX_BUFFER_NAME(  "SPK_DX9QuadRenderer_Index");

	D3DXVECTOR3* SharedQuadRenderer::vertexBuffer   = NULL;
	D3DXVECTOR3* SharedQuadRenderer::vertexIterator = NULL;
	DWORD* SharedQuadRenderer::colorBuffer          = NULL;
	DWORD* SharedQuadRenderer::colorIterator        = NULL;
	float* SharedQuadRenderer::textureBuffer        = NULL;
	float* SharedQuadRenderer::textureIterator      = NULL;
	short* SharedQuadRenderer::indexBuffer          = NULL;
	short* SharedQuadRenderer::indexIterator        = NULL;

	LPDIRECT3DVERTEXBUFFER9 SharedQuadRenderer::DX9VertexBuffer  = NULL;
	LPDIRECT3DVERTEXBUFFER9 SharedQuadRenderer::DX9ColorBuffer   = NULL;
	LPDIRECT3DVERTEXBUFFER9 SharedQuadRenderer::DX9TextureBuffer = NULL;
	LPDIRECT3DINDEXBUFFER9  SharedQuadRenderer::DX9IndexBuffer   = NULL;

	LPDIRECT3DVERTEXDECLARATION9 SharedQuadRenderer::pVertexDecl   = NULL;
	LPDIRECT3DVERTEXDECLARATION9 SharedQuadRenderer::pVertexDecl2D = NULL;
	LPDIRECT3DVERTEXDECLARATION9 SharedQuadRenderer::pVertexDecl3D = NULL;
#else
	const std::string                         SharedQuadRenderer::QUAD_BUFFER_NAME("SPK_TTQuadRenderer_Quads");
	TTQuadBuffer*                             SharedQuadRenderer::ms_quadBufferForActiveGroup = 0;
	tt::engine::renderer::BatchQuadCollection SharedQuadRenderer::ms_quadCollection;
#endif // SPARK_USE_BUFFERHANDLER

	void (SharedQuadRenderer::*SharedQuadRenderer::renderParticle)(const Particle&) const = NULL;

	SharedQuadRenderer::SharedQuadRenderer(float p_scaleX, float p_scaleY) :
		TTRenderer(),
		QuadRendererInterface(p_scaleX, p_scaleY),
		Oriented3DRendererInterface()
	{}

	SharedQuadRenderer::~SharedQuadRenderer()
	{
#if SPARK_USE_BUFFERHANDLER
		DX9DestroyAllBuffers();
#endif
	}

	bool SharedQuadRenderer::checkBuffers(const Group& group)
	{
#if SPARK_USE_BUFFERHANDLER
		ArrayBuffer<D3DXVECTOR3>* pvbBuffer = NULL;
		ArrayBuffer<DWORD>* dwColorBuffer = NULL;
		ArrayBuffer<short>* sIndexBuffer = NULL;

		if ((pvbBuffer = dynamic_cast<ArrayBuffer<D3DXVECTOR3>*>(group.getBuffer(VERTEX_BUFFER_NAME))) == NULL)
			return false;

		if ((dwColorBuffer = dynamic_cast<ArrayBuffer<DWORD>*>(group.getBuffer(COLOR_BUFFER_NAME))) == NULL)
			return false;

		if ((sIndexBuffer = dynamic_cast<ArrayBuffer<short>*>(group.getBuffer(INDEX_BUFFER_NAME))) == NULL)
			return false;

		if( texturingMode != TEXTURE_NONE )
		{
			FloatBuffer* fTextureBuffer;

			if ((fTextureBuffer = dynamic_cast<FloatBuffer*>(group.getBuffer(TEXTURE_BUFFER_NAME,texturingMode))) == NULL)
				textureBuffer = createTextureBuffer(group);

			textureIterator = textureBuffer = fTextureBuffer->getData();
		}

		vertexIterator = vertexBuffer = pvbBuffer->getData();
		colorIterator = colorBuffer = dwColorBuffer->getData();
		indexIterator = indexBuffer = sIndexBuffer->getData();

#else
		ms_quadBufferForActiveGroup = dynamic_cast<TTQuadBuffer*>(group.getBuffer(QUAD_BUFFER_NAME));
		TT_NULL_ASSERT(ms_quadBufferForActiveGroup);
		if (ms_quadBufferForActiveGroup == 0 || // no buffer
		    ms_quadBufferForActiveGroup->getQuadBuffer() == 0 || // no (intern) quad buffer
		    ms_quadBufferForActiveGroup->getQuadBuffer()->getCapacity() < static_cast<s32>(group.getNbParticles())) // Not enough capacity. (Not sure if we need to check this...)
		{
			return false;
		}
		ms_quadBufferForActiveGroup->clear();
#endif
		return true;
	}

	void SharedQuadRenderer::createBuffers(const Group& group)
	{
#if SPARK_USE_BUFFERHANDLER
		ArrayBuffer<D3DXVECTOR3>* vbVertexBuffer = dynamic_cast<ArrayBuffer<D3DXVECTOR3>*>(group.createBuffer(VERTEX_BUFFER_NAME, ArrayBufferCreator<D3DXVECTOR3>(4),0,false));
		ArrayBuffer<DWORD      >* vbColorBuffer  = dynamic_cast<ArrayBuffer<DWORD      >*>(group.createBuffer(COLOR_BUFFER_NAME , ArrayBufferCreator<DWORD      >(4),0,false));
		ArrayBuffer<short      >* ibIndexBuffer  = dynamic_cast<ArrayBuffer<short      >*>(group.createBuffer(INDEX_BUFFER_NAME , ArrayBufferCreator<short      >(6),0,false));
		vertexIterator = vertexBuffer = vbVertexBuffer->getData();
		colorIterator  = colorBuffer  = vbColorBuffer->getData();
		indexIterator  = indexBuffer  = ibIndexBuffer->getData();

		if( texturingMode != TEXTURE_NONE )
			textureIterator = textureBuffer = createTextureBuffer(group);

		short offsetIndex = 0;

		// initialisation de l'index buffer
		for(size_t i = 0; i < group.getParticles().getNbReserved(); i++)
		{
			*(indexIterator++) = 0 + offsetIndex;
			*(indexIterator++) = 1 + offsetIndex;
			*(indexIterator++) = 2 + offsetIndex;
			*(indexIterator++) = 0 + offsetIndex;
			*(indexIterator++) = 2 + offsetIndex;
			*(indexIterator++) = 3 + offsetIndex;

			offsetIndex += 4;
		}
#else // SPARK_USE_BUFFERHANDLER
		ms_quadBufferForActiveGroup = dynamic_cast<TTQuadBuffer*>(group.createBuffer(QUAD_BUFFER_NAME, TTQuadBufferCreator(),0,false));
		
		TT_NULL_ASSERT(ms_quadBufferForActiveGroup);
		
		if (ms_quadCollection.empty())
		{
			ms_quadCollection.push_back(tt::engine::renderer::BatchQuad());
		}
		TT_ASSERT(ms_quadCollection.size() == 1);
		tt::engine::renderer::BatchQuad& quad = ms_quadCollection.front();
		
		quad.topLeft    .setTexCoord(0.0f, 0.0f);
		quad.topRight   .setTexCoord(1.0f, 0.0f);
		quad.bottomRight.setTexCoord(1.0f, 1.0f);
		quad.bottomLeft .setTexCoord(0.0f, 1.0f);
#endif // else SPARK_USE_BUFFERHANDLER
	}

	void SharedQuadRenderer::destroyBuffers(const Group& group)
	{
#if SPARK_USE_BUFFERHANDLER
		group.destroyBuffer(VERTEX_BUFFER_NAME );
		group.destroyBuffer(COLOR_BUFFER_NAME  );
		group.destroyBuffer(TEXTURE_BUFFER_NAME);
		group.destroyBuffer(INDEX_BUFFER_NAME  );
#else  // SPARK_USE_BUFFERHANDLER
		group.destroyBuffer(QUAD_BUFFER_NAME);
#endif // else SPARK_USE_BUFFERHANDLER
	}

#if SPARK_USE_BUFFERHANDLER
	float* SharedQuadRenderer::createTextureBuffer(const Group& group) const
	{
		FloatBuffer* fbuffer = NULL;

		switch(texturingMode)
		{
			case TEXTURE_2D :
				fbuffer = dynamic_cast<FloatBuffer*>(group.createBuffer(TEXTURE_BUFFER_NAME,FloatBufferCreator(8),TEXTURE_2D,false));
				if (!group.getModel()->isEnabled(PARAM_TEXTURE_INDEX))
				{
					float t[8] = {0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f};
					for (size_t i = 0; i < group.getParticles().getNbReserved() << 3; ++i)
						fbuffer->getData()[i] = t[i & 7];
				}
				break;

			case TEXTURE_3D :
				fbuffer = dynamic_cast<FloatBuffer*>(group.createBuffer(TEXTURE_BUFFER_NAME,FloatBufferCreator(12),TEXTURE_3D,false));
				float t[12] =  {0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f};
				for (size_t i = 0; i < group.getParticles().getNbReserved() * 12; ++i)
					fbuffer->getData()[i] = t[i % 12];
				break;
		}

		return fbuffer->getData();
	}
#endif // SPARK_USE_BUFFERHANDLER

	bool SharedQuadRenderer::setTexturingMode(TexturingMode mode)
	{
		texturingMode = mode;
		return true;
	}
	
	void SharedQuadRenderer::render(const Group& group)
	{
		//HRESULT hr = 0;
		int nb_part = static_cast<int>(group.getNbParticles());

		if(nb_part == 0)
			return;
#if SPARK_USE_BUFFERHANDLER
		if (!DX9PrepareBuffers(group))
			return;
#endif
		
		if( !prepareBuffers(group) )
			return;
		
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
#if SPARK_USE_TT_MATRIX == 0
		D3DXMATRIX view,world,modelView;
		DX9Info::getDevice()->GetTransform(D3DTS_VIEW, &view);
		DX9Info::getDevice()->GetTransform(D3DTS_WORLD, &world);
		modelView = world * view;
		D3DXMatrixInverse((D3DXMATRIX*)&invModelView, NULL, &modelView);
#else
		tt::engine::scene::CameraPtr camera(renderer->getMainCamera(renderer->getActiveViewPort()));
		tt::engine::renderer::MatrixStack* stack = tt::engine::renderer::MatrixStack::getInstance();
		stack->setMode(tt::engine::renderer::MatrixStack::Mode_Position);
		const tt::math::Matrix44 viewMtx  = camera->getViewMatrix();
		const tt::math::Matrix44 worldMtx = stack->getCurrent();
		tt::math::Matrix44 invModelViewMtx = (viewMtx * worldMtx).getInverse();
#endif
		
		initBlending();
		initRenderingHints(); // Marco: Added to correctly setup render state
		
		// No tt renderer replacement needed when doing shaders.
		//DX9Info::getDevice()->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		renderer->setCullingEnabled(false);
		//DX9Info::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		switch(texturingMode)
		{
			case TEXTURE_2D :
				
#if SPARK_USE_BUFFERHANDLER
				tt::engine::renderer::Renderer::getInstance()->setTexture(textureIndex);

				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);

				// Marco: Fixed this for correct alpha blending
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

				DX9Info::getDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				DX9Info::getDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
#else
				ms_quadBufferForActiveGroup->getQuadBuffer()->setTexture(textureIndex);
#endif
				if (!group.getModel()->isEnabled(PARAM_TEXTURE_INDEX))
				{
					if (!group.getModel()->isEnabled(PARAM_ANGLE))
						renderParticle = &SharedQuadRenderer::render2D;
					else
						renderParticle = &SharedQuadRenderer::render2DRot;
				}
				else
				{
					if (!group.getModel()->isEnabled(PARAM_ANGLE))
						renderParticle = &SharedQuadRenderer::render2DAtlas;
					else
						renderParticle = &SharedQuadRenderer::render2DAtlasRot;
				}
				break;

			case TEXTURE_3D :
#if SPARK_USE_BUFFERHANDLER
				tt::engine::renderer::Renderer::getInstance()->setTexture(textureIndex);

				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);

				// Marco: Fixed this for correct alpha blending
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				DX9Info::getDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

				DX9Info::getDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				DX9Info::getDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

				if (!group.getModel()->isEnabled(PARAM_ANGLE))
					renderParticle = &SharedQuadRenderer::render3D;
				else
					renderParticle = &SharedQuadRenderer::render3DRot;
#else
				TT_PANIC("3D texture unsupported!");
#endif
				break;

			case TEXTURE_NONE :
#if SPARK_USE_BUFFERHANDLER
				tt::engine::renderer::Renderer::getInstance()->setTexture(
					tt::engine::renderer::TexturePtr()); // Empty texture
				DX9Info::getDevice()->SetRenderState(D3DRS_COLORVERTEX, true);
#else
				ms_quadBufferForActiveGroup->getQuadBuffer()->setTexture(tt::engine::renderer::TexturePtr());  // Empty texture
#endif
				if (!group.getModel()->isEnabled(PARAM_ANGLE))
					renderParticle = &SharedQuadRenderer::render2D;
				else
					renderParticle = &SharedQuadRenderer::render2DRot;
				break;
		}

		bool globalOrientation = precomputeOrientation3D(
			group,
#if SPARK_USE_TT_MATRIX == 0
			Vector3D(invModelView[8],invModelView[9],invModelView[10]),
			Vector3D(invModelView[4],invModelView[5],invModelView[6]),
			Vector3D(invModelView[12],invModelView[13],invModelView[14])
#else
			//  1  2  3  4
			// -------------
			//  0  1  2  3 - 1
			//  4  5  6  7 - 2
			//  8  9 10 11 - 3
			// 12 13 14 15 - 4
			// 
			Vector3D(invModelViewMtx.m_31, invModelViewMtx.m_32, invModelViewMtx.m_33),
			Vector3D(invModelViewMtx.m_21, invModelViewMtx.m_22, invModelViewMtx.m_23),
			Vector3D(invModelViewMtx.m_41, invModelViewMtx.m_42, invModelViewMtx.m_43)
#endif
			);

		if (globalOrientation)
		{
			computeGlobalOrientation3D();

			for (size_t i = 0; i < group.getNbParticles(); ++i)
				(this->*renderParticle)(group.getParticle(i));
		}
		else
		{
			for (size_t i = 0; i < group.getNbParticles(); ++i)
			{
				const Particle& particle = group.getParticle(i);
				computeSingleOrientation3D(particle);
				(this->*renderParticle)(particle);
			}
		}
		
		
#if SPARK_USE_BUFFERHANDLER
		// bind buffers and draw
		{
			LPDIRECT3DDEVICE9 device = DX9Info::getDevice();
			void *ptr;

			switch( texturingMode )
			{
				case TEXTURE_2D:
					{
						DX9TextureBuffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD);
						memcpy(ptr, textureBuffer, 4 * group.getNbParticles() * sizeof(D3DXVECTOR2));
						DX9TextureBuffer->Unlock();

						device->SetStreamSource(2, DX9TextureBuffer, 0, sizeof(D3DXVECTOR2));
						device->SetVertexDeclaration(pVertexDecl2D);
					}
					break;

				case TEXTURE_3D:
					{
						DX9TextureBuffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD);
						memcpy(ptr, textureBuffer, 4 * group.getNbParticles() * sizeof(D3DXVECTOR3));
						DX9TextureBuffer->Unlock();

						device->SetStreamSource(2, DX9TextureBuffer, 0, sizeof(D3DXVECTOR3));
						device->SetVertexDeclaration(pVertexDecl3D);
					}
					break;

				case TEXTURE_NONE:
					{
						device->SetVertexDeclaration(pVertexDecl);
					}
					break;
			}

			DX9VertexBuffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD);
			memcpy(ptr, vertexBuffer, 4 * group.getNbParticles() * sizeof(D3DXVECTOR3));
			DX9VertexBuffer->Unlock();
			device->SetStreamSource(0, DX9VertexBuffer, 0, sizeof(D3DXVECTOR3));

			DX9ColorBuffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD);
			memcpy(ptr, colorBuffer, 4 * group.getNbParticles() * sizeof(DWORD));
			DX9ColorBuffer->Unlock();
			device->SetStreamSource(1, DX9ColorBuffer, 0, sizeof(DWORD));

			DX9IndexBuffer->Lock(0, 0, &ptr, D3DLOCK_DISCARD);
			memcpy(ptr, indexBuffer, 6 * group.getNbParticles() * sizeof(short));
			DX9IndexBuffer->Unlock();
			device->SetIndices(DX9IndexBuffer);

			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nb_part<<2, 0, nb_part<<1);
		}
		//---------------------------------------------------------------------------
#else
		ms_quadBufferForActiveGroup->getQuadBuffer()->applyChanges();
		ms_quadBufferForActiveGroup->getQuadBuffer()->render();
#endif // SPARK_USE_BUFFERHANDLER
	}
	void SharedQuadRenderer::render2D(const Particle& particle) const
	{
		scaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		nextQuad();
	}

	void SharedQuadRenderer::render2DRot(const Particle& particle) const
	{
		rotateAndScaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		nextQuad();
	}

	void SharedQuadRenderer::render3D(const Particle& particle) const
	{
		scaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		DX9CallTexture3D(particle);
		nextQuad();
	}

	void SharedQuadRenderer::render3DRot(const Particle& particle) const
	{
		rotateAndScaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		DX9CallTexture3D(particle);
		nextQuad();
	}

	void SharedQuadRenderer::render2DAtlas(const Particle& particle) const
	{
		scaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		DX9CallTexture2DAtlas(particle);
		nextQuad();
	}

	void SharedQuadRenderer::render2DAtlasRot(const Particle& particle) const
	{
		rotateAndScaleQuadVectors(particle,scaleX,scaleY);
		DX9CallColorAndVertex(particle);
		DX9CallTexture2DAtlas(particle);
		nextQuad();
	}

#if SPARK_USE_BUFFERHANDLER
	bool SharedQuadRenderer::DX9CheckBuffers(const Group& group)
	{
		if( !DX9Bind(group, DX9_VERTEX_BUFFER_KEY, (void**)&DX9VertexBuffer) )
		{
			DX9VertexBuffer = DX9ColorBuffer = DX9TextureBuffer = NULL;
			DX9IndexBuffer = NULL;
			return false;
		}
		if( !DX9Bind(group, DX9_COLOR_BUFFER_KEY, (void**)&DX9ColorBuffer) )
		{
			DX9VertexBuffer = DX9ColorBuffer = DX9TextureBuffer = NULL;
			DX9IndexBuffer = NULL;
			return false;
		}
		if( !DX9Bind(group, DX9_INDEX_BUFFER_KEY, (void**)&DX9IndexBuffer) )
		{
			DX9VertexBuffer = DX9ColorBuffer = DX9TextureBuffer = NULL;
			DX9IndexBuffer = NULL;
			return false;
		}
		if( texturingMode != TEXTURE_NONE )
		{
			if( !DX9Bind(group, DX9_TEXTURE_BUFFER_KEY, (void**)&DX9TextureBuffer) )
			{
				DX9VertexBuffer = DX9ColorBuffer = DX9TextureBuffer = NULL;
				DX9IndexBuffer = NULL;
				return false;
			}
		}
		
		return true;
	}
#endif // SPARK_USE_BUFFERHANDLER
	
#if SPARK_USE_BUFFERHANDLER
	bool SharedQuadRenderer::DX9CreateBuffers(const Group& group)
	{
		std::cout << "SharedQuadRenderer::DX9CreateBuffers" << std::endl;

		if( DX9Info::getDevice() == NULL ) return false;

		if( pVertexDecl == 0 )
		{
			if(SUCCEEDED(DX9Info::getDevice()->CreateVertexDeclaration(QuadVertexDecl, &pVertexDecl)) == false)
			{
				return false;
			}
		}
		if( pVertexDecl2D == 0 )
		{
			if(SUCCEEDED(DX9Info::getDevice()->CreateVertexDeclaration(QuadVertexDecl2D, &pVertexDecl2D)) == false)
			{
				return false;
			}
		}
		if( pVertexDecl3D == 0 )
		{
			if(SUCCEEDED(DX9Info::getDevice()->CreateVertexDeclaration(QuadVertexDecl3D, &pVertexDecl3D)) == false)
			{
				return false;
			}
		}

		LPDIRECT3DVERTEXBUFFER9 vb;

		// vertex buffer
		if( DX9Info::getDevice()->CreateVertexBuffer(
			group.getParticles().getNbReserved() * 4 * sizeof(D3DXVECTOR3),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
			D3DFVF_XYZ,
			D3DPOOL_DEFAULT, &vb, NULL) != S_OK )
		{
			return false;
		}

		std::pair<const Group *, int> key(&group, DX9_VERTEX_BUFFER_KEY);
		DX9Buffers[key] = vb;
		DX9VertexBuffer = vb;
		//-----------------------------------------------------------------------------------------------

		// color buffer
		if( DX9Info::getDevice()->CreateVertexBuffer(
			group.getParticles().getNbReserved() * 4 * sizeof(D3DCOLOR),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
			D3DFVF_DIFFUSE,
			D3DPOOL_DEFAULT, &vb, NULL) != S_OK)
		{
			return false;
		}

		key = std::pair<const Group *, int>(&group, DX9_COLOR_BUFFER_KEY);
		DX9Buffers[key] = vb;
		DX9ColorBuffer = vb;
		//-----------------------------------------------------------------------------------------------

		// index buffer
		LPDIRECT3DINDEXBUFFER9 ib;

		if( DX9Info::getDevice()->CreateIndexBuffer(
			group.getParticles().getNbReserved() * 6 * sizeof(short),
			D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT, &ib, NULL) != S_OK )
		{
			return false;
		}

		key = std::pair<const Group *, int>(&group, DX9_INDEX_BUFFER_KEY);
		DX9Buffers[key] = ib;
		DX9IndexBuffer = ib;
		//-----------------------------------------------------------------------------------------------

		// texture buffer
		switch(texturingMode)
		{
		case TEXTURE_2D :
			if( DX9Info::getDevice()->CreateVertexBuffer(
				group.getParticles().getNbReserved() * 8 * sizeof(float),
				D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
				D3DFVF_TEX1,
				D3DPOOL_DEFAULT, &vb, NULL) != S_OK )
			{
				return false;
			}

			key = std::pair<const Group *, int>(&group, DX9_TEXTURE_BUFFER_KEY);
			DX9Buffers[key] = vb;
			DX9TextureBuffer = vb;
			break;

		case TEXTURE_3D :
			if( DX9Info::getDevice()->CreateVertexBuffer(
				group.getParticles().getNbReserved() * 12 * sizeof(float),
				D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
				D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE1(3),
				D3DPOOL_DEFAULT, &vb, NULL) != S_OK )
			{
				return false;
			}

			key = std::pair<const Group *, int>(&group, DX9_TEXTURE_BUFFER_KEY);
			DX9Buffers[key] = vb;
			DX9TextureBuffer = vb;
			break;
		}
		//-----------------------------------------------------------------------------------------------
		return true;
	}

	bool SharedQuadRenderer::DX9DestroyBuffers(const Group& group)
	{

		DX9Release(group, DX9_VERTEX_BUFFER_KEY);
		DX9Release(group, DX9_COLOR_BUFFER_KEY);
		DX9Release(group, DX9_INDEX_BUFFER_KEY);
		DX9Release(group, DX9_TEXTURE_BUFFER_KEY);

		DX9VertexBuffer = DX9ColorBuffer = DX9TextureBuffer = NULL;
		DX9IndexBuffer = NULL;

		return true;
	}


	void SharedQuadRenderer::DX9ReleaseVertexDeclarations()
	{
		SAFE_RELEASE( pVertexDecl );
		SAFE_RELEASE( pVertexDecl2D );
		SAFE_RELEASE( pVertexDecl3D );
	}
	
#endif // SPARK_USE_BUFFERHANDLER

#endif // SPARK_USE_DX9QUAD

}}
