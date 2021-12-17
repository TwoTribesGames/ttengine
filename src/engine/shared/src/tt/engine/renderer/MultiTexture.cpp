#include <tt/engine/renderer/MultiTexture.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {


u32       MultiTexture::ms_dirtyHighestStage = 0;
u32       MultiTexture::ms_channelCount      = 0;
Texture** MultiTexture::ms_activeTexture     = 0;
bool*     MultiTexture::ms_activeStages      = 0;


//--------------------------------------------------------------------------------------------------
// Private Functions


void MultiTexture::setChannelCount(u32 p_channelCount)
{
	TT_ASSERTMSG(ms_channelCount == 0, "MultiTexture already had channelCount set.");
	
	ms_channelCount = p_channelCount;
	
	ms_activeTexture = new Texture*[ms_channelCount];
	ms_activeStages  = new bool    [ms_channelCount];
	for (u32 i = 0; i < ms_channelCount; ++i)
	{
		ms_activeTexture[i] = 0;
		ms_activeStages [i] = 0;
	}
}


void MultiTexture::resetChannelCount()
{
	ms_channelCount = 0;
	delete[] ms_activeTexture;
	delete[] ms_activeStages;
}



// Namespace end
}
}
}
