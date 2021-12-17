#if !defined(INC_TOKI_GAME_PATHFINDING_TILERASTERIZATIONCONTEXT_H)
#define INC_TOKI_GAME_PATHFINDING_TILERASTERIZATIONCONTEXT_H

#include <recastnavigation/Detour/DetourNavMesh.h>
#include <recastnavigation/Detour/DetourNavMeshQuery.h>
#include <recastnavigation/DetourTileCache/DetourTileCacheBuilder.h>
#include <recastnavigation/Recast/Recast.h>

#include <tt/compression/fastlz.h>

#include <toki/level/fwd.h>
#include <toki/serialization/fwd.h>
#include <toki/game/pathfinding/fwd.h>


namespace toki {
namespace game {
namespace pathfinding {

static const int MAX_LAYERS = 2; //32;

struct TileCacheData
{
	unsigned char* data;
	int dataSize;
};


struct FastLZCompressor : public dtTileCacheCompressor
{
	virtual int maxCompressedSize(const int bufferSize)
	{
		return (int)(bufferSize * 1.05f);
	}
	
	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
	                          unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
	{
		*compressedSize = fastlz_compress((const void *)buffer, bufferSize, compressed);
		return DT_SUCCESS;
	}
	
	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
	                            unsigned char* buffer, const int maxBufferSize, int* bufferSize)
	{
		*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
		return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
	}
};


// This class is based on RasterizationContext en rasterizeTileLayers from recast's Sample_TempObstacles.cpp

class TileRasterizationContext
{
public:
	TileRasterizationContext();
	~TileRasterizationContext();
	
	void cleanup();
	int rasterizeTileLayers(rcContext* ctx,
	                        const level::AttributeLayerPtr& p_layer,
	                        const int tx, const int ty,
	                        const rcConfig& cfg,
	                        TileCacheData* tiles,
	                        const int maxTiles);
	void render() const;
	
private:
	// FIXME: Move this rasterizeBitmap to recast lib so the other *Span functions can be removed.
	//        They are duplicates of code from recast lib.
	static void rasterizeBitmap(rcHeightfield& solid, const level::AttributeLayerPtr& p_layer);
	static void addSpan(rcHeightfield& hf, const int x, const int y,
	                    const unsigned short smin, const unsigned short smax,
	                    const unsigned char area, const int flagMergeThr);
	static rcSpan* allocSpan(rcHeightfield& hf);
	static void    freeSpan(rcHeightfield& hf, rcSpan* ptr);
	
	rcHeightfield*        m_solid;
	rcCompactHeightfield* m_chf;
	//rcContourSet*         m_cset;
	//rcPolyMesh*           m_pmesh;
	//rcConfig              m_cfg;
	//rcPolyMeshDetail*     m_dmesh;
	
	//unsigned char*         m_triareas;
	rcHeightfieldLayerSet* m_lset;
	TileCacheData          m_tiles[MAX_LAYERS];
	int                    m_ntiles;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_PATHFINDING_TILERASTERIZATIONCONTEXT_H)
