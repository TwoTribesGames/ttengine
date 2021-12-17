include_entity("effects/BaseEffect");

class ColorGradingEffect extends BaseEffect
</
	editorImage    = "editor.colorgradingeffect"
	libraryImage   = "editor.library.colorgradingeffect"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	displayName    = "Effect - Color Grading"
	group          = "08. Soft Rect Effects"
/>
{
	</
		type        = "string"
		choice      = getColorGradingNames()
		order       = 1
	/>
	colorGrading = null;
	
	colorGradingTexture = null;
	
	function onInit()
	{
		//base.onInit();
		if (colorGrading != null)
		{
			colorGradingTexture = ::getColorGradingTexture(colorGrading);
			if (colorGradingTexture == null)
			{
				editorWarning("Invalid Color grading '" + colorGrading + "'!");
			}
		}
		else
		{
			editorWarning("No Color grading selected!");
		}
	}
	
	function setEffect()
	{
		EffectMgr.addColorGrading(this, getEffectArea(), baseStrength, colorGradingTexture);
	}
}
