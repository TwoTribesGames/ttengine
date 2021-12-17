include_entity("rewind/conversation/ConversationMgr");
include_entity("triggers/Trigger");

class ConversationAbortTrigger extends Trigger
</
	editorImage    = "editor.conversationaborttrigger"
	libraryImage   = "editor.library.conversationaborttrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Conversation Abort"
	group          = "04.3 Conversation Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "string"
		choice         = ["Enter", "Exit"]
		group          = "Specific Settings"
		order          = 0.0
	/>
	abortOn = "Enter";
	
	// Usually we only want this conversation to be displayed once
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		if (abortOn == "Enter")
		{
			::ConversationMgr.playAbortSequence();
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		if (abortOn == "Exit")
		{
			::ConversationMgr.playAbortSequence();
		}
	}
}
