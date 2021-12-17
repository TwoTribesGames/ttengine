// Cannot reside in the ::ColorGrading.nut file because this needs to be known when all entity scripts are compiled
enum ColorGradingPriority
{
	Lowest  = -2,
	Low     = -1,
	Normal  = 0,
	Medium  = 1,
	High    = 2,
	Highest = 3
};
