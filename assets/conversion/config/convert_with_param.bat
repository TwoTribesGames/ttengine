@echo off
rem param 1 pause flag          (true/false)
rem param 2 platform            (win)
rem param 3 database file       (windows)
rem param 4 debug out           (true/false)
rem param 5 check time          (true/false)
rem param 6 errors allowed      (true/false)
rem param 7 output path postfix (win/osx)
rem param 8 only convert changed files (true/false)

rem --------------------------------------------------------
rem     Check correctness of parameters.

rem param 1 pause flag (true/false)
IF "%1" == "true" GOTO pauseflag_correct
IF "%1" == "false" GOTO pauseflag_correct

echo Parameter 1 (pause flag) incorrect! Expects true/false
pause
GOTO call_exit

:pauseflag_correct
rem --------------------------------------------------------
rem param 2 platform       (win/steamfinal)
IF "%2" == "win" GOTO platform_correct
IF "%2" == "steamfinal" GOTO platform_correct

echo Parameter 2 (platform) incorrect! Expects win/steamfinal/cat/ps4/nx/xbo
GOTO exit_with_error

:platform_correct

rem --------------------------------------------------------
rem param 3 database file
IF "%3" == "windows" GOTO databasefile_correct

echo Parameter 3 (database file) incorrect! Expects windows
GOTO exit_with_error

:databasefile_correct

rem --------------------------------------------------------
rem param 4 debug out      (true/false)
IF "%4" == "true" GOTO debugout_correct
IF "%4" == "false" GOTO debugout_correct

echo Parameter 4 (debug out) incorrect! Expects true/false
GOTO exit_with_error

:debugout_correct

rem --------------------------------------------------------
rem param 5 check time     (true/false)
IF "%5" == "true" GOTO checktime_correct
IF "%5" == "false" GOTO checktime_correct

echo Parameter 5 (check time) incorrect! Expects true/false
GOTO exit_with_error

:checktime_correct

rem --------------------------------------------------------
rem param 6 errors allowed (true/false)
IF "%6" == "true" GOTO errorsallowed_correct
IF "%6" == "false" GOTO errorsallowed_correct

echo Parameter 6 (errors allowed) incorrect! Expects true/false
GOTO exit_with_error

:errorsallowed_correct

rem --------------------------------------------------------
rem param 7 output path postfix
IF "%7" == "win" GOTO pathpostfix_correct
IF "%7" == "osx" GOTO pathpostfix_correct

echo Parameter 7 (output path postfix) incorrect! Expects win/osx
GOTO exit_with_error

:pathpostfix_correct

rem --------------------------------------------------------
rem param 8 only convert changed files (true/false)
IF "%8" == "true" GOTO onlyconvertchangedfilesflag_correct
IF "%8" == "false" GOTO onlyconvertchangedfilesflag_correct

echo Parameter 8 (onlyconvertchangedfiles flag) incorrect! Expects true/false
GOTO exit_with_error

:onlyconvertchangedfilesflag_correct


rem --------------------------------------------------------
rem param 9 should not exist
IF "%9" == "" GOTO paramcount_correct

echo Parameter 9 should not exist. Only 8 parameters are needed.
GOTO exit_with_error

:paramcount_correct

rem --------------------------------------------------------
rem Parameters are correct
rem --------------------------------------------------------

:start_conversion
echo.
echo Starting asset conversion...
echo.

rem If check time is true don't clean output.
IF "%5" == "true" GOTO no_clean

echo Rebuild
echo Cleaning output and intermediate files.

rem Clear all existing files
if exist ..\output\%7 rmdir /S /Q ..\output\%7
if exist intermediate\%7 rmdir /S /Q intermediate\%7

echo Output and intermediate files are cleared.

:no_clean

rem --------------------------------------------------------
rem Run unpack archive first to make sure all archived files
rem are restored before asset conversion
rem --------------------------------------------------------
IF "%2" == "steamfinal" GOTO unpack_archive
GOTO no_archive

:unpack_archive

if not exist ..\output\%7\archive.ma goto no_archive
echo Unpacking Memory Archive...
..\..\tools\memarchive\memarchive.exe --unpack ..\output\%7\archive.ma --output ..\output\%7
if errorlevel 1 goto problem


:no_archive

rem Create the output directory if it does not exist yet
if not exist ..\output mkdir ..\output
if not exist ..\output\%7 mkdir ..\output\%7
if not exist intermediate mkdir intermediate
if not exist intermediate\%7 mkdir intermediate\%7

echo Running asset conversion commandline

rem Have the asset tool convert all assets
set ASSETPLATFORM=%2
IF "%2" == "steamfinal" set ASSETPLATFORM=win
IF "%7" == "osx" set ASSETPLATFORM=osx

..\..\tools\assetconverters\commandline.exe -inputfile ..\%3.xml -debugout %4 -pathpostfix %7 -checktime %5 -platform %ASSETPLATFORM% -errorallowed %6 -onlyconvertchangedfiles %8 -pauseonerror %1
if errorlevel 1 goto problem
if errorlevel 0 goto post_conversion_fixup
goto problem


:post_conversion_fixup
IF "%2" == "steamfinal" goto trim_assets
goto done


rem -------------------------------
rem For final builds, remove assets that we don't want to ship with
:trim_assets

rem Remove all levels that are only for internal usage
del ..\output\%7\levels\bug_*.* > NUL
del ..\output\%7\levels\demo_*.* > NUL
del ..\output\%7\levels\temp_*.* > NUL
del ..\output\%7\levels\test_*.* > NUL

IF "%2" == "steamfinal" goto post_conversion_fixup_steam

:post_conversion_fixup_steam
rem Separate the assets that are only for the level editor
if not exist ..\output\steamfinal-editor mkdir ..\output\steamfinal-editor
if exist ..\output\%7\levels_unsavedchanges move ..\output\%7\levels_unsavedchanges ..\output\steamfinal-editor
if exist ..\output\%7\userlevels move ..\output\%7\userlevels ..\output\steamfinal-editor
if exist ..\output\%7\workshop_temp_preview_image.png move ..\output\%7\workshop_temp_preview_image.png ..\output\steamfinal-editor

goto done

rem -------------------------------

rem --------- problem ------------
:problem
echo.
echo !!!!!  CONVERSION ERROR  !!!!!
echo.

rem -----------------------------
:exit_with_error
IF "%1" == "false" GOTO call_exit
pause

:call_exit
exit /B 1


rem ------------------------------

:done

IF "%7" == "osx" GOTO compile_squirrel_manual
IF "%2" == "steamfinal" GOTO compile_squirrel
GOTO compile_squirrel_done

:compile_squirrel

echo Compile squirrel...
pushd ..\output
..\..\packs\win\full\rive.exe --compile_squirrel
popd
if errorlevel 1 goto problem
if errorlevel 0 goto compile_squirrel_done
goto problem

:compile_squirrel_manual
echo "Prep scripts in win tree"
..\..\tools\memarchive\memarchive.exe --unpack ..\output\win\archive.ma --output ..\output\win

echo "Clear existing OS X scripts"
if exist ..\output\osx\scripts rmdir /S /Q ..\output\osx\scripts

echo "Copy scripts from win tree"
xcopy /S /I ..\output\win\scripts ..\output\osx\scripts

:compile_squirrel_done

rem -------------------------------

IF "%2" == "steamfinal" GOTO pack_archive
GOTO localize_achievements

:pack_archive
echo Packing Memory Archive...

..\..\tools\memarchive\memarchive.exe --pack ..\source\shared\archives\archive.ac --empty_originals --compression lz4hc --input ..\output\%7 --output ..\output\%7
if errorlevel 1 goto problem


:localize_achievements

echo.
echo Creating Steam achievements localization file...
echo.
rem Parameters for AchievementLocalizer:
rem 1) Input localization file
rem 2) Input localization sheet (tab within Excel file)
rem 3) Output file
rem 4...) Desired languages
..\..\tools\achievements\AchievementLocalizer.exe ..\source\shared\localization\localization.xls achievements ..\source\shared\localization\achievements.vdf english german french spanish italian dutch portuguese
IF ERRORLEVEL 1 GOTO problem


:reallydone
echo.
echo Asset conversion was successful.
echo.
