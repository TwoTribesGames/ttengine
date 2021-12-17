include_entity("effects/BaseEffect");

class CameraPositionEffect extends BaseEffect
</
	editorImage    = "editor.camerapositioneffect"
	libraryImage   = "editor.library.camerapositioneffect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Camera Position"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type   = "entity"
		filter = ["Waypoint"]
		order  = 0
	/>
	wayPoint = null;
	
	function onValidateScriptState()
	{
		if (::isValidAndInitializedEntity(wayPoint) == false)
		{
			editorWarning("No valid waypoint set!");
		}
	}
	
	function onSpawn()
	{
		::assert(wayPoint != null, entityIDString(this) + " has no waypoint set!");
		base.onSpawn();
	}
	
	function setEffect()
	{
		local positon = wayPoint.getCenterPosition();
		
		EffectMgr.addCameraPosition(getEffectArea(), baseStrength, positon);
	}
}
