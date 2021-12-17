#ifndef INC_TT_AUDIO_CHIBI_XMUTIL_H
#define INC_TT_AUDIO_CHIBI_XMUTIL_H

#include <string>

#include <tt/audio/chibi/types.h>


namespace tt {
namespace audio {
namespace chibi {

// forward declaration
class XMMemoryManager;

class XMUtil
{
public:
	enum
	{
		XM_NoteError = 255
	};
	
	static u8 getNote(const std::string& p_str);
	static u8 getNoteOffset(const std::string& p_str);
	
	static void setMemoryManager(XMMemoryManager* p_mem);
	static XMMemoryManager* getMemoryManager();
	
private:
	static XMMemoryManager* ms_memoryManager;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMUTIL_H
