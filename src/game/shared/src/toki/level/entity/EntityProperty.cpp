#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>

#include <toki/level/entity/EntityProperty.h>


namespace toki {
namespace level {
namespace entity {

EntityProperty EntityProperty::ms_emptyEntityProperty;

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityProperty::Conditional::Conditional()
:
m_targetPropertyName(),
m_operator(Operator_Invalid),
m_value()
{
}


EntityProperty::Conditional EntityProperty::Conditional::create(const std::string p_conditional)
{
	tt::str::Strings strings(tt::str::explode(p_conditional, " \t"));
	if (strings.size() != 3)
	{
		TT_PANIC("Conditional '%s' has incorrect form. It should be in the form 'property operator value'.",
			p_conditional.c_str());
		return Conditional();
	}
	
	// verify operator
	Conditional conditional;
	const std::string& op(strings[1]);
	if      (op == "==")          conditional.m_operator = Operator_Equals;
	else if (op == "!=")          conditional.m_operator = Operator_NotEquals;
	else if (op == "contains")    conditional.m_operator = Operator_Contains;
	else if (op == "notcontains") conditional.m_operator = Operator_NotContains;
	else
	{
		TT_PANIC("Conditional '%s' contains unsupported operator '%s'",
			p_conditional.c_str(), op.c_str());
		return Conditional();
	}
	conditional.m_targetPropertyName = strings[0];
	conditional.m_value              = strings[2];
	
	return conditional;
}


bool EntityProperty::Conditional::test(const std::string& p_value) const
{
	switch (m_operator)
	{
	case Operator_Equals:      return p_value == m_value;
	case Operator_NotEquals:   return p_value != m_value;
	case Operator_Contains:    return p_value.find(m_value) != std::string::npos;
	case Operator_NotContains: return p_value.find(m_value) == std::string::npos;
	default:
		TT_PANIC("Unknown operator '%d'", m_operator);
		break;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityProperty::EntityProperty(const std::string& p_name, const script::attributes::AttributeCollection& p_attributes)
:
m_type(Type_None),
m_name(p_name),
m_description(),
m_group(),
m_referenceColor(tt::engine::renderer::ColorRGB::white)
{
	using namespace script::attributes;
	
	if (p_attributes.contains("type") == false)
	{
		TT_PANIC("Property '%s' is missing required 'type' attribute", m_name.c_str());
		return;
	}
	
	// Set type
	const std::string type(p_attributes.get("type").getString());
	m_type = getTypeFromName(type);
	if (m_type == Type_None)
	{
		TT_PANIC("Property '%s' has unsupported type '%s'.", m_name.c_str(), type.c_str());
		return;
	}
	
	// Set group (optional)
	if (p_attributes.contains("group"))
	{
		m_group = p_attributes.get("group").getString();
	}
	
	// Check if all required attributes for this type are present
	if (checkForAllRequiredAttributes(p_attributes) == false)
	{
		return;
	}
	
	// Set the default value
	if (validateAndSetDefault(p_attributes) == false)
	{
		return;
	}
	
	// Now set all attributes
	const AttributeCollection::AttributeMap& attributes = p_attributes.getAll();
	for (AttributeCollection::AttributeMap::const_iterator it = attributes.begin();
	     it != attributes.end(); ++it)
	{
		// skip default, group and type, as they should already be handled at this point
		if ((*it).first != "_default" && (*it).first != "type" && (*it).first != "group")
		{
			validateAndSetAttribute((*it).second);
		}
	}
}


const char* EntityProperty::getTypeName(Type p_type)
{
	switch (p_type)
	{
	case Type_None:         return "none";
	
	case Type_Integer:              return "integer";
	case Type_Float:                return "float";
	case Type_Bool:                 return "bool";
	case Type_String:               return "string";
	case Type_IntegerArray:         return "integer_array";
	case Type_FloatArray:           return "float_array";
	case Type_BoolArray:            return "bool_array";
	case Type_StringArray:          return "string_array";
	case Type_Entity:               return "entity";
	case Type_EntityArray:          return "entity_array";
	case Type_EntityID:             return "entityid";
	case Type_EntityIDArray:        return "entityid_array";
	case Type_DelayedEntityID:      return "delayed_entityid";
	case Type_DelayedEntityIDArray: return "delayed_entityid_array";
	case Type_ColorRGB:             return "color_rgb";
	case Type_ColorRGBA:            return "color_rgba";
	
	default:
		TT_PANIC("Type '%d' not implemented", p_type);
		return "";
	}
}


EntityProperty::Type EntityProperty::getTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Type_Count; ++i)
	{
		const Type type = static_cast<Type>(i);
		if (p_name == getTypeName(type))
		{
			return type;
		}
	}
	
	return Type_None;
}


bool EntityProperty::isArrayType(Type p_type)
{
	return p_type == Type_IntegerArray  ||
	       p_type == Type_FloatArray    ||
	       p_type == Type_BoolArray     ||
	       p_type == Type_StringArray   ||
	       p_type == Type_EntityArray   ||
	       p_type == Type_EntityIDArray ||
	       p_type == Type_DelayedEntityIDArray;
}


bool EntityProperty::isEntityType(Type p_type)
{
	return p_type == Type_Entity ||
	       p_type == Type_EntityID ||
	       p_type == Type_EntityArray ||
	       p_type == Type_EntityIDArray ||
	       p_type == Type_DelayedEntityID ||
	       p_type == Type_DelayedEntityIDArray;
}


bool EntityProperty::validate(const std::string& p_value) const
{
	TT_ERR_CREATE("");
	bool success = false;
	
	switch (m_type)
	{
	case Type_Integer:
		if (hasChoice())
		{
			success = validateChoice(tt::str::parseS32(p_value, &errStatus), m_choice.getIntegerArray());
		}
		else
		{
			success = validateMinMax(tt::str::parseS32(p_value, &errStatus), m_min.getInteger(), m_max.getInteger());
		}
		break;
		
	case Type_Float:
		if (hasChoice())
		{
			success = validateChoice(tt::str::parseReal(p_value, &errStatus), m_choice.getFloatArray());
		}
		else
		{
			success = validateMinMax(tt::str::parseReal(p_value, &errStatus), m_min.getFloat(), m_max.getFloat());
		}
		break;
		
	case Type_Bool:
		if (hasChoice())
		{
			success = validateChoice(tt::str::parseBool(p_value, &errStatus), m_choice.getBoolArray());
		}
		else
		{
			tt::str::parseBool(p_value, &errStatus);
			success = true;
		}
		break;
		
	case Type_String:
		if (hasChoice())
		{
			success = validateChoice(p_value, m_choice.getStringArray());
		}
		else
		{
			success = true;
		}
		break;
		
	case Type_ColorRGB:
		{
			const tt::str::Strings rgbComponents(tt::str::explode(p_value, ","));
			success = rgbComponents.size() == 3;
		}
		break;
		
	case Type_ColorRGBA:
		{
			const tt::str::Strings rgbComponents(tt::str::explode(p_value, ","));
			success = rgbComponents.size() == 3 || rgbComponents.size() == 4;
		}
		break;
		
	case Type_IntegerArray:
	case Type_FloatArray:
	case Type_BoolArray:
	case Type_StringArray:
	case Type_Entity:
	case Type_EntityArray:
	case Type_EntityID:
	case Type_EntityIDArray:
	case Type_DelayedEntityID:
	case Type_DelayedEntityIDArray:
		// TODO: Validate these types.
		success = true;
		break;
		
	default:
		TT_PANIC("Unhandled type '%s' for EntityProperty::validate", 
		        EntityProperty::getTypeName(getType()));
	}
	
	return success && (errStatus.hasError() == false);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool EntityProperty::checkForAllRequiredAttributes(
		const script::attributes::AttributeCollection& p_attributes) const
{
	// Default value should be present
	if (p_attributes.contains("_default") == false)
	{
		TT_PANIC("Property '%s' is missing required '_default' attribute", m_name.c_str());
		return false;
	}
	
	// Define all required attributes per type below
	switch (m_type)
	{
	case Type_Integer:
	case Type_Float:
		// Integer or float should defined min/max OR choice (not both)
		{
			const bool containsChoice = p_attributes.contains("choice");
			const bool containsMin    = p_attributes.contains("min");
			const bool containsMax    = p_attributes.contains("max");
			
			if (containsChoice && (containsMin || containsMax))
			{
				TT_PANIC("Property '%s' of type '%s' has both min/max and choice attributes, "
				         "which is not allowed.", m_name.c_str(), getTypeName(m_type));
				return false;
			}
			
			if (containsChoice == false && (containsMin == false || containsMax == false))
			{
				TT_PANIC("Property '%s' of type '%s' should have min/max or choice attributes.", 
				         m_name.c_str(), getTypeName(m_type));
				return false;
			}
		}
		break;
		
	default:
		// No required attributes for other types.
		break;
	}
	
	return true;
}


bool EntityProperty::validateAndSetAttribute(const script::attributes::Attribute& p_attribute)
{
	using namespace script::attributes;
	
	script::attributes::Attribute attribute(p_attribute);
	
	const std::string& name = attribute.getName();
	
	// Make sure that the default value has been set at this point!
	if (m_default.getType() == Attribute::Type_None)
	{
		TT_PANIC("Property '%s': Default value not set. "
		         "validateAndSetDefault() has failed or hasn't been called", m_name.c_str());
		return false;
	}
	
	if (name == "min")
	{
		// Type should be Integer or Float
		if (m_type != Type_Integer && m_type != Type_Float)
		{
			TT_PANIC("Property '%s' has type '%s' which has no support for attribute 'min'",
			         m_name.c_str(), getTypeName(m_type));
			return false;
		}
		
		// Validate type
		Attribute::Type type        = attribute.getType();
		Attribute::Type correctType = (m_type == Type_Integer) ?
			Attribute::Type_Integer : Attribute::Type_Float;
		
		// Implicit cast of int to float
		if (type == Attribute::Type_Integer && m_type == Type_Float)
		{
			// Cast to float
			attribute.setFloat(static_cast<real>(attribute.getInteger()));
			type = Attribute::Type_Float;
		}
		
		if (type != correctType)
		{
			TT_PANIC("Property '%s' has attribute 'min' of type '%s', expected type '%s'",
			         m_name.c_str(), Attribute::getTypeName(type),
			         Attribute::getTypeName(correctType));
			return false;
		}
		
		// Validate against max, if set
		if (m_max.getType() != Attribute::Type_None)
		{
			if (m_type == Type_Integer && m_max.getInteger() < attribute.getInteger())
			{
				TT_PANIC("Property '%s' has max (%d) < min (%d)", m_name.c_str(),
				         m_max.getInteger(), attribute.getInteger());
				return false;
			}
			else if (m_type == Type_Float && m_max.getFloat() < attribute.getFloat())
			{
				TT_PANIC("Property '%s' has max (%f) < min (%f)", m_name.c_str(),
				         m_max.getFloat(), attribute.getFloat());
				return false;
			}
		}
		
		// Validate default
		if (m_type == Type_Integer && m_default.getInteger() < attribute.getInteger())
		{
			TT_PANIC("Property '%s' has default (%d) < min (%d)", m_name.c_str(),
			         m_default.getInteger(), attribute.getInteger());
			return false;
		}
		else if (m_type == Type_Float && m_default.getFloat() < attribute.getFloat())
		{
			TT_PANIC("Property '%s' has default (%f) < min (%f)", m_name.c_str(),
			         m_default.getFloat(), attribute.getFloat());
			return false;
		}
		
		// Attribute valid, set it
		m_min = attribute;
	}
	else if (name == "max")
	{
		// Type should be Integer or Float
		if (m_type != Type_Integer && m_type != Type_Float)
		{
			TT_PANIC("Property '%s' has type '%s' which has no support for attribute 'max'",
			         m_name.c_str(), getTypeName(m_type));
			return false;
		}
		
		// Validate type
		Attribute::Type type        = attribute.getType();
		Attribute::Type correctType = (m_type == Type_Integer) ?
			Attribute::Type_Integer : Attribute::Type_Float;
		
		// Implicit cast of int to float
		if (type == Attribute::Type_Integer && m_type == Type_Float)
		{
			// Cast to float
			attribute.setFloat(static_cast<real>(attribute.getInteger()));
			type = Attribute::Type_Float;
		}
		
		if (type != correctType)
		{
			TT_PANIC("Property '%s' has attribute 'max' of type '%s', expected type '%s'",
			         m_name.c_str(), Attribute::getTypeName(type),
			         Attribute::getTypeName(correctType));
			return false;
		}
		
		// Validate against min, if set
		if (m_min.getType() != Attribute::Type_None)
		{
			if (m_type == Type_Integer && m_min.getInteger() > attribute.getInteger())
			{
				TT_PANIC("Property '%s' has min (%d) > max (%d)", m_name.c_str(),
				         m_min.getInteger(), attribute.getInteger());
				return false;
			}
			else if (m_type == Type_Float && m_min.getFloat() > attribute.getFloat())
			{
				TT_PANIC("Property '%s' has min (%f) > max (%f)", m_name.c_str(),
				         m_min.getFloat(), attribute.getFloat());
				return false;
			}
		}
		
		// Validate default
		if (m_type == Type_Integer && m_default.getInteger() > attribute.getInteger())
		{
			TT_PANIC("Property '%s' has default (%d) > max (%d)", m_name.c_str(),
			         m_default.getInteger(), attribute.getInteger());
			return false;
		}
		else if (m_type == Type_Float && m_default.getFloat() > attribute.getFloat())
		{
			TT_PANIC("Property '%s' has default (%f) > max (%f)", m_name.c_str(),
			         m_default.getFloat(), attribute.getFloat());
			return false;
		}
		
		// Attribute valid, set it
		m_max = attribute;
	}
	else if (name == "choice")
	{
		Attribute::Type type        = attribute.getType();
		Attribute::Type correctType = Attribute::Type_None;
		switch (m_type)
		{
		case Type_Integer: correctType = Attribute::Type_IntegerArray; break;
		case Type_Float:   correctType = Attribute::Type_FloatArray;   break;
		case Type_Bool:    correctType = Attribute::Type_BoolArray;    break; // stupid?
		case Type_String : correctType = Attribute::Type_StringArray;  break;
			
		default:
			TT_PANIC("Property '%s' has type '%s' which has no support for attribute 'choice'",
			         m_name.c_str(), getTypeName(m_type));
			return false;
		}
		
		if (correctType != type)
		{
			TT_PANIC("Property '%s' has type '%s' with attribute 'choice' of type '%s', expected type '%s'",
			         m_name.c_str(), getTypeName(m_type),
			         Attribute::getTypeName(type), Attribute::getTypeName(correctType));
			return false;
		}
		
		if (m_default.getType() != Attribute::Type_Null)
		{
			// Make sure that default value is part of the choice container
			switch (type)
			{
			case Attribute::Type_IntegerArray:
				if (validateChoice(m_default.getInteger(), attribute.getIntegerArray()) == false)
				{
					TT_PANIC("Property '%s': default value '%d' is not part of 'choice' attribute",
							 m_name.c_str(), m_default.getInteger());
					return false;
				}
				break;
			
			case Attribute::Type_FloatArray:
				if (validateChoice(m_default.getFloat(), attribute.getFloatArray()) == false)
				{
					TT_PANIC("Property '%s': default value '%f' is not part of 'choice' attribute",
							 m_name.c_str(), m_default.getFloat());
					return false;
				}
				break;
			
			case Attribute::Type_BoolArray:
				if (validateChoice(m_default.getBool(), attribute.getBoolArray()) == false)
				{
					TT_PANIC("Property '%s': default value '%d' is not part of 'choice' attribute",
							 m_name.c_str(), m_default.getBool());
					return false;
				}
				break;
			
			case Attribute::Type_StringArray:
				if (validateChoice(m_default.getString(), attribute.getStringArray()) == false)
				{
					TT_PANIC("Property '%s': default value '%s' is not part of 'choice' attribute",
							 m_name.c_str(), m_default.getString().c_str());
					return false;
				}
				break;
			
			default:
				TT_PANIC("Property '%s' has unhandled choice array type %d", m_name.c_str(), type);
				return false;
			}
		}
		
		// Attribute valid, set it
		m_choice = attribute;
	}
	else if (name == "description")
	{
		// Validate type
		if (attribute.getType() != Attribute::Type_String)
		{
			TT_PANIC("Property '%s' has attribute 'description' "
			         "with type '%s', expected '%s'", m_name.c_str(),
			         Attribute::getTypeName(attribute.getType()),
			         Attribute::getTypeName(Attribute::Type_String));
			return false;
		}
		
		// Attribute valid, set it
		m_description = attribute.getString();
	}
	else if (name == "filter")
	{
		// Filter only valid with Entity(Array)
		if (EntityProperty::isEntityType(m_type) == false)
		{
			TT_PANIC("Property '%s' has type '%s' which has no support for attribute 'filter'",
			         m_name.c_str(), getTypeName(m_type));
			return false;
		}
		
		// Validate type
		if (attribute.getType() != Attribute::Type_StringArray)
		{
			TT_PANIC("Property '%s' has attribute 'filter' of type '%s', expected '%s'",
			         m_name.c_str(), Attribute::getTypeName(attribute.getType()),
			         Attribute::getTypeName(Attribute::Type_StringArray));
			return false;
		}
		
		// Attribute valid, set it
		const tt::str::Strings& filter(attribute.getStringArray());
		m_filter = tt::str::StringSet(filter.begin(), filter.end());
	}
	else if (name == "conditional")
	{
		// Validate type
		if (attribute.getType() != Attribute::Type_String)
		{
			TT_PANIC("Property '%s' has attribute 'conditional' of type '%s', expected '%s'",
			         m_name.c_str(), Attribute::getTypeName(attribute.getType()),
			         Attribute::getTypeName(Attribute::Type_String));
			return false;
		}
		
		// Attribute valid, set it
		m_conditional = Conditional::create(attribute.getString());
	}
	else if (name == "referenceColor")
	{
		// Type should be Entity or EntityArray
		if (EntityProperty::isEntityType(m_type) == false)
		{
			TT_PANIC("Property '%s' has type '%s' which has no support for attribute 'referenceColor'",
			         m_name.c_str(), getTypeName(m_type));
			return false;
		}
		
		// Validate type
		if (attribute.getType() != Attribute::Type_IntegerArray)
		{
			TT_PANIC("Property '%s' has attribute 'referenceColor' of type '%s', expected '%s'",
			         m_name.c_str(), Attribute::getTypeName(attribute.getType()),
			         Attribute::getTypeName(Attribute::Type_IntegerArray));
			return false;
		}
		
		// Attribute valid, set it
		const script::attributes::Attribute::IntegerArray& components(attribute.getIntegerArray());
		
		TT_ASSERTMSG(components.size() == 3 || components.size() == 4,
		             "Property type '%s': referenceColor array should have 3 or 4 elements: "
		             "color R, G, B and optional A. This type's array has %d elements.",
		             m_name.c_str(), static_cast<s32>(components.size()));
		
		if (components.size() >= 3)
		{
			m_referenceColor.r = static_cast<u8>(components[0]);
			m_referenceColor.g = static_cast<u8>(components[1]);
			m_referenceColor.b = static_cast<u8>(components[2]);
			m_referenceColor.a = (components.size() >= 4) ? static_cast<u8>(components[3]) : 255;
		}
	}
	else
	{
		// Not implemented yet
		TT_PANIC("Property '%s' has unknown attribute '%s'", m_name.c_str(), name.c_str());
		return false;
	}
	
	return true;
}


bool EntityProperty::validateAndSetDefault(const script::attributes::AttributeCollection& p_attributes)
{
	using namespace script::attributes;
	
	if (p_attributes.contains("_default") == false)
	{
		TT_PANIC("Property '%s' has no default value", m_name.c_str());
		return false;
	}
	
	Attribute defaultValue = p_attributes.get("_default");
	
	// Validate that the type of the default, matches the expected type
	Attribute::Type type        = defaultValue.getType();
	Attribute::Type correctType = Attribute::Type_None;
	switch (m_type)
	{
	case Type_Integer:              correctType = Attribute::Type_Integer;      break;
	case Type_Float:                correctType = Attribute::Type_Float;        break;
	case Type_Bool:                 correctType = Attribute::Type_Bool;         break;
	case Type_String:               correctType = Attribute::Type_String;       break;
	case Type_IntegerArray:         correctType = Attribute::Type_IntegerArray; break;
	case Type_FloatArray:           correctType = Attribute::Type_FloatArray;   break;
	case Type_BoolArray:            correctType = Attribute::Type_BoolArray;    break;
	case Type_StringArray:          correctType = Attribute::Type_StringArray;  break;
	case Type_Entity:               correctType = Attribute::Type_Null;         break;
	case Type_EntityArray:          correctType = Attribute::Type_Null;         break;
	case Type_EntityID:             correctType = Attribute::Type_Null;         break;
	case Type_EntityIDArray:        correctType = Attribute::Type_Null;         break;
	case Type_DelayedEntityID:      correctType = Attribute::Type_Null;         break;
	case Type_DelayedEntityIDArray: correctType = Attribute::Type_Null;         break;
	case Type_ColorRGB:             correctType = Attribute::Type_Null;         break;
	case Type_ColorRGBA:            correctType = Attribute::Type_Null;         break;
		
	default:
		TT_PANIC("Property '%s': Unknown type '%d", m_name.c_str(), m_type);
		return false;
	}
	
	// Implicit cast of int to float
	if (type == Attribute::Type_Integer && m_type == Type_Float)
	{
		// Cast to float
		defaultValue.setFloat(static_cast<real>(defaultValue.getInteger()));
		type = Attribute::Type_Float;
	}
	
	// If type is null,
	if (correctType == type || (type == Attribute::Type_Null && p_attributes.contains("choice")))
	{
		// All correct, now set the default value of this property
		m_default = defaultValue;
		return true;
	}
	
	TT_PANIC("Property '%s': default value is of type '%s', expected type '%s'",
	         m_name.c_str(),
	         Attribute::getTypeName(type),
	         Attribute::getTypeName(correctType));
	
	return false;
}

// Namespace end
}
}
}
