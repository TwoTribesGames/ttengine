SuperStrict

Import "ShoePlane.bmx"
Rem
Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
EndRem
Type Fog Extends ShoePlane


	Field rangeXML:TxmlNode
	Field colorXML:TxmlNode
	
	Field R:Int, G:Int, B:Int
	Field Near:Float, Far:Float
	
	'----------------------------------------------------------------------------
	Method InitFog:Fog( node:TxmlNode, p_isDummy:Int = False)
		
		Local children:TList = node.getChildren()
		
		If children
			For Local n:TxmlNode = EachIn children
				If n.getName() = "color" Then
					colorXML = n
					
					R = Int(n.GetAttribute("r"))
					G = Int(n.GetAttribute("g"))
					B = Int(n.GetAttribute("b"))
'					win.SetFogColor(Int(n.GetAttribute("r")), Int(n.GetAttribute("g")), Int(n.GetAttribute("b")))
					
				ElseIf n.getName() = "range" Then
					rangeXML = n
					
					Near = Float(n.GetAttribute("near"))
					Far  = Float(n.GetAttribute("far"))
'					win.SetFogRange(Int(n.GetAttribute("near")), Int(n.GetAttribute("far")))
				
				EndIf
			Next
		EndIf
		
		Super.Init( node, p_isDummy )	
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False, p_parent:TxmlNode = Null)
				
		Local xmlnode:TxmlNode = DuplicateXML(p_isDummy, p_parent)
		
		Local e:ShoePlane = New Fog.InitFog( xmlNode, p_isDummy )
		
		Return e
	
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()
		
		'DebugStop()
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		
		' don't create a mesh
		
	EndMethod
		
	'----------------------------------------------------------------------------
	Method GetToolText:String()	
	
		'Return "pos(" + GetX() + ", " + GetY() + ", " + GetZ() + ") effect(" + xmlNode.getAttribute("effect_file") + ")"
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method UpdateXML()
		' Set range node
		rangeXML.SetAttribute( "near", FloatToFixedPoint(Near) )
		rangeXML.SetAttribute( "far",  FloatToFixedPoint(Far) )
		
		' Set color node
		colorXML.SetAttribute( "r", R )
		colorXML.SetAttribute( "g", G )
		colorXML.SetAttribute( "b", B )

	EndMethod
		
EndType

