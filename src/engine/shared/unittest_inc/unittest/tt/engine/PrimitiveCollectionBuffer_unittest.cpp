#include <unittestpp/unittestpp.h>

#include <tt/engine/renderer/PrimitiveCollectionBuffer.h>
#include <tt/engine/renderer/BufferVtx.h>
#include <tt/engine/renderer/BufferVtx_util.h>


SUITE(tt_engine_primitiveCollectionBuffer)
{

struct SetupSingleQuadPrimitiveBuffer
{
	SetupSingleQuadPrimitiveBuffer()
	{}
	
	enum Constants
	{
		VerticesPerPrimitive          = 4,
		InitialReservedPrimitiveCount = 1,
		UVSetCount                    = 1
	};
	
private:
	SetupSingleQuadPrimitiveBuffer(const SetupSingleQuadPrimitiveBuffer& p_rhs);                  // Not implemented
	const SetupSingleQuadPrimitiveBuffer& operator=(const SetupSingleQuadPrimitiveBuffer& p_rhs); // Not implemented.
};


TEST(BufferVtx)
{
	using tt::engine::renderer::BufferVtxUV;
	typedef BufferVtxUV<1> VertexTypeOne;
	typedef BufferVtxUV<2> VertexTypeTwo;
	
	// ---------------------------------------------------------------------------------------------
	// Default construction.
	
	const VertexTypeOne defaultOne;
	CHECK_EQUAL(tt::math::Vector3(),               defaultOne.getPosition());
	CHECK_EQUAL(tt::engine::renderer::ColorRGBA(), defaultOne.getColor());
	CHECK_EQUAL(tt::math::Vector2(),               defaultOne.getTexCoord(0));
	
	const VertexTypeTwo defaultTwo;
	CHECK_EQUAL(tt::math::Vector3(),               defaultTwo.getPosition());
	CHECK_EQUAL(tt::engine::renderer::ColorRGBA(), defaultTwo.getColor());
	CHECK_EQUAL(tt::math::Vector2(),               defaultTwo.getTexCoord(0));
	CHECK_EQUAL(tt::math::Vector2(),               defaultTwo.getTexCoord(1));
	
	// ---------------------------------------------------------------------------------------------
	// Explicit construction.
	
	tt::math::Vector3               ctorPos(0.5f, 4.2f, -566.24f);
	tt::engine::renderer::ColorRGBA ctorCol(1, 35, 250, 230);
	tt::math::Vector2               ctorTexCoord[2] =
		{ tt::math::Vector2(0.25f, 0.55f), tt::math::Vector2() };
	
	const VertexTypeOne ctorOne(ctorPos, ctorCol, ctorTexCoord[0]);
	
	CHECK_EQUAL(ctorPos,         ctorOne.getPosition());
	CHECK_EQUAL(ctorCol,         ctorOne.getColor());
	CHECK_EQUAL(ctorTexCoord[0], ctorOne.getTexCoord(0));
	
	const VertexTypeTwo ctorTwo(ctorPos, ctorCol, ctorTexCoord[0]);
	
	CHECK_EQUAL(ctorPos,             ctorTwo.getPosition());
	CHECK_EQUAL(ctorCol,             ctorTwo.getColor());
	CHECK_EQUAL(ctorTexCoord[0],     ctorTwo.getTexCoord(0));
	CHECK_EQUAL(tt::math::Vector2(), ctorTwo.getTexCoord(1)); // Should be default value.
	
	// ---------------------------------------------------------------------------------------------
	// Setters
	
	tt::math::Vector3               testPos(0.2f, -3.5f, 36.32f);
	tt::engine::renderer::ColorRGBA testCol(42, 189, 9, 160);
	tt::math::Vector2               testTexCoord[2] =
		{ tt::math::Vector2(0.333333f, 0.42f), tt::math::Vector2(0.75f, 3.45f) };
	
	VertexTypeOne testOne;
	testOne.setPosition(testPos);
	testOne.setColor(testCol);
	testOne.setTexCoord(testTexCoord[0], 0);
	
	CHECK_EQUAL(testPos,         testOne.getPosition());
	CHECK_EQUAL(testCol,         testOne.getColor());
	CHECK_EQUAL(testTexCoord[0], testOne.getTexCoord(0));
	
	VertexTypeTwo testTwo;
	testTwo.setPosition(testPos);
	testTwo.setColor(testCol);
	testTwo.setTexCoord(testTexCoord[0], 0);
	testTwo.setTexCoord(testTexCoord[1], 1);
	
	CHECK_EQUAL(testPos,             testTwo.getPosition());
	CHECK_EQUAL(testCol,             testTwo.getColor());
	CHECK_EQUAL(testTexCoord[0],     testTwo.getTexCoord(0));
	CHECK_EQUAL(testTexCoord[1], testTwo.getTexCoord(1)); // Should be default value.
	
	// ---------------------------------------------------------------------------------------------
	// Copy construction.
	
	VertexTypeOne copyOne(testOne);
	CHECK_EQUAL(testOne.getPosition(),  copyOne.getPosition());
	CHECK_EQUAL(testOne.getColor(),     copyOne.getColor());
	CHECK_EQUAL(testOne.getTexCoord(0), copyOne.getTexCoord(0));
	CHECK_EQUAL(testOne, copyOne);
	
	VertexTypeTwo copyTwo(testTwo);
	CHECK_EQUAL(testTwo.getPosition(),  copyTwo.getPosition());
	CHECK_EQUAL(testTwo.getColor(),     copyTwo.getColor());
	CHECK_EQUAL(testTwo.getTexCoord(0), copyTwo.getTexCoord(0));
	CHECK_EQUAL(testTwo.getTexCoord(1), copyTwo.getTexCoord(1));
	CHECK_EQUAL(testTwo, copyTwo);
	
	// ---------------------------------------------------------------------------------------------
	// Assigment operator
	
	VertexTypeOne assignOne;
	assignOne = testOne;
	CHECK_EQUAL(testOne.getPosition(),  assignOne.getPosition());
	CHECK_EQUAL(testOne.getColor(),     assignOne.getColor());
	CHECK_EQUAL(testOne.getTexCoord(0), assignOne.getTexCoord(0));
	CHECK_EQUAL(testOne, assignOne);
	
	VertexTypeTwo assignTwo;
	assignTwo = testTwo;
	CHECK_EQUAL(testTwo.getPosition(),  assignTwo.getPosition());
	CHECK_EQUAL(testTwo.getColor(),     assignTwo.getColor());
	CHECK_EQUAL(testTwo.getTexCoord(0), assignTwo.getTexCoord(0));
	CHECK_EQUAL(testTwo.getTexCoord(1), assignTwo.getTexCoord(1));
	CHECK_EQUAL(testTwo, assignTwo);
}


TEST_FIXTURE( SetupSingleQuadPrimitiveBuffer, PrimitiveBuffer_Quad_Creation )
{
	using tt::engine::renderer::PrimitiveCollectionBuffer;
	PrimitiveCollectionBuffer buffer(VerticesPerPrimitive, InitialReservedPrimitiveCount, UVSetCount);
	
	using tt::engine::renderer::BufferVtxUV;
	typedef BufferVtxUV<UVSetCount> VertexType;
	const s32 checkArraySize = VerticesPerPrimitive;
	VertexType checkArray[checkArraySize] = { VertexType() };
	
	// Buffer is created empty, but has a reserved size.
	CHECK(buffer.isEmpty());
	CHECK_EQUAL(0, buffer.getRawVertexDataSize() );
	CHECK_EQUAL(0, buffer.getPrimitiveCount()    );
	CHECK_EQUAL(0, buffer.getTotalVerticesCount());
	
	// Check reserved buffer sizes.
	CHECK_EQUAL(sizeof(checkArray), static_cast<size_t>(buffer.getBufferSize())         );
	CHECK_EQUAL(UVSetCount,                             buffer.getUVSetCount()          );
	CHECK_EQUAL(sizeof(VertexType), static_cast<size_t>(buffer.getVertexSize())         );
	CHECK_EQUAL(VerticesPerPrimitive,                   buffer.getVerticesPerPrimitive());
	
	
	// Fill buffer with empty vertices
	buffer.resize<UVSetCount>(VerticesPerPrimitive);
	CHECK(buffer.isEmpty() == false);
	CHECK_EQUAL(sizeof(checkArray),   static_cast<size_t>(buffer.getRawVertexDataSize()) );
	CHECK_EQUAL(1,                    buffer.getPrimitiveCount()    );
	CHECK_EQUAL(VerticesPerPrimitive, buffer.getTotalVerticesCount());
	
	CHECK_ARRAY_EQUAL(reinterpret_cast<u8*>(checkArray),
	                  reinterpret_cast<u8*>(buffer.getRawVertexData()),
	                  sizeof(checkArray));
	CHECK_EQUAL(0, std::memcmp(checkArray, buffer.getRawVertexData(), sizeof(checkArray)));
	
	CHECK_EQUAL(VertexType(), buffer.getVtx<UVSetCount>(0));
	CHECK_EQUAL(VertexType(), buffer.getVtx<UVSetCount>(1));
	CHECK_EQUAL(VertexType(), buffer.getVtx<UVSetCount>(2));
	CHECK_EQUAL(VertexType(), buffer.getVtx<UVSetCount>(3));
	
	for (u8 i = 0; i < VerticesPerPrimitive; ++i)
	{
		CHECK_EQUAL(VertexType(),  checkArray[i]);
		CHECK_EQUAL(checkArray[i], buffer.getVtx<UVSetCount>(i));
		CHECK_EQUAL(VertexType(),  buffer.getVtx<UVSetCount>(i));
	}
	
	// Modify vertex data.
	for (u8 i = 0; i < VerticesPerPrimitive; ++i)
	{
		const VertexType vtx(tt::math::Vector3(i * 0.5f, i * 2.3f, i * 3.9f),
		                     tt::engine::renderer::ColorRGBA(i, i * 2, i * 5, i * 10),
		                     tt::math::Vector2(i * 0.1f, i * 0.35f));
		checkArray[i]       = vtx;
		buffer.modifyVtx<UVSetCount>(i) = vtx;
		
		CHECK_EQUAL(vtx, checkArray[i]);
		CHECK_EQUAL(vtx, buffer.getVtx<UVSetCount>(i));
	}
	
	CHECK_ARRAY_EQUAL(reinterpret_cast<u8*>(checkArray),
	                  reinterpret_cast<u8*>(buffer.getRawVertexData()),
	                  sizeof(checkArray));
	CHECK_EQUAL(0, std::memcmp(checkArray, buffer.getRawVertexData(), sizeof(checkArray)));
	
	const s32 PrimitiveCount = 2;
	const s32 VerticesCount = VerticesPerPrimitive * PrimitiveCount;
	
	// Create new vertexArray and add data old array.
	VertexType bigCheckArray[VerticesCount] = { VertexType() };
	for (s32 i = 0; i < checkArraySize; ++i)
	{
		bigCheckArray[i] = checkArray[i];
	}
	
	// Increase reserved size.
	buffer.reservePrimitiveCount(PrimitiveCount, true);
	
	// Buffer should still have same data and size. (data size != reserved size.)
	CHECK(buffer.isEmpty() == false);
	CHECK_EQUAL(sizeof(checkArray),   static_cast<size_t>(buffer.getRawVertexDataSize()) );
	CHECK_EQUAL(1,                    buffer.getPrimitiveCount()    );
	CHECK_EQUAL(VerticesPerPrimitive, buffer.getTotalVerticesCount());
	CHECK_ARRAY_EQUAL(reinterpret_cast<u8*>(checkArray),
	                  reinterpret_cast<u8*>(buffer.getRawVertexData()),
	                  sizeof(checkArray));
	CHECK_EQUAL(0, std::memcmp(checkArray, buffer.getRawVertexData(), sizeof(checkArray)));
	
	// Check reserved buffer sizes.
	CHECK_EQUAL(sizeof(bigCheckArray), static_cast<size_t>(buffer.getBufferSize())         );
	CHECK_EQUAL(UVSetCount,                                buffer.getUVSetCount()          );
	CHECK_EQUAL(sizeof(VertexType),    static_cast<size_t>(buffer.getVertexSize())         );
	CHECK_EQUAL(VerticesPerPrimitive,                      buffer.getVerticesPerPrimitive());
	
	buffer.resize(VerticesCount, VertexType());
	CHECK_ARRAY_EQUAL(reinterpret_cast<u8*>(bigCheckArray),
	                  reinterpret_cast<u8*>(buffer.getRawVertexData()),
	                  sizeof(bigCheckArray));
	CHECK_EQUAL(0, std::memcmp(bigCheckArray, buffer.getRawVertexData(), sizeof(bigCheckArray)));
}

TEST( EmptyPrimitiveCollectionBuffer )
{
	using tt::engine::renderer::PrimitiveCollectionBuffer;
	const s32 VerticesPerPrimitive = 1;
	enum { UVSetCount = 2};
	PrimitiveCollectionBuffer buffer(VerticesPerPrimitive, 0, UVSetCount);
	
	CHECK(buffer.isEmpty());
	CHECK_EQUAL(0, buffer.getBufferSize());
	CHECK_EQUAL(0, buffer.getRawVertexDataSize());
	CHECK_EQUAL(0, buffer.getPrimitiveCount());
	CHECK_EQUAL(0, buffer.getTotalVerticesCount());
	
	// -------------------------------------------------
	const s32 PrimitiveCount = 3;
	buffer.reservePrimitiveCount(PrimitiveCount, true);
	CHECK_EQUAL(0, buffer.getRawVertexDataSize());
	CHECK_EQUAL(0, buffer.getPrimitiveCount());
	
	typedef tt::engine::renderer::BufferVtxUV<UVSetCount> VtxType;
	buffer.resize(PrimitiveCount, VtxType());
	CHECK_EQUAL(PrimitiveCount, buffer.getPrimitiveCount());
	CHECK_EQUAL(PrimitiveCount * VerticesPerPrimitive, buffer.getTotalVerticesCount());
	CHECK_EQUAL(sizeof(VtxType), static_cast<size_t>(buffer.getVertexSize()));
	CHECK_EQUAL(PrimitiveCount * VerticesPerPrimitive * sizeof(VtxType),
	            static_cast<size_t>(buffer.getRawVertexDataSize()));
}


// End SUITE
}
