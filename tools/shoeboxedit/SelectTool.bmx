SuperStrict

Import "ShoeTool.bmx"
Import "PickedUVW.bmx"


Type SelectTool Extends ShoeTool
	

	Const SELECT_ADD:Int = 1, SELECT_REMOVE:Int = 2, SELECT_SINGLE:Int = 4, SELECT_HIDE:Int = 8, SELECT_HIDE_ADD:Int = 16
	Field selectMode:Int = SELECT_SINGLE
	
	Field paintMode:Int = False
	
	'----------------------------------------------------------------------------
	Method InitSelect:SelectTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Select", aSelection, awin )
		
		win      = awin
	
		paintMode = False
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		Super.UpdateText()
	
	EndMethod
	
	Method UnhidePlanesFromList(p_list:TList)
		If p_list <> Null
			For Local sp:ShoePlane = EachIn p_list
				sp.OnShow()
			Next
		EndIf
	EndMethod
	
	Method GetFirstNonTransparentPlane:ShoePlane( x:Int, y:Int, threshold:Int = 255 )
		Local hiddenPlanes:TList   = New TList
		Local foundPlanes:TList    = New TList
		Local matchedZ:Float       = 100000
		
		While True
			Local ent:TEntity = win.GetObjectFrom( x, y )
			If ent = Null Exit
			
			Local sp:ShoePlane = ShoePlane.GetShoePlaneFromMesh( ent )
			Local pixel:Int = sp.ReadPixel(PickedU(), PickedV())
			Local alpha:Int = (pixel & $FF000000) Shr 24
			'Print "U: " + PickedU() + " V: " + PickedV()	+ " VALUE: " + Hex(pixel) + " ALPHA: " + alpha
			If (alpha >= threshold)
				If (foundPlanes.IsEmpty())
					matchedZ = sp.GetZ()
				ElseIf sp.GetZ() < matchedZ
					' Moved to another Z; exit while loop
					Exit
				EndIf
				foundPlanes.addLast(sp)
			End If
			'Print "Hiding plane"
			hiddenPlanes.addLast(sp)
			sp.OnHide()
		Wend
		
		' Unhide all planes
		UnhidePlanesFromList(hiddenPlanes);
		
		' Search plane with highest prio
		Local foundPlane:ShoePlane = Null
		Local highestPrio:Int = -100000
		For Local p:ShoePlane = EachIn foundPlanes
			If (p.priority > highestPrio)
				foundPlane = p
				highestPrio = p.priority
			End If
		Next
		Return foundPlane
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		
		Super.MouseUp( but, x, y )
		
		If but = MOUSE_LEFT
			Local foundPlane:Shoeplane = GetFirstNonTransparentPlane(x, y, 1)
				
			If foundPlane <> Null
				'Print "Found! " + foundPlane .ToString() + " :" + selectMode	
				
				Select selectMode
					Case SELECT_SINGLE
						selected.Clear
						selected.Add( foundPlane )
					Case SELECT_ADD
						selected.Add( foundPlane )
					Case SELECT_REMOVE
						selected.Remove( foundPlane )
					Case SELECT_HIDE_ADD
						selected.Add( foundPlane )
						foundPlane.OnHide()
					Case SELECT_HIDE
						selected.Clear
						selected.Add( foundPlane )
						foundPlane.OnHide()
				EndSelect
				
			ElseIf selectMode = SELECT_SINGLE
				'Print "Nothing found! Deselect all"
				
				selected.Clear()
			EndIf
			
			
			Return True
		EndIf
		
	EndMethod
	
	Method MouseMove( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		Super.MouseMove( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		If (paintMode)
			Local ent:TEntity = win.GetObjectFrom( x, y )
			
			If ent
				Local sp:ShoePlane = ShoePlane.GetShoePlaneFromMesh( ent )
				
				If (selectMode = SELECT_REMOVE)
					selected.Remove( sp )
				Else
					selected.Add( sp )
				EndIf
			EndIf
		EndIf
	EndMethod

	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )

		If (akey = KEY_P) paintMode = True	
		
		If ( amodifiers & MODIFIER_OPTION )
			selectMode = SELECT_REMOVE
			Return True
		ElseIf ( amodifiers & MODIFIER_CONTROL And amodifiers & MODIFIER_SHIFT)
			selectMode = SELECT_HIDE_ADD
			Return True
		ElseIf ( amodifiers & MODIFIER_CONTROL )
			selectMode = SELECT_HIDE
			Return True
		ElseIf ( amodifiers & MODIFIER_SHIFT )
			selectMode = SELECT_ADD
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		
		
		'If ( amodifiers & MODIFIER_OPTION )
			selectMode = SELECT_SINGLE
		'	Return True
		'EndIf
		
		If (akey = KEY_P) paintMode = False

		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Random:Int(big:Int)
		
		Local chance:Float = 0.9
		If big chance = 0.5
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			If (Rnd() > chance) selected.Remove( p )
		Next

		StoreUndoInfo()
		Return False
	EndMethod
	
EndType

