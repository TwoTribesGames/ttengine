include_entity("gui/GUIButton");

class GUIConfirmationButton extends GUIButton
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.085, 0.05 ]
/>
{
	static c_fadeAndCloseAll = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onCursorReleasedOnEntity(p_duration)
	{
		local titleID = "POPUPMENU_HEADER_" + _content[0].toupper();
		
		// Hack
		local isCustomPopup =  _content.len() == 3 && typeof(_content[1]) == "string" && typeof(_content[2]) == "Vector2";
		
		::MenuScreen.pushScreen("PopupMenu",
		{
			_titleID         = titleID,
			_subTextID       = isCustomPopup ? titleID +  _content[1].toupper() : null,
			_subTextPosition = isCustomPopup ? _content[2] : null,
			_caller          = this.weakref(),
			_activator       = _caller._activator,
			_fadeAndCloseAll = c_fadeAndCloseAll
		});
	}
	
	function onPopupMenuAccepted(p_menu)
	{
		// call button delegate
		if (_delegate != null)
		{
			_delegate.acall(_args);
		}
	}
}
