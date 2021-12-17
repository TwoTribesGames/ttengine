Import sidesign.minib3d

' Taken from http://www.blitzbasic.com/Community/posts.php?topic=41898
Function FloatToFixedPoint:String(f:Float,decimals:Int=3)
	Local isNegative:Int = f < 0
	f = Abs(f)
	Local multResult:Float = (10^decimals)*f;
	Local i:Long
	If multResult > 0
		multResult :+ 0.5
	Else 
		multResult :- 0.5
	EndIf
	
	Local value:String = String.fromlong(multResult)
	Local result:String
	If value.length <= decimals
		result = "0." + (RSet("",decimals-value.length)).Replace(" ","0") + value
	Else
		result = value[0..value.length-decimals] + "." + value[value.length-decimals..value.length]  
	EndIf
	
	If isNegative
		Return "-" + result;
	EndIf
	
	Return result
End Function

Function _CreatePlane:TMesh(parent_ent:TEntity=Null)
		Local mesh:TMesh = TMesh.CreateMesh(parent_ent)
	
		Local surf:TSurface = mesh.CreateSurface()
			
		surf.AddVertex(-1.0,-1.0,0.0)
		surf.AddVertex(-1.0, 1.0,0.0)
		surf.AddVertex( 1.0, 1.0,0.0)
		surf.AddVertex( 1.0,-1.0,0.0)
		
		surf.VertexNormal(0,0.0,0.0,-1.0)
		surf.VertexNormal(1,0.0,0.0,-1.0)
		surf.VertexNormal(2,0.0,0.0,-1.0)
		surf.VertexNormal(3,0.0,0.0,-1.0)
		
		surf.VertexTexCoords(0, 0, 1)
		surf.VertexTexCoords(1, 0, 0)
		surf.VertexTexCoords(2, 1, 0)
		surf.VertexTexCoords(3, 1, 1)
	
		surf.AddTriangle(0,1,2) ' front
		surf.AddTriangle(0,2,3)
		
		Return mesh
EndFunction
