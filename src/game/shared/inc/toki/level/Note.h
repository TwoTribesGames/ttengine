#if !defined(INC_TOKI_LEVEL_NOTE_H)
#define INC_TOKI_LEVEL_NOTE_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Rect.h>

#include <toki/level/fwd.h>


namespace toki {
namespace level {

/*! \brief A simple text note in the level, meant for the designers (not used by the game code). */
class Note
{
public:
	enum VisualType
	{
		VisualType_Note,
		VisualType_Warning
	};
	static NotePtr create(
			const tt::math::VectorRect& p_rect = tt::math::VectorRect(tt::math::Vector2(0.0f, 0.0f), 0.0f, 0.0f),
			const std::wstring&         p_text = std::wstring(),
			VisualType                  p_visualType = VisualType_Note);
	/*! \brief Create Note which has the size needed to fit all the text. */
	static NotePtr create(
			const tt::math::Vector2& p_pos,
			const std::wstring&      p_text,
			VisualType               p_visualType = VisualType_Note);
	
	~Note();
	
	void update(real p_deltaTime);
	void render();
	void destroyVisual();
	
	inline void setEditCaretVisible(bool p_caretVisible) { m_editCaretVisible = p_caretVisible; }
	inline bool isEditCaretVisible() const               { return m_editCaretVisible;           }
	
	NotePtr clone() const;
	
	void setPosition (const tt::math::Vector2&    p_pos);
	void setWorldRect(const tt::math::VectorRect& p_rect);
	void setText     (const std::wstring&         p_text);
	
	inline const tt::math::VectorRect& getWorldRect() const { return m_worldRect; }
	inline const std::wstring&         getText()      const { return m_text;      }
	
private:
	class NoteVisual
	{
	public:
		enum Change
		{
			Change_Pos,
			Change_Size,
			Change_Text,
			
			Change_Count
		};
		
		explicit NoteVisual(Note* p_note, VisualType p_visualType);
		
		void update(real p_deltaTime, VisualType p_visualType);
		void render();
		void handleNoteChanged(Change p_change);
		
	private:
		typedef tt::code::BitMask<Change, Change_Count> ChangeMask;
		
		// No copying
		NoteVisual(const NoteVisual&);
		NoteVisual& operator=(const NoteVisual&);
		
		
		const Note* const m_note;
		ChangeMask        m_changes;
		
		tt::engine::renderer::QuadSpritePtr m_backgroundQuad;
		tt::engine::renderer::TexturePtr    m_textTex;
		tt::engine::renderer::QuadSpritePtr m_textQuad;
		tt::math::Point2                    m_textAvailablePixelArea;
		bool                                m_editCaretVisible;
		bool                                m_appendCaretToText;
		real                                m_caretToggleTime;
	};
	
	static const s32 textMargin    = 5;
	static const s32 pixelsInTiles = 64;
	
	Note(const tt::math::VectorRect& p_rect, const std::wstring& p_text, VisualType p_visualType);
	void createVisual();
	void handleNoteChanged(NoteVisual::Change p_change);
	
	// No copying
	Note(const Note&);
	Note& operator=(const Note&);
	
	
	tt::math::VectorRect m_worldRect;
	std::wstring         m_text;
	NoteVisual*          m_visual;
	VisualType           m_visualType;
	bool                 m_editCaretVisible;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_NOTE_H)
