#include <tt/engine/debug/DebugStats.h>
#include <tt/mem/util.h>
#include <tt/system/Time.h>
#include <algorithm>


namespace tt {
namespace engine {
namespace debug {

FrameStatistics DebugStats::ms_frameStats;
s32             DebugStats::ms_frameCount = 0;
u64 DebugStats::ms_beginUpdate = 0;
u64 DebugStats::ms_beginRender = 0;
DebugStats::Times DebugStats::ms_frameTimes;
DebugStats::Times DebugStats::ms_updateTimes;
DebugStats::Times DebugStats::ms_renderTimes;


static const s32 g_maxFrameTimes = 120;

//--------------------------------------------------------------------------------------------------
// Public members

void DebugStats::clearFrameStats()
{
	mem::zero8(&ms_frameStats, sizeof(FrameStatistics));
}


void DebugStats::endFrame()
{
	static u64 prevTime = 0;
	u64 now = system::Time::getInstance()->getMicroSeconds();
	
	if (prevTime != 0)
	{
		ms_frameTimes.push_back(now - prevTime);
		
		if (static_cast<s32>(ms_frameTimes.size()) > g_maxFrameTimes)
		{
			ms_frameTimes.pop_front();
		}
	}

	prevTime = now;
	++ms_frameCount;
}


void DebugStats::beginUpdate()
{
	ms_beginUpdate = system::Time::getInstance()->getMicroSeconds();
}


void DebugStats::endUpdate()
{
	if (ms_beginUpdate != 0)
	{
		const u64 now = system::Time::getInstance()->getMicroSeconds();
		ms_updateTimes.push_back(now - ms_beginUpdate);
		if (static_cast<s32>(ms_updateTimes.size()) > g_maxFrameTimes)
		{
			ms_updateTimes.pop_front();
		}
		ms_beginUpdate = 0;
	}
}


void DebugStats::beginRender()
{
	ms_beginRender = system::Time::getInstance()->getMicroSeconds();
}


void DebugStats::endRender()
{
	if (ms_beginRender != 0)
	{
		const u64 now = system::Time::getInstance()->getMicroSeconds();
		ms_renderTimes.push_back(now - ms_beginRender);
		if (static_cast<s32>(ms_renderTimes.size()) > g_maxFrameTimes)
		{
			ms_renderTimes.pop_front();
		}
		ms_beginRender = 0;
	}
}


real DebugStats::getAverageFrameTime(s32 p_frameCount)
{
	return getAverageTimeInUs(p_frameCount, ms_frameTimes);
}


real DebugStats::getMaxFrameTime(s32 p_frameCount)
{
	return getMaxTimeInUs(p_frameCount, ms_frameTimes) / 1000.0f;
}


real DebugStats::getAverageFPS(s32 p_frameCount)
{
	return 1000000.0f / getAverageTimeInUs(p_frameCount, ms_frameTimes);
}


real DebugStats::getAverageUpdateTime(s32 p_frameCount)
{
	return getAverageTimeInUs(p_frameCount, ms_updateTimes);
}


real DebugStats::getAverageRenderTime(s32 p_frameCount)
{
	return getAverageTimeInUs(p_frameCount, ms_renderTimes);
}


u64 DebugStats::getMaxUpdateTime(s32 p_frameCount)
{
	return getMaxTimeInUs(p_frameCount, ms_updateTimes);
}


u64 DebugStats::getMaxRenderTime(s32 p_frameCount)
{
	return getMaxTimeInUs(p_frameCount, ms_renderTimes);
}

//--------------------------------------------------------------------------------------------------
// Private members

real DebugStats::getAverageTimeInUs(s32 p_frameCount, const Times& p_times)
{
	const s32 framesAvailable = static_cast<s32>(p_times.size());
	const s32 frameCount = std::min(p_frameCount, framesAvailable);
	
	if (frameCount <= 0)
	{
		return 0;
	}
	
	u64 totalTime(0);
	for (s32 i = framesAvailable - frameCount; i < framesAvailable; ++i)
	{
		totalTime += p_times[i];
	}
	
	return static_cast<real>(totalTime / frameCount);
}


u64 DebugStats::getMaxTimeInUs(s32 p_frameCount, const Times& p_times)
{
	const s32 framesAvailable = static_cast<s32>(p_times.size());
	const s32 frameCount = std::min(p_frameCount, framesAvailable);
	
	u64 maxTime(0);
	for (s32 i = framesAvailable - frameCount; i < framesAvailable; ++i)
	{
		maxTime = std::max(maxTime, p_times[i]);
	}
	
	return maxTime;
}

}
}
}
