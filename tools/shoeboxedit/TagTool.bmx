SuperStrict

Import "ShoeTool.bmx"

Import MaxGui.Drivers

Type TagTool Extends ShoeTool
	Global instance:TagTool 
	Global popupWindow:TGadget
	Global popupTextField:TGadget
	Global popupButtonOK:TGadget
	Global popupButtonCancel:TGadget
	
	'----------------------------------------------------------------------------
	Method InitTag:TagTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Tag", aSelection, awin )
		
		If instance RuntimeError( "No more than one instance allowed!" )
		instance = Self
		
		'Some temp variables
		Local width:Int, height:Int
		
		popupWindow = CreateWindow("Tag name", Null, Null, 250, 150, Null, WINDOW_TITLEBAR | WINDOW_CENTER)
		
		'Get width and height
		width = ClientWidth(popupWindow)
		height = ClientHeight(popupWindow)
		
		'Position our gadgets
		popupButtonOK     = CreateButton( "Apply", 45, height - 30, 70, 25, popupWindow)
		popupButtonCancel = CreateButton( "Cancel", 135, height - 30, 70, 25, popupWindow)
		popupTextField    = CreateTextField( width / 2 - 60, height/2 - 40, 120, 30, popupWindow)
		
		AddHook(EmitEventHook, TagTool.EventHook, Null, 1)
		
		HideGadget( popupWindow )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		If Super.UpdateText()
			Local p:ShoePlane = selected.GetFirst()
			
			SetToolText( "tag:" + p.tag  )
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		Local p:ShoePlane = selected.GetFirst()
		If p
			SetGadgetText(popupTextField, p.tag)
		Else
			SetGadgetText(popupTextField, "")
		EndIf
		ShowGadget( popupWindow )
		
		Super.OnActivate()
	EndMethod
	
	'----------------------------------------------------------------------------
	Method setTag(p_tag:String)
		For Local p:ShoePlane = EachIn selected.GetSelection()
			p.tag = p_tag
		Next
	EndMethod
	
	'----------------------------------------------------------------------------
	Function EventHook:Object( p_id:Int, p_data:Object, p_context:Object )
		If Not instance Return Null
		
		Local event:TEvent = TEvent(p_data)
		If ( event <> Null )
			Select event.id
				Case EVENT_GADGETACTION
					Select event.source
						Case popupButtonCancel
							HideGadget( popupWindow )
						Case popupButtonOK
							instance.setTag(GadgetText(popupTextField))
							HideGadget( popupWindow )
					End Select 
				Case EVENT_WINDOWCLOSE
					Select event.source
					Case popupWindow
						HideGadget( popupWindow )
					End Select
			End Select
		End If
		Return p_data
	EndFunction
EndType

