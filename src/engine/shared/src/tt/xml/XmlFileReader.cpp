#include <tt/code/helpers.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlFileReader.h>


namespace tt {
namespace xml {

//--------------------------------------------------------------------------------------------------
// Public member functions

XmlFileReader::XmlFileReader()
:
m_textData(0),
m_position(0),
m_textBegin(0),
m_textSize(0),
m_filename()
{
}


XmlFileReader::~XmlFileReader()
{
	delete[] m_textData;
}


bool XmlFileReader::loadFile(const fs::FilePtr& p_file)
{
#if defined(TT_BUILD_FINAL)
	// NOTE: Cannot get filename from file pointer in final mode
	m_filename.clear();
#else
	m_filename = p_file->getPath();
#endif
	
	// Read whole xml file
	if (readFile(p_file) == false)
	{
		TT_PANIC("Error reading file '%s'.", p_file->getPath());
		return false;
	}
	
	// Set pointer to text begin
	m_position = m_textBegin;
	
	return true;
}


bool XmlFileReader::loadFile(const std::string& p_filename, fs::identifier p_type)
{
	if (fs::fileExists(p_filename, p_type) == false)
	{
		return false;
	}
	
	// Open the file
	fs::FilePtr file = fs::open(p_filename, fs::OpenMode_Read, p_type);
	if (file == 0)
	{
		TT_PANIC("Unable to open file '%s'.", p_filename.c_str());
		return false;
	}
	
	return loadFile(file);
}


bool XmlFileReader::read()
{
	// if not end reached, parse the node
	if ((m_position != 0) &&
	    (m_position - m_textBegin) < m_textSize &&
	    (*m_position != 0))
	{
		if (parseCurrentNode())
		{
			return true;
		}
	}
	
	return false;
}


std::string XmlFileReader::getSourceName() const
{
	return m_filename;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool XmlFileReader::parseCurrentNode()
{
	char* start = m_position;
	
	// move forward until '<' found
	while (*m_position != '<' && *m_position)
	{
		++m_position;
	}
	
	if (*m_position == 0)
	{
		return false;
	}
	
	if (m_position - start > 0)
	{
		// we found some text, store it
		if (setText(start, m_position))
		{
			return true;
		}
	}
	
	// Read '<' character
	++m_position;
	
	// based on current token, parse and report next element
	switch (*m_position)
	{
	case '/':
		parseClosingXMLElement();
		break;
		
	case '?':
		ignoreDefinition();
		break;
		
	case '!':
		if (parseCDATA() == false)
		{
			parseComment();
		}
		break;
		
	default:
		parseOpeningXMLElement();
		break;
	}
	
	return true;
}


bool XmlFileReader::setText(char* p_start, char* p_end)
{
	// Check if there is only white space, so that this text won't be reported
	char* pos = p_start;
	
	for ( ; pos != p_end; ++pos)
	{
		if (isWhiteSpace(*pos) == false)
		{
			break;
		}
	}
	
	if (pos == p_end)
	{
		return false;
	}
	
	// set current text to the parsed text, and replace xml special characters
	std::string s(p_start, static_cast<u32>(p_end - p_start));
	replaceSpecialCharacters(s);
	setNodeName(s);
	
	// current XML node type is text
	setCurrentNodeType(EnumNode_Text);
	
	return true;
}


void XmlFileReader::ignoreDefinition()
{
	setCurrentNodeType(EnumNode_Unknown);
	
	// move until end marked with '>' reached
	while (*m_position != '>')
	{
		++m_position;
	}
	
	// Skip '>'
	++m_position;
}


void XmlFileReader::parseComment()
{
	setCurrentNodeType(EnumNode_Comment);
	m_position += 1;
	
	char* pCommentBegin = m_position;
	
	// move until end of comment reached
	while ((*m_position == '>' && *(m_position-1) == '-' && *(m_position-2) == '-') == false)
	{
		++m_position;
	}
	
	// Set pointer back to not include comment end "-->"
	m_position -= 3;
	
	// Save comment string
	setNodeName(std::string(pCommentBegin + 2, static_cast<u32>(m_position - pCommentBegin - 2)));
	
	// Move pointer back to end of comment
	m_position += 4;
}


void XmlFileReader::parseOpeningXMLElement()
{
	setCurrentNodeType(EnumNode_Element);
	setIsEmpty(false);
	clearAttributes();
	
	const char* startName = m_position;
	
	// find end of element
	while (*m_position != '>' && !isWhiteSpace(*m_position))
	{
		++m_position;
	}
	
	const char* endName = m_position;
	
	// find m_attributes
	while (*m_position != '>')
	{
		// Skip white space
		if (isWhiteSpace(*m_position))
		{
			++m_position;
		}
		else
		{
			if (*m_position != '/')
			{
				// we've got an attribute
				
				// Read attribute name
				const char* attributeNameBegin = m_position;
				
				while (isWhiteSpace(*m_position) == false &&
				       *m_position != '=')
				{
					++m_position;
				}
				
				const char* attributeNameEnd = m_position;
				
				// read '=' sign
				++m_position;
				
				// read the attribute value
				// check for quotes and single quotes
				while (*m_position != '\"' &&
				       *m_position != '\'' &&
				       *m_position != 0)
				{
					++m_position;
				}
				
				if (*m_position == 0) // malformatted xml file
				{
					TT_PANIC("unexpected end-of-file.");
					return;
				}
				
				// Store quote character
				const char attributeQuoteChar = *m_position;
				
				// Read quote character
				++m_position;
				
				const char* attributeValueBegin = m_position;
				
				// Read value
				while(*m_position != attributeQuoteChar && *m_position != 0)
				{
					if (*m_position == '\r' ||
					    *m_position == '\n')
					{
						TT_PANIC("Newline found in attribute '%s' of node '%s', "
						         "did you forget the closing %c ?\n",
						         std::string(attributeNameBegin, attributeNameEnd).c_str(),
						         std::string(startName, endName).c_str(),
						         attributeQuoteChar);
						return;
					}
					
					++m_position;
				}
				
				if (*m_position == 0) // malformatted xml file
				{
					TT_PANIC("unexpected end-of-file.");
					return;
				}
				
				const char* attributeValueEnd = m_position;
				
				// Read quote char
				++m_position;
				
				// Create attribute
				Attribute attribute;
				
				// Store name
				attribute.name = std::string(attributeNameBegin,
					static_cast<u32>(attributeNameEnd - attributeNameBegin));
				
				// Store value
				attribute.value = std::string(attributeValueBegin,
					static_cast<u32>(attributeValueEnd - attributeValueBegin));
				
				// Replace special characters in value
				replaceSpecialCharacters(attribute.value);
				
				// Store attribute
				addAttribute(attribute);
			}
			else
			{
				// tag is closed directly, read '/'
				++m_position;
				setIsEmpty(true);
				break;
			}
		}
	}
	
	// check if this tag is closing directly
	if (endName > startName && *(endName - 1) == '/')
	{
		// directly closing tag
		setIsEmpty(true);
		--endName;
	}
	
	setNodeName(std::string(startName, static_cast<u32>(endName - startName)));
	
	// Read '>'
	++m_position;
}


void XmlFileReader::parseClosingXMLElement()
{
	setCurrentNodeType(EnumNode_ElementEnd);
	setIsEmpty(false);
	clearAttributes();
	
	// Skip '/' character
	++m_position;
	
	const char* pBeginClose = m_position;
	
	// Move to end of tag
	while (*m_position != '>')
	{
		++m_position;
	}
	
	// Store node name
	setNodeName(std::string(pBeginClose,
		static_cast<u32>(m_position - pBeginClose)));
	
	// Skip '>' character
	++m_position;
}


bool XmlFileReader::parseCDATA()
{
	if (*(m_position + 1) != '[')
	{
		// Not a CDATA section
		return false;
	}
	
	setCurrentNodeType(EnumNode_CData);
	
	// skip '<![CDATA['
	int count = 0;
	while (*m_position != 0 && count < 8)
	{
		++m_position;
		++count;
	}
	
	if (*m_position == 0)
	{
		return true;
	}
	
	char* cDataBegin = m_position;
	char* cDataEnd   = 0;
	
	// find end of CDATA
	while (*m_position != 0 && cDataEnd == 0)
	{
		if (*m_position == '>' &&
		    *(m_position - 1) == ']' &&
		    *(m_position - 2) == ']')
		{
			cDataEnd = m_position - 2;
		}
		
		++m_position;
	}
	
	if (cDataEnd != 0)
	{
		// Create string from data
		setNodeName(std::string(cDataBegin,
			static_cast<u32>(cDataEnd - cDataBegin)));
	}
	else
	{
		// Set empty string
		setNodeName("");
	}
	
	return true;
}


bool XmlFileReader::readFile(const fs::FilePtr& p_file)
{
	if (p_file == 0)
	{
		return false;
	}
	
	// Store size of file
	m_textSize = static_cast<int>(p_file->getLength());
	
	// Release previously allocated memory
	if (m_textData != 0)
	{
		code::helpers::safeDeleteArray(m_textData);
	}
	
	// We need a terminating 0 at the end so 1 byte extra is allocated
	m_textData = new char[m_textSize + 1];
	
	// Check if allocation succeeded
	if (m_textData == 0)
	{
		TT_PANIC("Allocation of %d bytes failed. Out of memory?",
		         m_textSize + 1);
		return false;
	}
	
	// Fill with 0's
	memset(m_textData, 0, static_cast<size_t>(m_textSize + 1));
	
	// Read complete file into memory
	if (p_file->read(m_textData, static_cast<fs::size_type>(m_textSize)) == 0)
	{
		tt::code::helpers::safeDeleteArray(m_textData);
		return false;
	}
	
	// Set text begin pointer
	m_textBegin = m_textData;
	
	return true;
}

// Namespace end
}
}
