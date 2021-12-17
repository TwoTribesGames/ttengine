include_entity("triggers/EntityEvaluationTrigger");

function getEnemyEvaluationConditions()
{
	return [
		"isHacked"
	];
}

class EnemyEvaluationTrigger extends EntityEvaluationTrigger
</
	editorImage    = "editor.enemyevaluationtrigger"
	libraryImage   = "editor.library.enemyevaluationtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Enemy Evaluation"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "entityid_array"
		filter         = ["BaseEnemy"]
		order          = 101
		group          = "Filter"
		referenceColor = ReferenceColors.List
	/>
	triggerFilter = null;
	
	</
		type   = "string"
		choice = ::getEnemyEvaluationConditions()
		group  = "Specific Settings"
		order  = 0.0
	/>
	condition = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (triggerFilter == null)
		{
			editorWarning("Specify filtered enemy");
			::killEntity(this);
			return;
		}
		
		base.onInit();
	}
}
