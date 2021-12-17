#if !defined(INC_TT_PRES_PRESENTATIONVALUE_H)
#define INC_TT_PRES_PRESENTATIONVALUE_H


#include <string>
#include <map>

#include <tt/code/OptionalValue.h>
#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>
#include <tt/math/Range.h>
#include <tt/xml/fwd.h>

namespace tt 
{
	namespace code
	{
		class ErrorStatus;
	}
}


namespace tt {
namespace pres {

class PresentationValue;

// helper functions to parse PresentationValue from xml
PresentationValue                      parsePresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
PresentationValue                      parsePresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute, real p_defaultCustomVal, code::ErrorStatus* p_errStatus);
code::OptionalValue<PresentationValue> parseOptionalPresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
PresentationValue                      parseOptionalPresentationValue(
		const xml::XmlNode* p_node, const std::string& p_attribute, 
		real p_default, code::ErrorStatus* p_errStatus);

/*! \brief Represents a value that can either be a real a range or a custom string  */
class PresentationValue
{
public:
	PresentationValue();
	explicit PresentationValue(real p_value);
	explicit PresentationValue(const math::Range& p_value);
	
	/*! \brief Resets this PresentationValue with a string. Parsing a range or a real.
	    \param p_value String representation of the PresentationValue. */ 
	bool resetValue(const std::string& p_value, code::ErrorStatus* p_errStatus);
	
	/*! \brief Converts this PresentationValue to the string representation it was parsed from */
	std::string toStr() const;
	
	/*! \brief Make this PresentationValue a real value and set it */
	void setValue(real p_value);
	/*! \brief Make this PresentationValue a Range value and set it */
	void setRange(const math::Range& p_range);
	/*! \brief Make this PresentationValue a Custom string value and set it */
	void setCustomValue(const std::string& p_customValue);
	
	/*! \brief Sets the value from a random value in the range*/
	void updateValue(const PresentationObject* p_presObj);
	
	/*! \brief Gets the current value */
	real get() const;
	
	/*! \brief Gets the minimum value this PresentationValue could return */
	real getMin() const;
	/*! \brief Gets the maximum value this PresentationValue could return */
	real getMax() const;
	
	/*! \brief Inverts the value. This is used by shoebox's Invert Y functionality */
	void invertValue();
	
	/*! \brief Load this value from memory */
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	/*! \brief Save this value to memory */
	bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	/*! \brief Returns the size of the buffer needed save this Value.
	    \return The size of the buffer.*/
	size_t getBufferSize() const;
	
	inline operator real() const { return get(); }
	
	/*! \brief Whether this PresentationValue has an updated Value */
	inline bool isValid() const { return m_valid; }
	inline bool isRandomRange() const { return m_valueType == ValueType_Range; }
private:
	enum ValueType
	{
		ValueType_Real,
		ValueType_Range,
		ValueType_CustomString,
		
		ValueType_Count,
		ValueType_Invalid
	};
	
	ValueType m_valueType;
	bool      m_valid;
	real      m_value;
	
	math::Range m_valueRange;
	
	std::string m_customValue;
};



//namespace end
}
}

#endif // !defined(INC_TT_PRES_PRESENTATIONVALUE_H)
