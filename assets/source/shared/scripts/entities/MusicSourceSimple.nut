include_entity("MusicSource");

class MusicSourceSimple extends MusicSource
</ 
	editorImage    = "editor.musicsource"
	libraryImage   = "editor.library.musicsource"
	placeable      = Placeable_UserLevelEditor
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	group          = "06. Sound"
	displayName    = "Sound - Music"
/>
{
	</
		type   = "string"
		choice = getMusicTrackNames()
		order  = 0
	/>
	musicTrack = null;
}
MusicSourceSimple.setattributes("soundLayer", null);
MusicSourceSimple.setattributes("fadeDuration", null);