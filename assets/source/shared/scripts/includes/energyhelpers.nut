::g_energyUsed <- 0;

class EnergyLevel
{
	static function reportEnergyUsage(p_usedEnergy)
	{
		::g_energyUsed += p_usedEnergy;
	}
	
	static function reportEnergyResupply(p_resuppliedEnergy)
	{
		reportEnergyUsage(-p_resuppliedEnergy);
	}
	
	static function getUsedEnergy()
	{
		return ::g_energyUsed;
	}
} 