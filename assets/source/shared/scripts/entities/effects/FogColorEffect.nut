include_entity("effects/BaseEffect");

class FogColorEffect extends BaseEffect
</
	editorImage    = "editor.fogcoloreffect"
	libraryImage   = "editor.library.fogcoloreffect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Fog Color"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type   = "color_rgb"
	/>
	color = null;
	
	function setEffect()
	{
		if (color != null)
		{
			EffectMgr.addFogColor(getEffectArea(), baseStrength, color);
		}
	}
}
