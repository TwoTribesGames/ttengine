#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <toki/game/editor/helpers.h>
#include <toki/game/Camera.h>
#include <toki/level/entity/editor/EntityInstanceEditorRepresentation.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/LevelData.h>
#include <toki/level/Note.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {
namespace entity {
namespace editor {

static std::wstring g_referenceTargetMissingText;


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityInstanceEditorRepresentation::EntityInstanceEditorRepresentation(
		const EntityInstance* p_entity)
:
m_entity(p_entity),
m_shouldRescanProperties(true),
m_posOffset(tt::math::Vector2::zero),
m_entityReferences(),
m_imageState(ImageState_Normal),
m_imageQuad(),
m_imageSize(tt::math::Vector2::allOne),
m_imageRotation(0.0f),
m_imageScale(1.0f),
m_editorWarnings(),
m_editorWarningIconQuad(tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TextureCache::get("entity_properties_warning", "textures.editor.ui"))),
m_sizeShapeType(SizeShapeType_Invalid),
m_sizeShapeRect(-1.0f, -1.0f),
m_sizeShapeRadius(-1.0f),
m_sizeShape(),
m_sizeShapeFromCenter(true)
{
	TT_NULL_ASSERT(m_entity);
	
	if (g_referenceTargetMissingText.empty())
	{
		g_referenceTargetMissingText = L" " +
				game::editor::translateString("ENTITY_PROPERTY_REFERENCE_TARGET_MISSING");
	}
	
	// Create an editor image quad for this entity
	const EntityInfo* typeInfo = AppGlobal::getEntityLibrary().getEntityInfo(m_entity->getType());
	
	std::string imageID("unsupported_by_upgrade");
	std::string imageNS("textures.editor");
	
	if (typeInfo != 0 && typeInfo->getEditorImage().empty() == false)
	{
		tt::str::Strings splitName(tt::str::explode(typeInfo->getEditorImage(), ".", true));
		imageID = splitName.back();
		splitName.pop_back();
		imageNS = "textures" + ((splitName.empty() ? "" : ".") + tt::str::implode(splitName, "."));
	}
	
	using namespace tt::engine::renderer;
	TexturePtr typeTex = TextureCache::get(imageID, imageNS, true);
	m_imageQuad        = QuadSprite::createQuad(typeTex, ColorRGB::white);
	if (typeTex != 0)
	{
		typeTex->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);
		
		m_imageQuad->setWidth( typeTex->getWidth()  / 32.0f);
		m_imageQuad->setHeight(typeTex->getHeight() / 32.0f);
	}
	else
	{
		m_imageQuad->setWidth (1.0f);
		m_imageQuad->setHeight(1.0f);
	}
	
	m_imageSize.setValues(m_imageQuad->getWidth(), m_imageQuad->getHeight());
	
	// Perform an initial parse of the entity's properties
	rescanProperties();

	m_editorWarningIconQuad->setWidth (1.0f);
	m_editorWarningIconQuad->setHeight(1.0f);
}


EntityInstanceEditorRepresentation::~EntityInstanceEditorRepresentation()
{
}


void EntityInstanceEditorRepresentation::update(real p_deltaTime, const LevelDataPtr& p_levelData)
{
	if (m_shouldRescanProperties)
	{
		rescanProperties();
	}
	
	const tt::math::Vector2 pos(m_entity->getPosition() + m_posOffset);
	
	// TODO: If we want each entity to render its own image (which definitely has my preference),
	//       all the 'states' the image can be in need to be reflected:
	//       - (DONE) Entity selected / not selected
	//       - (DONE) Entity not available for picking
	//       - Overlay icon for entities whose properties were changed by script
	//       - Position offset for "floating sections"
	//       Furthermore, the "level section" entities require more tweaks
	
	m_imageQuad->setRotation(-m_imageRotation);
	m_imageQuad->setScale(m_imageScale);
	m_imageQuad->setPosition(pos.x, -(pos.y + (m_imageSize.y * 0.5f)), 0.0f);
	m_imageQuad->update();
	
	// Update entity-to-entity references
	for (EntityReferencesPerProperty::iterator it = m_entityReferences.begin(); it != m_entityReferences.end(); ++it)
	{
		EntityReferences& refs((*it).second);
		
		for (EntityReferences::iterator refIt = refs.begin(); refIt != refs.end(); ++refIt)
		{
			EntityReference& ref(*refIt);
			
			if (ref.targetID < 0) continue;  // ignore references with no target set
			
			if (ref.lineBuffer == 0)
			{
				using namespace tt::engine::renderer;
				const s32 vertexCount = 6;  // three lines, two vertices per line
				ref.lineBuffer.reset(new TrianglestripBuffer(
						vertexCount, 1, TexturePtr(), BatchFlagTrianglestrip_UseVertexColor,
 						TrianglestripBuffer::PrimitiveType_Lines));
				ref.lineBuffer->resize<1>(vertexCount);
			}
			
			EntityInstancePtr targetEntity = p_levelData->getEntityByID(ref.targetID);
			if (targetEntity != 0)
			{
				// FIXME: Should we also check types? Does add extra type retrieval overhead...
				ref.status           = EntityReference::Status_Valid;
				ref.targetPos        = targetEntity->getPosition();
				ref.targetIsSelected = false;
				
				EntityInstanceEditorRepresentation* targetEditorRep = targetEntity->getEditorRepresentation();
				if (targetEditorRep != 0)
				{
					ref.targetPos   += targetEditorRep->getPosOffset();          // take offset into account
					ref.targetPos.y += targetEditorRep->getImageSize().y * 0.5f; // point to the center of the image
					ref.targetIsSelected = targetEditorRep->m_imageState == ImageState_Selected ||
					                       targetEditorRep->m_imageState == ImageState_SelectedHidden;
				}
				
				// Update the arrow lines
				using tt::math::Vector3;
				// FIXME: If we ever support a "placement offset", this image position offset needs to be variable
				const Vector3 ourCenter(m_entity->getPosition() + tt::math::Vector2(0.0f, getImageSize().y * 0.5f));
				const Vector3 targetPos(ref.targetPos);
				
				// - Line from source to target
				ref.lineBuffer->modifyVtx<1>(0).setPosition(ourCenter);
				ref.lineBuffer->modifyVtx<1>(1).setPosition(targetPos);
				
				// - Arrow head line 1
				const real angle = tt::math::atan2(targetPos.y - ourCenter.y,
				                                   targetPos.x - ourCenter.x) + tt::math::pi;
				
				static const real length       = 0.5f;
				static const real arrowDegrees = 0.5f;
				
				ref.lineBuffer->modifyVtx<1>(2).setPosition(
						targetPos.x + length * tt::math::cos(angle - arrowDegrees),
						targetPos.y + length * tt::math::sin(angle - arrowDegrees),
						0.0f);
				ref.lineBuffer->modifyVtx<1>(3).setPosition(targetPos);
				
				// - Arrow head line 2
				ref.lineBuffer->modifyVtx<1>(4).setPosition(
						targetPos.x + length * tt::math::cos(angle + arrowDegrees),
						targetPos.y + length * tt::math::sin(angle + arrowDegrees),
						0.0f);
				ref.lineBuffer->modifyVtx<1>(5).setPosition(targetPos);
				
				const s32 vertexCount = ref.lineBuffer->getTotalVerticesCount();
				for (s32 i = 0; i < vertexCount; ++i)
				{
					ref.lineBuffer->modifyVtx<1>(i).setColor(ref.color);
				}
			}
			else
			{
				ref.status = EntityReference::Status_TargetDoesNotExist;
				
				// Not required anymore as we don't render the invalid references anymore
				/*
				// Change the color of the lines to indicate the reference status
				const s32 vertexCount = ref.lineBuffer->getTotalVerticesCount();
				for (s32 i = 0; i < vertexCount; ++i)
				{
					ref.lineBuffer->modifyVtx<1>(i).setColor(tt::engine::renderer::ColorRGB::red);
				}
				*/
			}
			
			ref.lineBuffer->applyChanges();
		}
	}
	
	// Update the size rect quad based on the current size
	if (m_sizeShape != 0)
	{
		if (m_sizeShapeType == SizeShapeType_Rectangle)
		{
			tt::math::Vector2 size(m_sizeShapeRect);
			tt::math::Vector2 rectPos(pos.x, pos.y + (size.y * 0.5f));
		
			if (m_sizeShapeFromCenter)
			{
				rectPos.y += (getImageSize().y - m_sizeShapeRect.y) * 0.5f;
			}
			
			if (size.x == 0.0f)
			{
				size.x    = static_cast<real>(p_levelData->getLevelWidth());
				rectPos.x = size.x * 0.5f;
			}
			if (size.y == 0.0f)
			{
				size.y    = static_cast<real>(p_levelData->getLevelHeight());
				rectPos.y = size.y * 0.5f;
			}
		
			m_sizeShape->setWidth   (size.x);
			m_sizeShape->setHeight  (size.y);
			m_sizeShape->setPosition(rectPos.x, -rectPos.y, 0.0f);
		}
		else if (m_sizeShapeType == SizeShapeType_Circle)
		{
			//tt::math::Vector2 size(m_sizeShapeRect);
			tt::math::Vector2 quadPos(pos.x, pos.y + m_sizeShapeRadius);
			
			if (m_sizeShapeFromCenter)
			{
				quadPos.y += (getImageSize().y * 0.5f) - m_sizeShapeRadius;
			}
			
			m_sizeShape->setWidth   (m_sizeShapeRadius * 2.0f);
			m_sizeShape->setHeight  (m_sizeShapeRadius * 2.0f);
			m_sizeShape->setPosition(quadPos.x, -quadPos.y, 0.0f);
		}
		else
		{
			TT_PANIC("Unknown sizeshape type '%d'", m_sizeShapeType);
		}
		m_sizeShape->update();
	}
	
	// Update warning graphics.
	updatePositionOfEditorWarnings();
	for (level::Notes::const_iterator it = m_editorWarnings.begin(); it != m_editorWarnings.end(); ++it)
	{
		// TODO: update positions
		const level::NotePtr ptr = (*it);
		ptr->update(p_deltaTime);
	}
}


void EntityInstanceEditorRepresentation::renderBack(const game::Camera& /*p_camera*/,
                                                    const RenderFlags&  p_renderFlags)
{
	if (p_renderFlags.checkFlag(RenderFlag_SizeShapes) && m_sizeShape != 0)
	{
		m_sizeShape->render();
	}
	
	if (p_renderFlags.checkFlag(RenderFlag_Image))
	{
		switch (m_imageState)
		{
		case ImageState_Normal:
			m_imageQuad->setColor(tt::engine::renderer::ColorRGBA(255, 255, 255, 255));
			break;
			
		case ImageState_Selected:
			m_imageQuad->setColor(tt::engine::renderer::ColorRGBA(255, 0, 0, 255));
			break;
		
		case ImageState_SelectedHidden:
			m_imageQuad->setColor(tt::engine::renderer::ColorRGBA(255, 0, 0, 31));
			break;
		
		case ImageState_Hidden:
			m_imageQuad->setColor(tt::engine::renderer::ColorRGBA(255, 255, 255, 31));
			break;
		
		default:
			TT_PANIC("Unhandled m_imageState '%d'", m_imageState);
			break;
		}
		
		m_imageQuad->render();
	}
}


void EntityInstanceEditorRepresentation::renderFront(const game::Camera& p_camera,
                                                     const RenderFlags&  p_renderFlags)
{
	tt::engine::debug::DebugRendererPtr debug(tt::engine::renderer::Renderer::getInstance()->getDebug());
	
	const bool renderAllReferencesForThisEntity = p_renderFlags.checkFlag(RenderFlag_AllEntityReferences) ||
		m_imageState == ImageState_Selected || m_imageState == ImageState_SelectedHidden;
	
	{
		using tt::math::Vector2;
		
		// FIXME: If we ever support a "placement offset", this image position offset needs to be variable
		const Vector2 ourCenter(m_entity->getPosition() + Vector2(0.0f, getImageSize().y * 0.5f));
		
		for (EntityReferencesPerProperty::iterator it = m_entityReferences.begin();
		     it != m_entityReferences.end(); ++it)
		{
			const EntityReferences& refs((*it).second);
			for (EntityReferences::const_iterator refIt = refs.begin(); refIt != refs.end(); ++refIt)
			{
				const EntityReference& ref(*refIt);
				if (ref.targetID < 0 || ref.status == EntityReference::Status_TargetDoesNotExist)
				{
					continue;  // ignore references with no or invalid target set 
				}
				
				if ((renderAllReferencesForThisEntity || ref.targetIsSelected) == false)
				{
					continue;
				}
				
				if (ref.lineBuffer != 0)
				{
					ref.lineBuffer->render();
				}
				
				if (p_renderFlags.checkFlag(RenderFlag_EntityReferencesLabels))
				{
					// Add a label for the property name
					std::wstring propName(tt::str::widen((*it).first));
					if (ref.status == EntityReference::Status_TargetDoesNotExist)
					{
						propName += g_referenceTargetMissingText;
					}
					
					const Vector2 midway(ourCenter + ((ref.targetPos - ourCenter) * 0.5f));
					
					const tt::engine::renderer::ColorRGBA lineColor(ref.status == EntityReference::Status_Valid ?
							ref.color : tt::engine::renderer::ColorRGB::red);
					
					const tt::math::Point2 textPos(p_camera.worldToScreen(midway));
					debug->renderText(propName, textPos.x + 7, textPos.y + 7, lineColor);
				}
			}
		}
	}
	
	// Render entity ID
	if (p_renderFlags.checkFlag(RenderFlag_AllEntityIDs))
	{
		const std::string idText(tt::str::toStr(m_entity->getID()));
		const tt::math::Point2 textPos(p_camera.worldToScreen(m_entity->getPosition()));
		debug->renderText(idText, textPos.x - 1, textPos.y + 1, tt::engine::renderer::ColorRGB::black);
		debug->renderText(idText, textPos.x, textPos.y, tt::engine::renderer::ColorRGB::white);
	}
}


void EntityInstanceEditorRepresentation::renderEditorWarnings()
{
	for (level::Notes::const_iterator it = m_editorWarnings.begin(); it != m_editorWarnings.end(); ++it)
	{
		const level::NotePtr ptr = (*it);
		ptr->render();
	}
}


void EntityInstanceEditorRepresentation::renderEditorWarningsIcon()
{
	if (m_editorWarnings.empty() == false)
	{
		m_editorWarningIconQuad->render();
	}
}


tt::math::Vector2 EntityInstanceEditorRepresentation::getSizeShapeSize() const
{
	switch (m_sizeShapeType)
	{
	case SizeShapeType_Rectangle:
		return m_sizeShapeRect; 
		
	case SizeShapeType_Circle:
		return tt::math::Vector2(m_sizeShapeRadius * 2, m_sizeShapeRadius * 2);
		
	default:
		TT_PANIC("Unhandled sizeshape type '%d'", m_sizeShapeType);
		break;
	}
	
	return tt::math::Vector2::zero;
}


void EntityInstanceEditorRepresentation::onPropertiesChanged()
{
	m_shouldRescanProperties = true;
}


void EntityInstanceEditorRepresentation::addEditorWarning(const std::wstring& p_warningStr)
{
	m_editorWarnings.push_back(level::Note::create(m_entity->getPosition(),
	                                               p_warningStr,
	                                               level::Note::VisualType_Warning));
	updatePositionOfEditorWarnings();
}


void EntityInstanceEditorRepresentation::clearEditorWarnings()
{
	m_editorWarnings.clear();
}


void EntityInstanceEditorRepresentation::resetLineBuffers()
{
	rescanProperties();
	
	// Reset entity-to-entity references linebuffers
	for (EntityReferencesPerProperty::iterator it = m_entityReferences.begin(); it != m_entityReferences.end(); ++it)
	{
		EntityReferences& refs((*it).second);
		
		for (EntityReferences::iterator refIt = refs.begin(); refIt != refs.end(); ++refIt)
		{
			EntityReference& ref(*refIt);
			ref.lineBuffer.reset();
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityInstanceEditorRepresentation::updatePositionOfEditorWarnings()
{
	// Position Icon to the top right corner.
	m_editorWarningIconQuad->setPosition(m_entity->getPosition().x   + (m_imageSize.x * 0.5f),
	                                     -(m_entity->getPosition().y + m_imageSize.y),
	                                     0.0f);
	m_editorWarningIconQuad->update();
	
	// Position all Warnings below the entity. (center aligned)
	tt::math::Vector2 pos = m_entity->getPosition();
	if (m_editorWarnings.empty() == false)
	{
		// Center align the warning notes (They should all have the same width.)
		pos.x -= (m_editorWarnings.front()->getWorldRect().getWidth() * 0.5f);
	}

	const real warningSpacing = 0.25f;
	for (level::Notes::const_iterator it = m_editorWarnings.begin(); it != m_editorWarnings.end(); ++it)
	{
		const level::NotePtr ptr = (*it);
		// Move with own height + some spacing. (Position is bottom left.)
		pos.y -= ptr->getWorldRect().getHeight() + warningSpacing;
		ptr->setPosition(pos);
	}
}


void EntityInstanceEditorRepresentation::rescanProperties()
{
	const EntityInfo* typeInfo = AppGlobal::getEntityLibrary().getEntityInfo(m_entity->getType());
	bool needSizeShapeGraphic = false;
	
	tt::math::Vector2 propsRect(-1.0f, -1.0f);
	real              propsRadius(-1.0f);
	
	if (typeInfo != 0)
	{
		needSizeShapeGraphic  = typeInfo->getSizeShapeColor().a > 0;
		m_sizeShapeFromCenter = typeInfo->isSizeShapeFromEntityCenter();
		
		// Start out with entity type defaults
		{
			const EntityProperties& typeProps(typeInfo->getProperties());
			for (EntityProperties::const_iterator it = typeProps.begin();
			     it != typeProps.end(); ++it)
			{
				if ((*it).getName() == "width")
				{
					if ((*it).getDefault().getType() == script::attributes::Attribute::Type_Float)
					{
						propsRect.x = (*it).getDefault().getFloat();
					}
					else
					{
						propsRect.x = static_cast<real>((*it).getDefault().getInteger());
					}
				}
				else if ((*it).getName() == "height")
				{
					if ((*it).getDefault().getType() == script::attributes::Attribute::Type_Float)
					{
						propsRect.y = (*it).getDefault().getFloat();
					}
					else
					{
						propsRect.y = static_cast<real>((*it).getDefault().getInteger());
					}
				}
				else if ((*it).getName() == "radius")
				{
					if ((*it).getDefault().getType() == script::attributes::Attribute::Type_Float)
					{
						propsRadius = (*it).getDefault().getFloat();
					}
					else
					{
						propsRadius = static_cast<real>((*it).getDefault().getInteger());
					}
				}
				else if ((*it).getName() == "rotation")
				{
					if ((*it).getDefault().getType() == script::attributes::Attribute::Type_Float)
					{
						m_imageRotation = tt::math::degToRad((*it).getDefault().getFloat());
					}
					else
					{
						m_imageRotation = tt::math::degToRad(static_cast<real>((*it).getDefault().getInteger()));
					}
				}
				
				// FIXME: Try not to recreate line buffers too much...
				if (EntityProperty::isEntityType((*it).getType()))
				{
					m_entityReferences[(*it).getName()].clear();
				}
			}
		}
		
		// Scan the properties set for this instance
		const EntityInstance::Properties& props(m_entity->getProperties());
		for (EntityInstance::Properties::const_iterator it = props.begin();
		     it != props.end(); ++it)
		{
			const std::string& propName((*it).first);
			
			if (typeInfo->hasProperty(propName))
			{
				const std::string& propValue((*it).second);
				
				const level::entity::EntityProperty& typeProp(typeInfo->getProperty(propName));
				if (EntityProperty::isEntityType(typeProp.getType()))
				{
					EntityReferences& refs(m_entityReferences[propName]);
					TT_ASSERT(refs.empty());
					
					// FIXME: Try not to recreate line buffers too much...
					const tt::str::Strings ids(tt::str::explode(propValue, ","));
					for (tt::str::Strings::const_iterator idIt = ids.begin(); idIt != ids.end(); ++idIt)
					{
						EntityReference entityRef;
						entityRef.targetID = tt::str::parseS32(*idIt, 0);  // FIXME: Error checking?
						entityRef.status   = EntityReference::Status_TargetDoesNotExist;
						entityRef.color    = typeProp.getReferenceColor();
						refs.push_back(entityRef);
					}
				}
				
				// Override type-default width and height if one of these is set in the instance
				if (propName == "width")
				{
					TT_ASSERTMSG(typeProp.getType() == EntityProperty::Type_Integer ||
					             typeProp.getType() == EntityProperty::Type_Float,
					             "Expect 'width' property to be of type 'integer' or 'float'.");
					propsRect.x = tt::str::parseReal(propValue, 0);
				}
				else if (propName == "height")
				{
					TT_ASSERTMSG(typeProp.getType() == EntityProperty::Type_Integer ||
					             typeProp.getType() == EntityProperty::Type_Float,
					             "Expect 'height' property to be of type 'integer' or 'float'.");
					propsRect.y = tt::str::parseReal(propValue, 0);
				}
				else if (propName == "radius")
				{
					TT_ASSERTMSG(typeProp.getType() == EntityProperty::Type_Integer ||
					             typeProp.getType() == EntityProperty::Type_Float,
					             "Expect 'radius' property to be of type 'integer' or 'float'.");
					propsRadius = tt::str::parseReal(propValue, 0);
				}
				else if (propName == "rotation")
				{
					TT_ASSERTMSG(typeProp.getType() == EntityProperty::Type_Integer ||
					             typeProp.getType() == EntityProperty::Type_Float,
					             "Expect 'rotation' property to be of type 'integer' or 'float'.");
					m_imageRotation = tt::math::degToRad(tt::str::parseReal(propValue, 0));
				}
			}
		}
	}
	
	// Determine size shape
	m_sizeShapeRect.setValues(-1.0f, -1.0f);
	m_sizeShapeRadius = -1.0;
	if (typeInfo == 0)
	{
		m_sizeShapeType = SizeShapeType_Invalid;
		needSizeShapeGraphic = false;
	}
	else if (typeInfo->hasFixedSizeShapeCircle())
	{
		m_sizeShapeType = SizeShapeType_Circle;
		m_sizeShapeRadius = typeInfo->getFixedSizeShapeRadius();
	}
	else if (typeInfo->hasFixedSizeShapeRectangle())
	{
		m_sizeShapeType = SizeShapeType_Rectangle;
		m_sizeShapeRect = typeInfo->getFixedSizeShapeRectangle();
	}
	else if (m_entity->isPropertyVisible("radius"))
	{
		m_sizeShapeType = SizeShapeType_Circle;
		m_sizeShapeRadius = propsRadius;
	}
	else if (propsRect.x >= 0.0f && propsRect.y >= 0.0f)
	{
		m_sizeShapeType = SizeShapeType_Rectangle;
		m_sizeShapeRect = propsRect;
	}
	else
	{
		m_sizeShapeType = SizeShapeType_Invalid;
		needSizeShapeGraphic = false;
	}
	
	if (needSizeShapeGraphic)
	{
		TT_NULL_ASSERT(typeInfo);
		switch (m_sizeShapeType)
		{
		case SizeShapeType_Rectangle:
			m_sizeShape = tt::engine::renderer::QuadSprite::createQuad(
					(m_sizeShapeRect.x > 0.0f) ? m_sizeShapeRect.x : 1.0f,
					(m_sizeShapeRect.y > 0.0f) ? m_sizeShapeRect.y : 1.0f,
					typeInfo->getSizeShapeColor());
			break;
			
		case SizeShapeType_Circle:
			m_sizeShape = tt::engine::renderer::QuadSprite::createQuad(
					tt::engine::renderer::TextureCache::get("sizeshape_circle", "textures.editor.ui"),
					typeInfo->getSizeShapeColor());
			break;
			
		default:
			TT_PANIC("Unhandled m_sizeShapeType '%d'", m_sizeShapeType);
			m_sizeShape.reset();
			break;
		}
	}
	else
	{
		m_sizeShape.reset();
	}
	
	// Properties have been scanned
	m_shouldRescanProperties = false;
}

// Namespace end
}
}
}
}
