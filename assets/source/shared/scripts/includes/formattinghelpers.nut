enum FormatType
{
	Number,
	Score,
	Percentage,
	LapDifference,
	Time
}

function formatString(p_string, p_type)
{
	switch (p_type)
	{
	case FormatType.Number:        return ::formatNumber(p_string); break;
	case FormatType.Score:         return ::formatScore(p_string); break;
	case FormatType.LapDifference: return ::formatLapDifference(p_string); break;
	case FormatType.Time:          return ::formatTime(p_string); break;
	case FormatType.Percentage:    return ::formatPercentage(p_string); break;
	default:
		::tt_panic("Unhandled type '" + p_type + "'");
		break;
	}
	return p_string;
}

function formatLapDifference(p_diff)
{
	local sign         = p_diff < 0 ? "-" : "+";
	p_diff = ::fabs(p_diff);
	local milliseconds = (p_diff * 1000) % 1000;
	local seconds      = p_diff;
	return sign + ::format("%d.%03d", p_diff, milliseconds);
}

function formatTime(p_time)
{
	local milliseconds = (p_time * 1000) % 1000;
	local seconds      = p_time % 60;
	local minutes      = (p_time / 60) % 60;
	local hours        = (p_time / 3600).tointeger();
	
	return hours > 0 ? 
		::format("%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds) : 
		::format("%02d:%02d.%03d", minutes, seconds, milliseconds);
}

function formatTimeDifference(p_diff)
{
	return (p_diff < 0 ? "-" : "+") + ::formatTime(::abs(p_diff));
}

function formatPercentage(p_number)
{
	return ::format("%.2f%%", p_number * 100.0);
}

function formatScore(p_score)
{
	return ::formatNumber(p_score);
}

function formatDate(p_date)
{
	local y = p_date.slice(0, 4);
	local m = p_date.slice(4, 6);
	local d = p_date.slice(6, 8);
	return y + "-" + m + "-" + d;
}

function formatNumber(p_number)
{
	local number = p_number.tointeger();
	local result = number >= 999 ? ::format("%03d", number % 1000) : ::format("%d", number % 1000);
	while (number >= 999)
	{
		number /= 1000;
		local digits = number >= 999 ? ::format("%03d", number % 1000) : ::format("%d", number % 1000);
		result = digits + "," + result;
	}
	return result;
}

function formatScoreDifference(p_diff)
{
	return (p_diff < 0 ? "-" : "+") + ::formatScore(::abs(p_diff));
}
