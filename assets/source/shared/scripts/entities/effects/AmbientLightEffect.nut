include_entity("effects/BaseEffect");

class AmbientLightEffect extends BaseEffect
</
	editorImage    = "editor.ambientlighteffect"
	libraryImage   = "editor.library.ambientlighteffect"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Ambient Light"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type  = "integer"
		min   = 0
		max   = 255
		order = 2
	/>
	levelAmbientLight = 128;
	
	// to let the user prefs have effect on the effectrect settings, uncomment this
	/*function onInit()
	{
		levelAmbientLight = ::LightHelpers.getUserAmbientValue(levelAmbientLight);
	}*/
	
	function setEffect()
	{
		EffectMgr.addLightAmbient(getEffectArea(), baseStrength, levelAmbientLight);
	}
}
