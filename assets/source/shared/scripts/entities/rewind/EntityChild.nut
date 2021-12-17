class EntityChild extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	static c_internalsRegistryPrefix         = "child_";
	static c_internalsRegistryUpdatePrefix   = "update_";
	static c_internalsStoredOnLevelExit      = null;
	static c_internalsStoredOnMissionRestart = null;
	static c_internalsStoredOnDeath          = null;
	static c_internalsStoredForUpdate        = null;
	
	_parent       = null;
	_permanentKey = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// FIXME: Get rid of this
		createMovementController(false); // needed for parenting
		setCanBePushed(false);
		setUpdateSurvey(false);
		
		if (c_internalsStoredOnLevelExit != null)
		{
			_restoreTemporaryInternals();
		}
		
		if (c_internalsStoredOnMissionRestart != null || c_internalsStoredOnDeath != null)
		{
			_permanentKey = c_internalsRegistryPrefix +
				::ProgressMgr.getGameMode().tostring() + 
				::ProgressMgr.getPlayMode().tostring() + getType();
		}
		
		// FIXME: Enable this
		//::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		if (_parent != null)
		{
			_parent.removeChild(this);
		}
		
		_storePermanentInternals(_permanentKey, c_internalsStoredOnDeath);
	}
	
	function onParentDied(p_parent)
	{
		// Always die when parent dies
		::killEntity(this);
	}
	
	function onStartMission(p_missionID)
	{
		if (c_internalsStoredForUpdate != null)
		{
			if (::ProgressMgr.getPlayMode() != PlayMode.Campaign)
			{
				return;
			}
			
			// Ensure lastMission ID equals this mission
			if (::ProgressMgr.getLastMissionID() != p_missionID)
			{
				::tt_panic("Last set mission ID '" + ::ProgressMgr.getLastMissionID() +
				           "' should equal this mission id '" + p_missionID + "' but it doesn't. " + 
				           "Fatal upgrade error!");
				return;
			}
			
			local key = getUpdatePrefix(::ProgressMgr.getGameMode()) + getType();
			_storePermanentInternals(key, c_internalsStoredForUpdate);
		}
	}
	
	function onRestoreFromUpdate()
	{
		if (c_internalsStoredForUpdate != null)
		{
			if (::ProgressMgr.getPlayMode() != PlayMode.Campaign)
			{
				return;
			}
			
			local key = getUpdatePrefix(::ProgressMgr.getGameMode()) + getType();
			if (::getRegistry().existsPersistent(key))
			{
				local table = ::getRegistry().getPersistent(key);
				_setInternalsFromTable(table);
			}
			else
			{
				::tt_panic("Missing key '" + key + "' in registry for update process");
			}
		}
	}
	
	function onMissionRestart()
	{
		_storePermanentInternals(_permanentKey, c_internalsStoredOnMissionRestart);
	}
	
	function onLevelExit()
	{
		_storeTemporaryInternals(c_internalsStoredOnLevelExit);
	}
	
	function onDebugPrepareProgressRestore()
	{
		// Treat same as death (both unserialize the gamestate)
		_storePermanentInternals(_permanentKey, c_internalsStoredOnDeath);
	}
	
	function onReloadRequested()
	{
		// This callback is only fired in test builds; treat reload request as level exit
		_storeTemporaryInternals(c_internalsStoredOnLevelExit);
	}
	
	function onProgressRestored(p_id)
	{
		if (c_internalsStoredOnMissionRestart != null || c_internalsStoredOnDeath != null)
		{
			_restorePermanentInternals();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getParent()
	{
		return _parent;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Child Update
	
	// override this; DON'T use update() for EntityChilds
	function childUpdate(p_deltaTime)
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Protected Methods
	
	function _storePermanentInternals(p_key, p_internals)
	{
		if (p_internals == null)
		{
			return;
		}
		
		local table = ::getRegistry().getPersistent(p_key);
		
		if (table == null)
		{
			table = {};
		}
		
		_addInternalsToTable(table, p_internals);
		
		::getRegistry().setPersistent(p_key, table);
	}
	
	function _storeTemporaryInternals(p_internals)
	{
		if (p_internals == null)
		{
			return;
		}
		
		local regKey = c_internalsRegistryPrefix + getType();
		local table = ::getRegistry().get(regKey);
		
		if (table == null)
		{
			table = {};
		}
		
		_addInternalsToTable(table, p_internals);
		
		::getRegistry().set(regKey, table);
	}
	
	function _restorePermanentInternals()
	{
		local table = ::getRegistry().getPersistent(_permanentKey);
		
		_setInternalsFromTable(table);
		
		// Make sure we delete the entry after we used it
		::getRegistry().erasePersistent(_permanentKey);
	}
	
	function _restoreTemporaryInternals()
	{
		local regKey = c_internalsRegistryPrefix + getType();
		local table = ::getRegistry().get(regKey);
		
		_setInternalsFromTable(table);
		
		// Make sure we delete the entry after we used it
		::getRegistry().erase(regKey);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods
	
	function _addInternalsToTable(p_table, p_internals)
	{
		foreach (key in p_internals)
		{
			local members = ::split(key, ".");
			switch (members.len())
			{
			case 1: p_table[key] <- this[key]; break;
			case 2: p_table[key] <- this[members[0]][members[1]]; break;
			case 3: p_table[key] <- this[members[0]][members[1]][members[2]]; break;
			default:
				::tt_panic("Unhandled members length '" + members.len() + "'");
				break;
			}
		}
	}
	
	function _setInternalsFromTable(p_table)
	{
		if (p_table != null)
		{
			foreach (key, value in p_table)
			{
				local members = ::split(key, ".");
				switch (members.len())
				{
				case 1: this[key]                                = value; break;
				case 2: this[members[0]][members[1]]             = value; break;
				case 3: this[members[0]][members[1]][members[2]] = value; break;
				default:
					::tt_panic("Unhandled members length '" + members.len() + "'");
					break;
				}
			}
		}
	}
	
	// Static methods
	function getUpdatePrefix(p_gameMode)
	{
		return c_internalsRegistryUpdatePrefix + p_gameMode.tostring();
	}
	
	function erasePermanentSettings(p_gameMode, p_playMode)
	{
		local count = ::getRegistry().sizePersistent();
		local prefix = c_internalsRegistryPrefix + p_gameMode.tostring() + p_playMode.tostring();
		local upgradePrefix = p_playMode == PlayMode.Campaign ? ::EntityChild.getUpdatePrefix(p_gameMode) : null;
		local removeKeys = [];
		for (local i = 0; i < count; ++i)
		{
			local key = ::getRegistry().getKeyPersistentAt(i);
			if (::stringStartsWith(key, prefix) || (upgradePrefix != null && ::stringStartsWith(key, upgradePrefix)))
			{
				removeKeys.push(key);
			}
		}
		
		foreach (key in removeKeys)
		{
			::getRegistry().erasePersistent(key);
		}
	}
}
