#include <tt/audio/chibi/XMUtil.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace chibi {

XMMemoryManager* XMUtil::ms_memoryManager = 0;


u8 XMUtil::getNote(const std::string& p_str)
{
	if ( p_str.length() <= 1 )
	{
		TT_WARN("'%s' is not a valid note", p_str.c_str());
		return XM_NoteError;
	}
	
	std::string note = p_str.substr(0, p_str.length() - 1);
	u8 offset = getNoteOffset(note);
	if ( offset == XM_NoteError )
	{
		TT_WARN("'%s' is not a valid note", p_str.c_str());
		return XM_NoteError;
	}
	
	switch ( p_str.at(p_str.length() - 1) )
	{
	case '0': return u8(offset + 1);
	case '1': return u8(offset + 13);
	case '2': return u8(offset + 25);
	case '3': return u8(offset + 37);
	case '4': return u8(offset + 49);
	case '5': return u8(offset + 61);
	case '6': return u8(offset + 73);
	case '7': return u8(offset + 85);
	}
	
	TT_WARN("'%s' is not a valid note", p_str.c_str());
	return XM_NoteError;
}


u8 XMUtil::getNoteOffset(const std::string& p_str)
{
	if ( p_str.length() == 1 )
	{
		switch ( p_str.at(0) )
		{
		case 'C': return 0;
		case 'D': return 2;
		case 'E': return 4;
		case 'F': return 5;
		case 'G': return 7;
		case 'A': return 9;
		case 'B': return 11;
		default: TT_WARN("'%s' is not a valid note", p_str.c_str()); return XM_NoteError;
		}
	}
	else if ( p_str.length() == 2 )
	{
		if ( p_str.at(1) != '#' )
		{
			TT_WARN("'%s' is not a valid note", p_str.c_str());
			return XM_NoteError;
		}
		switch ( p_str.at(0) )
		{
		case 'C': return 1;
		case 'D': return 3;
		case 'F': return 6;
		case 'G': return 8;
		case 'A': return 10;
		default: TT_WARN("'%s' is not a valid note (no # allowed)", p_str.c_str());
		return XM_NoteError;
		}
	}
	TT_WARN("'%s' is not a valid note", p_str.c_str());
	return XM_NoteError;
}


void XMUtil::setMemoryManager(XMMemoryManager* p_mem)
{
	ms_memoryManager = p_mem;
}


XMMemoryManager* XMUtil::getMemoryManager()
{
	return ms_memoryManager;
}


} // namespace end
}
}
