include_entity("effects/BaseEffect");

class FovEffect extends BaseEffect
</
	editorImage    = "editor.foveffect"
	libraryImage   = "editor.library.foveffect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - FOV"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type   = "integer"
		min    = 1
		max    = 180
		order  = 1
	/>
	FOV = 40;
	
	function setEffect()
	{
		EffectMgr.addCameraFov(getEffectArea(), baseStrength, FOV);
	}
}
