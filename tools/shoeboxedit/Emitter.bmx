SuperStrict

Import "ShoePlane.bmx"
Rem
Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
EndRem
Type Emitter Extends ShoePlane

	'----------------------------------------------------------------------------
	Method InitEmitter:Emitter( node:TxmlNode, p_isDummy:Int = False)
		
		Super.Init( node, p_isDummy )
		
		planeType = ShoePlaneType_Emitter
	
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False, p_parent:TxmlNode = Null)
				
		Local xmlnode:TxmlNode = DuplicateXML(p_isDummy, p_parent)
		
		Local e:ShoePlane = New Emitter.InitEmitter( xmlNode, p_isDummy )
		
		Return e
	
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()
		
		Local x:Float        = Float(xmlNode.getAttribute( "x" ) )
		Local y:Float        = Float(xmlNode.getAttribute( "y" ) )
		Local z:Float        = Float(xmlNode.getAttribute( "z" ) )
		
		Local width:Float    = Float(xmlNode.getAttribute( "scale" )) * 2
		If width <= 0 width = 2
		Local height:Float   = width
		
		Local children:TList = xmlNode.GetChildren()		
		If children
			For Local n:TxmlNode = EachIn children
				Select n.GetName()
					Case "tag"
						tag = n.GetAttribute("name")
					' moar?
				EndSelect
			Next
		EndIf
		
		InitShoeMesh( x,y,z, 0, width, height, TConfig.getStringValue("emittertexture") )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		
		CreatePlane()
		SetSize( width, height )
		SetTexture( tex )
		SetPosition( x, y, z )
	
		SetPriority(100)
		?debug
		Print "Created Emitter: " + tex + "~t~t " + width + "x" + height + "~t~t~t(" + x + ", " + y + ", " + z + ")~t~tangle: " + rotation
		?
	EndMethod
	
	'----------------------------------------------------------------------------	
	
	Method GetID:String()
		Return xmlNode.getAttribute("effect_file")
	EndMethod
		
	'----------------------------------------------------------------------------
	Method GetToolText:String()
		Local x:String = FloatToFixedPoint(GetX() / internalScale)
		Local y:String = FloatToFixedPoint(GetY() / internalScale)
		Local z:String = FloatToFixedPoint(GetZ() / internalScale)	
	
		Local r:String = "pid: " + pid + " " + .. 
		       "pos: (" + x + ", " + y + ", " + z + ") effect: (" + xmlNode.getAttribute("effect_file") + ")"
		
		If (tag <> "")
			r :+ " tag: " + Chr$(34) + tag + Chr$(34)
		EndIf
		
		Return r
	EndMethod

	Method Scale( ahorizontal:Float, avertical:Float, keepratio:Int = False )
		Super.Scale(ahorizontal, ahorizontal, True )
	EndMethod
	
	Method ScaleSize( adw:Float, adh:Float )
		Super.ScaleSize(adw, adw)
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method UpdateXML()
	
		' Store settings before unsetting
		Local effect:String = xmlNode.getAttribute("effect_file")
			
		' FIXME: Unset all attributes, so that the order is guaranteed to be the same
		'For Local attribute:TxmlAttribute = EachIn xmlNode.getAttributeList()
		'	xmlNode.unsetAttribute(attribute.getName())
		'Next
		xmlNode.SetAttribute( "pid",         pid )
		xmlNode.SetAttribute( "effect_file", effect )
		xmlNode.SetAttribute( "x",           FloatToFixedPoint(GetX() / internalScale))
		xmlNode.SetAttribute( "y",           FloatToFixedPoint(GetY() / internalScale))
		xmlNode.SetAttribute( "z",           FloatToFixedPoint(GetZ() / internalScale))
		xmlNode.SetAttribute( "scale",       FloatToFixedPoint(GetWidth() * 0.5))
		' mmmmh, maybe remove the rotation etc. attributes here?
		
		UpdateTagInXML()
	EndMethod
		
EndType

