// Make sure these two data sets are in sync!
::g_unlockedViruses <- 
{
	VirusUploadTrigger = true,
	HealthBot          = false,
	Turret             = false,
	BumperEnemy        = false,
	Train              = false,
	KamikazeEnemy      = false
};

::g_orderedViruses <-
[
	"VirusUploadTrigger",
	"HealthBot",
	"Turret",
	"Train",
	"BumperEnemy",
	"KamikazeEnemy"
]; 

class VirusHelpers
{
	function getVirusData()
	{
		return ::g_unlockedViruses;
	}
	
	function getAllViruses()
	{
		return ::g_orderedViruses;
	}
	
	function getLockedViruses()
	{
		local result = [];
		foreach (virus in ::g_orderedViruses)
		{
			if (::g_unlockedViruses[virus] == false)
			{
				result.push(virus);
			}
		}
		return result;
	}
	
	//////////////////////////////////////////////////////////////////////////////
	// Public methods
	
	function resetAllViruses()
	{
		foreach (virus, unlocked in ::g_unlockedViruses)
		{
			::g_unlockedViruses[virus] = false;
		}
		
		// VirusUploadTrigger is on by default
		::g_unlockedViruses.VirusUploadTrigger = true;
	}
	
	function hasUnlockedAnyVirus()
	{
		foreach (unlocked in ::g_unlockedViruses)
		{
			if (unlocked)
			{
				return true;
			}
		}
		return false;
	}
	
	function getTotalUnlockedVirusses()
	{
		local total = 0;
		foreach (unlocked in ::g_unlockedViruses)
		{
			if (unlocked)
			{
				++total;
			}
		}
		return total;
	}
	
	function hasUnlockedVirus(p_virus)
	{
		return ::g_unlockedViruses[p_virus];
	}
	
	function unlockVirus(p_virus)
	{
		::g_unlockedViruses[p_virus] = true;
	}
	
	function canUploadVirus(p_entity)
	{
		return ::isValidEntity(p_entity) &&
		       p_entity.hasProperty("noticeVirusUploader") &&
		       p_entity.containsVirus() == false &&
		       ::g_unlockedViruses[p_entity.getType()];
	}
}
