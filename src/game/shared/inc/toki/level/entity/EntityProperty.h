#if !defined(INC_TOKI_LEVEL_ENTITY_ENTITYPROPERTY_H)
#define INC_TOKI_LEVEL_ENTITY_ENTITYPROPERTY_H


#include <tt/engine/renderer/fwd.h>

#include <algorithm>
#include <vector>

#include <toki/script/attributes/ClassAttributes.h>


namespace toki {
namespace level {
namespace entity {

class EntityProperty;
typedef std::vector<EntityProperty> EntityProperties;

/*! \brief Encapsulates the information related to an entity type (based on a script file). */
class EntityProperty
{
public:
	class Conditional
	{
	public:
		enum Operator
		{
			Operator_Equals,
			Operator_NotEquals,
			Operator_Contains,
			Operator_NotContains,
			
			Operator_Count,
			Operator_Invalid
		};
		
		Conditional();
		static Conditional create(const std::string p_conditional);
		
		// Tests whether or the internal value matches p_value given the operator
		bool test(const std::string& p_value) const;
		
		inline bool isValid() const { return m_operator < Operator_Count; }
		inline const std::string& getTargetPropertyName() const { return m_targetPropertyName; }
		
	private:
		std::string m_targetPropertyName;
		Operator    m_operator;
		std::string m_value;
	};
	
	enum Type
	{
		Type_None,
		
		// Squirrel types, except Type_Null (unused)
		Type_Integer,
		Type_Float,
		Type_Bool,
		Type_String,
		Type_IntegerArray,
		Type_FloatArray,
		Type_BoolArray,
		Type_StringArray,
		
		// Custom types
		Type_Entity,
		Type_EntityArray,
		Type_EntityID,
		Type_EntityIDArray,
		Type_DelayedEntityID,
		Type_DelayedEntityIDArray,
		Type_ColorRGB,
		Type_ColorRGBA,
		
		Type_Count  // Not an actual type: this is how many Type values there are
	};
	
	
	inline EntityProperty()
	:
	m_type(Type_None)
	{}
	
	EntityProperty(const std::string& p_name, const script::attributes::AttributeCollection& p_attributes);
	
	inline Type getType() const { return m_type; }
	inline const std::string& getName() const { return m_name; }
	inline const std::string& getDescription() const { return m_description; }
	inline bool hasGroup() const { return m_group.empty() == false; }
	inline const std::string& getGroup() const { return m_group; }
	inline const tt::engine::renderer::ColorRGBA& getReferenceColor() const { return m_referenceColor; }
	inline const script::attributes::Attribute& getDefault() const { return m_default; }
	
	inline bool hasMin() const { return m_min.getType() != script::attributes::Attribute::Type_None; }
	inline const script::attributes::Attribute& getMin() const { return m_min; }
	
	inline bool hasMax() const { return m_max.getType() != script::attributes::Attribute::Type_None; }
	inline const script::attributes::Attribute& getMax() const { return m_max; }
	
	inline bool hasChoice() const { return m_choice.getType() != script::attributes::Attribute::Type_None; }
	inline const script::attributes::Attribute& getChoice() const { return m_choice; }
	
	inline bool hasFilter() const { return m_filter.empty() == false; }
	inline const tt::str::StringSet& getFilter() const { return m_filter; }
	
	inline bool hasConditional() const { return m_conditional.isValid(); }
	inline const Conditional& getConditional() const { return m_conditional; }
	
	static const char* getTypeName(Type p_type);
	static Type        getTypeFromName(const std::string& p_name);
	static bool        isArrayType(Type p_type);
	static bool        isEntityType(Type p_type);
	
	static EntityProperty ms_emptyEntityProperty;
	
	bool validate(const std::string& p_value) const;
	
private:
	template <typename T>
	inline bool validateChoice(const T& p_choice, const std::vector<T>& p_container) const
	{
		//TT_ASSERT(hasChoice());
		return std::find(p_container.begin(), p_container.end(), p_choice) != p_container.end();
	}
	
	template <typename T>
	inline bool validateMinMax(const T& p_value, const T& p_min, const T& p_max) const
	{
		TT_ASSERT(hasChoice() == false);
		if (hasMin() && hasMax())
		{
			return p_value >= p_min && p_value <= p_max;
		}
		else if (hasMin())
		{
			return p_value >= p_min;
		}
		else if (hasMax())
		{
			return p_value <= p_max;
		}
		return true;
	}
	
	bool checkForAllRequiredAttributes(const script::attributes::AttributeCollection& p_attributes) const;
	bool validateAndSetDefault(const script::attributes::AttributeCollection& p_attributes);
	bool validateAndSetAttribute(const script::attributes::Attribute& p_attributes);
	
	Type                            m_type;
	std::string                     m_name;
	std::string                     m_description;
	std::string                     m_group;
	tt::engine::renderer::ColorRGBA m_referenceColor;
	
	script::attributes::Attribute m_default;
	script::attributes::Attribute m_min;
	script::attributes::Attribute m_max;
	script::attributes::Attribute m_choice;
	tt::str::StringSet            m_filter;
	Conditional                   m_conditional;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_ENTITYPROPERTY_H)
