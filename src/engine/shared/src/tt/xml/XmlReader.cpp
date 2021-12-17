#include <tt/xml/XmlReader.h>


namespace tt {
namespace xml {

// Special character constants
static const char* const c_special_characters[] = { "&amp;", "<lt;" , ">gt;", "\"quot;", "'apos;"};
static const u32         c_special_char_count   = 5;
static const std::string c_empty_string;


//--------------------------------------------------------------------------------------------------
// Public member functions

IXmlReader::IXmlReader()
:
m_current_node_type(EnumNode_None),
m_current_node_name(),
m_current_attributes(),
m_current_node_is_empty(false)
{
}


IXmlReader::~IXmlReader()
{
}


const std::string& IXmlReader::getAttributeName(u32 p_index) const
{
	// Check parameter
	if (p_index >= m_current_attributes.size())
	{
		return c_empty_string;
	}
	
	return m_current_attributes[p_index].name;
}


const std::string& IXmlReader::getAttributeValue(u32 p_index) const
{
	// Check parameter
	if (p_index >= m_current_attributes.size())
	{
		return c_empty_string;
	}
	
	return m_current_attributes[p_index].value;
}


const std::string& IXmlReader::getAttributeValue(const std::string& p_name) const
{
	const Attribute* attribute = getAttributeByName(p_name);
	if (attribute == 0)
	{
		return c_empty_string;
	}
	
	return attribute->value;
}


s32 IXmlReader::getAttributeValueAsInt(const std::string& p_name) const
{
	return static_cast<s32>(getAttributeValueAsFloat(p_name));
}


s32 IXmlReader::getAttributeValueAsInt(u32 p_index) const
{
	return static_cast<s32>(getAttributeValueAsFloat(p_index));
}


float IXmlReader::getAttributeValueAsFloat(const std::string& p_name) const
{
	const Attribute* attribute = getAttributeByName(p_name);
	if (attribute == 0)
	{
		return 0.0f;
	}
	return fast_atof(attribute->value.c_str());
}


float IXmlReader::getAttributeValueAsFloat(u32 p_index) const
{
	const std::string& attrvalue = getAttributeValue(p_index);
	if (attrvalue.empty())
	{
		return 0.0f;
	}
	
	return fast_atof(attrvalue.c_str());
}


// finds a current attribute by name, returns 0 if not found
const Attribute* IXmlReader::getAttributeByName(const std::string& p_name) const
{
	for (std::vector<Attribute>::const_iterator it = m_current_attributes.begin();
	     it != m_current_attributes.end(); ++it)
	{
		const Attribute& attrib(*it);
		if (attrib.name == p_name)
		{
			return &attrib;
		}
	}
	
	return 0;
}


// replaces xml special characters in a string and creates a new one
void IXmlReader::replaceSpecialCharacters(std::string& p_src)
{
	// Start search for special characters
	size_t pos = p_src.find('&');
	
	// If found, process special char
	while (pos != std::string::npos)
	{
		// Don't bother searching if followed by space (skip common use of &)
		if (p_src[pos + 1] != ' ')
		{
			// Find size of this token (delimited by ';', + 1 to include ';')
			size_t token_size = p_src.find(';', pos) - pos + 1;
			
			// Compare to string constants
			for (u32 i = 0; i < c_special_char_count; ++i)
			{
				// Compare excluding first char
				if (strcmp(c_special_characters[i] + 1, p_src.substr(pos + 1, token_size - 1).c_str()) == 0)
				{
					// Replace complete token by the special character
					p_src.replace(pos, token_size, 1, c_special_characters[i][0]);
					
					// Stop searching for match
					break;
				}
			}
		}
		
		// Find next special character
		pos = p_src.find('&', pos + 1);
	}
}

// Namespace end
}
}
