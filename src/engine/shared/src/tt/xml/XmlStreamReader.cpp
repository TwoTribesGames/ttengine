
// Include complement header file
#include <tt/xml/XmlStreamReader.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace xml {

//--------------------------------------------------------------------------------------------------
// Public member functions

XmlStreamReader::XmlStreamReader()
:
m_xmlStream(),
m_filename()
{
}


XmlStreamReader::~XmlStreamReader()
{
}


bool XmlStreamReader::initStream(const std::string& p_filename)
{
	// Open stream from file
	m_filename = p_filename;
	m_xmlStream.open(p_filename.c_str());
	return m_xmlStream.is_open();
}


bool XmlStreamReader::read()
{
	// Check if stream is initialized
	if (m_xmlStream.is_open() == false)
	{
		TT_Printf("XmlStreamReader::read(): stream not initialized.\n");
		return false;
	}
	
	// If not end reached, parse the node
	if (m_xmlStream.eof() == false)
	{
		// Parse the current node
		if (parseCurrentNode())
		{
			return true;
		}
	}
	
	// Close the stream
	m_xmlStream.close();
	
	return false;
}


std::string XmlStreamReader::getSourceName() const
{
	return m_filename;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool XmlStreamReader::parseCurrentNode()
{
	// Create string object only once
	static std::string text;
	text.clear();
	
	// First check current character for beginning of node
	if (m_xmlStream.peek() != '<')
	{
		// more forward until '<' found
		std::getline(m_xmlStream, text, '<');
	}
	
	// Check for end of stream
	if (m_xmlStream.eof())
	{
		return false;
	}
	
	// Check for text before the '<'
	if (text.empty() == false)
	{
		// Put back '<'
		m_xmlStream.unget();
		
		// we found some text, store it if needed
		if (setText(text))
		{
			return true;
		}
	}
	
	// Read '<' character
	m_xmlStream.ignore();
	
	// based on current token, parse and report next element
	switch (m_xmlStream.peek())
	{
	case '/':
		parseClosingXMLElement();
		break;
		
	case '?':
		ignoreDefinition();
		break;
		
	case '!':
		if (!parseCDATA() == false)
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


bool XmlStreamReader::setText(std::string& p_text)
{
	// Check if there is only white space, so that this text won't be reported
	bool allWhiteSpace = true;
	for (std::string::iterator it = p_text.begin(); it != p_text.end(); ++it)
	{
		if (isWhiteSpace(*it) == false)
		{
			allWhiteSpace = false;
			break;
		}
	}
	
	if (allWhiteSpace)
	{
		return false;
	}
	
	// current XML node type is text
	setCurrentNodeType(EnumNode_Text);
	
	// set current text to the parsed text, and replace xml special characters
	replaceSpecialCharacters(p_text);
	setNodeName(p_text);
	
	return true;
}


void XmlStreamReader::ignoreDefinition()
{
	setCurrentNodeType(EnumNode_Unknown);
	
	// move until end marked with '>' reached
	m_xmlStream.ignore(std::numeric_limits<std::streamsize>::max(), '>');
	
	// Skip '>'
	m_xmlStream.ignore();
}


void XmlStreamReader::parseComment()
{
	setCurrentNodeType(EnumNode_Comment);
	
	// Skip '!--'
	m_xmlStream.ignore(3);
	
	std::string comment;
	
	// move until end of comment reached
	while ((m_xmlStream.peek() == '>' && comment.substr(comment.length()-2, 2) == "--") == false)
	{
		comment += static_cast<char>(m_xmlStream.get());
	}
	
	// Skip '>'
	m_xmlStream.ignore(1);
	
	// Remove comment end "--"
	comment.resize(comment.length() - 2);
	
	// Save comment string
	setNodeName(comment);
}


void XmlStreamReader::parseOpeningXMLElement()
{
	setCurrentNodeType(EnumNode_Element);
	setIsEmpty(false);
	clearAttributes();
	
	// Containers for element contents and name
	static std::string element;
	static std::string name;
	element.clear();
	name.clear();
	
	// Read complete element into string
	std::getline(m_xmlStream, element, '>');
	
	// Check string for white space
	std::string::iterator iter = element.begin();
	for( ; iter != element.end(); ++iter)
	{
		if(isWhiteSpace(*iter)) break;
	}
	
	if (iter == element.end())
	{
		// No attributes, element = name
		name = element;
	}
	else
	{
		// Attributes or empty space in tag
		name = std::string(element.begin(), iter);
		
		while (iter != element.end())
		{
			// Skip white space
			if (isWhiteSpace(*iter))
			{
				// Read white space
				++iter;
			}
			else
			{
				if (*iter == '/')
				{
					// tag is closed directly, set state and exit loop
					setIsEmpty(true);
					break;
				}
				else
				{
					// we've got an attribute
					
					// Read attribute name and value
					Attribute attribute;
					
					while (*iter != '=' && (isWhiteSpace(*iter) == false) &&
					       iter != element.end())
					{
						attribute.name += *iter;
						++iter;
					}
					
					// search for quote after '=' char
					// check for quotes and single quotes
					while (iter != element.end() && *iter != '\"' && *iter != '\'')
					{
						++iter;
					}
					
					if (iter == element.end()) // malformatted xml file
					{
						TT_Printf("XmlStreamReader::parseOpeningElement: "
						          "unexpected end of attribute.\n");
						return;
					}
					
					// Store quote character
					const char attributeQuoteChar = *iter;
					++iter;
					
					// Read value
					while (*iter != attributeQuoteChar && iter != element.end())
					{
						attribute.value += *iter;
						++iter;
					}
					
					if (iter == element.end()) // malformatted xml file
					{
						TT_Printf("XmlStreamReader::parseOpeningElement: "
						          "unexpected end of attribute.\n");
						return;
					}
					
					// Read quote char
					++iter;
					
					// Replace special characters in value
					replaceSpecialCharacters(attribute.value);
					
					// Store attribute
					addAttribute(attribute);
				}
			}
		}
	}
	
	// check if this tag is closing directly
	if (name.empty() == false && *name.rbegin() == '/')
	{
		// directly closing tag
		setIsEmpty(true);
		
		// Strip '/' char from name
		name.resize(name.length() - 1);
	}
	
	// Store element name
	setNodeName(name);
}


void XmlStreamReader::parseClosingXMLElement()
{
	setCurrentNodeType(EnumNode_ElementEnd);
	setIsEmpty(false);
	clearAttributes();
	
	// Skip '/' character
	m_xmlStream.ignore();
	
	static std::string name;
	name.clear();
	
	// Read element name
	std::getline(m_xmlStream, name, '>');
	
	if (m_xmlStream.eof())
	{
		TT_Printf("XmlStreamReader::parseClosingElement: "
		          "unexpected end-of-file.\n");
		return;
	}
	
	// Store node name
	setNodeName(name);
}


bool XmlStreamReader::parseCDATA()
{
	// Check if this is a comment or CDATA section
	m_xmlStream.get();
	if (m_xmlStream.peek() != '[')
	{
		// Not a CDATA section
		m_xmlStream.unget();
		return false;
	}
	setCurrentNodeType(EnumNode_CData);
	
	// skip '<![CDATA['
	m_xmlStream.ignore(7);
	
	if (m_xmlStream.eof())
	{
		return true;
	}
	
	// Create data container
	static std::string data;
	data.clear();
	
	// find end of CDATA
	std::getline(m_xmlStream, data, '>');
	
	// Check whether it is really the end
	while (data.substr(data.length() - 3, 2) != "]]")
	{
		// If not append more data
		std::string more_data;
		std::getline(m_xmlStream, more_data, '>');
		data += more_data;
	}
	
	// Remove "]]>" from string
	data.resize(data.length() - 3);
	
	// Store data
	setNodeName(data);
	
	return true;
}

// Namespace end
}
}
