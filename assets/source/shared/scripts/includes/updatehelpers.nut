tt_include("includes/optionshelpers");

function getVersionString()
{
	local version = "v1.22 (build: " + ::getVersion();
	local region = ::getRegion();
	if (region.len() > 0) version += " " + region.toupper();
	version += ", save: " + ::ProgressMgr.c_version + ")";
	return version;
}

function handleUpdateStats(p_version)
{
	::echo("*** UPDATING STATS FROM " + p_version + " TO " + ::ProgressMgr.c_version + " ***");

	// Do upgrade
}

function handleUpdate(p_progressData)
{
	::echo("*** UPDATING PROGRESS FROM " + p_progressData.version + " TO " + ::ProgressMgr.c_version + " ***");

	// Remove all checkpoints when updating; they cannot be trusted anymore
	::CheckPointMgr.clearAll();

	if (p_progressData.version < 26 || p_progressData.version > ::ProgressMgr.c_version)
	{
		// Cannot upgrade; pre 26 version or going back from higher version
		::tt_panic("Cannot upgrade savedata from version '" + p_progressData.version + "' to version '" + ::ProgressMgr.c_version + "'");
		return null;
	}

	// Do upgrade

	p_progressData.version = ::ProgressMgr.c_version;
	return p_progressData;
}
