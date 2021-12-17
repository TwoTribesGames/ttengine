SuperStrict

Import "ShoePlane.bmx"

Type Selection

	Field selected:TMap = New TMap 'TList = New TList ' abuse a map to avoid duplicate entries in the selected collection ( you know, like a real map :P )
	Field numSelected:Int
	Field first:ShoePlane
	
	'----------------------------------------------------------------------------
	Method Copy:Selection()
		
		Local s:Selection = New Selection
		
		s.numSelected = numSelected
		s.selected = selected.Copy()
		
		Return s
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method GetSelection:TMapEnumerator()
		
		Return selected.Keys()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method GetFirst:ShoePlane()
		' crappy loop, not getter to get the first or only item
		'For Local sp:ShoePlane = EachIn selected.Keys()
		'	Return ShoePlane( sp )
		'Next
		Return first
	EndMethod
	
	'----------------------------------------------------------------------------
	Method StoreStates()
		For Local sp:ShoePlane = EachIn selected.Keys()
			sp.StoreState()
		Next
	EndMethod
	
	'----------------------------------------------------------------------------
	Method IsEmpty:Int()
		
		Return selected.IsEmpty()

	EndMethod

	'----------------------------------------------------------------------------
	Method Size:Int()
	
		Return numSelected
		
	EndMethod
		
	'----------------------------------------------------------------------------
	Method Add( aShoePlane:ShoePlane )
		
		If aShoePlane = Null Return
		
		If Not selected.Contains( aShoePlane ) And ..
		   aShoePlane.IsSelectAble() 

			aShoePlane.OnSelect(numSelected = 0)
			If numSelected = 0 first = aShoePlane
			
			selected.Insert( aShoePlane, Null )

			numSelected:+ 1
			?debug
			'Print "+ Added " + aShoeplane.ToString() + " to selection"
			?
Rem
		Else
			?debug
			Print "! Selection already contains" + aShoeplane.ToString()
			?
EndRem
		EndIf
				
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Remove( aShoePlane:ShoePlane )
	
		If selected.Contains( aShoePlane )
			selected.Remove( aShoePlane )
			?debug
			Print "- Removed " + aShoeplane.ToString() + " from selection"
			?
			aShoePlane.OnDeselect()
			
			numSelected:- 1
		EndIf
				
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Clear()
	
		For Local p:ShoePlane = EachIn selected.Keys()
			p.OnDeselect() ' Remove( p )? might be slower
		Next
		
		Clean()
		?debug
		Print "Deselect all"
		?
	EndMethod

	'----------------------------------------------------------------------------
	Method Clean()
	
		selected.clear()
		first = Null
		numSelected = 0
		?debug
		Print "Clean"
		?
	EndMethod	
EndType