#if !defined(INC_TT_MENU_ELEMENTS_SKINELEMENTIDS_H)
#define INC_TT_MENU_ELEMENTS_SKINELEMENTIDS_H


namespace tt {
namespace menu {
namespace elements {

/*! \brief Identifiers for menu element skinning information. */
enum SkinElementIDs
{
	SkinElement_Window,
	SkinElement_Label,
	SkinElement_SunkenBorderDecorator,
	SkinElement_Scrollbar,
	SkinElement_BackButton,
	SkinElement_FileBlocksBar,
	SkinElement_ProgressBar,
	SkinElement_BusyBar
};


/*! \brief Skinning components for Window. */
enum SkinComponents_Window
{
	WindowSkin_BgTexture          = 0, // Texture
	WindowSkin_BgColor            = 0, // Vertex color + vertex alpha
	
	WindowSkin_PoleTexture        = 1, // Texture
	WindowSkin_PoleColor          = 1, // Vertex color + vertex alpha
	
	WindowSkin_DecalTopTexture    = 2, // Texture
	WindowSkin_DecalTopColor      = 2, // Vertex color + vertex alpha
	
	WindowSkin_DecalBottomTexture = 3, // Texture
	WindowSkin_DecalBottomColor   = 3, // Vertex color + vertex alpha
	
	WindowSkin_DecalLeftTexture   = 4, // Texture
	WindowSkin_DecalLeftColor     = 4, // Vertex color + vertex alpha
	
	WindowSkin_DecalRightTexture  = 5, // Texture
	WindowSkin_DecalRightColor    = 5, // Vertex color + vertex alpha
	
	WindowSkin_CaptionColor       = 6  // Vertex color + vertex alpha
};


/*! \brief Skinning components for Label. */
enum SkinComponents_Label
{
	LabelSkin_TextNormalColor     = 0, // Vertex color + vertex alpha
	LabelSkin_TextSelectedColor   = 1, // Vertex color + vertex alpha
	LabelSkin_TextDisabledColor   = 2, // Vertex color + vertex alpha
	
	LabelSkin_ShadowNormalColor   = 3, // Vertex color + vertex alpha
	LabelSkin_ShadowSelectedColor = 4, // Vertex color + vertex alpha
	LabelSkin_ShadowDisabledColor = 5  // Vertex color + vertex alpha
};


/*! \brief Skinning components for SunkenBorderDecorator. */
enum SkinComponents_SunkenBorderDecorator
{
	SunkenBorderSkin_BgTexture = 0, // Texture index
	SunkenBorderSkin_BgColor   = 0  // Vertex color + vertex alpha indices
};


/*! \brief Skinning components for Scrollbar. */
enum SkinComponents_Scrollbar
{
	ScrollbarSkin_BgTexture = 0, // Texture index
	ScrollbarSkin_BgColor   = 0, // Vertex color + vertex alpha indices
	
	ScrollbarSkin_PickerTexture = 1, // Texture index
	ScrollbarSkin_PickerColor   = 1, // Vertex color + vertex alpha indices
	
	ScrollbarSkin_ArrowTexture = 2, // Texture index
	ScrollbarSkin_ArrowColor   = 2,  // Vertex color + vertex alpha indices

	ScrollbarSkin_ArrowPressedTexture = 3, // Texture index
	ScrollbarSkin_ArrowPressedColor   = 3  // Vertex color + vertex alpha indices
};


/*! \brief Skinning components for the system back button. */
enum SkinComponents_BackButton
{
	BackButtonSkin_Texture        = 0, // Texture index
	BackButtonSkin_Color          = 0, // Vertex color + vertex alpha indices
	BackButtonSkin_OverlayTexture = 1, // Texture index
	BackButtonSkin_OverlayColor   = 1  // Vertex color + vertex alpha indices
};


/*! \brief Skinning components for the filesystem blocks bar. */
enum SkinComponents_FileBlocksBar
{
	FileBlocksBarSkin_BackgroundTexture = 0, // Background bar texture index
	FileBlocksBarSkin_BackgroundColor   = 0, // Background bar vertex color + vertex alpha indices
	
	FileBlocksBarSkin_OverlayTexture = 1, // Overlay bar texture index
	FileBlocksBarSkin_OverlayColor   = 1  // Overlay bar vertex color + vertex alpha indices
};


/*! \brief Skinning components for the filesystem blocks bar. */
enum SkinComponents_ProgressBar
{
	ProgressBarSkin_BackgroundTexture = 0, // Background bar texture index
	ProgressBarSkin_BackgroundColor   = 0, // Background bar vertex color + vertex alpha indices
	
	ProgressBarSkin_OverlayTexture = 1, // Overlay bar texture index
	ProgressBarSkin_OverlayColor   = 1  // Overlay bar vertex color + vertex alpha indices
};


/*! \brief Skinning components for the busy bar. */
enum SkinComponents_BusyBar
{
	BusyBarSkin_OverlayTexture = 0, // Overlay bar texture index
	BusyBarSkin_OverlayColor   = 0  // Overlay bar vertex color + vertex alpha indices
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SKINELEMENTIDS_H)
