#include <tt/doc/str/StringDocument.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/str/common.h>


namespace tt {
namespace doc {
namespace str {

StringDocument::StringDocument(const std::string& p_filename, bool p_cachedContent)
:
m_names(),
m_file(),
m_filename(p_filename),
m_content(),
m_cachedContent(p_cachedContent)
{
	construct();
}


StringDocument::StringDocument(const StringDocument& p_rhs)
:
m_names(),
m_file(),
m_filename(p_rhs.m_filename),
m_content(),
m_cachedContent(p_rhs.m_cachedContent)
{
	construct();
}


std::string StringDocument::getString(s32 p_index)
{
	if (m_cachedContent == false)
	{
		// First, get name from list of names
		NameEntry ne = m_names.at(static_cast<Names::size_type>(p_index));
		
		// Create buffer to read name
		char* name = new char[ne.length];
		
		if (m_file == 0)
		{
			TT_PANIC("File '%s' has not been openened!",
			         m_filename.c_str());
			delete[] name;
			return "";
		}
		
		// Seek in file
		m_file->seek(static_cast<fs::pos_type>(ne.start), fs::SeekPos_Set);
		
		// And read from file to buffer
		m_file->read(name, static_cast<fs::size_type>(ne.length));
		
		// Now put name in a string
		std::string namestr(name);
		
		// Clean up buffer
		delete[] name;
		
		return namestr;
	}
	else
	{
		// First, get name from list of names
		NameEntry ne = m_names.at(static_cast<Names::size_type>(p_index));
		
		// get substring from content
		std::string name = m_content.substr(static_cast<std::string::size_type>(ne.start),
		                                    static_cast<std::string::size_type>(ne.length));
		
		return name;
	}
}


std::wstring StringDocument::getWString(s32 p_index)
{
	// make a widestring from the getString result
	return tt::str::widen(getString(p_index));
}


s32 StringDocument::getIndex(const std::string& p_string)
{
	for (s32 i = 0; i < static_cast<s32>(m_names.size()); ++i)
	{
		if (getString(i) == p_string)
		{
			return i;
		}
	}
	return -1;
}


s32 StringDocument::getIndex(const std::wstring& p_string)
{
	return getIndex(tt::str::narrow(p_string));
}


void StringDocument::construct()
{
	m_file = fs::open(m_filename, fs::OpenMode_Read);
	if (m_file == 0)
	{
		TT_PANIC("unable to open '%s'.\n", m_filename.c_str());
		return;
	}
	
	s32 filelength = static_cast<s32>(m_file->getLength());
	
	char* content   = new char[filelength+1];
	s32   bytesRead = static_cast<s32>(m_file->read(content, static_cast<fs::size_type>(filelength)));
	filelength      = bytesRead;
	if (bytesRead == 0)
	{
		TT_PANIC("Error reading from %s.", m_filename.c_str());
		delete [] content;
		return;
	}
	
	s32 index  = 0;
	s32 start  = 0;
	s32 length = 0;
	while (index < filelength)
	{
		// Values are comma or newline separated
		if (content[index] == '\n' ||
		    content[index] == '\r' ||
		    content[index] == ',')
		{
			if (length > 0)
			{
				NameEntry ne;
				ne.length = length;
				ne.start  = start;
				m_names.push_back(ne);
				length = 0;
			}
			
			// Next name probably starts at next character
			start = index + 1;
		}
		else
		{
			++length;
		}
		
		++index;
	}
	content[index] = '\0';
	
	if (length > 0)
	{
		NameEntry ne;
		ne.length = length;
		ne.start  = start;
		m_names.push_back(ne);
	}
	
	if (m_cachedContent)
	{
		m_content = content;
		m_file.reset();
	}
	
	delete[] content;
}

// Namespace end
}
}
}

