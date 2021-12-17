include_entity("triggers/Trigger");

class CameraNofollowTrigger extends Trigger
</ 
	editorImage    = "editor.cameranofollowtrigger"
	libraryImage   = "editor.library.cameranofollowtrigger"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Nofollow"
	group          = "05. Camera"
	stickToEntity  = true
/>

{
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		::Camera.setFollowEntityStacked(false);
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		::Camera.setFollowEntityStacked(true);
	}
}
