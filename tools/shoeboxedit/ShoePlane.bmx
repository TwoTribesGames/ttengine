SuperStrict

Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
Import "PlaneHelper.bmx"
Import "TextureLoader.bmx"

Const ShoePlaneType_None:Int    = 0
Const ShoePlaneType_Normal:Int  = 1
Const ShoePlaneType_Include:Int = 2
Const ShoePlaneType_Emitter:Int = 3

Type ShoePlane

	Field texture:TTexture
	Field mesh:TMesh
	Field pixmap:TPixmap
	
	Field TextureName:String
	
	Field xmlNode:TxmlNode
	
	Field width:Float
	Field height:Float
	Field priority:Int
	Field inforeground:Int = True
	Field aspect:Double
	Field internalScale:Float = 1.0
	Field pid:Int = -1
	Field tag:String = ""
	
	Field r:Int = 255, g:Int = 255, b:Int = 255, a:Int = 255
	
	Field tileu:Float = 1
	Field tilev:Float = 1
	Field posu:Float = 0
	Field posv:Float = 0
	
	Field m_z:Float = 0.0
	
	Field originalWidth:Int = 0
	Field originalHeight:Int = 0
	Field originalRatio:Float = 1
	
	Global EntityMap:TMap = New TMap
	
	
	Field previousState:ShoePlane
	Field isDummy:Int = False
	Field isVisible:Int = True
	
	Field parent:TEntity = Null
	
	' IncludePlanes only use this
	Field planeType:Int = ShoePlaneType_Normal
	Field childPlanes:TList = New TList
	
	Method SetChildPlanes(p_childPlanes:TList, p_moveChildren:Int)
		Print "Should not get here"
	End Method
	
	'----------------------------------------------------------------------------
	Method ToString:String()
		If isDummy Return "[[[Dummy]]]~n"
		
		Local s:String = xmlNode.ToString()
		
		s:+ "tile: (" + tileu + ", " + tilev +") (" + posu + ", " + posv + ")"
		s:+ "~n"
				
		Return s
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Init:ShoePlane( node:TxmlNode, p_isDummy:Int = False )
		
		' store the node
		xmlNode = node
		isDummy = p_isDummy
		
		GetShoePlaneAttributes()
		If Not isDummy
			StoreState()
				
			' add mesh to lookup map for picking
			If mesh
				EntityMap.Insert( mesh, Self )
			EndIf
		EndIf
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method StoreState()
		If Not isDummy
			' clear prev state
			ClearStoredState()
?debug
'Print "STORESTATE: " + Self.ToString()
?
			previousState = Self.Duplicate(True)
			previousState.isDummy = True
			
			previousState.HideMesh()
		EndIf
	EndMethod
	Method ClearStoredState()
		If previousState previousstate.Remove()
		previousState = Null
	EndMethod
	Method HideMesh()
		If mesh mesh.HideEntity()
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False, p_parent:TxmlNode = Null)
		
		Local newxmlnode:TxmlNode = DuplicateXML(p_isDummy, p_parent)
		
		Local s:ShoePlane = New ShoePlane.Init( newxmlnode, p_isDummy )
		
		Return s
	
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method DuplicateXML:TxmlNode(p_isDummy:Int = False, p_parent:TxmlNode = Null)

		updatexml()
		
		Local node:TxmlNode 
	'	If p_isDummy
			node = xmlNode.copy()
	'	Else
	'		node = copyXmlNode( xmlNode )
	'	EndIf
		If (p_parent = Null)
			xmlNode.addNextSibling( node )
		Else
			p_parent.addNextSibling( node )
		EndIf 
		
		Return node
		
	EndMethod
	Rem
	Method copyXmlNode:TxmlNode(p_node:TxmlNode)
		DebugLog "---------------"
		DebugLog (p_node.getName())
		DebugLog TxmlNode(p_node.GetParent()).getname()
		
		Local node:TxmlNode = p_node.copy(0)'TxmlNode(p_node.GetParent()).addChild(p_node.getName())'

		Local attrs:TList = p_node.getAttributeList()
		If attrs <> Null
			For Local a:TxmlAttribute = EachIn attrs
				node.setAttribute(a.getName(), a.getValue())
			Next
		EndIf
'		Rem
		If p_node.getChildren()
		
			Local children:TList = New TList
			For Local c:TxmlNode = EachIn p_node.getChildren()
				children.addLast(copyXmlNode(c))
			Next
			node.addChildList(children)
		EndIf
'		EndRem
		Return p_node
	EndMethod
	EndRem
	'----------------------------------------------------------------------------
	Method Remove()
		ClearStoredState()
		If xmlNode
			xmlNode.unlinkNode()
			xmlNode.freeNode()
			xmlNode = Null
		EndIf
		
		If mesh mesh.FreeEntity()
		
	EndMethod
	
	
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()

		
		width    = Float(xmlNode.getAttribute( "width" ) )
		height   = Float(xmlNode.getAttribute( "height" ) )
		
		Local rotation:Int = Int(xmlNode.getAttribute( "rotation" ) )
		
		Local x:Float      = Float(xmlNode.getAttribute( "x" ) )
		Local y:Float      = Float(xmlNode.getAttribute( "y" ) )
		Local z:Float      = Float(xmlNode.getAttribute( "z" ) )
		
		Local tex:String   = xmlNode.getAttribute( "texture" )
		
		If xmlNode.hasAttribute( "texture_top_right_u" )
			posu  = Float(xmlNode.getAttribute( "texture_top_left_u" )) 
			posv  = Float(xmlNode.getAttribute( "texture_top_left_v" ))

			tileu = Float(xmlNode.getAttribute( "texture_top_right_u"   )) - posu
			tilev = Float(xmlNode.getAttribute( "texture_bottom_left_v" )) - posv
		EndIf
		
		InitShoeMesh( x,y,z, ..
		              rotation, ..
		              width, height, ..
		              tex )
		
		SetPriority( Int(xmlnode.getAttribute( "priority" )) )
		inforeground = (xmlNode.GetParent() And xmlNode.GetParent().GetName() = "foreground")	
		
		If Not inforeground
			mesh.EntityOrder(2)
		EndIf

		' MESH FX		
		Local fxflags:Int = 1 | 4 | 32' full bright + flatshaded + force alpha
		
		If xmlNode.getAttribute("blend_mode") = "add"
			mesh.EntityBlend(3)
			fxflags :| 8 ' ignore fog 
		Else If xmlNode.getAttribute("blend_mode") = "modulate"
			mesh.EntityBlend(2)
			fxflags :| 8 ' ignore fog 
		EndIf
		If xmlNode.getAttribute("ignore_fog") = "true"
			fxflags :| 8 ' ignore fog 
		EndIf
		
		mesh.EntityFX(fxflags)
		
		' end MESH FX
		
		
		Local children:TList = xmlNode.GetChildren()
		
		If children
			
			For Local n:TxmlNode = EachIn children
				
				Select n.GetName()
				
					Case "color_whole_quad"
						
						r = Int( n.GetAttribute("r") )
						g = Int( n.GetAttribute("g") )
						b = Int( n.GetAttribute("b") )
						a = Int( n.GetAttribute("a") )
						
						mesh.EntityColor( r, g, b )
						mesh.EntityAlpha( a / 255.0 )
					Case "tag"
						tag = n.GetAttribute("name")
					' moar?
					
				EndSelect
			Next
			
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetID:String()
		Return TextureName
	EndMethod

	'----------------------------------------------------------------------------	
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		
		CreatePlane()
		
		SetPosition( x, y, z )
		SetSize( width, height )
		SetRotation( rotation )
		SetInternalScale(1.0)

		SetTexture(TConfig.getStringValue("gfxdir") + tex)
		SetTexPos(posu, posv)
		SetTexSize(tileu, tilev)
		
?debug
'Print "Created ShoePlane: " + tex + "~t~t " + width + "x" + height + "~t~t~t(" + x + ", " + y + ", " + z + ")~t~tangle: " + rotation
?
	EndMethod

	Method SetIgnoreFog(ignore:Int)
		If ignore 
			mesh.brush.fx :| 8
		Else
			mesh.brush.fx :& ~8
		EndIf
	EndMethod
	
	Method GetIgnoreFog:String()
		If mesh.brush.fx & 8
			Return "true"
		Else
			Return "false"
		EndIf
	EndMethod
	
	Method SetInternalScale(p_scale:Float)
		internalScale = p_scale
		If mesh
			mesh.ScaleMesh(p_scale, p_scale, 1.0)
			ResizePos(p_scale)
		EndIf
	EndMethod
	
	Method GetInternalScale:Float()
		Return internalScale
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method UpdateXML()
		
		' FIXME: Unset all attributes, so that the order is guaranteed to be the same
		'For Local attribute:TxmlAttribute = EachIn xmlNode.getAttributeList()
		'	xmlNode.unsetAttribute(attribute.getName())
		'Next
		
		xmlNode.SetAttribute( "pid", pid )
		
		xmlNode.SetAttribute( "texture", GetTexture() )
		
		xmlNode.SetAttribute( "x", FloatToFixedPoint(GetX() / internalScale) )
		xmlNode.SetAttribute( "y", FloatToFixedPoint(GetY() / internalScale) )
		xmlNode.SetAttribute( "z", FloatToFixedPoint(GetZ() / internalScale) )
		
		xmlNode.SetAttribute( "width", FloatToFixedPoint(GetWidth()) )
		xmlNode.SetAttribute( "height", FloatToFixedPoint(GetHeight()) )
		
		xmlNode.SetAttribute( "priority", GetPriority() )
		
		xmlNode.SetAttribute( "rotation", FloatToFixedPoint(GetAngle()) )
		
		If True'tileu <> 1.0 Or tilev <> 1.0 Or posu <> 0.0 Or posv <> 0.0
			'xmlNode
			' add or update uvcoords...
			xmlNode.SetAttribute( "texture_top_left_u", FloatToFixedPoint(posu) )
			xmlNode.SetAttribute( "texture_top_left_v", FloatToFixedPoint(posv) )
			
			xmlNode.SetAttribute( "texture_top_right_u", FloatToFixedPoint(posu + tileu) )
			xmlNode.SetAttribute( "texture_top_right_v", FloatToFixedPoint(posv) )
			
			xmlNode.SetAttribute( "texture_bottom_right_u", FloatToFixedPoint(posu + tileu) )
			xmlNode.SetAttribute( "texture_bottom_right_v", FloatToFixedPoint(posv + tilev) )
			
			xmlNode.SetAttribute( "texture_bottom_left_u", FloatToFixedPoint(posu) )
			xmlNode.SetAttribute( "texture_bottom_left_v", FloatToFixedPoint(posv + tilev) )
		EndIf
		
		xmlNode.SetAttribute( "ignore_fog", GetIgnoreFog() )
		
		UpdateTagInXML()
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateTagInXML()
		Local found:Int = False
		Local children:TList = xmlNode.GetChildren()
		If children
			For Local n:TxmlNode = EachIn children
				If (n.GetName() = "tag")
					If (tag <> "")
						n.SetAttribute("name", tag)
						found = True
					Else
						n.unlinkNode()
						n.freeNode()
					EndIf
					Exit
				EndIf
			Next
		EndIf
		If (found = False And tag <> "")
			Local tagNode:TxmlNode = xmlNode.addChild("tag")
			tagNode.addAttribute("name", tag)
		EndIf	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnSelect(isfirst:Int = False)

		mesh.draw_wire = True
		If isFirst selectionMarker.ShowEntity()

	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnDeselect()
		
		mesh.draw_wire = False
		selectionMarker.HideEntity()
		
	EndMethod

	'----------------------------------------------------------------------------
	Method OnHide()
		isVisible = False
		If mesh mesh.HideEntity()
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method OnShow()
		isVisible = True
		If mesh mesh.ShowEntity()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Move( dx:Float, dy:Float, dz:Float )
		If mesh
			mesh.TranslateEntity( dx, dy, dz )
			SetZ(getZ() + dz)
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetPosition( p_x:Float, p_y:Float, p_z:Float )
		m_z = p_z
		If mesh
			mesh.PositionEntity( p_x, -p_y, -m_z)
			SetZ(m_z)
		EndIf
		'mesh.EntityOrder( -GetZ() )
				
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetZ( p_z:Float )
		m_z = p_z
		SetPriority(GetPriority())
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetX:Float()
		If mesh Return mesh.px Else Return 0
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetY:Float()	
		If mesh Return -mesh.py Else Return 0
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetMinX:Float()
		If mesh
			Return mesh.px - mesh.sx
		EndIf
		Return 0
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetMinY:Float()	
		If mesh 
			Return -mesh.py - mesh.sy
		EndIf
		Return 0
	EndMethod

	
	'----------------------------------------------------------------------------	
	Method GetZ:Float()
		If mesh
			Return m_z
		Else
			Return 0
		EndIf
	EndMethod

	'----------------------------------------------------------------------------	
	Method GetPriority:Int()
		Return priority
	EndMethod
	
	Method SetPriority:Int( p:Int )
		If (p < -499) p = -499
		If (p >  499) p =  499
			
		priority = p
		If mesh
			Local prioZ:Double = (500 + p) / 1000000.0
			mesh.PositionEntity(GetX(), -GetY(), -(m_z + prioZ))
		EndIf
	EndMethod
	
	Method MovePriority:Int( p:Int )
		SetPriority( priority + p )
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Rotate( angle:Float )
		If (mesh <> Null) mesh.TurnEntity( 0, 0, angle)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetRotation( angle:Float )
		If (mesh <> Null) mesh.RotateEntity( 0, 0, angle)
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetAngle:Float()
		Return mesh.rz
	EndMethod

	'----------------------------------------------------------------------------	
	' only used to set the texture in the xml
	Method GetTexture:String()
		'DebugStop()
		Local name:String = RealPath(TextureName)
		
		If (name.Contains(RealPath(TConfig.getStringValue("gfxdir"))))
		
			name = name.Replace( RealPath(TConfig.getStringValue("gfxdir")) , "" )
			name = name[1..]
			
			Return name
		Else
			Return TextureName
		EndIf
		
	EndMethod

	Const REPEATU:Int = 1, REPEATV:Int = 2
	
	Method GetTextureRepeat:Int()
		Local ret:Int = 0
		
		' U
		If (Abs(tileu) > 1) Or (posu <> 0 And Abs(posu) <> 1) ret :| REPEATU
		If (Abs(tilev) > 1) Or (posv <> 0 And Abs(posv) <> 1) ret :| REPEATV

		Rem
		Return ((posu <> 0 And posu <> -1) Or ..
		        (posv <> 0 And posv <> -1) Or ..
 		        (tileu <> 1 And tileu <> -1) Or ..
		        (tilev <> 1 And tilev <> -1))
		EndRem
		
		Return ret
	EndMethod

	'----------------------------------------------------------------------------	
	' Set the color of the mesh and the xml child, creates child if needed.
	Method SetColor(p_r:Int, p_g:Int, p_b:Int, p_a:Int)
		
		Local children:TList = xmlNode.GetChildren()
		Local node:TxmlNode = Null
		
		If children
			For Local n:TxmlNode = EachIn children
				If n.GetName() = "color_whole_quad"
					node = n
				EndIf
			Next
		EndIf

		If node = Null
			node = xmlNode.addChild("color_whole_quad")
		EndIf
		
		r = p_r
		g = p_g
		b = p_b
		a = p_a
		
		' remove the child if the color is pure white or alpha = 0
		If ((r = 255 And g = 255 And b = 255 And a = 255) Or a = 0) Then
			node.unlinkNode()
			node.freeNode()
		Else
			node.SetAttribute( "r", r )
			node.SetAttribute( "g", g )
			node.SetAttribute( "b", b )
			node.SetAttribute( "a", a )
		EndIf
		
		mesh.EntityColor( r, g, b )
		mesh.EntityAlpha( a / 255.0 )
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method GetMeshTexture:TTexture()
		
		Return texture
		
	EndMethod

	'----------------------------------------------------------------------------
	Method Scale( ahorizontal:Float, avertical:Float, keepratio:Int = False )
	
		width:* ahorizontal
		
		If keepratio
			height = width * aspect
		Else
			height:* avertical
		EndIf
		
		SetSize( width, height )
		
	EndMethod
	
	Field _scaleZ:Int = False
	
	'----------------------------------------------------------------------------
	Method SetSize( awidth:Float, aheight:Float )
		
		width  = awidth
		height = aheight
		
		aspect = height / width
		'Print aspect
		
		If mesh <> Null 
			Local z:Float = 1
			' FIXME: MARTIJN: Is this still useful?
			'If _scaleZ z = width * 0.5
			
			mesh.ScaleEntity( width*0.5, height*0.5, z )
		EndIf
		
	EndMethod
	'----------------------------------------------------------------------------
	Method ScaleSize( adw:Float, adh:Float )
		
		width  :* adw
		height :* adh
		
		If (width  < 0.1) width  = 0.1
		If (height < 0.1) height = 0.1
		
		If (width  > 100000) width  = 100000
		If (height > 100000) height = 100000
		
		SetSize(width, height)
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	Method ScaleTex( adu:Float, adv:Float )
		
		tileu = ((adu + width) /previousState.width)  * previousState.tileu
		tilev = ((adv + height)/previousState.height) * previousState.tilev
		
?debug
Print "tileu: " + tileu + " tilev:" + tilev
Print "~tprev tileu: " + previousState.tileu
?
		SetTexSize( tileu, tilev )

	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetTexSize( au:Float, av:Float)
		
		tileu = au
		tilev = av
		
		UpdateTexCoords()
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetTexPos( au:Float, av:Float )
		
		posu = au
		posv = av
		
		UpdateTexCoords()
		
	EndMethod
	'----------------------------------------------------------------------------
	Method MoveTex( du:Float, dv:Float)
		
		posu :+ du
		posv :+ dv
		
		SetTexPos( posu, posv )
		
	EndMethod

	'----------------------------------------------------------------------------
	' stuff for shoebox scaling
	Method Resize(p_factor:Float)
		ResizeScale(p_factor)
		ResizePos(p_factor)
	EndMethod
	Method ResizeScale(p_factor:Float)
		Scale(p_factor, p_factor, True)
	EndMethod
	Method ResizePos(p_factor:Float)
		SetPosition(GetX() * p_factor, GetY() * p_factor, GetZ() * p_factor)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method FlipTex(flipu:Int, flipv:Int)
		
		If flipu
			posu = posu + tileu
			tileu = - tileu
		EndIf
		If flipv
			posv = posv + tilev
			tilev = - tilev
		EndIf
		
		UpdateTexCoords()
	EndMethod
	
	Method UpdateTexCoords()
		Local surf:TSurface = mesh.GetSurface(1) ' stupid 1-based crap
		
		surf.VertexTexCoords(0, posu        , posv + tilev)
		surf.VertexTexCoords(1, posu        , posv)
		surf.VertexTexCoords(2, posu + tileu, posv)
		surf.VertexTexCoords(3, posu + tileu, posv + tilev)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetAutoSize()
		
		'SetSize( texture.TextureWidth(), texture.TextureHeight() )
		SetSize( originalWidth / 32.0, originalHeight / 32.0)

		
		SetTexSize(1, 1)
		SetTexPos(0, 0)
	EndMethod

	Const UVMARGIN:Float = 0.01
	
	'----------------------------------------------------------------------------	
	Method FixTextureSize()
		If Abs(Abs(tileu) - 1) < UVMARGIN Then tileu = 1
		If Abs(Abs(tilev) - 1) < UVMARGIN Then tilev = 1
		If Abs(posu)           < UVMARGIN Then posu = 0
		If Abs(posv)           < UVMARGIN Then posv = 0
		
		UpdateTexCoords()	
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetWidth:Float()
		Return mesh.sx * 2
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetHeight:Float()
		Return mesh.sy * 2
	EndMethod

	'----------------------------------------------------------------------------
	Method GetToolText:String()
		Local x:String = FloatToFixedPoint(GetX() / internalScale)
		Local y:String = FloatToFixedPoint(GetY() / internalScale)
		Local z:String = FloatToFixedPoint(GetZ() / internalScale)
	
		Local r:String = "pid: " + pid + " " + .. 
		                 "pos: (" + x + "," + y + "," + z + ")[" + GetPriority() + "] " + ..
		                 "angle: " + FloatToFixedPoint(GetAngle()) + ") " + ..
		                 "size: (" + FloatToFixedPoint(GetWidth()) + "," + .FloatToFixedPoint(GetHeight()) + ") " + ..
		                 "tex: (" + GetTexture() + ")"
		If (tag <> "")
			r :+ " tag: " + Chr$(34) + tag + Chr$(34)
		EndIf
		
		r:+ " <" + xmlNode.GetParent().GetName() + ">"
		r:+ " <" + xmlNode.GetLineNumber() + ">"
		
		Return r
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method SetTexture( texturefile:String )

		Local flags:Int = 1
		
		pixmap = TextureLoader.GetTexture(textureFile)
		
		If pixmap = Null
			pixmap = TextureLoader.GetTexture(CurrentDir() + "/missing.png")
			DebugStop()
		EndIf
		
		originalWidth  = pixmap.width
		originalHeight = pixmap.height
		originalRatio  = Float(pixmap.height)/Float(pixmap.width) 
	
		If pixmap.format = PF_RGBA8888 Or pixmap.format = PF_BGRA8888
			flags :| 2
		EndIf
		
		Rem
		' chache (hopefully) both tiled an non tiled textures
		Local clampedTexture:TTexture = LoadTexture( texturefile, flags + 16 + 32 )
		Local tiledTexture:TTexture   = LoadTexture( texturefile, flags )

		If GetTextureRepeat()
			texture = tiledTexture
		Else
			texture = clampedTexture
		EndIf
		EndRem
		' check tiling
		Local tile:Int = GetTextureRepeat()
		
		If Not (tile & REPEATU) Then flags :| 16
		If Not (tile & REPEATV) Then flags :| 32

		'texture = LoadTexture( texturefile, flags )
		texture = LoadTexture( String(pixmap._source), flags )
	
		If texture
			mesh.EntityTexture( texture )
		EndIf
		
		TextureName = texturefile ' the unedited texture file path
	EndMethod
	
	Method ReadPixel:Int(p_u:Float, p_v:Float)
		If (pixmap = Null)
			DebugStop()	
			Return 0
		EndIf
		Return pixmap.ReadPixel(p_u * pixmap.width, p_v * pixmap.height)
	EndMethod

	'----------------------------------------------------------------------------	
	' reloads the currently active texture, can be used to update its tiling
	' etc.
	Method ReloadTexture()
	
	EndMethod

	Method IsSelectAble:Int()	
		Return mesh <> Null
	EndMethod


	Field selectionMarker:TMesh
	
	'----------------------------------------------------------------------------	
	Method CreatePlane:TMesh()' texturefile:String, tile:Int = False,  )
		mesh = TMesh.CreateMesh()
	
		Local surf:TSurface = mesh.CreateSurface()
			
		surf.AddVertex(-1.0,-1.0,0.0)
		surf.AddVertex(-1.0, 1.0,0.0)
		surf.AddVertex( 1.0, 1.0,0.0)
		surf.AddVertex( 1.0,-1.0,0.0)
		
		surf.VertexNormal(0,0.0,0.0,-1.0)
		surf.VertexNormal(1,0.0,0.0,-1.0)
		surf.VertexNormal(2,0.0,0.0,-1.0)
		surf.VertexNormal(3,0.0,0.0,-1.0)
		
		surf.VertexTexCoords(0, posu        , posv + tilev)
		surf.VertexTexCoords(1, posu        , posv)
		surf.VertexTexCoords(2, posu + tileu, posv)
		surf.VertexTexCoords(3, posu + tileu, posv + tilev)
	
		surf.AddTriangle(0,1,2) ' front
		surf.AddTriangle(0,2,3)
		
		If parent = Null mesh.EntityPickMode( 2 ) ' pick faces
		
		mesh.EntityFX(1 + 4 + 32 ) ' +8 is disable fog
		
		selectionMarker = _CreatePlane(mesh)
		selectionMarker.PositionEntity(0, 0, 0.1)
		selectionMarker.ScaleEntity(-1.01, 1.01, 1)
		selectionMarker.EntityColor(0, 255, 200)
		selectionMarker.EntityPickMode(0)
		selectionMarker.draw_wire = True
		selectionMarker.EntityTexture(LoadTexture("selected.png", 1 + 2))
		selectionMarker.HideEntity()


Rem
		' add texture
		texturefile = TConfig.getStringValue("gfxdir") + texturefile
		
		Local flags:Int
		
		If tile
			flags = 1+2
		Else
			flags = 1+2+16+32
		EndIf
		
		texture = LoadTexture( texturefile, flags )
		If texture
			mesh.EntityTexture( texture )
		EndIf
EndRem
		
		mesh.EntityAlpha( 1.0 )
		Return mesh
		
	EndMethod

	
	'----------------------------------------------------------------------------
	Function GetShoePlaneFromMesh:ShoePlane( aMesh:TEntity )

		Local s:ShoePlane = ShoePlane( EntityMap.ValueForKey( aMesh ) )
	
		'If s = Null DebugStop()
		
		Return s
		
	EndFunction
	
	'----------------------------------------------------------------------------
	Function ClearMap()
		
		EntityMap.Clear()
			
	EndFunction

EndType