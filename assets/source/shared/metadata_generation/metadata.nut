// Called from generate meta data startup mode
function shouldGenerateMetaData(p_levelname)
{
	// generate metadata for all slice levels
	local match = regexp(@"section_.*").match(p_levelname);
	
	//::echo(match, p_levelname);
	return match;
}

// Called from generate meta data startup mode
function generateMetaData(p_levelData, p_resultData)
{
	local levelMetaData = {}; // meta data to actually save
	
	MetaData.prepareAllLevelsData(p_resultData);
	MetaData.prepareLevelMetaData(p_levelData, levelMetaData);
	
	foreach (id, val in p_levelData.entities)
	{
		MetaData.generateEntityData(id, val, levelMetaData);
		MetaData.cacheEntityData(id, val, p_levelData);
	}
	
	//::echo(p_resultData.all_levels, p_resultData.len(), "META all_levels");
	MetaData.updateAllLevelsData(levelMetaData, p_resultData);
	
	// store the level meta data in the result data table, using the level name as key
	p_resultData[p_levelData.properties.name] <- levelMetaData;
}
/*
function validateValueEquals(p_new, p_old, p_failureMessage)
{
	if (p_new != p_old)
	{
		tt_panic(p_failureMessage + " (new: " + p_new + " old: " + p_old + ")");
	}
}

function validateLevel(p_new, p_old, p_levelName)
{
	foreach (newCollectibleId, newCollectibleVal in p_new.entities.Collectible)
	{
		if ((newCollectibleId in p_old.entities.Collectible) == false)
		{
			tt_panic("New Collectible '" + newCollectibleId + "' found in level '" + p_levelName + "'!");			
		}
	}
	
	foreach (oldCollectibleId, oldCollectibleVal in p_old.entities.Collectible)
	{
		if ((oldCollectibleId in p_new.entities.Collectible) == false)
		{
			tt_panic("Missing Collectible '" + oldCollectibleId + "' in level '" + p_levelName + "'!");
		}
	}
}

function validateLevelHasNoCollectible(p_level, p_levelName)
{
	if (p_level.entities.Collectible.len() > 0)
	{
		tt_panic("Found collectibles " + p_level.entities.Collectible.len() + 
		         " for level: '" + p_levelName + "' expected it to be empty!");
	}
}

// Called from generate meta data after new data is generated.
// (p_oldData is the metadata table from the previously submitted build.)
function validateMetaData(p_newData, p_oldData)
{
	// Deposits validation
	validateValueEquals(p_newData.all_levels.deposits.totalWeight,
	                    p_oldData.all_levels.deposits.totalWeight,
	                    "Total weight deposits not the same!");
	
	foreach (newLevelId, newLevelVal in p_newData.all_levels.deposits.levels)
	{
		validateValueEquals(newLevelVal,
		                    p_oldData.all_levels.deposits.levels[newLevelId],
							"level '" + newLevelId + "' has a differece deposit count!");
	}
	foreach (oldLevelId, oldLevelVal in p_oldData.all_levels.deposits.levels)
	{
		if ((oldLevelId in p_newData.all_levels.deposits.levels) == false)
		{
			tt_panic("Old deposits level missing! level: '" + oldLevelId);
		}
	}
	
	// Collectible validation
	validateValueEquals(p_newData.all_levels.totals.Collectible,
	                    p_oldData.all_levels.totals.Collectible,
	                    "Total Collectible not the same!");
	
	foreach (newLevelId, newLevelVal in p_newData)
	{
		if (newLevelId == "all_levels" || newLevelId == "menu_worldmap")
		{
			continue;
		}
		
		if (newLevelId in p_oldData)
		{
			// compare level in both tables.
			validateLevel(newLevelVal, p_oldData[newLevelId], newLevelId);
		}
		else
		{
			// New level not in the old data.
			validateLevelHasNoCollectible(newLevelVal, newLevelId);
		}
	}
	foreach (oldLevelId, oldLevelVal in p_oldData)
	{
		if (oldLevelId == "all_levels" || oldLevelId == "menu_worldmap")
		{
			continue;
		}
		
		if ((oldLevelId in p_newData) == false)
		{
			// Old level which isn't in the new data.
			validateLevelHasNoCollectible(oldLevelVal, oldLevelId);
		}
	}
}
*/
// metadata "namespace"
MetaData <-
{
	// This tables filters what entities to store in the meta data. Each entity type in this table is
	// counted per level and for the entire game.
	// The array can contain a names of properties to store or a function which should return a tuple with a
	// name and value that'll be stored.
	filter =
	{
		RewindSpawnPoint   = ["requiredEnergy"]
		//LevelExit     = ["isDoor", "_ID", function(p_entityProperties) { if (p_entityProperties.isDoor) return ["level", p_entityProperties.level]; }]

	}
	
	// entity data that is cached only during the metadata generation
	cache =
	{
		//CollectibleDeposit  = []
	}
	
	// temp storage
	temp =
	{
	}
	
	function prepareLevelMetaData(p_levelData, p_levelMetaData)
	{
		// copy the level properties (i.e. sizes)
		p_levelMetaData.properties <- clone p_levelData.properties;
		
		// create categories
		p_levelMetaData.totals <- {};
		p_levelMetaData.entities <- {};
		
		// create a subcategory for each entity type
		foreach (key, val in filter)
		{
			p_levelMetaData.totals[key] <- 0;
			p_levelMetaData.entities[key] <- {};
		}
	}
	
	function generateEntityData(p_id, p_entityData, p_levelMetaData)
	{
		local type = p_entityData.type;
		
		if (type in filter)
		{
			// create a new slot for entity data and store assign it to a local var for convenience
			local entityData = (p_levelMetaData.entities[type][p_id] <- {});
			
			entityData.pos <- (clone p_entityData.position);
			entityData.properties <- {};
			
			foreach(prop in filter[type])
			{
				switch (typeof(prop))
				{
					case "function":
						local result = prop(p_entityData.properties);
						if (result != null) entityData.properties[result[0]] <- result[1];
						break;
					case "string":
					case "float":
					case "integer":
						entityData.properties[prop] <- p_entityData.properties[prop];
						break;
				}
			}
			
			p_levelMetaData.totals[type] += 1;
			//::echo(entityData, p_id);
		}
	}
	
	function cacheEntityData(p_id, p_entityData, p_levelMetaData)
	{
		local type = p_entityData.type;
		
		if (type in cache)
		{
			cache[type].append(p_entityData);
			cache[type].top()["level"] <- p_levelMetaData.properties.name;
		}
	}
	
	// make sure the all_levels key in the result data exists and has the proper categories
	function prepareAllLevelsData(p_resultData)
	{
		if (("all_levels" in p_resultData) == false) 
		{
			// add the default keys
			p_resultData.all_levels <- 
			{ 
				totals       = {}
				rewindPoints = []
			};
			
			// add each filter entity as key and set its total to 0
			foreach(type, val in filter)
			{
				p_resultData.all_levels.totals[type] <- 0;
			}
		}
	}
	
	function updateAllLevelsData(p_levelMetaData, p_resultData)
	{
		// update totals
		foreach(type, val in p_levelMetaData.totals)
		{
			p_resultData.all_levels.totals[type] += val;
		}
		
		// store the max level width for the minimap
		if ("maxWidth" in p_resultData.all_levels)
		{
			p_resultData.all_levels.maxWidth = max(p_levelMetaData.properties.width, p_resultData.all_levels.maxWidth);
		}
		else
		{
			p_resultData.all_levels.maxWidth <- p_levelMetaData.properties.width;
		}
		
		::echo(p_levelMetaData, "levelmetadata");
		
		foreach(id, entity in p_levelMetaData.entities.RewindSpawnPoint)
		{
			p_resultData.all_levels.rewindPoints.append(
				{
					levelName      = p_levelMetaData.properties.name
					requiredEnergy = entity.properties.requiredEnergy
					id             = id
				});
		}
		
		// reverse sort the rewind points
		p_resultData.all_levels.rewindPoints.sort(@(a, b) b.requiredEnergy <=> a.requiredEnergy);
	}
}