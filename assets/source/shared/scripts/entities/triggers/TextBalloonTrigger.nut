include_entity("rewind/conversation/ConversationMgr");
include_entity("triggers/Trigger");

class TextBalloonTrigger extends Trigger
</
	editorImage    = "editor.textballoontrigger"
	libraryImage   = "editor.library.textballoontrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Text Balloon"
	group          = "04.3 Conversation Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "string"
		order       = 0
		group       = "Specific Settings"
	/>
	textID = "";
	
	</
		type        = "entity"
		filter      = ["BaseEnemy", "PlayerBot"]
		order       = 1
		group       = "Specific Settings"
		description = "If not set, player is used."
	/>
	actor = null;
	
	</
		type        = "string"
		choice      = ["neutral", "happy", "angry", "scared", "happy_shout", "angry_shout", "scared_shout"]
		group       = "Specific Settings"
		order       = 2
	/>
	emotion = "neutral";
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		order       = 1
		group       = "Specific Settings"
		description = "Talk delay. Note: trigger itself is not delayed!"
	/>
	delay = 0.01;
	
	</
		type        = "integer"
		min         = 0
		max         = 10
		order       = 2
		group       = "Specific Settings"
		description = "Maximum amount of displays (survives checkpoints, not app reset). Use 0 for infinite."
	/>
	maxDisplayed = 0;
	
	// Usually we only want this conversation to be displayed once
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (actor == null)
		{
			actor = ::getFirstEntityByTag("PlayerBot");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		if (maxDisplayed > 0)
		{
			local table = ::getRegistry().getPersistent("displayedTextIDs");
			if (table == null)
			{
				table = {};
			}
			if ((textID in table) == false)
			{
				table[textID] <- maxDisplayed;
			}
			--table[textID];
			::getRegistry().setPersistent("displayedTextIDs", table);
			if (table[textID] < 0)
			{
				return;
			}
		}
		
		::ConversationMgr.queueConversation(
		{
			textID   = textID,
			actor    = actor,
			delay    = delay,
			duration = 0.01,
			emotion  = emotion,
			trigger  = null,
			callback = null
		});
	}
}
