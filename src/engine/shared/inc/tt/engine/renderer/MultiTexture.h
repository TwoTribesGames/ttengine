#if !defined(INC_TT_ENGINE_RENDERER_MULTITEXTURE_H)
#define INC_TT_ENGINE_RENDERER_MULTITEXTURE_H

#include <tt/engine/renderer/TextureStageData.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Contains data for multitexturing (or single texture operations with e.g. a constant color)
           for the underlying platform, client delivers the texture stage data with TextureStageData objects
           The stages will be loaded from stage index 0 to maxStages or to a disabled stage,
           this is when the color OR alpha blend operation is disabled) 
    \note  Disabling alpha blend with a color blend results in undefined behaviour (atleast in Windows),
           but disabling color and use alpha blend only could be usefull in the future */
class MultiTexture
{
public:
	/*! \brief The number of channels (stage) that are avaliabe on the current hardware.
	    \Note Set by the renderer on construction, not valid before that time. */
	static inline u32 getChannelCount() { return ms_channelCount; }
	
	static inline bool isChannelSupported(u32 p_channel) { return p_channel < ms_channelCount; }
	
	static inline Texture* getActiveTexture(u32 p_channel)
	{
		if (isChannelSupported(p_channel) == false)
		{
			TT_PANIC("Getting active texture for invalid channel %d! (ms_channelCount: %d)\n",
			         p_channel, ms_channelCount);
			return 0;
		}
		return ms_activeTexture[p_channel];
	}
	
	static inline void resetAllActiveTextures()
	{
		for(u32 i = 0; i < getChannelCount(); ++i)
		{
			ms_activeTexture[i] = 0;
		}
	}
	
	static inline void resetActiveTexture(u32 p_index)
	{
		if (isChannelSupported(p_index) == false)
		{
			TT_PANIC("Channel index out of bounds when resetting active texture");
			return;
		}
		ms_activeTexture[p_index] = 0;
	}
	
	static inline void setActiveTexture(Texture* p_texture, u32 p_channel)
	{
		if (isChannelSupported(p_channel) == false)
		{
			TT_PANIC("Setting active texture for invalid channel %d! (ms_channelCount: %d)\n",
			         p_channel, ms_channelCount);
			return;
		}
		ms_activeTexture[p_channel] = p_texture;
	}
	
	static inline void setStageEnabled(u32 p_channel, bool p_enabled)
	{
		if (isChannelSupported(p_channel) == false)
		{
			TT_PANIC("Setting active stage for invalid channel %d! (ms_channelCount: %d)\n",
			         p_channel, ms_channelCount);
			return;
		}
		ms_activeStages[p_channel] = p_enabled;
	}
	
	static inline bool isStageEnabled(u32 p_channel)
	{
		if (isChannelSupported(p_channel) == false)
		{
			TT_PANIC("Getting active stage for invalid channel %d! (ms_channelCount: %d)\n",
			         p_channel, ms_channelCount);
			return false;
		}
		return ms_activeStages[p_channel];
	}
	
private:
	static void setChannelCount(u32 p_channelCount); // Called by Renderer on creation.
	static void resetChannelCount();                 // Called by Renderer on destruction.
	
	static u32 ms_dirtyHighestStage; //!< \brief Keeps track of the highest modified stage
	static u32 ms_channelCount; /*!< \brief The number of channels supported by the current hardware.
	                                 \Note Set by the renderer on construction, not valid before that time. */
	
	static Texture** ms_activeTexture; /*!< \brief pointer to the current active Texture for a specific channel.
	                                        \note Each time the devices SetTexture method is called, 
	                                              mirroring with ms_activeTexture[] needs an update. */
	static bool*     ms_activeStages;
	
	friend class Renderer; // For setting the ms_channelCount. (and creation and deletion of static members.)
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_MULTITEXTURE_H
