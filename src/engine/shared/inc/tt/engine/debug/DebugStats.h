#if !defined(INC_TT_ENGINE_DEBUG_DEBUGSTATS_H)
#define INC_TT_ENGINE_DEBUG_DEBUGSTATS_H


#include <deque>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace debug {


struct FrameStatistics
{
	s32 objectsRendered;
	s32 objectsCulled;
	s32 polygonsRendered;
	s32 quadsRendered;
};


class DebugStats
{
public:
	inline static void addToObjectsCulled   (s32 p_culled)  {ms_frameStats.objectsCulled    += p_culled; }
	inline static void addToObjectsRendered (s32 p_objects) {ms_frameStats.objectsRendered  += p_objects;}
	inline static void addToPolygonsRendered(s32 p_polys)   {ms_frameStats.polygonsRendered += p_polys;  }
	inline static void addToQuadsRendered   (s32 p_quads)   {ms_frameStats.quadsRendered    += p_quads;  }
	
	static void clearFrameStats();
	
	inline static const FrameStatistics& getStats() { return ms_frameStats; }
	
	static void endFrame();
	
	static void beginUpdate();
	static void endUpdate();
	
	static void beginRender();
	static void endRender();
	
	static real getAverageFrameTime (s32 p_frameCount);
	static real getMaxFrameTime     (s32 p_frameCount);
	static real getAverageFPS       (s32 p_frameCount);
	static real getAverageUpdateTime(s32 p_frameCount);
	static real getAverageRenderTime(s32 p_frameCount);
	static u64  getMaxUpdateTime    (s32 p_frameCount);
	static u64  getMaxRenderTime    (s32 p_frameCount);
	
private:
	typedef std::deque<u64> Times;
	
	DebugStats()  { }
	~DebugStats() { }
	
	static real getAverageTimeInUs(s32 p_frameCount, const Times& p_times);
	static u64  getMaxTimeInUs(s32 p_frameCount, const Times& p_times);
	
	static FrameStatistics ms_frameStats;
	static s32             ms_frameCount;
	
	static u64 ms_beginUpdate;
	static u64 ms_beginRender;
	static Times ms_frameTimes;
	static Times ms_updateTimes;
	static Times ms_renderTimes;
};


}
}
}

#endif // INC_TT_ENGINE_DEBUG_DEBUGSTATS_H
