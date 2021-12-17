#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/PresentationValue.h>
#include <tt/pres/PresentationObject.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/str/str.h>
#include <tt/xml/XmlNode.h>

namespace tt {
namespace pres {


static const u16 s_version = 2;

//--------------------------------------------------------------------------------------------------
// Public helper functions

PresentationValue parsePresentationValue(const xml::XmlNode* p_node, const std::string& p_attribute,
                                         code::ErrorStatus* p_errStatus)
{
	return parsePresentationValue(p_node, p_attribute, 0.0f, p_errStatus);
}


PresentationValue parsePresentationValue(const xml::XmlNode* p_node, const std::string& p_attribute,
                                         real p_defaultCustomVal, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PresentationValue, pres::PresentationValue(0), "parsing attribute '" << p_attribute << "'");
	TT_ERR_ASSERTMSG(p_node != 0, "Null XML node pointer passed.");
	
	TT_ERR_ADD_LOC(" from node '" << p_node->getName() << "'");
	
	TT_ERR_ASSERTMSG(p_node->hasAttribute(p_attribute),
	                 "Expected attribute '" << p_attribute << "' in node '" << p_node->getName() << "'");
	
	PresentationValue val(p_defaultCustomVal);
	val.resetValue(p_node->getAttribute(p_attribute), &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	return val;
}


code::OptionalValue<PresentationValue> parseOptionalPresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return code::OptionalValue<PresentationValue>();
	}
	return parsePresentationValue(p_node, p_attribute, p_errStatus);
}


PresentationValue parseOptionalPresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute,
		real p_default, code::ErrorStatus* p_errStatus)
{
	PresentationValue defaultPresVal(p_default);
	
	TT_ERR_CHAIN(PresentationValue, defaultPresVal, "Parsing PresentationValue " << p_attribute);
	code::OptionalValue<PresentationValue> range(parseOptionalPresentationValue(p_node, p_attribute, &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	if(range.isValid())
	{
		return range.get();
	}
	else 
	{
		return defaultPresVal;
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions


PresentationValue::PresentationValue()
:
m_valueType(ValueType_Invalid),
m_valid(false),
m_value(0),
m_valueRange(),
m_customValue()
{
}


PresentationValue::PresentationValue(real p_value)
:
m_valueType(ValueType_Real),
m_valid(true),
m_value(p_value),
m_valueRange(p_value),
m_customValue()
{
}


PresentationValue::PresentationValue(const math::Range& p_value)
:
m_valueType(ValueType_Range),
m_valid(false),
m_value(0.0f),
m_valueRange(p_value),
m_customValue()
{
	
}


bool PresentationValue::resetValue(const std::string& p_value, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Resetting PresentationValue to " << p_value);
	
	TT_ERR_ASSERTMSG(p_value.empty() == false, "Value can not be empty");
	
	// FIXME: Cleanup this parsing code.
	switch (*p_value.begin())
	{
	case 'c': // custom values
		{
			TT_ERR_ASSERTMSG(p_value.length() > 3 && p_value[1] == '(' && p_value[p_value.length() - 1] == ')',
			                 "Expected custom value in format 'c(name)' or 'r(range)' or a number. Got '" << p_value << "'.");
			TT_ERR_ASSERTMSG(p_value[2] != '_', "First character in custom value '" << p_value << "' should not be '_'.");
			m_customValue = p_value.substr(2, p_value.length() - 3);
			
			std::string nameStr;
			std::string defaultStr;
			if (tt::str::split(m_customValue, ',', nameStr, defaultStr))
			{
				// Found , expect c(<name>,<default value>).
				m_customValue = tt::str::trim(nameStr);
				m_value       = tt::str::parseReal(tt::str::trim(defaultStr), &errStatus);
				TT_ERR_RETURN_ON_ERROR();
			}
			m_valueType   = ValueType_CustomString;
			m_valid       = false;
		}
		break;
		
	case 'r': // ranges and reals
	default:
		m_valueRange = str::parseRange(p_value, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
		
		if (m_valueRange.getRange() == 0) // if the range equals 0 we can handle it as a real
		{
			m_valueType = ValueType_Real;
			m_value     = m_valueRange.getMin();
			m_valid     = true;
		}
		else
		{
			m_valueType = ValueType_Range;
			m_valid     = false;
		}
		break;
	}
	
	return true;
}


std::string PresentationValue::toStr() const
{
	switch (m_valueType)
	{
	case ValueType_Real:         return str::toStr(m_value);
	case ValueType_Range:        return str::toStr(m_valueRange);
	case ValueType_CustomString: return m_customValue;
	
	default: TT_PANIC("Invalid Value Type for Presentation Value");
	}
	return "<invalid>";
}


void PresentationValue::setValue(real p_value)
{
	m_valueRange.setMinMax(p_value);
	m_value = p_value;
	m_valueType = ValueType_Real;
	m_valid = true;
}


void PresentationValue::setRange(const math::Range& p_range)
{
	m_valueType = ValueType_Range;
	m_valueRange = p_range;
}


void  PresentationValue::setCustomValue(const std::string& p_customValue)
{
	m_customValue = p_customValue;
	m_valueType   = ValueType_CustomString;
	m_valid       = false;
}


void PresentationValue::updateValue(const PresentationObject* p_presObj)
{
	switch (m_valueType)
	{
	case ValueType_Real:
		m_valid = true;
		break;
	case ValueType_Range:
		m_value = m_valueRange.getRandom(tt::math::Random::getEffects());
		m_valid = true;
		break;
	case ValueType_CustomString:
		{
			m_valid = true;
			
			// No values available. Use whatever's currently in m_value
			if (p_presObj == 0) break;
			
			if (p_presObj->getCustomValue(m_customValue, &m_value) == false)
			{
				TT_PANIC("Custom Presentation Value '%s' not specified. Using %f", 
				         m_customValue.c_str(), realToFloat(m_value));
			}
		}
		break;
	
	default: TT_PANIC("Invalid Value Type for Presentation Value");
	}
}


real PresentationValue::get() const 
{
	if (isValid())
	{
		return m_value;
	}
	else
	{
		TT_PANIC("Using invalid PresentationValue");
		return 0;
	}
}


real PresentationValue::getMin() const
{
	switch (m_valueType)
	{
	case ValueType_Real:  return m_value;
	case ValueType_Range: return m_valueRange.getMin();
	
	case ValueType_CustomString: return m_value;
	
	default: TT_PANIC("Invalid Value Type for Presentation Value");
		return 0;
	}
}


real PresentationValue::getMax() const
{
	switch (m_valueType)
	{
	case ValueType_Real:  return m_value;
	case ValueType_Range: return m_valueRange.getMax();
	
	case ValueType_CustomString: return m_value;
	
	default: TT_PANIC("Invalid Value Type for Presentation Value");
		return 0;
	}
}


void PresentationValue::invertValue()
{
	switch (m_valueType)
	{
	case ValueType_Real:  m_value = -m_value; break;
	case ValueType_Range: m_valueRange.setMinMax(-m_valueRange.getMax(), -m_valueRange.getMin()); break;
	
	// Don't invert custom values
	case ValueType_CustomString: break;
	
	default: TT_PANIC("Invalid Value Type for Presentation Value"); break;
	}
}


bool PresentationValue::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading PresentationValue");
	using namespace code::bufferutils;
	
	// version
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_version)
	{
		TT_PANIC("Invalid version, code %d, data %d. Update your Converters!", s_version, version);
		return false;
	}
	
	// value type
	m_valueType = static_cast<ValueType>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	
	// valid
	m_valid = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	// value
	switch (m_valueType)
	{
	case ValueType_Real:
		TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(real), "Not enough space remaining in buffer.");
		m_value = be_get<real>(p_bufferOUT, p_sizeOUT); 
		break;
		
	case ValueType_Range:
		{
			TT_ERR_ASSERTMSG(p_sizeOUT >= (sizeof(real) + sizeof(real)),
			                 "Not enough space remaining in buffer.");
			real min = be_get<real>(p_bufferOUT, p_sizeOUT);
			real max = be_get<real>(p_bufferOUT, p_sizeOUT);
			m_valueRange.setMinMax(min, max);
		}
		break;
		
	case ValueType_CustomString:
		m_customValue = be_get<std::string>(p_bufferOUT, p_sizeOUT);
		TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(real), "Not enough space remaining in buffer.");
		m_value       = be_get<real       >(p_bufferOUT, p_sizeOUT);
		break;
		
	default:
		TT_ERR_AND_RETURN("Invalid Value Type for Presentation Value");
	}
	
	return true;
}


bool PresentationValue::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "Saving PresentationValue");
	using namespace code::bufferutils;
	
	// version
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	// value type
	be_put(static_cast<u8>(m_valueType), p_bufferOUT, p_sizeOUT);
	
	// valid
	be_put(m_valid, p_bufferOUT, p_sizeOUT);
	
	// value
	switch (m_valueType)
	{
	case ValueType_Real:
		be_put(m_value, p_bufferOUT, p_sizeOUT); 
		break;
		
	case ValueType_Range: 
		be_put(m_valueRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_valueRange.getMax(), p_bufferOUT, p_sizeOUT);
		break;
		
	case ValueType_CustomString:
		be_put(m_customValue, p_bufferOUT, p_sizeOUT);
		be_put(m_value,       p_bufferOUT, p_sizeOUT);
		break;
		
	default:
		TT_ERR_AND_RETURN("Invalid Value Type for Presentation Value");
	}
	
	return true;
}


size_t PresentationValue::getBufferSize() const
{
	size_t size = 2 + // version
	              1 + // type
	              1;  // valid
	
	switch(m_valueType)
	{
	case ValueType_Real:
		size += 4; // real
		break;
	case ValueType_Range:
		size += 4  // real min value
		      + 4; // real max value
		break;
	case ValueType_CustomString:
		size += 2                    // string length
		     +  m_customValue.size() // string
		     + 4;                    // default value.
		break;
	default: break;
	}
	
	return size;
}


//--------------------------------------------------------------------------------------------------
// Private member functions




//namespace end
}
}
