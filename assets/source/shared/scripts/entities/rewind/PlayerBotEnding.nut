include_entity("rewind/PlayerBot");

class PlayerBotEnding extends PlayerBot
</
	editorImage         = "editor.playerbot"
	libraryImage        = "editor.library.playerbot"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [0.0, 0.75, 1.5, 1.5]
	order               = 0
	ignoreSpawnSections = true
	group               = "99. Misc."
/>
{
	</
		type           = "entity"
		filter         = ["Trigger"]
		referenceColor = ReferenceColors.Enable
		order          = 0.0
	/>
	fuelEmptyTrigger = null;
	
	static c_refuelInfo = [[0.1, 1.5], [0.1, 0.35],[0.1, 0.5], [0.1, 0.35], [0.25, 0.5], [0.1, 0.25], [0.1, 0.1], [0.05, 0.05], [0.5, 0.1]]; // delay, fuel
	_refuelIdx = 0;
	
	function onInit()
	{
		base.onInit();
		
		addThrusters();
		
		addProperty("dieQuietly");
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		_healthBar.setNormalizedHealth(0.5);
	}
	
	function onFuelEmpty()
	{
		if (_refuelIdx < c_refuelInfo.len()-1)
		{
			startCallbackTimer("onRefuel", c_refuelInfo[_refuelIdx][0]);
		}
		else if (fuelEmptyTrigger != null)
		{
			fuelEmptyTrigger._triggerEnter(this, this);
			fuelEmptyTrigger._triggerExit(this, this);
		}
	}
	
	function onRefuel()
	{
		_thrusters.zeroGravity._presentation.addTag(_refuelIdx.tostring());
		_thrusters.zeroGravity._fuel = c_refuelInfo[_refuelIdx][1];
		_refuelIdx++;
	}
	
	function onDie()
	{
		base.onDie();
		
		local fade = createPersistentFade(this, "ending");
		fade.setDelegate(function() 
		{
			if (::ProgressMgr.getPlayMode() == PlayMode.Campaign)
			{
				::ProgressMgr.resetCampaign();
			}
			
			::ProgressMgr.startMainMenu();
		});
	}
	
	function addThrusters()
	{
		_thrusters =
			{
				normal = addChild("Thruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_movementSettings       = _movementSettings,
					_thrusterPower          = 0,
					_thrusterBoostPower     = 62,
					_thrusterBoostDampening = 0.37,
					_maxThrusterBoosts      = 2,
					_flySteerForce          = 90,
					_flyDrag                = 4.0,
					_walkSteerForce         = 1050,
					_walkDrag               = 50
				}),
				
				zeroGravity = addChild("RotatableThruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_soundCueActiveName     = "playerbot_thruster_zerogravity"
					_movementSettings       = _movementSettings,
					_thrusterPower          = 3.5,
					_thrusterBoostPower     = 2,
					_thrusterBoostDampening = 0.5,
					_maxThrusterBoosts      = 0,
					_drag                   = 8.0,
					_fuel                   = 1.0
				}),
				
				water = addChild("WaterThruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_soundCueActiveName     = "playerbot_thruster_water"
					_movementSettings       = _movementSettings,
					_thrusterPower          = 2.0,
					_maxThrusterPower       = 4.5,
					_thrusterBoostPower     = 1.5,
					_thrusterBoostDampening = 0.5,
					_maxThrusterBoosts      = 0,
					_drag                   = 10.0,
				})
			};
		
		setThruster(_thrusters.zeroGravity);
	}
}
PlayerBotEnding.setattributes("positionCulling", null);
PauseMenu.makePauseMenuOpener(PlayerBotEnding);
Trigger.makeTriggerParentable(PlayerBotEnding);
