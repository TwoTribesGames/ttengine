#include <tt/code/ErrorStatus.h>
#include <tt/doc/ini/IniDocument.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


namespace tt {
namespace doc {
namespace ini {

//--------------------------------------------------------------------------------------------------
// Public member functions

IniDocument::IniDocument()
:
m_config()
{
}


IniDocument::IniDocument(const std::string& p_filename, fs::identifier p_type)
:
m_config()
{
	fs::FilePtr file = fs::open(p_filename, fs::OpenMode_Read, p_type);
	if (file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	parseDocument(file);
}


IniDocument::IniDocument(const fs::FilePtr& p_file)
:
m_config()
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return;
	}
	parseDocument(p_file);
}


bool IniDocument::hasSection(const std::string& p_section) const
{
	return m_config.find(p_section) != m_config.end();
}


bool IniDocument::hasKey(const std::string& p_section,
                         const std::string& p_key) const
{
	Config::const_iterator it = m_config.find(p_section);
	if (it == m_config.end())
	{
		return false;
	}
	return (*it).second.find(p_key) != (*it).second.end();
}


std::string IniDocument::getString(const std::string& p_section,
                                   const std::string& p_key) const
{
	Config::const_iterator section = m_config.find(p_section);
	if (section == m_config.end())
	{
		return std::string();
	}
	Section::const_iterator value = (*section).second.find(p_key);
	if (value == (*section).second.end())
	{
		return std::string();
	}
	return (*value).second;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

// convenience function
// FIXME: Verify whether this implementation is an exact functional duplicate of tt::str::explode. Remove this duplication if so
static str::Strings explode(const std::string& p_delims, const std::string& p_str)
{
	str::Strings ret;
	std::string::size_type pos = 0;
	std::string::size_type end = 0;
	while ((end = p_str.find_first_of(p_delims, pos)) != std::string::npos)
	{
		ret.push_back(std::string(p_str.substr(pos, end - pos)));
		pos = end + 1;
	}
	if (pos < p_str.length())
	{
		ret.push_back(p_str.substr(pos));
	}
	return ret;
}


void IniDocument::parseDocument(const fs::FilePtr& p_file)
{
	if (p_file->seekToBegin() == false)
	{
		TT_PANIC("Unable to seek file to begin.");
		return;
	}
	
	fs::size_type length = p_file->getLength();
	char* data = new char[length];
	if (data == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", length);
		return;
	}
	
	fs::size_type readLen = p_file->read(data, length);
	if (readLen == 0)
	{
		TT_PANIC("Unable to read ini file.");
		delete[] data;
		return;
	}
	
	std::string content(data, data + readLen);
	delete[] data;
	
	// remove whitespace
	tt::str::replace(content, "\r", "");
	tt::str::replace(content, "\t", "");
	tt::str::replace(content, " ", "");
	
	str::Strings str = explode("\n", content);
	
	// preprocess, parse comments
	for (str::Strings::iterator it = str.begin(); it != str.end(); ++it)
	{
		if ((*it).empty())
		{
			continue;
		}
		std::string::size_type pos = 0;
		while ((pos = (*it).find(';', pos)) != std::string::npos)
		{
			if ((pos > 0 && (*it).at(pos - 1) == '\\') == false)
			{
				// ; is not escaped
				if (pos == 0)
				{
					(*it).clear();
				}
				else
				{
					(*it) = (*it).substr(0, pos);
				}
				break;
			}
			++pos;
			if (pos == (*it).length())
			{
				break;
			}
		}
	}
	
	// process, parse sections and key/value pairs
	Section     section;
	std::string sectionName;
	int line = 1;
	
	for (str::Strings::iterator it = str.begin(); it != str.end(); ++it)
	{
		if ((*it).empty())
		{
			// skip;
			++line;
			continue;
		}
		if ((*it).at(0) == '[')
		{
			if ((*it).at((*it).length() - 1) == ']')
			{
				// section name found
				if (sectionName.empty() == false)
				{
					// store previous section
					m_config[sectionName] = section;
					section.clear();
					sectionName.clear();
				}
				sectionName = (*it).substr(1, (*it).length() - 2);
				if (m_config.find(sectionName) != m_config.end())
				{
					TT_PANIC("Section '%s' appears twice in ini file(line %d).",
					         sectionName.c_str(), line);
					m_config.clear();
					return;
				}
			}
			else
			{
				TT_PANIC("Illegal character '[' in ini file at line %d (%s).",
				         line, (*it).c_str());
				m_config.clear();
				return;
			}
		}
		else if (sectionName.empty())
		{
			// check if the line only contains whitespace
			if ((*it).find_first_not_of("\t ") != std::string::npos)
			{
				TT_PANIC("Data found before first section at line %d (%s).",
				         line, (*it).c_str());
				m_config.clear();
				return;
			}
		}
		else
		{
			std::string::size_type pos = (*it).find('=');
			if (pos == std::string::npos)
			{
				TT_PANIC("Missing '=' at line %d (%s).", line, (*it).c_str());
				m_config.clear();
				return;
			}
			std::string key = (*it).substr(0, pos);
			++pos;
			std::string value = (*it).substr(pos, (*it).length() - pos);
			
			if (section.find(key) != section.end())
			{
				// ignore
				++line;
				continue;
			}
			
			// preprocess, parse escaped characters
			pos = 0;
			while ((pos = value.find('\\', pos)) != std::string::npos)
			{
				if ((pos + 1) < value.length())
				{
					switch (value.at(pos + 1))
					{
					case ';':
						value = value.substr(0, pos) + value.substr(pos + 1);
						break;
					}
				}
				++pos;
			}
			
			section[key] = value;
		}
		++line;
	}
	if (sectionName.empty() == false)
	{
		m_config[sectionName] = section;
	}
}

// Namespace end
}
}
}
