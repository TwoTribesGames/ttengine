class MetaDataHelper
{
	//
	function getLevelMetaData(p_level)
	{
		local metaData = getMetaData();
		
		if (p_level in metaData)
		{
			return metaData[p_level];
		}
		return null;
	}
}