#if !defined(INC_TT_MENU_ELEMENTS_IMAGEEDITOR_H)
#define INC_TT_MENU_ELEMENTS_IMAGEEDITOR_H


#include <tt/platform/tt_error.h>
#include <tt/engine/renderer/fwd.h>

#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/MenuSystem.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Image Editor menu element. */
class ImageEditor : public MenuElement
{
public:
	ImageEditor(const std::string& p_name,
	            const MenuLayout&  p_layout,
	            const std::string& p_image,
	            s32                p_imageWidth,
	            s32                p_imageHeight,
	            s32                p_zoomLevel,
	            const std::string& p_paletteElement,
	            const std::string& p_pencilElement,
	            const std::string& p_bucketElement,
	            const std::string& p_pickerElement,
	            const std::string& p_templateFile,
	            s32                p_templateColumns,
	            s32                p_templateRows);
	virtual ~ImageEditor();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void addChild(MenuElement* p_child);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual s32 getRequestedHorizontalPosition() const;
	virtual s32 getRequestedVerticalPosition()   const;
	
	virtual bool onStylusPressed(s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	
	inline void setColor(u32 p_color)
	{ TT_ASSERT(p_color < 32); m_color = p_color; }
	inline u32 getColor() const { return m_color; }
	
	void clean();
	void usePencil();
	void useBucket();
	void usePicker();
	void setBrushSize(s32 p_brushSize);
	void useTemplate(s32 p_templateID);
	
	void undo();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual ImageEditor* clone() const;
	
protected:
	ImageEditor(const ImageEditor& p_rhs);
	
private:
	enum Mode
	{
		MODE_PENCIL,
		MODE_PICKER,
		MODE_BUCKET
	};
	
	enum
	{
		BORDER_THICKNESS = 1
	};
	
	
	bool hasChanged();
	void makeBackup();
	void seedFill(u8* p_data, s32 p_x, s32 p_y, u32 p_fillColor);
	void floodFill(u8* p_data, s32 p_x, s32 p_y,
	               u32 p_fillColor, u32 p_seedColor);
	void drawPoint(u8* p_data, s32 p_x, s32 p_y);
	void plotPixel(u8* p_data, s32 p_x, s32 p_y);
	void plotLine(u8* p_data, s32 p_x0, s32 p_y0, s32 p_x1, s32 p_y1);
	
	
	s32                             m_zoomLevel;
	s32                             m_reqZoomLevel;
	engine::renderer::TexturePtr    m_image;
	engine::renderer::QuadSpritePtr m_imageQuad;
	engine::renderer::QuadSpritePtr m_border;
	
	u32 m_color;
	
	s32 m_imageWidth;
	s32 m_imageHeight;
	
	s32 m_textureWidth;
	s32 m_textureHeight;
	
	u8* m_backupData;
	u8* m_original;
	
	s32 m_templateColumns;
	s32 m_templateRows;
	
	const std::string m_paletteElement;
	const std::string m_pencilElement;
	const std::string m_bucketElement;
	const std::string m_pickerElement;
	const std::string m_templates;
	
	s32 m_brushSize;
	
	s32 m_prevX;
	s32 m_prevY;
	
	Mode m_mode;
	Mode m_prevMode;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_IMAGEEDITOR_H)
