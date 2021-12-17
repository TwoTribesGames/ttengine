SuperStrict

Import "Selection.bmx"
Import "ShoePlane.bmx"

Type Action
	
	Global actions:TList = New TList
	Global currentAction:TLink ', lastAction:Tlink
	Global atBeginning:Int, atEnd:Int

	Field Selected:Selection
		
	'----------------------------------------------------------------------------
	Method Do( reverse:Int = False )
		' do the actual action	
	EndMethod

	'----------------------------------------------------------------------------
	' add a new action and clear everthing after the current action
	Method Add( aSelection:Selection )
		
		If Not actions.IsEmpty()
			
			Local l:TLink = actions.LastLink()
			While l <> currentAction
				l.Remove()
				l = actions.LastLink()
			Wend
		
		EndIf
				
		currentAction = actions.AddLast( Self )
		atEnd         = True
		atBeginning   = False
		
		If aSelection Selected = aSelection.Copy()
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	Function Undo()

		If Not ( IsAtFirstAction() )
			
			Action( currentAction.Value() ).Do( True )
			currentAction = currentAction.PrevLink()
			
			If currentAction = actions.FirstLink()
				atBeginning = True
			Else
				atBeginning = False
			EndIf
			atEnd = False
		
		EndIf
		
	EndFunction
	
	'----------------------------------------------------------------------------
	Function Redo()
		
		If Not ( IsAtLasttAction() )
			
			currentAction = currentAction.NextLink()
			Action( currentAction.Value() ).Do()
			
			If currentAction = actions.LastLink()
				atEnd = True
			Else
				atEnd = False
			EndIf
			atBeginning = False
			
		EndIf

	EndFunction

	
	'----------------------------------------------------------------------------
	Function Purge()
	
		actions.Clear()	
		New Action.Add( Null ) ' one action for padding... :( quick and very dirty
		atEnd = False
		atBeginning = True
		
	EndFunction
	
	'----------------------------------------------------------------------------
	Function GetUndoBufferSize:Int()
		
		Return actions.count()
		
	EndFunction
	
	
	'----------------------------------------------------------------------------
	Function IsAtFirstAction:Int()
		
		Return atBeginning Or actions.IsEmpty()
		
	EndFunction
	
	'----------------------------------------------------------------------------
	Function IsAtLasttAction:Int()
		
		Return atEnd Or actions.IsEmpty()
		
	EndFunction

EndType

Action.Purge()