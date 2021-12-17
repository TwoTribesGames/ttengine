#include <tt/doc/xap/XapReader.h>
#include <tt/fs/File.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace doc {
namespace xap {

XapReader::XapReader()
:
m_file(),
m_buffer(0),
m_index(0),
m_size(0),
m_line(0)
{
}


xml::XmlNode* XapReader::readFile(const fs::FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return 0;
	}
	
	m_file = p_file;
	
	m_buffer = new s8[1024];
	m_line = 0;
	
	tt::xml::XmlNode* node = createXmlNode("xap");
	
	delete[] m_buffer;
	m_buffer = 0;
	m_file.reset();
	m_index = 0;
	m_size = 0;
	
	return node;
}


xml::XmlNode* XapReader::readFile(const std::string& p_filename, fs::identifier p_type)
{
	fs::FilePtr file = fs::open(p_filename, fs::OpenMode_Read, p_type);
	if (file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return 0;
	}
	
	return readFile(file);
}


// Private Functions

xml::XmlNode* XapReader::createXmlNode(const std::string& p_name, bool p_first)
{
	xml::XmlNode* node = new xml::XmlNode(p_name);
	
	// now parse all lines untill we're done
	bool done = false;
	while (done == false)
	{
		std::string str = getLine();
		if (str.empty())
		{
			if (p_first == false)
			{
				TT_PANIC("unexpected end of file\n");
				
				delete node;
				return 0;
			}
			else
			{
				done = true;
				break;
			}
		}
		
		// see if it's the end of this node (last character is '}')
		if (str[str.size() - 1] == '}')
		{
			done = true;
			break;
		}
		else if (str[str.size() - 1] == ';')
		{
			// attribute
			std::string::size_type pos = str.find('=');
			
			// find first character of attribute
			std::string::size_type attrbeg = 0;
			
			// last character of attribute
			std::string::size_type attrend = pos - 1;
			
			// first character of value
			std::string::size_type valbeg = pos + 2;
			
			// last character of value
			std::string::size_type valend = str.size() - 1;
			
			std::string attribute = str.substr(attrbeg, attrend - attrbeg);
			std::string value = str.substr(valbeg, valend - valbeg);
			
			node->setAttribute(attribute, value);
		}
		else
		{
			std::string next = getLine();
			if (next != "{")
			{
				TT_PANIC("Expected { after '%s' (found '%s') (line %d)\n", str.c_str(), next.c_str(), m_line);
				
				delete node;
				return 0;
			}
			
			xml::XmlNode* subnode = createXmlNode(str, false);
			
			if (subnode == 0)
			{
				TT_PANIC("Error creating subnode '%s'\n", str.c_str());
				
				delete node;
				return 0;
			}
			
			node->addChild(subnode);
		}
	}
	return node;
}


std::string XapReader::getLine()
{
	++m_line;
	if (m_size == 0)
	{
		m_size = static_cast<int>(m_file->read(m_buffer, 1024));
		m_index = 0;
	}
	
	if (m_size == 0)
	{
		return "";
	}
	
	std::string ret;
	
	// Make sure we reserve some space to prevent reallocs for each and every character
	ret.reserve(150);
	
	bool done = false;
	while (done == false)
	{
		// read till first newline
		int end = -1;
		for (; m_index < m_size; ++m_index)
		{
			if (m_buffer[m_index] == '\r' || m_buffer[m_index] == '\n')
			{
				end = m_index;
				break;
			}
			else
			{
				ret += m_buffer[m_index];
			}
		}
		
		if (end == -1)
		{
			// no newline found, read more
			m_size = static_cast<int>(m_file->read(m_buffer, 1024));
			m_index = 0;
			if (m_size == 0)
			{
				// no more data, done
				done = true;
			}
		}
		else
		{
			// skip past newline (and leading whitespaces)
			while (m_buffer[end] == '\r' || m_buffer[end] == '\n' || m_buffer[end] == ' ')
			{
				++end;
				if (end >= m_size)
				{
					m_size = static_cast<int>(m_file->read(m_buffer, 1024));
					end = 0;
					if (m_size == 0)
					{
						// no more data, done
						done = true;
						m_index = 0;
						break;
					}
				}
			}
			done = true;
			m_index = end;
		}
	}
	
	return ret;
}

// Namespace end
}
}
}
