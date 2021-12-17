include_entity("triggers/EntityEvaluationTrigger");

function getPlayerEvaluationConditions()
{
	return [
		"hasHackedTurret",
		"hasHackedBumper",
		"hasHackedKamikaze",
		"hasHackedHealthBot",
		"hasSecondaryWeapon",
		"hasNotUsedSecondariesInCampaign",
		"hasNotShotDuringMission",
		"hasTurretHack",
		"hasKamikazeHack",
		"hasBumperHack",
		"hasHealthBotHack",
		"hasTrainHack",
		"isInHackMode"
	];
}

class PlayerEvaluationTrigger extends EntityEvaluationTrigger
</
	editorImage    = "editor.playerevaluationtrigger"
	libraryImage   = "editor.library.playerevaluationtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Player Evaluation"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ::getPlayerEvaluationConditions()
		group  = "Specific Settings"
		order  = 0.0
	/>
	condition = null;
}
PlayerEvaluationTrigger.setattributes("triggerFilter", null);
PlayerEvaluationTrigger.setattributes("filterAllEntitiesOfSameType", null);
