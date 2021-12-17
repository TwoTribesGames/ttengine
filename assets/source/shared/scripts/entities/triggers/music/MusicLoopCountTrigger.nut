include_entity("triggers/Trigger");

class MusicLoopCountTrigger extends Trigger
</
	editorImage    = "editor.musicloopcounttrigger"
	libraryImage   = "editor.library.musicloopcounttrigger"
	placeable      = Placeable_Developer
	displayName    = "Sound - Music Loop Count Trigger"
	group          = "06. Sound"
	stickToEntity  = false
/>
{
	</
		type           = "entity"
		filter         = ["MusicSource"]
		group          = "Specific Settings"
		order          = 0.0
	/>
	musicSource = null;
	
	</
		type           = "integer"
		min            = 1
		max            = 10
		group          = "Specific Settings"
		order          = 0.0
	/>
	loopCount = 1;
	
	_currentLoopCount = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (musicSource == null)
		{
			editorWarning("no musicsource");
			return;
		}
		
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		musicSource.subscribeLoopEventListener(this);
	}
	
	function onDisabled()
	{
		musicSource.unsubscribeLoopEventListener(this);
	}
	
	function onMusicTrackLooped(p_track)
	{
		_currentLoopCount++;
		
		if (loopCount == _currentLoopCount)
		{
			_triggerEnter(this, null);
			_triggerExit(this, null);
			setEnabled(false);
		}
	}
}
MusicLoopCountTrigger.setattributes("once", null);
MusicLoopCountTrigger.setattributes("type", null);
MusicLoopCountTrigger.setattributes("width", null);
MusicLoopCountTrigger.setattributes("height", null);
MusicLoopCountTrigger.setattributes("radius", null);
MusicLoopCountTrigger.setattributes("triggerOnTouch", null);
MusicLoopCountTrigger.setattributes("triggerOnUncull", null);
MusicLoopCountTrigger.setattributes("parentTrigger", null);
MusicLoopCountTrigger.setattributes("triggerFilter", null);
MusicLoopCountTrigger.setattributes("filterAllEntitiesOfSameType", null);
