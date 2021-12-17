SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"

Type FileTool Extends ShoeTool

	'----------------------------------------------------------------------------
	Method InitFile:FileTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "File", aSelection, awin )
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key 'And amodifiers & modifier
			Local allSameType:Int = True
			Local planeType:Int = ShoePlaneType_None
			' Check if all planes have same type
			For Local p:ShoePlane = EachIn selected.GetSelection()
				If (planeType = ShoePlaneType_None)
					planeType = p.planeType
				ElseIf (planeType <> p.planeType)
					allSameType = False
					Exit
				EndIf
			Next
			
			If (allSameType)
				Select planeType
				Case ShoePlaneType_Normal
					If (amodifiers & MODIFIER_CONTROL)
						Local texname:String = RequestFile( "Select texture to open", "PNG files:png", False, TConfig.getStringValue("texturedir") )
					
						If texname
							For Local p:ShoePlane = EachIn selected.GetSelection()
								p.SetTexture( texname )
								
								If (amodifiers & MODIFIER_SHIFT)
									p.SetAutoSize()
								EndIf
								
								p.UpdateXML()
							Next
						EndIf
					ElseIf (amodifiers & MODIFIER_SHIFT)
						For Local p:ShoePlane = EachIn selected.GetSelection()
							p.SetAutoSize()
						Next
					Else
						For Local p:ShoePlane = EachIn selected.GetSelection()
							p.FixTextureSize()
						Next
					EndIf
					
				Case ShoePlaneType_Include
					If (amodifiers & MODIFIER_CONTROL)
						Local name:String = RequestFile( "Select include plane", "XML files:xml", False, TConfig.getStringValue("shoeboxdir") )
						If (name)
							name = "levels/shoebox_includes/" + StripAll(name)
							For Local p:ShoePlane = EachIn selected.GetSelection()
								p.xmlNode.SetAttribute("filename", name)
								p.UpdateXML()
							Next
						EndIf
					EndIf
					
				Case ShoePlaneType_Emitter
					If (amodifiers & MODIFIER_CONTROL)
						Local name:String = RequestFile( "Select particle emitter ", "XML files:xml", False, TConfig.getStringValue("emitterdir") )
						If (name)				
							name = StripAll(name) + ".trigger"
							For Local p:ShoePlane = EachIn selected.GetSelection()							
								p.xmlNode.SetAttribute("effect_file", name)
								p.UpdateXML()
							Next
						EndIf
					EndIf
				EndSelect
			Else
				Notify("Selected planes are of different types", True )	
			EndIf
			
			EmitEvent CreateEvent(EVENT_KEYUP, Null, amodifiers)
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType



