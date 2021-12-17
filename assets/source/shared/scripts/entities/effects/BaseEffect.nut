
class BaseEffect extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
/>
{
	</
		type   = "entity"
		filter = ["SoftRect"]
	/>
	effectArea  = null; // the entity that contains the effect area
	_effectArea = null; // the actual effect shape (effectRect, but possibly in the future circles too?)
	
	</
		type = "float"
		min  = 0.0
		max  = 1.0
	/>
	baseStrength = 1.0;
	
	function getEffectArea()
	{
		return _effectArea;
	}
	
	function onSpawn()
	{
		_effectArea = effectArea.getEffectArea();
		setEffect();
		::killEntity(this);
	}
	
	// base function
	function setEffect()
	{
	}
	
	function _typeof()
	{
		return "Effect";
	}
}
