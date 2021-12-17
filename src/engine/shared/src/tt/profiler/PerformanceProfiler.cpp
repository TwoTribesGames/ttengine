#include <tt/platform/tt_printf.h>
#include <tt/profiler/PerformanceProfiler.h>


#ifdef PERFORMANCE_PROFILER_ENABLED

#include <string>

namespace tt {
namespace profiler {

MeasureCollection PerformanceProfiler::m_measures;
int  PerformanceProfiler::m_indent = 0;
int  PerformanceProfiler::m_frameIdx = 0;
bool PerformanceProfiler::m_enabled = false;

void PerformanceProfiler::outputProfileInfo()
{
	TT_PROFILER_PRINTF("  #Frames\t   #Calls\t Avg t/F\tPeak t/F\t Avg C/F\tPeak C/F\t Avg t/C\tPeak t/C\tName\n");
	for (MeasureCollection::const_iterator measure = m_measures.begin();
		 measure != m_measures.end(); ++measure)
	{
		u64 elapsedTotal = 0;       // total time elapsed
		u64 elapsedPerFrame = 0;    // total time elapsed this frame
		u64 elapsedFramePeak = 0;   // peak time elapsed per frame
		
		int callsPerFrame = 0;      // total calls this frame
		int callsFramePeak = 0;     // peak calls per frame
		u64 elapsedCallPeak = 0;    // peak time elapsed per call
		int prevFrame = -1;

		int totalFrames = 0;
		
		for (ProfileCollection::const_iterator profile = (*measure).second.begin();
			 profile != (*measure).second.end(); ++profile)
		{
			if (profile->frameIdx != prevFrame)
			{
				prevFrame = profile->frameIdx;
				
				elapsedPerFrame = 0;
				callsPerFrame = 0;
				totalFrames++;
			}
			
			callsPerFrame++;
			elapsedPerFrame += profile->elapsedTime;
			elapsedTotal += profile->elapsedTime;
			
			if (elapsedPerFrame > elapsedFramePeak)
			{
				elapsedFramePeak = elapsedPerFrame;
			}
			
			if (callsPerFrame > callsFramePeak)
			{
				callsFramePeak = callsPerFrame;
			}
			
			if (profile->elapsedTime > elapsedCallPeak)
			{
				elapsedCallPeak = profile->elapsedTime;
			}
		}
		
		ProfileCollection::size_type totalCalls = (*measure).second.size();
		
		u64 elapsedFrameAvg  = elapsedTotal / totalFrames;
		u32 callsFrameAvg  = totalCalls / totalFrames;
		u64 elapsedCallAvg = elapsedTotal / totalCalls;
		/*
		real stddev = 0.0f;
		if (size > 1)
		{
			// now compute standard deviation
			for (std::vector<u64>::const_iterator it2 = info.measures.begin(); 
				 it2 != info.measures.end(); ++it2)
			{
				real dev = (*it2) - average;
				stddev += dev * dev;
			}
			stddev = sqrt(stddev / (size - 1));
		}
		*/
		
		TT_PROFILER_PRINTF("%8d\t%8d\t%8llu\t%8llu\t%8d\t%8d\t%8llu\t%8llu\t%s\n", 
			totalFrames, totalCalls, elapsedFrameAvg, elapsedFramePeak, 
			callsFrameAvg, callsFramePeak, elapsedCallAvg, elapsedCallPeak, (*measure).first.c_str());
	}
	TT_PROFILER_PRINTF("\n");
}


void PerformanceProfiler::update()
{
	m_frameIdx++;
}



// namespace
}
}

#endif // #ifdef PERFORMANCE_PROFILER_ENABLED
