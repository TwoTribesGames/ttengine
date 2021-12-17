#include <sstream>
#include <stack>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/DefaultColours.h>
#include <tt/menu/elements/ImageEditor.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>


namespace tt {
namespace menu {
namespace elements {

// Used for flood fill
struct FloodPoint
{
	FloodPoint(s32 p_x = 0, s32 p_y = 0) : x(p_x), y(p_y) { }
	
	s32 x;
	s32 y;
};

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

ImageEditor::ImageEditor(const std::string& p_name,
                         const MenuLayout&  p_layout,
                         const std::string& /* p_image */,
                         s32                p_imageWidth,
                         s32                p_imageHeight,
                         s32                p_zoomLevel,
                         const std::string& p_paletteElement,
                         const std::string& p_pencilElement,
                         const std::string& p_bucketElement,
                         const std::string& p_pickerElement,
                         const std::string& p_templateFile,
                         s32                p_templateColumns,
                         s32                p_templateRows)
:
MenuElement(p_name, p_layout),
m_zoomLevel(1),
m_reqZoomLevel(p_zoomLevel),
m_color(1),
m_imageWidth(p_imageWidth),
m_imageHeight(p_imageHeight),
m_textureWidth(0),
m_textureHeight(0),
m_backupData(0),
m_original(0),
m_templateColumns(p_templateColumns),
m_templateRows(p_templateRows),
m_paletteElement(p_paletteElement),
m_pencilElement(p_pencilElement),
m_bucketElement(p_bucketElement),
m_pickerElement(p_pickerElement),
m_templates(p_templateFile),
m_brushSize(1),
m_prevX(-1),
m_prevY(-1),
m_mode(MODE_PENCIL),
m_prevMode(MODE_PENCIL)
{
	MENU_CREATION_Printf("ImageEditor::ImageEditor: Element '%s': "
	                     "New image editor for image '%s'.\n",
	                     getName().c_str(), p_image.c_str());
	TT_ASSERT(m_reqZoomLevel > 0);
	
	
	m_textureWidth  = math::roundToPowerOf2(m_imageWidth);
	m_textureHeight = math::roundToPowerOf2(m_imageHeight);
	
	m_image = engine::renderer::Texture::createForText(
		static_cast<s16>(m_textureWidth),
		static_cast<s16>(m_textureHeight));
	
	/*
	m_image->Initialise(static_cast<s16>(m_textureWidth),
	                    static_cast<s16>(m_textureHeight),
	                    CO3DImage::eFORMAT_256Col);
	*/
	m_backupData = new u8[m_textureWidth * m_textureHeight];
	
	m_imageQuad = engine::renderer::QuadSprite::createQuad(m_image);
	/*
	m_imageQuad->setWidth(m_imageWidth);
	m_imageQuad->setHeight(m_imageHeight);
	
	m_imageQuad->setTexcoordRight(static_cast<u16>(m_imageWidth));
	m_imageQuad->setTexcoordBottom(static_cast<u16>(m_imageHeight));
	*/
	
	m_border = engine::renderer::QuadSprite::createQuad(
		static_cast<real>(m_imageWidth),
		static_cast<real>(m_imageHeight),
		engine::renderer::ColorRGB::black);
	
	
	/*
	u16* pal = m_image->LockPalette();
	TT_NULL_ASSERT(pal);
	for (s32 i = 0; i < 32; ++i)
	{
		pal[i] = g_defaultColors[i];
	}
	m_image->UnlockPalette();
	*/
	
	setCanHaveFocus(true);
}


ImageEditor::~ImageEditor()
{
	MENU_CREATION_Printf("ImageEditor::~ImageEditor: Element '%s'.\n ",
	                     getName().c_str());
	
	delete[] m_backupData;
	delete[] m_original;
}


void ImageEditor::clean()
{
	makeBackup();
	/*
	u8* data = m_image->LockImage();
	TT_NULL_ASSERT(data);
	for (s32 y = 0; y < m_imageHeight; ++y)
	{
		for (s32 x = 0; x < m_imageWidth; ++x)
		{
			data[(y * m_textureWidth) + x] = 0;
		}
	}
	m_image->UnlockImage();
	*/
}


void ImageEditor::usePencil()
{
	if (m_mode != MODE_PENCIL)
	{
		m_mode = MODE_PENCIL;
		
		if (m_pencilElement.empty() == false)
		{
			MenuElementAction mea(m_pencilElement, "show_value");
			mea.addParameter("pressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_bucketElement.empty() == false)
		{
			MenuElementAction mea(m_bucketElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_pickerElement.empty() == false)
		{
			MenuElementAction mea(m_pickerElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
	}
}


void ImageEditor::useBucket()
{
	if (m_mode != MODE_BUCKET)
	{
		m_mode = MODE_BUCKET;
		if (m_pencilElement.empty() == false)
		{
			MenuElementAction mea(m_pencilElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_bucketElement.empty() == false)
		{
			MenuElementAction mea(m_bucketElement, "show_value");
			mea.addParameter("pressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_pickerElement.empty() == false)
		{
			MenuElementAction mea(m_pickerElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
	}
}


void ImageEditor::usePicker()
{
	if (m_mode != MODE_PICKER)
	{
		m_prevMode = m_mode;
		m_mode = MODE_PICKER;
		
		if (m_pencilElement.empty() == false)
		{
			MenuElementAction mea(m_pencilElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_bucketElement.empty() == false)
		{
			MenuElementAction mea(m_bucketElement, "show_value");
			mea.addParameter("depressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		if (m_pickerElement.empty() == false)
		{
			MenuElementAction mea(m_pickerElement, "show_value");
			mea.addParameter("pressed");
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
	}
}


void ImageEditor::setBrushSize(s32 p_brushSize)
{
	TT_MINMAX_ASSERT(p_brushSize, 0, 5);
	m_brushSize = p_brushSize;
}


void ImageEditor::useTemplate(s32 p_templateID)
{
	if (p_templateID == -1 || m_templates.empty())
	{
		return;
	}
	
	// Try to load the specified texture
	// FIXME: MUST SUPPORT NAMESPACES
	engine::renderer::TexturePtr templates = 
		engine::renderer::TextureCache::get(m_templates, "");
	
	//templates->setFlag(CO3DTexture::eFLAG_TextureColor0Trans);
	//templates->resetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
	TT_ASSERTMSG(templates != 0, "Loading templates texture '%s' failed.",
	             m_templates.c_str());
	
	s32 u = (p_templateID % m_templateColumns) * m_imageWidth;
	s32 v = (p_templateID / m_templateColumns) * m_imageHeight;
	
	TT_ASSERTMSG((v + m_imageHeight) <= templates->getHeight() &&
	             (u + m_imageWidth)  <= templates->getWidth(),
	             "Template index %d is out of bounds!", p_templateID);
	
	makeBackup();
	/*
	u8* data = m_image->LockImage();
	TT_NULL_ASSERT(data);
	u8* templatedata = templates->LockImage();
	TT_NULL_ASSERT(templatedata);
	s32 template_width = templates->GetWidth();
	for (s32 y = 0; y < m_imageHeight; ++y)
	{
		for (s32 x = 0; x < m_imageWidth; ++x)
		{
			data[y * m_textureWidth + x] =
				templatedata[(v + y) * template_width + (u + x)];
		}
	}
	templates->UnlockImage();
	m_image->UnlockImage();
	*/
}


void ImageEditor::doLayout(const PointRect& p_rect)
{
	// See if requested size is larger than available size
	if (p_rect.getWidth() < (m_imageWidth * m_reqZoomLevel) +
	    0 + (2 * BORDER_THICKNESS))
	{
		s32 availableWidth = p_rect.getWidth() - 0 - (2 * BORDER_THICKNESS);
		m_zoomLevel = availableWidth / m_imageWidth;
	}
	else
	{
		m_zoomLevel = m_reqZoomLevel;
	}
	
	if (p_rect.getHeight() < (m_imageHeight * m_reqZoomLevel) +
	    0 + (2 * BORDER_THICKNESS))
	{
		s32 availableHeight = p_rect.getHeight() - 0 - (2 * BORDER_THICKNESS);
		m_zoomLevel = std::min(m_zoomLevel, availableHeight / m_imageHeight);
	}
	else
	{
		m_zoomLevel = std::min(m_zoomLevel, m_reqZoomLevel);
	}
	
	TT_ASSERT(m_zoomLevel >= 1);
	
	/*
	m_border->setHeight((m_imageHeight * m_zoomLevel) +
	                    0 + (2 * BORDER_THICKNESS));
	m_border->setWidth((m_imageWidth * m_zoomLevel) +
	                   0 + (2 * BORDER_THICKNESS));
	*/
	
	m_imageQuad->setScale(static_cast<real>(m_zoomLevel));
}


void ImageEditor::undo()
{
	/*
	u8* data = m_image->LockImage();
	TT_NULL_ASSERT(data);
	for (s32 y = 0; y < m_imageHeight; ++y)
	{
		for (s32 x = 0; x < m_imageWidth; ++x)
		{
			std::swap(data[y * m_textureWidth + x],
			          m_backupData[y * m_textureWidth + x]);
		}
	}
	m_image->UnlockImage();
	*/
}


void ImageEditor::makeBackup()
{
	/*
	u8* data = m_image->LockImage();
	TT_NULL_ASSERT(data);
	for (s32 y = 0; y < m_imageHeight; ++y)
	{
		for (s32 x = 0; x < m_imageWidth; ++x)
		{
			m_backupData[y * m_textureWidth + x] = data[y * m_textureWidth + x];
		}
	}
	
	if (m_original == 0)
	{
		m_original = new u8[m_textureWidth * m_textureHeight];
		for (s32 y = 0; y < m_imageHeight; ++y)
		{
			for (s32 x = 0; x < m_imageWidth; ++x)
			{
				m_original[y * m_textureWidth + x] = data[y * m_textureWidth + x];
			}
		}
	}
	m_image->UnlockImage();
	*/
}


bool ImageEditor::hasChanged()
{
	if (m_original == 0)
	{
		return false;
	}
	
	/*
	u8* data = m_image->LockImage();
	bool ret = false;
	
	for (s32 y = 0; y < m_imageHeight; ++y)
	{
		for (s32 x = 0; x < m_imageWidth; ++x)
		{
			if (m_original[y * m_textureWidth + x] !=
			    data[y * m_textureWidth + x])
			{
				ret = true;
			}
		}
	}
	m_image->UnlockImage();
	
	return ret;
	*/
	return false;
}


void ImageEditor::render(const PointRect& p_rect, s32 p_z)
{
	math::Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	                  static_cast<real>(p_rect.getCenterPosition().y),
	                  static_cast<real>(p_z));
	m_border->setPosition(pos);
	m_border->update();
	m_border->render();
	
	m_imageQuad->setPosition(pos + math::Vector3(0.0f, 0.0f, -1.0f));
	m_imageQuad->update();
	m_imageQuad->render();
}


void ImageEditor::addChild(MenuElement* /* p_child */)
{
	TT_PANIC("ImageEditor cannot have any children.");
}


s32 ImageEditor::getMinimumWidth() const
{
	return (m_imageWidth + 1) + (2 * BORDER_THICKNESS);
}


s32 ImageEditor::getMinimumHeight() const
{
	return (m_imageHeight + 1) + (2 * BORDER_THICKNESS);
}


s32 ImageEditor::getRequestedWidth() const
{
	return ((m_imageWidth * (m_reqZoomLevel)) + 1) + (2 * BORDER_THICKNESS);
}


s32 ImageEditor::getRequestedHeight() const
{
	return ((m_imageHeight * (m_reqZoomLevel)) + 1) + (2 * BORDER_THICKNESS);
}


s32 ImageEditor::getRequestedHorizontalPosition() const
{
	return 0;
}


s32 ImageEditor::getRequestedVerticalPosition() const
{
	return 0;
}


bool ImageEditor::onStylusPressed(s32 p_x, s32 p_y)
{
	MENU_STYLUS_Printf("ImageEditor::onStylusPressed: Element '%s': "
	                   "ImageEditor pressed at (%d, %d).\n",
	                   getName().c_str(), p_x, p_y);
	if (isEnabled() == false)
	{
		return false;
	}
	s32 x = (p_x - BORDER_THICKNESS) / m_zoomLevel;
	s32 y = (p_y - BORDER_THICKNESS) / m_zoomLevel;
	
	if (x >= m_imageWidth || x < 0 || y >= m_imageHeight || y < 0)
	{
		return false;
	}
	
	switch (m_mode)
	{
	case MODE_PENCIL:
		{
			m_prevX = -1;
			m_prevY = -1;
			makeBackup();
			
			/*
			u8* data = m_image->LockImage();
			TT_NULL_ASSERT(data);
			drawPoint(data, x, y);
			m_image->UnlockImage();
			*/
			MenuSystem::getInstance()->playSound(MenuSound_FlagEditor_Brush);
		}
		break;
		
	case MODE_BUCKET:
		{
			makeBackup();
			
			/*
			u8* data = m_image->LockImage();
			TT_NULL_ASSERT(data);
			seedFill(data, x, y, m_color);
			m_image->UnlockImage();
			*/
			
			MenuSystem::getInstance()->playSound(MenuSound_FlagEditor_Fill);
			
			// do some filling
		}
		break;
		
	case MODE_PICKER:
		{
			/*
			u8* data = m_image->LockImage();
			TT_NULL_ASSERT(data);
			m_color = data[y * m_imageWidth + x];
			m_image->UnlockImage();
			*/
			
			if (m_paletteElement.empty() == false)
			{
				MenuAction ma("menu_element_action");
				std::ostringstream oss;
				oss << m_color;
				
				ma.addParameter(m_paletteElement);
				ma.addParameter("select_by_value");
				ma.addParameter(oss.str());
				MenuElementAction mea(ma);
				
				MenuSystem::getInstance()->doMenuElementAction(mea);
			}
		}
		break;
		
	default:
		TT_PANIC("Unknown editor mode: %d", m_mode);
		return false;
	}
	
	return true;
}


bool ImageEditor::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	if (isEnabled() == false)
	{
		return false;
	}
	s32 x = (p_x - BORDER_THICKNESS) / m_zoomLevel;
	s32 y = (p_y - BORDER_THICKNESS) / m_zoomLevel;
	
	if (x >= m_imageWidth || x < 0 || y >= m_imageHeight || y < 0)
	{
		p_isInside = false;
	}
	
	if (p_isInside)
	{
		MENU_STYLUS_Printf("ImageEditor::onStylusDragging: Element '%s': "
		                   "Dragging on image editor at (%d, %d).\n",
		                   getName().c_str(), p_x, p_y);
		switch (m_mode)
		{
		case MODE_PENCIL:
			{
				/*
				u8* data = m_image->LockImage();
				TT_NULL_ASSERT(data);
				drawPoint(data, x, y);
				m_image->UnlockImage();
				*/
				
				if (m_prevX != x || m_prevY != y)
				{
					MenuSystem::getInstance()->playSound(MenuSound_FlagEditor_Brush);
				}
			}
			break;
		
		case MODE_PICKER:
			{
				/*
				u8* data = m_image->LockImage();
				TT_NULL_ASSERT(data);
				m_color = data[y * m_textureWidth + x];
				m_image->UnlockImage();
				*/
				
				if (m_paletteElement.empty() == false)
				{
					MenuAction ma("menu_element_action");
					std::ostringstream oss;
					oss << m_color;
					
					ma.addParameter(m_paletteElement);
					ma.addParameter("select_by_value");
					ma.addParameter(oss.str());
					MenuElementAction mea(ma);
					
					MenuSystem::getInstance()->doMenuElementAction(mea);
				}
			}
			break;
			
		default:
			// Don't do anything
			break;
		}
	}
	else
	{
		MENU_STYLUS_Printf("ImageEditor::onStylusDragging: Element '%s': "
		                   "Dragging outside imageeditor at (%d, %d).\n",
		                   getName().c_str(), p_x, p_y);
		m_prevX = -1;
		m_prevY = -1;
	}
	
	return true;
}


bool ImageEditor::onStylusReleased(s32 /* p_x */, s32 /* p_y */)
{
	MENU_STYLUS_Printf("ImageEditor::onStylusReleased: Element '%s': "
	                   "Stylus released at (%d, %d) == button pressed. "
	                   "Perform ImageEditor action.\n",
	                   getName().c_str(), p_x, p_y);
	if (isEnabled() == false)
	{
		return false;
	}
	
	if (m_mode == MODE_PICKER)
	{
		if (m_prevMode == MODE_PENCIL)
		{
			usePencil();
		}
		else if (m_prevMode == MODE_BUCKET)
		{
			useBucket();
		}
		m_mode = m_prevMode;
	}
	
	m_prevX = -1;
	m_prevY = -1;
	
	//performActions();
	
	return true;
}


bool ImageEditor::onStylusRepeat(s32 /* p_x */, s32 /* p_y */)
{
	// Do nothing
	return true;
}


bool ImageEditor::onKeyPressed(const MenuKeyboard& p_keys)
{
	if (isEnabled() == false)
	{
		return false;
	}
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_ACCEPT))
	{
		MENU_KEY_Printf("ImageEditor::onKeyPressed: Element '%s': "
		                "Perform ImageEditor action.\n",
		                getName().c_str());
		
		//performActions();
		
		return true;
	}
	
	return false;
}


bool ImageEditor::doAction(const MenuElementAction& p_action)
{
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (isEnabled() == false)
	{
		return false;
	}
	
	const std::string cmd(p_action.getCommand());
	if (cmd == "set_color")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'set_color' requires 1 parameter.");
		TT_ERR_CREATE(cmd);
		u32 color = str::parseU32(p_action.getParameter(0), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid color index specified: '%s'",
			         p_action.getParameter(0).c_str());
		}
		else
		{
			setColor(color);
		}
		return true;
	}
	else if (cmd == "set_brush_size")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'set_brush_size' requires "
		             "1 parameter (brush size).");
		TT_ERR_CREATE(cmd);
		s32 size = str::parseS32(p_action.getParameter(0), &errStatus);
		if (errStatus.hasError() || size <= 0)
		{
			TT_PANIC("Invalid brush size specified: '%s'",
			         p_action.getParameter(0).c_str());
		}
		else
		{
			setBrushSize(size);
		}
		return true;
	}
	else if (cmd == "use_pencil")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		usePencil();
		return true;
	}
	else if (cmd == "use_bucket")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		useBucket();
		return true;
	}
	else if (cmd == "use_picker")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		usePicker();
		return true;
	}
	else if (cmd == "clean")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		clean();
		return true;
	}
	else if (cmd == "save")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		
		/*
		if (m_original == 0)
		{
			m_original = new u8[m_textureWidth * m_textureHeight];
		}
		
		u8* data = m_image->LockImage();
		for (s32 y = 0; y < m_imageHeight; ++y)
		{
			for (s32 x = 0; x < m_imageWidth; ++x)
			{
				m_original[y * m_textureWidth + x] =
					data[y * m_textureWidth + x];
			}
		}
		m_image->UnlockImage(false);
		*/
		return true;
	}
	else if (cmd == "undo")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.", cmd.c_str());
		undo();
		return true;
	}
	else if (cmd == "use_template")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'use_template' requires 1 "
		             "parameter (template ID).");
		TT_ERR_CREATE(cmd);
		s32 id = str::parseS32(p_action.getParameter(0), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid template ID specified: '%s'",
			         p_action.getParameter(0).c_str());
		}
		else
		{
			useTemplate(id);
		}
		return true;
	}
	else if (cmd == "has_changed")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'has_changed' requires 1 "
		             "parameter (variable name).");
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(0), str::toStr(hasChanged()));
		return true;
	}
	
	return false;
}


ImageEditor* ImageEditor::clone() const
{
	return new ImageEditor(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

ImageEditor::ImageEditor(const ImageEditor& p_rhs)
:
MenuElement(p_rhs),
m_zoomLevel(p_rhs.m_zoomLevel),
m_reqZoomLevel(p_rhs.m_reqZoomLevel),
m_image(p_rhs.m_image),
m_imageQuad(new engine::renderer::QuadSprite(*(p_rhs.m_imageQuad))),
m_border(new engine::renderer::QuadSprite(*(p_rhs.m_border))),
m_color(p_rhs.m_color),
m_imageWidth(p_rhs.m_imageWidth),
m_imageHeight(p_rhs.m_imageHeight),
m_textureWidth(p_rhs.m_textureWidth),
m_textureHeight(p_rhs.m_textureHeight),
m_backupData(0),
m_templateColumns(p_rhs.m_templateColumns),
m_templateRows(p_rhs.m_templateRows),
m_paletteElement(p_rhs.m_paletteElement),
m_pencilElement(p_rhs.m_pencilElement),
m_bucketElement(p_rhs.m_bucketElement),
m_pickerElement(p_rhs.m_pickerElement),
m_templates(p_rhs.m_templates),
m_brushSize(p_rhs.m_brushSize),
m_prevX(p_rhs.m_prevX),
m_prevY(p_rhs.m_prevY),
m_mode(p_rhs.m_mode),
m_prevMode(p_rhs.m_prevMode)
{
	// Copy the backup data
	if (p_rhs.m_backupData != 0)
	{
		std::size_t dataLen = static_cast<std::size_t>(m_textureWidth *
		                                               m_textureHeight);
		m_backupData = new u8[dataLen];
		std::memcpy(m_backupData, p_rhs.m_backupData, dataLen);
	}
}


//------------------------------------------------------------------------------
// Private member functions

void ImageEditor::seedFill(u8* p_data, s32 p_x, s32 p_y, u32 p_fillColor)
{
	u32 seedColor = p_data[p_y * m_textureWidth + p_x];
	if (seedColor == p_fillColor)
	{
		return;
	}
	floodFill(p_data, p_x, p_y, p_fillColor, seedColor);
}


void ImageEditor::floodFill(u8* /*p_data*/, s32 /*p_x*/, s32 /*p_y*/,
                            u32 /*p_fillColor*/, u32 /*p_seedColor*/)
{
}


void ImageEditor::drawPoint(u8* p_data, s32 p_x, s32 p_y)
{
	if (m_prevX == -1 || m_prevY == -1)
	{
		m_prevX = p_x;
		m_prevY = p_y;
	}
	
	switch (m_brushSize)
	{
	case 0:
		break;
		
	case 1:
		plotLine(p_data, p_x, p_y, m_prevX, m_prevY);
		break;
		
	case 2:
		plotLine(p_data, p_x - 1, p_y - 1, m_prevX - 1, m_prevY - 1);
		plotLine(p_data, p_x,     p_y - 1, m_prevX,     m_prevY - 1);
		plotLine(p_data, p_x - 1, p_y,     m_prevX - 1, m_prevY);
		plotLine(p_data, p_x,     p_y,     m_prevX,     m_prevY);
		break;
		
	case 3:
		plotLine(p_data, p_x,     p_y - 1, m_prevX,     m_prevY - 1);
		plotLine(p_data, p_x - 1, p_y,     m_prevX - 1, m_prevY);
		plotLine(p_data, p_x,     p_y,     m_prevX,     m_prevY);
		plotLine(p_data, p_x + 1, p_y,     m_prevX + 1, m_prevY);
		plotLine(p_data, p_x,     p_y + 1, m_prevX,     m_prevY + 1);
		break;
		
	case 4:
		plotLine(p_data, p_x - 1, p_y - 2, m_prevX - 1, m_prevY - 2);
		plotLine(p_data, p_x,     p_y - 2, m_prevX,     m_prevY - 2);
		plotLine(p_data, p_x - 2, p_y - 1, m_prevX - 2, m_prevY - 1);
		plotLine(p_data, p_x - 1, p_y - 1, m_prevX - 1, m_prevY - 1);
		plotLine(p_data, p_x,     p_y - 1, m_prevX,     m_prevY - 1);
		plotLine(p_data, p_x + 1, p_y - 1, m_prevX + 1, m_prevY - 1);
		plotLine(p_data, p_x - 2, p_y,     m_prevX - 2, m_prevY);
		plotLine(p_data, p_x - 1, p_y,     m_prevX - 1, m_prevY);
		plotLine(p_data, p_x,     p_y,     m_prevX,     m_prevY);
		plotLine(p_data, p_x + 1, p_y,     m_prevX + 1, m_prevY);
		plotLine(p_data, p_x - 1, p_y + 1, m_prevX - 1, m_prevY + 1);
		plotLine(p_data, p_x,     p_y + 1, m_prevX,     m_prevY + 1);
		break;
		
	case 5:
		plotLine(p_data, p_x - 1, p_y - 2, m_prevX - 1, m_prevY - 2);
		plotLine(p_data, p_x,     p_y - 2, m_prevX,     m_prevY - 2);
		plotLine(p_data, p_x + 1, p_y - 2, m_prevX + 1, m_prevY - 2);
		plotLine(p_data, p_x - 2, p_y - 1, m_prevX - 2, m_prevY - 1);
		plotLine(p_data, p_x - 1, p_y - 1, m_prevX - 1, m_prevY - 1);
		plotLine(p_data, p_x,     p_y - 1, m_prevX,     m_prevY - 1);
		plotLine(p_data, p_x + 1, p_y - 1, m_prevX + 1, m_prevY - 1);
		plotLine(p_data, p_x + 2, p_y - 1, m_prevX + 2, m_prevY - 1);
		plotLine(p_data, p_x - 2, p_y,     m_prevX - 2, m_prevY);
		plotLine(p_data, p_x - 1, p_y,     m_prevX - 1, m_prevY);
		plotLine(p_data, p_x,     p_y,     m_prevX,     m_prevY);
		plotLine(p_data, p_x + 1, p_y,     m_prevX + 1, m_prevY);
		plotLine(p_data, p_x + 2, p_y,     m_prevX + 2, m_prevY);
		plotLine(p_data, p_x - 2, p_y + 1, m_prevX - 2, m_prevY + 1);
		plotLine(p_data, p_x - 1, p_y + 1, m_prevX - 1, m_prevY + 1);
		plotLine(p_data, p_x,     p_y + 1, m_prevX,     m_prevY + 1);
		plotLine(p_data, p_x + 1, p_y + 1, m_prevX + 1, m_prevY + 1);
		plotLine(p_data, p_x + 2, p_y + 1, m_prevX + 2, m_prevY + 1);
		plotLine(p_data, p_x - 1, p_y + 2, m_prevX - 1, m_prevY + 2);
		plotLine(p_data, p_x,     p_y + 2, m_prevX,     m_prevY + 2);
		plotLine(p_data, p_x + 1, p_y + 2, m_prevX + 1, m_prevY + 2);
		break;
	}
	
	m_prevX = p_x;
	m_prevY = p_y;
}


void ImageEditor::plotPixel(u8* p_data, s32 p_x, s32 p_y)
{
	if (p_x < 0 || p_x >= m_imageWidth)
	{
		return;
	}
	
	if (p_y < 0 || p_y >= m_imageHeight)
	{
		return;
	}
	
	p_data[p_y * m_textureWidth + p_x] = static_cast<u8>(m_color);
}


// Fast Bresenham line drawing algorithm:
void ImageEditor::plotLine(u8* p_data, s32 p_x0, s32 p_y0, s32 p_x1, s32 p_y1)
{
	s32 dx = p_x1 - p_x0;
	s32 dy = p_y1 - p_y0;
	
	s32 stepX = 1;
	s32 stepY = 1;
	
	if (dx < 0)
	{
		dx    = -dx;
		stepX = -1;
	}
	if (dy < 0)
	{
		dy    = -dy;
		stepY = -1;
	}
	
	// Scan first pixel
	plotPixel(p_data, p_x0, p_y0);
	
	if (dx >= dy)  // Scan horizontalish lines
	{
		s32 two_dxdy = 2 * (dx - dy);
		s32 two_dy   = 2 * dy;
		s32 d        = dx - two_dy;
		while (p_x0 != p_x1)
		{
			if (d <= 0)
			{
				d    += two_dxdy;   // choose NE
				p_y0 += stepY;
			}
			else
			{
				d  -= two_dy;     // choose E
			}
			p_x0 += stepX;
			plotPixel(p_data, p_x0, p_y0);
		}
	}
	else           // Scan verticalish lines
	{
		s32 two_dydx = 2 * (dy - dx);
		s32 two_dx   = 2 * dx;
		s32 d        = dy - two_dx;
		
		while (p_y0 != p_y1)
		{
			if (d <= 0)
			{
				d    += two_dydx;   // choose W
				p_x0 += stepX;
			}
			else
			{
				d  -= two_dx;     // choose NW
			}
			p_y0 += stepY;
			plotPixel(p_data, p_x0, p_y0);
		}
	}
}

// Namespace end
}
}
}
