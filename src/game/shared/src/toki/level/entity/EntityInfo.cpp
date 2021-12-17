#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>

#include <toki/game/movement/MovementSet.h>
#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/level/entity/EntityInfo.h>
#include <toki/level/entity/EntityProperty.h>


namespace toki {
namespace level {
namespace entity {

const EntityInfo EntityInfo::invalid;


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityInfo::EntityInfo()
:
m_name(),
m_displayName(),
m_placeable(Placeable_Hidden),
m_editorImage(),
m_libraryImage(),
m_movementSetName(),
m_order(std::numeric_limits<s32>::max()),
m_ignoreSpawnSections(false),
m_collisionRect(),
m_pathFindAgentRadius(-1.0f),
m_pathCrowdSeparation(false),
m_maxEntityCount(-1),
m_workshopTags(),
m_hasFixedSizeShapeRectangle(false),
m_hasFixedSizeShapeCircle(false),
m_fixedSizeShapeRadius(0.0f),
m_fixedSizeShapeRectangle(tt::math::Vector2::zero),
m_sizeShapeColor(0, 0, 0, 0),
m_sizeShapeFromEntityCenter(true),
m_group(),
m_properties()
{
}


EntityInfo EntityInfo::create(const game::script::EntityScriptClassPtr& p_entityClass)
{
	if (p_entityClass == 0)
	{
		TT_PANIC("Cannot create EntityInfo when EntityScriptClassPtr is 0");
		return EntityInfo();
	}
	
	EntityInfo info;
	
	using namespace script::attributes;
	const ClassAttributes& attributes = p_entityClass->getAttributes();
	
	AttributeCollection root = attributes.getRootAttributes();
	
	if (root.contains("_className") == false)
	{
		TT_PANIC("Couldn't find _className in root attributes in class: '%s'",
		         p_entityClass->getName().c_str());
		return EntityInfo::invalid;
	}
	info.m_name = root.get("_className").getString();
	
	info.m_displayName = root.contains("displayName") ? root.get("displayName").getString() : info.m_name;
	
	info.m_placeable = Placeable_Hidden;
	if (root.contains("placeable"))
	{
		info.m_placeable = static_cast<Placeable>(root.get("placeable").getInteger());
		if (isValidPlaceable(info.m_placeable) == false)
		{
			TT_PANIC("%s: Invalid 'placeable' class attribute value: %d. Using Placeable_Hidden as default.",
			         info.m_name.c_str(), info.m_placeable);
			info.m_placeable = Placeable_Hidden;
		}
	}
	
	// Check for attributes required for the editor, but only if this entity is placeable.
	if (info.m_placeable != Placeable_Hidden)
	{
		if (root.contains("editorImage") == false)
		{
			TT_PANIC("Couldn't find attribute 'editorImage' in root attributes in class: '%s'",
			         p_entityClass->getName().c_str());
			return EntityInfo::invalid;
		}
		
		if (root.contains("libraryImage") == false)
		{
			TT_PANIC("Couldn't find attribute 'libraryImage' in root attributes in class: '%s'",
			         p_entityClass->getName().c_str());
			return EntityInfo::invalid;
		}
		
		// FIXME: get rid of debugImage
		//TT_ASSERTMSG(root.contains("debugImage"),
		//			 "Couldn't find attribute 'debugImage' in root attributes in class: '%s'",
		//			 p_entityClass->getName().c_str());
	}
	
	info.m_editorImage         = root.contains("editorImage")         ? root.get("editorImage"        ).getString() : std::string();
	info.m_libraryImage        = root.contains("libraryImage")        ? root.get("libraryImage"       ).getString() : std::string();
	info.m_movementSetName     = root.contains("movementset")         ? root.get("movementset"        ).getString() : std::string();
	info.m_pathFindAgentRadius = root.contains("pathFindAgentRadius") ? root.get("pathFindAgentRadius").getFloat()  : -1.0f;
	info.m_pathCrowdSeparation = root.contains("pathCrowdSeparation") ? root.get("pathCrowdSeparation").getBool()   : false;
	info.m_maxEntityCount      = root.contains("maxEntityCount")      ? root.get("maxEntityCount"     ).getInteger() : -1;
	info.m_workshopTags        = root.contains("workshopTags")        ? root.get("workshopTags"       ).getStringArray() : tt::str::Strings();
	info.m_group               = root.contains("group")               ? root.get("group"              ).getString() : std::string();
	
	info.m_sizeShapeFromEntityCenter = root.contains("sizeShapeFromEntityCenter") ? root.get("sizeShapeFromEntityCenter").getBool() : true;
	
	if (root.contains("sizeShapeColor"))
	{
		script::attributes::Attribute attr = root.get("sizeShapeColor");
		const std::vector<s32>& components(attr.getIntegerArray());
		TT_ASSERTMSG(components.size() == 3 || components.size() == 4,
		             "Entity type '%s': sizeShapeColor array should have 3 or 4 elements: "
		             "color R, G, B and optional A. This type's array has %d elements.",
		             info.m_name.c_str(), static_cast<s32>(components.size()));
		if (components.size() >= 3)
		{
			info.m_sizeShapeColor.r = static_cast<u8>(components[0]);
			info.m_sizeShapeColor.g = static_cast<u8>(components[1]);
			info.m_sizeShapeColor.b = static_cast<u8>(components[2]);
			info.m_sizeShapeColor.a = (components.size() >= 4) ? static_cast<u8>(components[3]) : 255;
		}
	}
	
	if (root.contains("sizeShapeRadius"))
	{
		info.m_fixedSizeShapeRadius = root.get("sizeShapeRadius").getFloat();
		info.m_hasFixedSizeShapeCircle = info.m_fixedSizeShapeRadius > 0.0f;
	}

	if (root.contains("sizeShapeRect"))
	{
		script::attributes::Attribute attr = root.get("sizeShapeRect");
		const std::vector<real>& floats(attr.getFloatArray());
		TT_ASSERTMSG(floats.size() == 2,
		             "Entity type '%s': sizeShapeRect array should have 2 elements: "
		             "width, height. This type's array has %d elements.",
		             info.m_name.c_str(), s32(floats.size()));
		if (floats.size() == 2)
		{
			info.m_fixedSizeShapeRectangle.x = floats[0];
			info.m_fixedSizeShapeRectangle.y = floats[1];
			info.m_hasFixedSizeShapeRectangle = floats[0] > 0.0f && floats[1] > 0.0f;
		}
	}
	
	if (root.contains("order"))
	{
		info.m_order = root.get("order").getInteger();
	}
	
	if (root.contains("ignoreSpawnSections"))
	{
		info.m_ignoreSpawnSections = root.get("ignoreSpawnSections").getBool();
	}
	
	bool useDefaultCollisionRect = true;
	if (root.contains("collisionRect"))
	{
		script::attributes::Attribute attr = root.get("collisionRect");
		const std::vector<real>& floats(attr.getFloatArray());
		TT_ASSERTMSG(floats.size() == 4,
		             "Entity type '%s': collisionRect array should have 4 elements: "
		             "center X, center Y, width, height. This type's array has %d elements.",
		             info.m_name.c_str(), s32(floats.size()));
		if (floats.size() == 4)
		{
			info.m_collisionRect = tt::math::VectorRect(
				tt::math::Vector2(floats[0], floats[1]),
				floats[2],
				floats[3]);
			info.m_collisionRect.setCenterPosition(info.m_collisionRect.getPosition());
			useDefaultCollisionRect = false;
		}
	}
	
	if (useDefaultCollisionRect)
	{
		// Default to a collision rect of slightly less than 2x2 tiles
		info.m_collisionRect = tt::math::VectorRect(tt::math::Vector2(-0.9f, 0.1f), 1.8f, 1.8f);
	}
	
	const MemberAttributesCollection& members = attributes.getMemberAttributesCollection();
	for (MemberAttributesCollection::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		// skip root attributes
		if ((*it).first.empty() == false)
		{
			info.m_properties.push_back(EntityProperty((*it).first, (*it).second));
		}
	}
	
	return info;
}


bool EntityInfo::hasProperty(const std::string& p_name) const
{
	for (EntityProperties::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		if ((*it).getName() == p_name)
		{
			return true;
		}
	}
	
	return false;
}


const EntityProperty& EntityInfo::getProperty(const std::string& p_name) const
{
	for (EntityProperties::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		if ((*it).getName() == p_name)
		{
			return (*it);
		}
	}
	
	return EntityProperty::ms_emptyEntityProperty;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

//

// Namespace end
}
}
}
