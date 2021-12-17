SuperStrict

Import sidesign.minib3d

Rem 
; Created by Mikkel Fredborg
; Adapted for MiniB3D by Logan Chittenden
; 
; Use as you please, but please Include a thank you :)
;
End Rem

'; PickedTri Type
'; Necessary For the PickedU(), PickedV(), And PickedW() commands
Type PickedTri
	Field ent:TEntity,surf:TSurface,tri:Float		';picked entity, surface And triangle
	Field px:Float,py:Float,pz:Float				';picked xyz
	Field pu:Float[2],  pv:Float[2]  ,pw:Float[2]  	';picked uvw x 2
	
	Field vx:Float[3],  vy:Float[3]  ,vz:Float[3]  	';vertex xyz
	Field vnx:Float[3], vny:Float[3] ,vnz:Float[3] 	';vertex normals
	Field vu:Float[6],  vv:Float[6]  ,vw:Float[6]  	';vertex uvw x 2
End Type

Global ptri:pickedtri = New pickedtri


'; Returns the Texture U coordinate of the last successful pick command
'; coordset may be set To either 0 Or 1
Function PickedU:Float(coordset:Byte = 0)
	
	'; If something New has been picked Then calculate the New uvw coordinates
	If (PickedX()<>ptri.px) Or (PickedY()<>ptri.py) Or (PickedZ()<>ptri.pz) Or (PickedSurface()<>ptri.surf)
		PickedUVW()
	End If
	
	Return ptri.pu[coordset]
	
End Function


'; Returns the Texture U coordinate of the last successful pick command
'; coordset may be set To either 0 Or 1
Function PickedV:Float(coordset:Byte = 0)
	
	'; If something New has been picked Then calculate the New uvw coordinates
	If (PickedX()<>ptri.px) Or (PickedY()<>ptri.py) Or (PickedZ()<>ptri.pz) Or (PickedSurface()<>ptri.surf)
		PickedUVW()
	End If
	
	Return ptri.pv[coordset]
	
End Function


'; Returns the Texture U coordinate of the last successful pick command
'; coordset may be set To either 0 Or 1
Function PickedW:Float(coordset:Byte = 0)
	
	'; If something New has been picked Then calculate the New uvw coordinates
	If (PickedX()<>ptri.px) Or (PickedY()<>ptri.py) Or (PickedZ()<>ptri.pz) Or (PickedSurface()<>ptri.surf)
		PickedUVW()
	End If
	
	Return ptri.pw[coordset]
	
End Function


'; Calculates the UVW coordinates of a pick
'; Do Not call this by yourself, as PickedU(), PickedV(), And PickedW()
'; takes care of calling it when nescessary
Function PickedUVW()
	Local i:Byte

	If PickedSurface()
		ptri.ent  = PickedEntity()
		ptri.surf = PickedSurface()
		ptri.tri  = PickedTriangle()
			
		ptri.px = PickedX()
		ptri.py = PickedY()
		ptri.pz = PickedZ()
		
		For i = 0 To 2
			TFormPoint VertexX(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i)),VertexY(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i)),VertexZ(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i)),ptri.ent,Null

			ptri.vx[i] = TFormedX()
			ptri.vy[i] = TFormedY()
			ptri.vz[i] = TFormedZ()

			ptri.vnx[i] = VertexNX(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i))
			ptri.vny[i] = VertexNY(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i))
			ptri.vnz[i] = VertexNZ(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i))
					
			ptri.vu[i+0] = VertexU(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),0)
			ptri.vv[i+0] = VertexV(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),0)
			ptri.vw[i+0] = VertexW(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),0)

			ptri.vu[i+3] = VertexU(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),1)
			ptri.vv[i+3] = VertexV(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),1)
			ptri.vw[i+3] = VertexW(ptri.surf,TriangleVertex(ptri.surf,ptri.tri,i),1)
		Next

		'; Select which component of xyz coordinates To ignore
		Local coords:Byte = 3

		If Abs(PickedNX()) > Abs(PickedNY())
			If Abs(PickedNX())>Abs(PickedNZ()) Then coords = 1
		Else
			If Abs(PickedNY())>Abs(PickedNZ()) Then coords = 2
		EndIf
		
		Local a0:Float,a1:Float,b0:Float,b1:Float,c0:Float,c1:Float
		
		'; xy components
		If (coords = 3)
			'; edge 0
			a0 = ptri.vx[1] - ptri.vx[0]
			a1 = ptri.vy[1] - ptri.vy[0]
		
			'; edge 1
			b0 = ptri.vx[2] - ptri.vx[0]
			b1 = ptri.vy[2] - ptri.vy[0]

			'; picked offset from triangle vertex 0
			c0 = PickedX() - ptri.vx[0]
			c1 = PickedY() - ptri.vy[0]
		Else		
			'; xz components
			If (coords = 2)
				'; edge 0
				a0 = ptri.vx[1] - ptri.vx[0]
				a1 = ptri.vz[1] - ptri.vz[0]
		
				'; edge 1
				b0 = ptri.vx[2] - ptri.vx[0]
				b1 = ptri.vz[2] - ptri.vz[0]

				'; picked offset from triangle vertex 0
				c0 = PickedX() - ptri.vx[0]
				c1 = PickedZ() - ptri.vz[0]
			Else
				'; yz components

				'; edge 0
				a0 = ptri.vy[1] - ptri.vy[0]
				a1 = ptri.vz[1] - ptri.vz[0]
		
				'; edge 1
				b0 = ptri.vy[2] - ptri.vy[0]
				b1 = ptri.vz[2] - ptri.vz[0]

				'; picked offset from triangle vertex 0
				c0 = PickedY() - ptri.vy[0]
				c1 = PickedZ() - ptri.vz[0]
			End If
		End If
						
		Rem
		; u And v are offsets from vertex 0 along edge 0 And edge 1
		; using these it is possible To calculate the Texture UVW coordinates
		; of the picked XYZ location
		;
		; a0*u + b0*v = c0
		; a1*u + b1*v = c1
		;
		; solve equation (standard equation with 2 unknown quantities)
		; check a math book To see why the following is True
		;
		End Rem
		Local u:Float = (c0*b1 - b0*c1) / (a0*b1 - b0*a1)
		Local v:Float = (a0*c1 - c0*a1) / (a0*b1 - b0*a1)
		
		'; If either u Or v is out of range Then the
		'; picked entity was Not a mesh, And therefore
		'; the uvw coordinates cannot be calculated
		If (u<0.0 Or u>1.0) Or (v<0.0 Or v>1.0)
			Return 
		End If
		
		'; Calculate picked uvw's for coordset 0 (and modulate them to be in the range of 0-1 nescessary)
		ptri.pu[0] = (ptri.vu[0] + ((ptri.vu[1] - ptri.vu[0]) * u) + ((ptri.vu[2] - ptri.vu[0]) * v)) Mod 1
		ptri.pv[0] = (ptri.vv[0] + ((ptri.vv[1] - ptri.vv[0]) * u) + ((ptri.vv[2] - ptri.vv[0]) * v)) Mod 1
		ptri.pw[0] = (ptri.vw[0] + ((ptri.vw[1] - ptri.vw[0]) * u) + ((ptri.vw[2] - ptri.vw[0]) * v)) Mod 1
		
		'; If any of the coords are negative
		If ptri.pu[0]<0.0 Then ptri.pu[0] = 1.0 + ptri.pu[0]
		If ptri.pv[0]<0.0 Then ptri.pv[0] = 1.0 + ptri.pv[0]
		If ptri.pw[0]<0.0 Then ptri.pw[0] = 1.0 + ptri.pw[0]
		
		'; Calculate picked uvw's for coordset 1 (and modulate them to be in the range of 0-1 nescessary)
		ptri.pu[1] = (ptri.vu[3] + ((ptri.vu[4] - ptri.vu[3]) * u) + ((ptri.vu[5] - ptri.vu[3]) * v)) Mod 1
		ptri.pv[1] = (ptri.vv[3] + ((ptri.vv[4] - ptri.vv[3]) * u) + ((ptri.vv[5] - ptri.vv[3]) * v)) Mod 1
		ptri.pw[1] = (ptri.vw[3] + ((ptri.vw[4] - ptri.vw[3]) * u) + ((ptri.vw[5] - ptri.vw[3]) * v)) Mod 1

		'; If any of the coords are negative
		If ptri.pu[1]<0.0 Then ptri.pu[1] = 1.0 + ptri.pu[1]
		If ptri.pv[1]<0.0 Then ptri.pv[1] = 1.0 + ptri.pv[1]
		If ptri.pw[1]<0.0 Then ptri.pw[1] = 1.0 + ptri.pw[1]
	End If

End Function
