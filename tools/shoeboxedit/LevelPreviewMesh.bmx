SuperStrict

Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"

Type TLevelPreviewMesh
	Field maxSurfaces:Int = 16
	Field maxVerticesPerSurface:Int = 60000
	Field brush:TBrush
	Field mesh:TMesh
	Field surfaces:TSurface[maxSurfaces]
	Field shoeboxname:String
	
	Field levelWidth:Float, levelHeight:Float
	
	Field vertices:Int
	
	Field fp:TStream
	
	Field offsetX:Float
	Field offsetY:Float
	Field offsetZ:Float
	Field shoeboxScale:Float
	
	Method Create:TMesh(p_levelfilename:String, p_shoeboxname:String)
	
		If GetStream(p_levelfilename)
		
			GetBrush()
			mesh    = TMesh.CreateMesh()
			For Local i:Int = 0 Until maxSurfaces
				surfaces[i] = mesh.CreateSurface(brush) ' use a brush ?
			Next
			
			shoeboxname = p_shoeboxname
			offsetX = 0.0
			offsetY = 0.0
			shoeboxScale = 1.0
			offsetZ = 0.0
			
			ParseStream()
			
			fp.Close()
			
			SetVisualProperties()
			SetCorrectPosition()
			
			Return mesh
		EndIf
		
		Return Null
	EndMethod
	
	Method GetShoeboxScale:Float()
		Return shoeboxScale
	End Method
	
	Method GetBrush()
		brush = TBrush.CreateBrush()
		Local texture:TTexture = LoadTexture(TConfig.getStringValue("gfxdir") + "attributeview_editor.png", 1 + 2 + 8)
		brush.BrushTexture(texture)
	EndMethod
	
	Method SetVisualProperties()
		mesh.EntityAlpha(TConfig.getFloatValue("levelalpha"))
		mesh.order_bias = TConfig.getIntValue("levelpriority")
		mesh.EntityOrder(2)
	EndMethod
	
	Method SetCorrectPosition()
		mesh.PositionMesh(-offsetX, -offsetY, offsetZ)
	EndMethod
	
	Method GetStream:Int(levelfilename:String)
		fp = ReadStream(levelfilename)
		If fp
			Return True
		Else
			Return False
		EndIf
	EndMethod
	
	Method ParseStream()
		'Print "File version: " + GetFileVersion()
		
		If MoveToChunk("tile")
			ReadTiles()
		EndIf
		
		If MoveToChunk("entt")
			ReadEntities()
		EndIf	
	EndMethod
	
	Method MoveToChunk:Int(p_id:String)
		' 9 for signature, 4 for version info
		Local filePos:Int = 13
		fp.Seek(filePos)
	
		While fp.Eof() = False
			Local chunkString:String = fp.ReadString(4)
			If chunkString <> "CHNK"
				Notify "Failed loading level file. Invalid chunk marker '" + chunkString + "'"
				Return False	
			EndIf
			Local size:Int    = fp.ReadInt()
			Local crc:Int     = fp.ReadInt()
			Local id:String   = fp.ReadString(4)
			Local version:Int = fp.ReadInt()
			
			If id = p_id
				Return True
			EndIf
			
			filePos = filePos + size + 12
			fp.Seek(filePos)
		Wend
		'Return False
	EndMethod
	
	Method GetFileVersion:Int()
		fp.Seek(9)
		Return fp.ReadInt()
	EndMethod
	
	Method ReadTiles()
		Local layercount:Int = fp.ReadInt()
		
		For Local i:Int = 0 Until layercount
			
			Local width:Int  = fp.ReadInt()
			Local height:Int = fp.ReadInt()
			' assume each layer is the same size as the level
			
			levelwidth  = width
			levelheight = height
			
			Local x:Int = 0
			Local y:Int = 0
			Local tile:Int = 0
			Local success:Int = True
			
			While tile < width * height
				Local tileid:Int = fp.ReadByte()
				tileid = tileid & %11111
				
				If tileid > 0 And success = True
					success = success And CreateTile(x, y, tileid)
				End If
				
				x :+ 1
				If x >= width 
					x = 0
					y :+ 1
				EndIf
				
				tile :+ 1
			Wend
			
			Rem
			For Local x:Int = 0 Until width
				For Local y:Int = 0 Until height
					If fp.ReadByte() > 0 CreateTile(x, height - y)
				Next
			Next
			EndRem
		Next
		
	EndMethod
	
	Method ReadTTString:String()
		' TT Strings use big endian s16 for char count
		Local c1:Byte = fp.ReadByte()
		Local c0:Byte = fp.ReadByte()
		Local charcount:Short = (c1 Shl 8) | c0
		Return fp.ReadString(charcount)
	End Method
	
	Method ParsePiston(p_x:Int, p_y:Int)
		Local propscount:Int = fp.ReadInt()
		
		Local hasCover:Int = True
		Local width:Int = 5
		Local height:Int = 5
		
		For Local j:Int = 0 Until propscount
			Local propname:String = ReadTTString()
			Local propvalue:String = ReadTTString()	
			
			If (propname = "width")
				width = propvalue.ToInt()
			ElseIf (propname = "height")
				height = propvalue.ToInt()
			ElseIf  (propname = "hasCoverGraphic")
				hasCover = (propvalue = "true")
			EndIf
		Next
		
		Local id:Int = 100
		If (hasCover) id = 101
		
		For Local x:Int = 0 Until width
			For Local y:Int = 0 Until height
				Local xoffset:Int = x - width / 2
				Local yoffset:Int = y
				
				CreateTile(p_x + xoffset, p_y + yoffset, id)
			Next
		Next
	EndMethod
	
	Method ParseLight(p_x:Int, p_y:Int)
		Local propscount:Int = fp.ReadInt()
		
		For Local j:Int = 0 Until propscount
			Local propname:String = ReadTTString()
			Local propvalue:String = ReadTTString()	
		Next
		
		Local id:Int = 102
		CreateTile(p_x, p_y, id)
	EndMethod
	
	Method ReadEntities()
		Local entitycount:Int = fp.ReadInt()
		offsetX = 0.0
		offsetY = 0.0
		
		For Local i:Int = 0 Until entitycount
			
			Local entity:String = ReadTTString()
			Local id:Int  = fp.ReadInt()
			Local x:Float = fp.ReadFloat()
			Local y:Float = fp.ReadFloat()
			
			If (entity = "Piston")
				ParsePiston(x, y)
				Continue
			Else If (entity = "LightSource")
				ParseLight(x, y)
				Continue
			EndIf
			
			Local propscount:Int = fp.ReadInt()
			
			Local parsedScale:Float   = 1.0
			Local parsedOffsetZ:Float = 0.0
			
			Local isValidShoeboxIncluder:Int = False
			
			For Local j:Int = 0 Until propscount
				Local propname:String = ReadTTString()
				Local propvalue:String = ReadTTString()
				If (entity = "ShoeboxIncluder")
					If propname = "shoeboxInclude" And (propvalue = shoeboxname Or (propValue + "_lightmask") = shoeboxname)
						isValidShoeboxIncluder = True
					End If
					If propname = "scale"
						parsedScale = propvalue.ToFloat()
					End If
					If propname = "zPosition"
						parsedOffsetZ = propvalue.ToFloat()
					End If
				End If
			Next		
			
			If isValidShoeboxIncluder
				offsetX = x
				offsetY = y
				offsetZ = parsedOffsetZ
				shoeboxScale = parsedScale
			End If
		Next
	End Method
	
	Method CreateTile:Int(x:Int, y:Int, id:Int)
		Local sid:Int = vertices / maxVerticesPerSurface
		Local v:Int  = vertices Mod maxVerticesPerSurface
		
		If (sid >= maxSurfaces)
			Notify("Too many vertices (" + vertices + ") to render level geometry. Increase number of surfaces.", True )
			Return False
		End If 
		
		surfaces[sid].AddVertex(x      , y      , 0.0)
		surfaces[sid].AddVertex(x      , y + 1.0, 0.0)
		surfaces[sid].AddVertex(x + 1.0, y + 1.0, 0.0)
		surfaces[sid].AddVertex(x + 1.0, y      , 0.0)
		
		surfaces[sid].VertexNormal(v, 0.0, 0.0, -1.0)
		surfaces[sid].VertexNormal(v + 1, 0.0, 0.0, -1.0)
		surfaces[sid].VertexNormal(v + 2, 0.0, 0.0, -1.0)
		surfaces[sid].VertexNormal(v + 3, 0.0, 0.0, -1.0)
		
		Local uvs:Float[] = GetUVsForTileID(id)
		
		surfaces[sid].VertexTexCoords(v    , uvs[0], uvs[1])
		surfaces[sid].VertexTexCoords(v + 1, uvs[2], uvs[3])
		surfaces[sid].VertexTexCoords(v + 2, uvs[4], uvs[5])
		surfaces[sid].VertexTexCoords(v + 3, uvs[6], uvs[7])
		
		surfaces[sid].AddTriangle(v, v + 1, v + 2)
		surfaces[sid].AddTriangle(v, v + 2, v + 3)
		
		vertices :+ 4
		Return True
	EndMethod
	
	Method GetUVsForTileID:Float[](id:Int)
		'Return [0.000, 1.000,  0.000, 0.000,  1.000, 0.000,  1.000, 1.000]

		Local stringvals:String[] = (TConfig.getStringValue("tiles.t" + id)).Split(",")
		Local uvoffsets:Float[]   = [0.000, 0.000,  0.000, 0.000,  1.000, 0.000,  1.000, 1.000]

		If stringvals.length < 8 
			Notify("UV info for tile number " + id + " not found, config.xml needs to be updated!", True)
			Return uvoffsets
		EndIf
		
		For Local i:Int = 0 Until 8
			uvoffsets[i] = stringvals[i].ToFloat()
		Next
		
		Return uvoffsets 
	EndMethod
EndType