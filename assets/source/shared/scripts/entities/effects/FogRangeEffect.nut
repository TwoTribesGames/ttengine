include_entity("effects/BaseEffect");

class FogRangeEffect extends BaseEffect
</
	editorImage    = "editor.fograngeeffect"
	libraryImage   = "editor.library.fograngeeffect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Fog Range"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type   = "integer"
		min    = -100
		max    = 250
		order  = 1
	/>
	near = -10;
	
	</
		type   = "integer"
		min    = -100
		max    = 250
		order  = 2
	/>
	far = 100;
	
	function setEffect()
	{
		EffectMgr.addFogNearFar(getEffectArea(), baseStrength, near, far);
	}
}
