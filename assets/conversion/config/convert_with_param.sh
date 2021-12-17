#!/bin/bash

# param 1 pause flag          (true/false)
# param 2 platform            (osx)
# param 3 database file       (rive)
# param 4 debug out           (true/false)
# param 5 check time          (true/false)
# param 6 errors allowed      (true/false)
# param 7 output path postfix (osx-mac)

# Check incoming parameters

# ($1) Pause flag
if [[ $1 != "true" && $1 != "false" ]]; then
	echo "Parameter 1 (pause flag) is incorrect! Expects true/false but is '$1'"
	exit 1
fi

# ($2) Platform
if [[ $2 != "osx" ]]; then
	echo "Parameter 2 (platform) incorrect! Expected 'osx' but got '$2'"
	exit 1
fi

# ($3) Database file
if [[ $3 != "rive_steam" ]]; then
	echo "Parameter 3 (database file) incorrect! Expected 'rive_steam' but got '$3'"
	exit 1
fi

# ($4) Debug output
if [[ $4 != "true" && $4 != "false" ]]; then
	echo "Parameter 4 (debug output) is incorrect! Expects true/false but is '$4'"
	exit 1
fi

# ($5) Check timestamps
if [[ $5 != "true" && $5 != "false" ]]; then
	echo "Parameter 5 (check time) is incorrect! Expects true/false but is '$5'"
	exit 1
fi

# ($6) Errors allowed
if [[ $6 != "true" && $6 != "false" ]]; then
	echo "Parameter 6 (errors allowed) is incorrect! Expects true/false but is '$6'"
	exit 1
fi

# ($7) Output path postfix
if [[ $7 != "osx-mac" ]]; then
	echo "Parameter 7 (output path postfix) is incorrect! Expects 'osx-mac' but is '$7'"
	exit 1
fi

# ($8) Only convert changed files
if [[ $8 != "true" && $8 != "false" ]]; then
	echo "Parameter 8 (only changed files) is incorrect! Expects true/false but is '$8'"
	exit 1
fi

# Only 8 parameters expected
if [[ $9 != "" ]]; then
	echo "Only 8 parameters expected, but more given!"
	exit 1
fi

# Parameters are correct - start conversion
echo
echo "Starting asset conversion..."
echo

# Clean output directory if check time is false
if [[ $5 == "false" ]]; then
	echo Rebuild
	echo Cleaning output and intermediate files.

	if [[ -d ../output/$7 ]]; then
		rm -rf ../output/$7
	fi

	if [[ -d intermediate/$7 ]]; then
		rm -rf intermediate/$7
	fi

	echo Output and intermediate files are cleared.
fi

# Create output and intermediate directories if they do not exist yet
if [[ ! -d ../output ]]; then
	mkdir ../output
fi

if [[ ! -d ../output/$7 ]]; then
	mkdir ../output/$7
fi

if [[ ! -d intermediate ]]; then
	mkdir intermediate
fi

if [[ ! -d intermediate/$7 ]]; then
	mkdir intermediate/$7
fi

# Let AssetTool convert all assets
echo Running asset conversion commandline

$TTSDK_ROOT/shared/tools/assetconverters/commandline -pauseonerror $1 -platform $2 -inputfile ../$3.xml -debugout $4 -checktime $5 -errorallowed $6 -pathpostfix $7 -onlyconvertchangedfiles $8

# Check status
if [[ $? != 0 ]]; then
	echo
	echo !!!!! CONVERSION ERROR !!!!!
	echo
	exit $?
fi

# TODO: Squirrel compilation?


# Remove all levels that are meant for CAT
rm -f ../output/$7/levels/*_cat.*

# Remove all levels that are only for internal usage
# FIXME: We need a steamfinal build setup for this!
#rm -f ../output/$7/levels/concept_*.*
#rm -f ../output/$7/levels/content_*.*
#rm -f ../output/$7/levels/demo_*.*
#rm -f ../output/$7/levels/sonicpicnic_*.*
#rm -f ../output/$7/levels/sprint_*.*
#rm -f ../output/$7/levels/temp_*.*
#rm -f ../output/$7/levels/test_*.*
