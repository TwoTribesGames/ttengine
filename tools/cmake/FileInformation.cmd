@echo off
rem helper script for FileInformation CMake module

setlocal enabledelayedexpansion

set "INPUT_OPTION=%~1"

if "!INPUT_OPTION!" == "--size" (
	echo %~z2
) else if "!INPUT_OPTION!" == "--timestamp" (
	rem do not use %~t2, its format depends on regional settings
	rem use Visual Basic to obtain ISO 8601 formatted time stamp
	cscript //Nologo "%~dpn0.vbs" "%~1" "%~f2"
) else if "!INPUT_OPTION!" == "--current_timestamp" (
	rem do not use %DATE% and %TIME%, their format depends on regional settings
	rem use Visual Basic to obtain ISO 8601 formatted time stamp
	cscript //Nologo "%~dpn0.vbs" "%~1"
) else (
	exit 1
)
