SuperStrict

Import sidesign.minib3d
Import bah.libxml
Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type BrushTool Extends ShoeTool

	Field sb:ShoeBox

	Field xmlDocs:TList = New TList
	
	Field addToSelection:Int = False
	Field forceZToFirstSelected:Int = True
	
	Field _pickMesh:TMesh
	
	'----------------------------------------------------------------------------
	Method InitBrush:BrushTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		sb = aShoebox
		
		Init( akey, amodifier, "Brush", aSelection, awin )
		
		_pickMesh = _CreatePlane()
		_pickMesh.ScaleMesh(1000, 1000, 1)
		
		_pickMesh.EntityAlpha(0)
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, p_x:Int, p_y:Int )
		
		' pick a random doc from the list
		Local doc:TxmlDoc = TxmlDoc(xmlDocs.ValueAtIndex(Rnd(0, xmlDocs.count()))).copy()
		
		Local rootNode:TxmlNode = doc.GetRootElement()
		Local children:TList = rootNode.getChildren()
		
		Local origPickEnts:TList = Tpick.ent_list.copy()

		If (forceZToFirstSelected)
			Tpick.ent_list.clear()
			_pickMesh.EntityPickMode(2)
			
			If Not selected.IsEmpty()
				Local sp:ShoePlane = selected.GetFirst()
				If (sp.mesh <> Null) 
					_pickMesh.PositionEntity(0, 0, -sp.mesh.pz)
				Else
					_pickMesh.PositionEntity(0, 0, 0)
				EndIf
			EndIf
		EndIf
		
		Local ent:TEntity = sb.win.Camera.CameraPick( p_x, p_y )

		If (forceZToFirstSelected)
			Tpick.ent_list = origPickEnts
			_pickMesh.EntityPickMode(0)
		EndIf
		
		If (ent = Null) Return Super.MouseDown(but, p_x, p_y)
		
		
		Local x:Float =  TPick.picked_x
		Local y:Float = -TPick.picked_y
		Local z:Float =  TPick.picked_z

		If children
			
			Local root:TxmlNode = sb.xml.GetRootElement()
			
			For Local node:TxmlNode = EachIn children

				' modify the nodes attributes
				modifyNodeAttributes(node, x, y, z)
				
				Local nodeCopy:TxmlNode = node.copyToDoc(sb.xml)
				
				''' AAAAAAAAAARGH
				root.addChildList(ListFromArray([nodeCopy]))
				
				sb.ParseNode(nodeCopy)
			Next
			
		EndIf
		
		If addToSelection
			For Local newplane:ShoePlane = EachIn sb.NewPlanes
				selected.Add(newplane)
			Next
		EndIf
		
		doc.free()
		
		Return Super.MouseDown( but, p_x, p_y )

	EndMethod

	Method modifyNodeAttributes(p_node:TxmlNode, x:Float, y:Float, z:Float)
		
		' modify the nodes attributes
		Local attributes:TList = p_node.getAttributeList()
		If (attributes)
			For Local attribute:TxmlAttribute = EachIn attributes
				modifyAttributeValue(p_node, attribute, x, y, z)
			Next
		EndIf
		
		Local children:TList = p_node.getChildren()
		
		If (children)
			For Local node:TxmlNode = EachIn children
				modifyNodeAttributes(node, x, y, z)
			Next
		EndIf
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method modifyAttributeValue(node:TxmlNode, attribute:TxmlAttribute, x:Float, y:Float, z:Float)
		
		Local name:String = attribute.getName()
		Local value:String = attribute.getValue()
		
		If (value.Find("r(") >= 0)
			Local startIdx:Int = value.Find("r(") + 2
			Local midIdx:Int   = value.Find(",", startIdx)
			Local endIdx:Int   = value.Find(")", midIdx)

			Local minVal:Float = Float(value[startIdx   .. midIdx])
			Local maxVal:Float = Float(value[midIdx + 1 .. endIdx])
			
			value = Rnd(minVal, maxVal)
			Print "randomized:" + value
		EndIf
		
		Select name
			
			Case "x", "x_offset"
				value = Float(value) + x
			Case "y", "y_offset"
				value = Float(value) + y
			Case "z", "z_offset"
				value = Float(value) - z
		End Select
		
		node.setAttribute(name, value)

	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )

		If ( akey = key And amodifiers & MODIFIER_CONTROL )
			openBrush()
		EndIf	
		
		If (amodifiers & MODIFIER_SHIFT)
			addToSelection = True
		EndIf
		If (amodifiers & MODIFIER_ALT)
			forceZToFirstSelected = False
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )

		If (amodifiers & MODIFIER_SHIFT Or akey = KEY_LSHIFT Or akey = KEY_RSHIFT)
			addToSelection = False
		EndIf
		
		If (amodifiers & MODIFIER_ALT Or akey = KEY_LALT Or akey = KEY_RALT)
			forceZToFirstSelected = True
		EndIf
		
		
		Return Super.Keyup( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method openBrush()
		
		Local brushfile:String = RequestFile( "Select brush to open", "XML files:xml", False, TConfig.getStringValue("brushdir") )
		Local brushdir:String = ExtractDir(brushfile)
		
		If (brushfile)
			Local dirs:String[] = brushdir.Split("/")
			name = "Brush: " + dirs[dirs.length - 1]
			
			Local files:String[]
	
			files = LoadDir(brushdir)
			
			For Local doc:TxmlDoc = EachIn xmlDocs
				doc.free()
			Next
			xmlDocs.clear()
			
			For Local file:String = EachIn files
				' assume everything is a-okay, no time for error checking
				Local doc:TxmlDoc = TxmlDoc.parseFile(brushdir + "/" + file)
				
				xmlDocs.addLast(doc)
			Next
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		
		Super.OnActivate()

		If (xmlDocs.isEmpty())
			openBrush()
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnDeactivate()
		Super.OnDeactivate()

	EndMethod
	
EndType