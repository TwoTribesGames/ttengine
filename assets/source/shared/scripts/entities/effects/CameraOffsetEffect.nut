include_entity("effects/BaseEffect");

class CameraOffsetEffect extends BaseEffect
</
	editorImage    = "editor.cameraoffseteffect"
	libraryImage   = "editor.library.cameraoffseteffect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Camera Offset"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type  = "float"
		min   = -32.0
		max   = 32.0
		order = 0
	/>
	offsetX = 0.0;
	
	</
		type  = "float"
		min   = -32.0
		max   = 32.0
		order = 1
	/>
	offsetY = 0.0;
	
	function setEffect()
	{
		EffectMgr.addCameraOffset(getEffectArea(), baseStrength, ::Vector2(offsetX, offsetY));
	}
}
