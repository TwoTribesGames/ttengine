SuperStrict

Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
Import "ShoePlane.bmx"
Import "Emitter.bmx"
Import "Trigger.bmx"
Import "IncludePlane.bmx"
'Import "Popup.bmx"
Import "Fog.bmx"
Import "Window.bmx"
Import "TextArea.bmx"
Import "Action.bmx"
Import "Selection.bmx"

Import "Timer.bmx"
Import "TextureLoader.bmx"

Import "LevelPreviewMesh.bmx"

Import MaxGui.Drivers

Type ShoeBox
	
	Field ShoePlanes:TList = New TList
	Field NewPlanes:TList = New TList

	Field includedShoeBoxes:TList = New TList

	Field LevelIndicator:TMesh
	Field LevelIndicatorTexture:TTexture	
	
	Field m_shoeboxFilename:String
	Field m_levelFilename:String
	Field m_shoeboxFiletime:Int
	Field m_levelFiletime:Int
	
	Field xml:TxmlDoc
	Field minz:Int, maxz:Int
	
	Field LevelWidth:Int
	Field LevelHeight:Int 
	Field win:Window
	
	Field fogNode:Fog
	
	'----------------------------------------------------------------------------
	Method Init:ShoeBox( aWindow:Window )
		
		win = aWindow
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Clear(isChild:Int = False)
		
		For Local sb:ShoeBox = EachIn includedShoeBoxes
			sb.Clear(True)
		Next
		
		fogNode = Null
		m_shoeboxFilename = ""
		maxz =  100
		minz = -100
		
		For Local sp:ShoePlane = EachIn shoeplanes
			sp.Remove()
		Next
		ShoePlanes.Clear()
		If (isChild = False) ShoePlane.ClearMap() ' only clear the map of the shoeplanes if we're loading the root shoebox
		
		Action.Purge()
		
		If xml
			xml.Free()
			xml = Null
		EndIf
		
		win.ClearScene()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method DuplicateSelection( aSelection:Selection, p_moveIntoView:Int = True, p_deepCopyIncludePlanes:Int = False )
		Local p:ShoePlane = aSelection.GetFirst()
		If p
			Local dx:Int = win.Camera.px - p.GetX()
			Local dy:Int = win.Camera.py + p.GetY() ' stupid inverted y coord
			
			For p = EachIn aSelection.GetSelection()
				If (p_deepCopyIncludePlanes And p.planeType = ShoePlaneType_Include)
					Local rootnode:TxmlNode = TxmlNode(xml.GetRootElement().getFirstChild())
					For Local cp:ShoePlane = EachIn p.childPlanes
						Local newPlane:ShoePlane = cp.Duplicate(False, rootnode);
						ShoePlanes.AddLast( newPlane )
						If p_moveIntoView newPlane.Move(dx, dy, 0)
					Next
				Else
					Local newPlane:ShoePlane = p.Duplicate();
					Rem
					' FIXME: Martijn: this code below, which copies the planes, doesn't work and causes numerous crashes
					' My guess is that something fishy with DuplicateXML is going on in combination with ShoePlane.Remove()
					If (p.planeType = ShoePlaneType_Include)
						Local box:ShoeBox = New ShoeBox.Init( win )
						box.SetParent(newPlane.mesh)
						For Local child:Shoeplane = EachIn p.childPlanes
							box.ShoePlanes.AddLast(child.Duplicate())
						Next
						newPlane.SetChildPlanes(box.ShoePlanes, False)
						includedShoeboxes.addLast(box)
					EndIf
					EndRem
					
					ShoePlanes.AddLast( newPlane )
				
					If p_moveIntoView p.Move(dx, dy, 0)
				EndIf
			
			Next
			
			UpdatePlaneIDs()
			SortbyXML()
		EndIf
	EndMethod

	'----------------------------------------------------------------------------
	' sort shoeplanes list by xml line position
	Method SortbyXML()
		ShoePlanes.Sort(True, Shoebox.XMLSort)
		
		' and then magically sort the meshes too
		TEntity.entity_list.Sort(True, MeshSort)
	EndMethod
	
	Function XMLSort:Int(o1:Object, o2:Object)
		Local m1:Shoeplane = Shoeplane(o1)
		Local m2:Shoeplane = Shoeplane(o2)
		
		Return (m1.xmlNode.getLineNumber() > m2.xmlNode.getLineNumber())
	EndFunction

	Function MeshSort:Int(o1:Object, o2:Object)
		Local m1:TMesh = TMesh(o1)
		Local m2:TMesh = TMesh(o2)

		If m1 And m2
			Local s1:ShoePlane = ShoePlane.GetShoePlaneFromMesh(m1)
			Local s2:ShoePlane = ShoePlane.GetShoePlaneFromMesh(m2)
			
			Local line1:Int = -1
			Local line2:Int = -1
			If s1 line1 = s1.xmlNode.getLineNumber()
			If s2 line2 = s2.xmlNode.getLineNumber()

			Return (line1 > line2)
		EndIf
		
		Return -1
	EndFunction
	
	'----------------------------------------------------------------------------
	Method RemoveSelection( aSelection:Selection )
		
		For Local p:ShoePlane = EachIn aSelection.GetSelection()
			If (p.planeType = ShoePlaneType_Include)
				' Find shoebox in includedShoeBoxes
				For Local sb:ShoeBox = EachIn includedShoeBoxes
					If sb.ShoePlanes = p.childPlanes
						' Found it; now manually remove this Shoebox. Cannot use Clear here. FIXME: Rewrite this in a method
						p.childPlanes = Null
						includedShoeBoxes.Remove(sb)
						For Local childSB:ShoeBox = EachIn sb.includedShoeBoxes
							childSB.Clear(True)
						Next
						
						For Local sp:ShoePlane = EachIn sb.shoeplanes
							sp.Remove()
						Next
						sb.ShoePlanes.Clear()
						SortbyXML()
						Exit
					End If
				Next
			EndIf
			
			ShoePlanes.Remove( p )
			p.Remove()
		Next
		
		aSelection.Clean()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdatePlaneIDs()
		' First look for highest pid
		Local highestID:Int = 0
		For Local plane:Shoeplane = EachIn ShoePlanes
			If plane.pid > 0 And plane.pid > highestID 
				highestID = plane.pid
			EndIf
		Next
		
		For Local plane:Shoeplane = EachIn ShoePlanes
			If plane.pid < 0
				plane.pid = highestID
				highestID :+ 1
			EndIf
		Next
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Open()
	
		If CheckUnsavedChanges()
			Local shoeboxFilename:String = RequestFile( "Select shoebox to open", "XML files:xml", False, TConfig.getStringValue("shoeboxdir") )
			Local levelFilename:String = RequestFile( "Select level file to open", "Level files:ttlvl", False, TConfig.getStringValue("leveldir") )
			If shoeboxFilename
				OpenFromXML(shoeboxFilename)
			EndIf
			If levelFilename
				OpenLevel  (levelFilename)
			EndIf
		EndIf
			
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OpenFromXML( p_shoeboxFilename:String, p_isChild:Int = False )
		
		SetPointer(POINTER_WAIT)
		Clear(p_isChild)
		' parse the config file document

		Local XMLDoc:TxmlDoc = TxmlDoc.parseFile( p_shoeboxFilename)
		
		Local err:TxmlError = xmlGetLastError()
		
		If err
			Notify( "Could not parse " + StripDir(p_shoeboxFilename) + "~n~n" + err.getErrorMessage() + "~nParse error at line " + err.GetLine(), True )
			If XMLDoc XMLDoc.Free()
			xmlCleanupParser()
			Return
		EndIf
		
		Rem
		If XMLDoc = Null
			' something went wrong parsing the file
			'RuntimeError("Could not parse: " + filename)
			Notify( "Could not parse " + filename + "~n Aborted loading.", True )
			Return
		EndIf
		EndRem
		
		xml = XMLDoc.Copy()
		XMLDoc.Free()
?debug
Local parseTimer:Timer = New Timer.Start("Parse XML")
?
		Local rootnode:TxmlNode = xml.GetRootElement()
	
		If rootnode = Null Then

			Notify( "Tried to open empty document?", True )
			xml.Free()
			Return
		End If

		If rootnode.getName() <> "shoebox"
			Notify( "Not a shoebox XML file ( the rootnode is not 'shoebox' )~nAborted Loading.", True )
			
			xml.Free()
			Return
		End If
		
		' start parsing the actual document
		ParseNode( rootnode )
		
		' Update the IDs
		UpdatePlaneIDs()
?debug
Print parseTimer.Report()
Local cleanTimer:Timer = New Timer.Start("Cleanup XML")
?
		xmlCleanupParser()
?debug
Print cleanTimer.Report()
?
		If p_isChild
			Return
		End If
		
		m_shoeboxFilename = p_shoeboxFilename
		m_shoeboxFiletime = FileTime(p_shoeboxFilename)

		' update window
'		win.SetCameraRange( minz, maxz )
		win.SetCameraRange(1, 50000)

		win.Caption.SetText( StripDir(m_shoeboxFilename) )
		
		'SetPointer(POINTER_DEFAULT)
	
	EndMethod
	
	Method OpenLevel( p_levelFilename:String )
		HideLevelIndicator()
		If p_levelFilename
			If FileSize(p_levelFilename) > -1
				m_levelFilename = p_levelFilename
				m_levelFiletime = FileTime(m_levelFilename)
				CreateLevelIndicator(m_levelFilename, StripDir(StripExt(m_shoeboxFilename)))
			EndIf
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ParseNode( node:TxmlNode, start:Int = True )
	
		If (start)
			NewPlanes.clear()
		EndIf
		
		' select?
		Local s:Shoeplane
		
		If node.getName() = "plane"
		
			s = New ShoePlane
			s.parent = parent
			s.Init( node )
				
		ElseIf node.GetName() = "particle"
		
			Local e:Emitter = New Emitter
			e.parent = parent
			e.InitEmitter( node )
			s = e
		
'		ElseIf node.GetName() = "script_trigger"
		
'			s = New Trigger.InitTrigger( node )
		
		'ElseIf node.GetName() = "popup"
		
		'	s = New Popup.InitPopup( node )

		ElseIf node.GetName() = "fog"
		
			fogNode = New Fog.InitFog(node)
			UpdateFog()
			
			s = fogNode
			
		ElseIf node.GetName() = "shoebox" And node.GetParent().getName() = "includes"

			Local ip:IncludePlane = New IncludePlane.InitIncludePlane(node) 
			
			parseIncludes(node, ip)
		
			s = ip
		EndIf
		
		If s
			Local planeID:Int = -1
			If node.hasAttribute("pid")
				planeID = Int(node.getAttribute("pid"))
			EndIf
			s.pid = planeID
			
			ShoePlanes.Addlast( s )
			NewPlanes.Addlast( s )
			
			s.StoreState()
			Local z:Int = -s.GetZ()
			If z < minz minz = z
			If z > maxz maxz = z
		EndIf
		
		' parse children if any
		Local children:TList = node.getChildren()
		
		If children
			For Local n:TxmlNode = EachIn children
				ParseNode( n, False )
			Next
		EndIf
				
	EndMethod

	'----------------------------------------------------------------------------
	Method CreateFogNode()
		Local fog:TxmlNode = xml.getRootElement().addChild("fog")
		Local range:TxmlNode = fog.addChild("range")
		Local color:TxmlNode = fog.addChild("color")	
	
		range.addAttribute("near",  "25")
		range.addAttribute("far" , "100")

		color.addAttribute("r", "255")
		color.addAttribute("g", "255")
		color.addAttribute("b", "255")
	
		ParseNode(fog)
	EndMethod
	
	Method ShowFog()
		If fognode = Null
			CreateFogNode()
		EndIf
		UpdateFog()
		win.ShowFog()
	EndMethod
	
	Method HideFog()
		win.HideFog()
	EndMethod
	
	Method UpdateFog()
		SetFogColor(fogNode.R, fogNode.G, fogNode.B)
		SetFogRange(fogNode.near, fogNode.far)
	EndMethod
	
	Method SetFogColor(r:Int, g:Int, b:Int)
		fogNode.R = r
		fogNode.G = g
		fogNode.B = b
		
		win.SetFogColor(fogNode.R, fogNode.G, fogNode.B)
	EndMethod
	
	Method SetFogRange(near:Float, far:Float)	
		fogNode.near = near
		fogNode.far  = far
		
		win.SetFogRange(fogNode.near, fogNode.far)
	EndMethod
	
	Method GetFogR:Int()
		Return fogNode.R
	EndMethod
	
	Method GetFogG:Int()
		Return fogNode.G
	EndMethod
	
	Method GetFogB:Int()
		Return fogNode.B
	EndMethod
	
	Method GetFogNear:Float()
		Return fogNode.Near
	EndMethod
	
	Method GetFogFar:Float()
		Return fogNode.Far
	EndMethod
	
	'----------------------------------------------------------------------------
	Method parseIncludes(node:TxmlNode, ip:IncludePlane)
	
		Local filename:String = node.getAttribute("filename")
		
		If filename <> Null Then
			DebugLog "[Including] " + (node.toString())
			
			Local box:ShoeBox = New ShoeBox.Init( win )
			box.SetParent(ip.mesh)
			box.OpenFromXML(TConfig.getStringValue("basedir") + "../" + filename + ".xml", True)
			ip.SetChildPlanes(box.ShoePlanes, True)
			includedShoeboxes.addLast(box)
		EndIf
	EndMethod
	
	Field parent:TEntity
	' sets the parent for all shoeplanes
	Method SetParent(p_parent:TEntity)
		
		parent = p_parent

	EndMethod
	
	'----------------------------------------------------------------------------
	Method NewPlane:ShoePlane(texturepath:String, x:Float = 0, y:Float = 0, z:Float = 0, baseplane:ShoePlane = Null)
		
		' first find an plane to clone the xml node from
		Local newplane:Shoeplane
		
		
		If baseplane
			newplane = baseplane.Duplicate()
		Else
			For Local plane:Shoeplane = EachIn ShoePlanes
				If plane.xmlNode.getName() = "plane"
					newplane = plane.Duplicate()
					Exit
				EndIf
			Next
		EndIf
		
		If newplane
			
		'	Local newplane:ShoePlane = New ShoePlane.Init(newnode)
			newplane.SetPosition(x, y, z)
			newplane.SetRotation(0)
			newplane.SetTexture(texturepath)
			newplane.SetAutoSize()
			newplane.UpdateXML()
			
			ShoePlanes.AddLast( newplane )
			
			Return newplane
		Else
			Return Null
		EndIf
	EndMethod

	'----------------------------------------------------------------------------
	Method Save()
	
		If m_shoeboxFilename 
			SaveToXML( m_shoeboxFilename )
		Else
			SaveAs()
		EndIf
		
	EndMethod

	'----------------------------------------------------------------------------
	Method SaveAs:Int( filename:String = "" )
		
		filename = RequestFile( "Select file to save", "XML Files:xml", True, TConfig.getStringValue("shoeboxdir") )
		
		If filename
			SaveToXML( filename )
			Return True
		EndIf

		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SaveToXML( filename:String )
	
		Local isLightmaskShoebox:Int = filename.Find("_lightmask.") >= 0
		
		For Local p:ShoePlane = EachIn ShoePlanes
			p.ClearStoredState()
			
			If isLightmaskShoebox 
				If (p.xmlNode)
					p.xmlNode.SetAttribute("blend_mode", "add" )
				EndIf
				' BLAAAAAAAAAAAAAAAAAAAh hack but set the plane's blend mode to additive too
				If (p.mesh)
					p.mesh.EntityBlend(3)
					p.mesh.brush.fx :| 8 ' ignore fog 
				EndIf 
			EndIf
			
			p.UpdateXML()
		Next
		
		ClearTextNodes()
		
		xml.saveFormatFile( filename, True ) ' assume it succeeds...
?debug
'xml.saveFormatFile( "-", True ) ' assume it succeeds...
Print xml.ToStringFormat(True)
?
		Print "Done"
		m_shoeboxFilename = filename
		win.Caption.SetText( StripDir( m_shoeboxFilename ) )
		
		
		TrimNameSpacesFromFile(m_shoeboxFilename)
		
		' saved, so purge undo buffer
		Action.Purge()
		
		' and store states again
		For Local p:ShoePlane = EachIn ShoePlanes
			p.StoreState()
		Next
		
		' update the filetime too
		m_shoeboxFiletime = FileTime(filename)
	EndMethod

	Const NAMESPACESTRING:String = " xmlns=~qhttp://intranet.twotribes.com/schemas/common/shoebox.xsd~q"
	Method TrimNameSpacesFromFile(p_filename:String)
		Local fp:TStream = ReadStream(p_filename)
		Local lineNum:Int = 1
		Local replaceAfterLine:Int = 2 ' the first line is the xml declaration, the second is the root element (libxml dumps one element on one single line so this should be safe)
		
		Local textToSave:String = ""
		
		If Not fp RuntimeError "Failed to open a ReadStream to file http::www.blitzbasic.com"
		
		While Not Eof(fp)
			
			Local line:String = ReadLine(fp)
			If (lineNum > replaceAfterLine)
				line = (line.Replace(NAMESPACESTRING, ""))
			EndIf
			
			textToSave :+ line + "~r~n"

			lineNum :+ 1
		Wend
		
		CloseStream fp
		
		SaveText(textToSave, p_filename)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method GetAllShoePlanes:TList()
	
		Return ShoePlanes
		
	EndMethod

	Method Resize(p_factor:Float)
		
		For Local p:ShoePlane = EachIn ShoePlanes
			p.Resize(p_factor)
		Next
		
		If fogNode <> Null
			fogNode.Near :* p_factor
			fogNode.Far  :* p_factor
		EndIf
	EndMethod
	
	Field SkinShoebox:ShoeBox
	Field SkinShoeboxAnchor:TEntity
	
	'----------------------------------------------------------------------------
	Method CreateLevelIndicator(levelfile:String, shoeboxname:String)
		
		If TConfig.getBoolValue("showlevel")
			
			If LevelIndicator 
				LevelIndicator.FreeEntity() ' I wonder why this works...
			EndIf
			
			Local levelPreview:TLevelPreviewMesh = New TLevelPreviewMesh
			LevelIndicator = levelPreview.Create(levelfile, shoeboxname)
			
			If SkinShoebox
				SkinShoebox.Clear(True)
				SkinShoeboxAnchor.FreeEntity()
			EndIf
			
			Local skinShoeboxPath:String = ExtractDir(m_shoeboxFilename) + "/skin/" + StripDir(m_shoeboxFilename)
			
			If (FileSize(skinShoeboxPath) > 0)
				Print skinShoeboxPath
				SkinShoebox       = New ShoeBox.Init( win )
				SkinShoeboxAnchor = TPivot.CreatePivot()

				SkinShoebox.SetParent(SkinShoeboxAnchor)
				SkinShoebox.OpenFromXML(skinShoeboxPath, True)
				
				' move the pivot half the level size to the lower left
				SkinShoeboxAnchor.PositionEntity(-levelPreview.levelWidth * 0.5, -levelPreview.levelHeight * 0.5, 0, True)
				'SkinShoeboxAnchor.HideEntity()
			EndIf
			' FIXME: Scale should also work
			
			For Local p:ShoePlane = EachIn ShoePlanes
				p.SetInternalScale(levelPreview.GetShoeboxScale())
			Next
		EndIf
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	Method ShowLevelIndicator()
		
		If LevelIndicator LevelIndicator.ShowEntity()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method HideLevelIndicator()
	
		If LevelIndicator LevelIndicator.HideEntity()
		
	EndMethod

	'----------------------------------------------------------------------------
	Method ShowLevelSkin()
		
		If SkinShoeboxAnchor SkinShoeboxAnchor.ShowEntity()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method HideLevelSkin()
	
		If SkinShoeboxAnchor SkinShoeboxAnchor.HideEntity()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method HasUnsavedChanges:Int()
		Return (Not Action.IsAtFirstAction())
	EndMethod
	
	'----------------------------------------------------------------------------
	Method CheckUnsavedChanges:Int()
		
		Local ret:Int = True
		
		If HasUnsavedChanges()
			Local answer:Int = Proceed( "You have unsaved changes, do you want to save your changes?" )
			
			If answer = 1
				ret = SaveAs()
			ElseIf answer = -1
				ret = False
			EndIf
		EndIf
		
		Return ret
	EndMethod	

	'----------------------------------------------------------------------------
	' checks if the user has local unsaved changes. If she has, she will be prompted
	' to reload the disk version and lose the local changes. Otherwise the newer version is
	' loaded.
	Method CheckReload:Int(p_filename:String)
		
		Local ret:Int = True
		Rem
		' old auto reloading
		If HasUnsavedChanges()
			Local answer:Int = Proceed( "You have unsaved changes, do you want to lose your changes and load the file on disk?" )
			
			If answer <> 1
				ret = False
			EndIf
		EndIf
		EndRem
		Local answer:Int = Proceed("File '" + p_filename + "' has been modified outside the editor. Should it be reloaded?")
		ret = answer = 1
		
		Return ret
	EndMethod
	
	Method IsShoeboxOnDiskNewer:Int()
		Return IsFileOnDiskNewer(m_shoeboxFilename, m_shoeboxFiletime)
	EndMethod
	
	Method IsFileOnDiskNewer:Int(p_filename:String, p_filetime:Int)
		Local diff:Int = (FileTime(p_filename) - p_filetime)
		Return (diff > 0)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ReloadIfNewer()
		If IsFileOnDiskNewer(m_shoeboxFilename, m_shoeboxFiletime) And CheckReload(m_shoeboxFilename)
			ReloadShoebox(False)
		EndIf
		
		If m_levelFilename
			If IsFileOnDiskNewer(m_levelFilename, m_levelFiletime) ' AUTO RELOAD
				ReloadLevel()
			EndIf		
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ReloadShoebox(p_reloadTextures:Int)
	
'		If CheckReload()
			Cls()
			Flip()
			'SetPointer(POINTER_APPSTARTING)
			If (p_reloadTextures)
				TTexture.ClearTextures()
				TextureLoader.Reset()
			EndIf
			OpenFromXML(m_shoeboxFilename)
			'SetPointer(POINTER_DEFAULT)
'		EndIf
		
	EndMethod
	
	Method ReloadLevel()
		OpenLevel(m_levelFilename)
	EndMethod
	

	'----------------------------------------------------------------------------
	Method ToString:String()
		
		Local s:String = "Shoebox "
		
		For Local p:Shoeplane = EachIn ShoePlanes
			s:+ "~t" + p.ToString()
		Next
		
		Return s
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method ClearTextNodes()
		Local rootnode:TxmlNode = xml.GetRootElement()

		Local list:TList = CreateList()
		ParseTextNodes(rootnode, list)
		
		For Local n:TxmlNode = EachIn list
			n.unlinkNode()
			n.freeNode()
		Next
	EndMethod

	Method ParseTextNodes( node:TxmlNode, list:TList )		
		' we don't want those nasty blank nodes!
		' let the xml parser do the layouting...
		If node.isBlankNode()
			list.addLast( node )
			?debug
			Print "blanknode: " + node.GetText()
			?
		EndIf
		
		Local children:TList = node.getChildren(0)
		
		If children
			For Local n:TxmlNode = EachIn children
				ParseTextNodes( n, list )
			Next
		EndIf
	EndMethod
EndType