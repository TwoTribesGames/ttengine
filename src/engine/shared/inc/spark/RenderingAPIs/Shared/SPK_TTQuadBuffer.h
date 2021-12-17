//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
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


#ifndef H_SPK_SHARED_TTQUADBUFFER
#define H_SPK_SHARED_TTQUADBUFFER

#include <tt/engine/renderer/QuadBuffer.h>

#include <spark/Core/SPK_Buffer.h>

namespace SPK {
class Group;
namespace Shared {
	
	class TTQuadBufferCreator;
	
	class TTQuadBuffer : public Buffer
	{
	friend class TTQuadBufferCreator;

	public:
		inline const tt::engine::renderer::QuadBufferPtr getQuadBuffer() const { TT_NULL_ASSERT(m_buffer); return m_buffer; }
		inline s32 getQuadIndex() const { return m_quadIndex; }
		
		inline void clear() { m_quadIndex = 0; m_buffer->clear(); }
		inline void addCollection(const tt::engine::renderer::BatchQuadCollection& p_collection)
		{
			m_buffer->setCollection(p_collection, m_quadIndex);
			++m_quadIndex;
		}
	private:
		s32                                       m_quadIndex;
		const tt::engine::renderer::QuadBufferPtr m_buffer;
		
		TTQuadBuffer(size_t nbParticles);
		TTQuadBuffer(const TTQuadBuffer& buffer); // no copy 
		const TTQuadBuffer& operator=(const TTQuadBuffer&); // no assigment.
		
		virtual void swap(size_t index0,size_t index1);
	};
	
	class TTQuadBufferCreator : public BufferCreator
	{
	public :
		inline TTQuadBufferCreator()
		:
		BufferCreator()
		{
		}
		
	private :
		
		virtual inline TTQuadBuffer* createBuffer(size_t nbParticles,const Group& /*group*/) const
		{
			return new TTQuadBuffer(nbParticles);
		}
	};
	
	inline TTQuadBuffer::TTQuadBuffer(size_t p_nbParticles)
	:
	Buffer(),
	m_buffer(new tt::engine::renderer::QuadBuffer(static_cast<s32>(p_nbParticles), tt::engine::renderer::TexturePtr(), tt::engine::renderer::BatchFlagQuad_UseVertexColor))
	{
	}

#if 0 // no copy ctor.
	inline TTQuadBuffer::TTQuadBuffer(const TTQuadBuffer& buffer) :
		Buffer(buffer)
	{
		//data = new T[dataSize];
		//memcpy(data,buffer.data,dataSize * sizeof(T));
	}
#endif

	inline void TTQuadBuffer::swap(size_t /*index0*/, size_t /*index1*/)
	{
		TT_PANIC("Swap is not supported by TTQuadBuffer!");
		//
		//T* address0 = data + index0 * particleSize;
		//T* address1 = data + index1 * particleSize;
		//for (size_t i = 0; i < particleSize; ++i)
		//	std::swap(address0[i],address1[i]);
	}

// end namespace
} }

#endif // H_SPK_SHARED_TTQUADBUFFER
