#include <tt/math/math.h>

#include <toki/game/editor/commands/CommandAddRemoveNotes.h>
#include <toki/game/editor/tools/NotesTool.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/Camera.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/Note.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

//--------------------------------------------------------------------------------------------------
// Public member functions

NotesTool::NotesTool(Editor* p_editor)
:
Tool(p_editor),
m_mode(Mode_None),
m_startedDrag(false),
m_dragEdge(DragEdge_None),
m_dragStartWorldPos(-1.0f, -1.0f),
m_dragStartScreenPos(-1, -1),
m_startingNoteRect(tt::math::Vector2(0.0f, 0.0f), 0.0f, 0.0f),
m_dragOffset(0.0f, 0.0f),
m_activeNote(),
m_addRemoveCommand(),
m_changeRectCommand()
{
}


NotesTool::~NotesTool()
{
}


void NotesTool::onActivate()
{
	m_mode = Mode_None;
}


void NotesTool::onDeactivate()
{
	getEditor()->restoreDefaultCursor();
}


bool NotesTool::canDeactivate() const
{
	return m_dragEdge == DragEdge_None;
}


void NotesTool::onPointerHover(const PointerState& p_pointerState)
{
	if (m_mode == Mode_None)
	{
		Editor* editor = getEditor();
		
		level::NotePtr hoverNote = editor->getLevelData()->findNoteOverlappingPosition(p_pointerState.worldPos);
		if (hoverNote != 0)
		{
			setCursorForDragEdge(getNoteResizeEdge(hoverNote, p_pointerState));
		}
		else
		{
			editor->setCursor(EditCursor_AddNote);
		}
	}
}


void NotesTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	if (m_mode == Mode_Remove)
	{
		return;
	}
	
	level::NotePtr existingNote = getEditor()->getLevelData()->findNoteOverlappingPosition(p_pointerState.worldPos);
	if (existingNote != 0)
	{
		// Clicked an existing note
		
		if (p_pointerState.alt.down)
		{
			m_mode = Mode_None;
			getEditor()->enterNoteTextEditMode(existingNote, false);
		}
		else
		{
			m_startedDrag = false;
			m_activeNote  = existingNote;
			m_dragEdge    = getNoteResizeEdge(existingNote, p_pointerState);
			setCursorForDragEdge(m_dragEdge);
			
			m_dragStartWorldPos  = p_pointerState.worldPos;
			m_dragStartScreenPos = p_pointerState.screenPosCur;
			
			if (m_dragEdge == DragEdge_None)
			{
				// Move note
				m_mode       = Mode_Move;
				m_dragOffset = p_pointerState.worldPos - existingNote->getWorldRect().getPosition();
			}
			else
			{
				// Resize note
				m_mode             = Mode_Resize;
				m_dragOffset       = tt::math::Vector2::zero;
				m_startingNoteRect = existingNote->getWorldRect();
			}
		}
	}
	else
	{
		// Did not click an existing note: create a new one
		m_mode = Mode_Add;
		
		m_dragOffset.setValues(2.0f, 1.0f);
		m_activeNote = level::Note::create(
				tt::math::VectorRect(p_pointerState.worldPos - m_dragOffset, 4.0f, 2.0f),
				L"");
		
		getEditor()->setFocusNote(m_activeNote);
		getEditor()->setCursor(EditCursor_GenericMove);
	}
}


void NotesTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	if (m_mode == Mode_Remove)
	{
		return;
	}
	
	// Can never really do anything useful if there is no note to work on
	if (m_activeNote == 0)
	{
		return;
	}
	
	if (m_mode == Mode_Add)
	{
		m_activeNote->setPosition(p_pointerState.worldPos - m_dragOffset);
	}
	else if (m_mode == Mode_Move)
	{
		if (m_startedDrag == false)
		{
			if (tt::math::distance(m_dragStartScreenPos, p_pointerState.screenPosCur) < 5)
			{
				// Pointer did not move enough yet
				return;
			}
			
			m_startedDrag = true;
			TT_ASSERT(m_changeRectCommand == 0);
			m_changeRectCommand = commands::CommandChangeNoteRect::create(m_activeNote);
		}
		
		TT_NULL_ASSERT(m_changeRectCommand);
		m_changeRectCommand->setPosition(p_pointerState.worldPos - m_dragOffset);
	}
	else if (m_mode == Mode_Resize)
	{
		if (m_dragEdge == DragEdge_None)
		{
			return;
		}
		
		if (m_startedDrag == false)
		{
			if (tt::math::distance(m_dragStartScreenPos, p_pointerState.screenPosCur) < 5)
			{
				// Pointer did not move enough yet
				return;
			}
			
			m_startedDrag = true;
			TT_ASSERT(m_changeRectCommand == 0);
			m_changeRectCommand = commands::CommandChangeNoteRect::create(m_activeNote);
		}
		
		const tt::math::Vector2 unitsMoved(p_pointerState.worldPos - m_dragStartWorldPos);
		
		// Reasoning behind the max size: a texture is created for the note text. This texture is
		// 64 times the note world size (so that the characters aren't blurry). So, multiply
		// the max note size by 64 to know how many pixels the note texture will be.
		// A max note size of 16 means a max texture size of 1024 pixels.
		static const tt::math::Vector2 minSize(0.8f,  0.8f);
		static const tt::math::Vector2 maxSize(16.0f, 16.0f);
		//static const tt::math::Vector2 maxSize(32.0f, 32.0f);  // == max texture of 2048 pixels
		
		tt::math::VectorRect newRect(m_startingNoteRect);
		switch (m_dragEdge)
		{
		case DragEdge_TopRight:
		case DragEdge_Right:
		case DragEdge_BottomRight:
			{
				real newWidth = m_startingNoteRect.getWidth() + unitsMoved.x;
				tt::math::clamp(newWidth, minSize.x, maxSize.x);
				newRect.setWidth(newWidth);
			}
			break;
			
		case DragEdge_TopLeft:
		case DragEdge_Left:
		case DragEdge_BottomLeft:
			{
				real newLeft = m_startingNoteRect.getLeft() + unitsMoved.x;
				tt::math::clamp(newLeft,
				                newRect.getMaxEdge().x - maxSize.x,
				                newRect.getRight()     - minSize.x);
				newRect.setLeft(newLeft);
			}
			break;
			
		default:
			break;
		}
		
		switch (m_dragEdge)
		{
		case DragEdge_TopLeft:
		case DragEdge_Top:
		case DragEdge_TopRight:
			{
				real newHeight = m_startingNoteRect.getHeight() + unitsMoved.y;
				tt::math::clamp(newHeight, minSize.y, maxSize.y);
				newRect.setHeight(newHeight);
			}
			break;
			
		case DragEdge_BottomLeft:
		case DragEdge_Bottom:
		case DragEdge_BottomRight:
			{
				real newTop = m_startingNoteRect.getTop() + unitsMoved.y;
				tt::math::clamp(newTop,
				                newRect.getMaxEdge().y - maxSize.y,
				                newRect.getBottom()    - minSize.y);
				newRect.setTop(newTop);
			}
			break;
			
		default:
			break;
		}
		
		TT_NULL_ASSERT(m_changeRectCommand);
		m_changeRectCommand->setRect(newRect);
	}
}


void NotesTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	if (m_mode == Mode_Remove)
	{
		return;
	}
	
	const bool     editAfterCommit = (m_mode == Mode_Add);
	level::NotePtr noteToEdit      = m_activeNote;
	
	// Releasing the pointer applies the command (whichever it is: add, move, resize, ...)
	commitUndoCommand();
	
	m_mode = Mode_None;
	m_activeNote.reset();
	
	m_startedDrag = false;
	m_dragEdge    = DragEdge_None;
	
	if (editAfterCommit)
	{
		TT_NULL_ASSERT(noteToEdit);
		getEditor()->enterNoteTextEditMode(noteToEdit, editAfterCommit);
	}
}


void NotesTool::onPointerRightPressed(const PointerState& p_pointerState)
{
	// Do not erase if tool is already in a different mode
	if (m_mode != Mode_None)
	{
		return;
	}
	
	Editor* editor = getEditor();
	
	editor->setCursor(EditCursor_Erase);
	
	level::Notes notesToDelete(editor->getLevelData()->findAllNotesOverlappingPosition(p_pointerState.worldPos));
	if (notesToDelete.empty() == false)
	{
		TT_ASSERT(m_addRemoveCommand == 0);
		m_addRemoveCommand = commands::CommandAddRemoveNotes::createForRemove(editor);
		m_addRemoveCommand->addNotes(notesToDelete);
	}
	
	m_mode = Mode_Remove;
}


void NotesTool::onPointerRightDown(const PointerState& p_pointerState)
{
	if (m_mode != Mode_Remove)
	{
		return;
	}
	
	Editor* editor = getEditor();
	
	level::Notes notesToDelete(editor->getLevelData()->findAllNotesOverlappingPosition(p_pointerState.worldPos));
	if (notesToDelete.empty() == false)
	{
		if (m_addRemoveCommand == 0)
		{
			m_addRemoveCommand = commands::CommandAddRemoveNotes::createForRemove(editor);
		}
		m_addRemoveCommand->addNotes(notesToDelete);
	}
}


void NotesTool::onPointerRightReleased(const PointerState& /*p_pointerState*/)
{
	if (m_mode == Mode_Remove)
	{
		commitUndoCommand();
		
		m_mode = Mode_None;
	}
}


std::wstring NotesTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_NOTES");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void NotesTool::commitUndoCommand()
{
	if (m_addRemoveCommand != 0 && m_addRemoveCommand->hasNotes())
	{
		getEditor()->pushUndoCommand(m_addRemoveCommand);
	}
	
	if (m_changeRectCommand != 0)
	{
		getEditor()->pushUndoCommand(m_changeRectCommand);
	}
	
	m_activeNote.reset();
	m_addRemoveCommand.reset();
	m_changeRectCommand.reset();
	
	getEditor()->resetFocusNote();
}


NotesTool::DragEdge NotesTool::getNoteResizeEdge(const level::NotePtr& p_note,
                                                 const PointerState&   p_pointerState) const
{
	TT_NULL_ASSERT(p_note);
	const tt::math::VectorRect& noteRect(p_note->getWorldRect());
	
	static const s32 borderThickness = 7;
	
	const Camera& cam(getEditor()->getEditorCamera());
	tt::math::Point2 minPosScr(cam.worldToScreen(noteRect.getMin()));
	tt::math::Point2 maxPosScr(cam.worldToScreen(noteRect.getMaxEdge()));
	
	using std::swap;
	swap(minPosScr.y, maxPosScr.y);
	
	DragEdge dragEdge = DragEdge_None;
	
	enum HitRegion
	{
		HitRegion_None,
		HitRegion_OuterMin,
		HitRegion_Inner,
		HitRegion_OuterMax
	};
	
	HitRegion hitHorz = HitRegion_None;
	HitRegion hitVert = HitRegion_None;
	
	if (p_pointerState.screenPosCur.x <= (minPosScr.x + borderThickness) &&
	    p_pointerState.screenPosCur.x >= minPosScr.x)
	{
		hitHorz = HitRegion_OuterMin;
	}
	else if (p_pointerState.screenPosCur.x >/*=*/ (minPosScr.x + borderThickness) &&
	         p_pointerState.screenPosCur.x </*=*/ (maxPosScr.x - borderThickness))
	{
		hitHorz = HitRegion_Inner;
	}
	else if (p_pointerState.screenPosCur.x >= (maxPosScr.x - borderThickness) &&
	         p_pointerState.screenPosCur.x <= maxPosScr.x)
	{
		hitHorz = HitRegion_OuterMax;
	}
	
	if (p_pointerState.screenPosCur.y <= (minPosScr.y + borderThickness) &&
	    p_pointerState.screenPosCur.y >= minPosScr.y)
	{
		hitVert = HitRegion_OuterMin;
	}
	else if (p_pointerState.screenPosCur.y >/*=*/ (minPosScr.y + borderThickness) &&
	         p_pointerState.screenPosCur.y </*=*/ (maxPosScr.y - borderThickness))
	{
		hitVert = HitRegion_Inner;
	}
	else if (p_pointerState.screenPosCur.y >= (maxPosScr.y - borderThickness) &&
	         p_pointerState.screenPosCur.y <= maxPosScr.y)
	{
		hitVert = HitRegion_OuterMax;
	}
	
	switch (hitVert)
	{
	case HitRegion_OuterMin:
	case HitRegion_OuterMax:
		switch (hitHorz)
		{
		case HitRegion_OuterMin: dragEdge = (hitVert == HitRegion_OuterMax) ? DragEdge_BottomLeft  : DragEdge_TopLeft;  break;
		case HitRegion_Inner:    dragEdge = (hitVert == HitRegion_OuterMax) ? DragEdge_Bottom      : DragEdge_Top;      break;
		case HitRegion_OuterMax: dragEdge = (hitVert == HitRegion_OuterMax) ? DragEdge_BottomRight : DragEdge_TopRight; break;
		default: break;
		}
		break;
		
	case HitRegion_Inner:
		if (hitHorz == HitRegion_OuterMin)
		{
			dragEdge = DragEdge_Left;
		}
		else if (hitHorz == HitRegion_OuterMax)
		{
			dragEdge = DragEdge_Right;
		}
		break;
		
	default: break;
	}
	
	return dragEdge;
}


NotesTool::DragEdge NotesTool::getDragEdge(const tt::math::Vector2& p_worldPos) const
{
	DragEdge dragEdge = DragEdge_None;
	
	// Determine which part of the border was picked
	const real borderThickness = getEditor()->getLevelBorderThickness();
	level::LevelDataPtr levelData(getEditor()->getLevelData());
	
	const tt::math::PointRect levelRect(levelData->getLevelRect());
	const tt::math::Vector2   levelWorldMinPos(level::tileToWorld(levelRect.getMin()));
	const tt::math::Vector2   levelWorldMaxPos(level::tileToWorld(levelRect.getMaxEdge()));
	
	enum HitRegion
	{
		HitRegion_None,
		HitRegion_OuterMin,
		HitRegion_Inner,
		HitRegion_OuterMax
	};
	
	HitRegion hitHorz = HitRegion_None;
	HitRegion hitVert = HitRegion_None;
	
	if (p_worldPos.x <= levelWorldMinPos.x &&
	    p_worldPos.x >= (levelWorldMinPos.x - borderThickness))
	{
		hitHorz = HitRegion_OuterMin;
	}
	else if (p_worldPos.x >= levelWorldMinPos.x &&
	         p_worldPos.x <= levelWorldMaxPos.x)
	{
		hitHorz = HitRegion_Inner;
	}
	else if (p_worldPos.x >= levelWorldMaxPos.x &&
	         p_worldPos.x <= (levelWorldMaxPos.x + borderThickness))
	{
		hitHorz = HitRegion_OuterMax;
	}
	
	if (p_worldPos.y <= levelWorldMinPos.y &&
	    p_worldPos.y >= (levelWorldMinPos.y - borderThickness))
	{
		hitVert = HitRegion_OuterMin;
	}
	else if (p_worldPos.y >= levelWorldMinPos.y &&
	         p_worldPos.y <= levelWorldMaxPos.y)
	{
		hitVert = HitRegion_Inner;
	}
	else if (p_worldPos.y >= levelWorldMaxPos.y &&
	         p_worldPos.y <= (levelWorldMaxPos.y + borderThickness))
	{
		hitVert = HitRegion_OuterMax;
	}
	
	switch (hitVert)
	{
	case HitRegion_OuterMin:
	case HitRegion_OuterMax:
		switch (hitHorz)
		{
		case HitRegion_OuterMin: dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_BottomLeft  : DragEdge_TopLeft;  break;
		case HitRegion_Inner:    dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_Bottom      : DragEdge_Top;      break;
		case HitRegion_OuterMax: dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_BottomRight : DragEdge_TopRight; break;
		default: break;
		}
		break;
		
	case HitRegion_Inner:
		if (hitHorz == HitRegion_OuterMin)
		{
			dragEdge = DragEdge_Left;
		}
		else if (hitHorz == HitRegion_OuterMax)
		{
			dragEdge = DragEdge_Right;
		}
		break;
		
	default: break;
	}
	
	return dragEdge;
}


void NotesTool::setCursorForDragEdge(DragEdge p_edge)
{
	EditCursor cursor = EditCursor_NormalArrow;
	switch (p_edge)
	{
	//case DragEdge_None:        cursor = EditCursor_NotAllowed;               break;
	//case DragEdge_None:        cursor = EditCursor_NormalArrow;              break;
	case DragEdge_None:        cursor = EditCursor_GenericMove;              break;
	case DragEdge_TopLeft:     cursor = EditCursor_ResizeTopLeftBottomRight; break;
	case DragEdge_Top:         cursor = EditCursor_ResizeTopBottom;          break;
	case DragEdge_TopRight:    cursor = EditCursor_ResizeTopRightBottomLeft; break;
	case DragEdge_Right:       cursor = EditCursor_ResizeLeftRight;          break;
	case DragEdge_BottomRight: cursor = EditCursor_ResizeTopLeftBottomRight; break;
	case DragEdge_Bottom:      cursor = EditCursor_ResizeTopBottom;          break;
	case DragEdge_BottomLeft:  cursor = EditCursor_ResizeTopRightBottomLeft; break;
	case DragEdge_Left:        cursor = EditCursor_ResizeLeftRight;          break;
	default: break;
	}
	
	getEditor()->setCursor(cursor);
}

// Namespace end
}
}
}
}
