' helper script for FileInformation CMake module

Function ToISODateTime(ByVal dt)
	ToISODateTime = CStr(Year(dt)) & "-" & _
		Right("00" & Month(dt), 2) & "-" & _
		Right("00" & Day(dt), 2) & "T" & _
		Right("00" & Hour(dt), 2) & ":" & _
		Right("00" & Minute(dt), 2) & ":" & _
		Right("00" & Second(dt), 2)
End Function

If WScript.Arguments.Count < 1 Then
	Wscript.Quit 1
End If

InputOption=WScript.Arguments.Item(0)

If InputOption = "--current_timestamp" Then
	Wscript.Echo ToISODateTime(Now)
	Wscript.Quit 0
End If

If WScript.Arguments.Count < 2 Then
	Wscript.Quit 1
End If

Set FSO=CreateObject("Scripting.FileSystemObject")
if FSO.FolderExists(WScript.Arguments.Item(1)) Then
	Set FSItem=FSO.GetFolder(WScript.Arguments.Item(1))
Else 
	Set FSItem=FSO.GetFile(WScript.Arguments.Item(1))
End If

If InputOption = "--size" Then
	Wscript.Echo CStr(FSItem.Size)
	Wscript.Quit 0
ElseIf InputOption = "--timestamp" Then
	Wscript.Echo ToISODateTime(FSItem.DateLastModified)
	Wscript.Quit 0
End If

Wscript.Quit 1
