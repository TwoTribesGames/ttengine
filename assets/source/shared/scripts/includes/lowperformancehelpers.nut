// actually store the level name in ::g_lowPerformanceToasterVisible so that when we change levels
// while the dialog is visible, it will be displayed again.
::g_lowPerformanceToasterVisible <- "";

function onBadPerformanceDetected()
{
	return; // disable the toaster!
	
	if (Options.wasLowPerfReported() == false && ::g_lowPerformanceToasterVisible != Level.getName())
	{
		local toaster = ::spawnEntity("Toaster", ::Vector2(0, 0), { _locID = "CRAPPY_COMPUTER_WARNING" });
		toaster._removedDelegate = ::reportLowPerformance;
		::g_lowPerformanceToasterVisible = Level.getName();
	}
}

function reportLowPerformance()
{
	::g_lowPerformanceToasterVisible = "";
	Options.setLowPerfReported(true);
}