#ifndef INC_TT_SND_TYPES_H
#define INC_TT_SND_TYPES_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace snd {

// forward declaration
class Buffer;
class SoundSystem;
class Stream;
class StreamSource;
class Voice;


// shared pointers
typedef tt_ptr<Buffer>::shared BufferPtr;
typedef tt_ptr<SoundSystem>::shared SoundSystemPtr;
typedef tt_ptr<Stream>::shared StreamPtr;
typedef tt_ptr<Voice>::shared VoicePtr;


// Typedefs
typedef int identifier;
typedef int size_type;
typedef int pos_type;
typedef long long time_type;


// Available reverb presets (currently only on CAT)
enum ReverbPreset
{
	ReverbPreset_None, // Turn off reverb effect
	ReverbPreset_SmallRoom,
	ReverbPreset_LargeRoom,
	ReverbPreset_Hall,
	ReverbPreset_Cathedral,
	ReverbPreset_MetalCorridor,
	
	// Custom presets
	ReverbPreset_SmallCave,
	ReverbPreset_LargeCave,
	
	ReverbPreset_Count
};


// namespace end
}
}

#endif // INC_TT_SND_TYPES_H
