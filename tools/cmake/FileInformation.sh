#!/bin/bash
# helper script for FileInformation CMake module

#logger $# "$@"

function to_cmake_permission() {
	local symbolic_perm=$1
	local owner_group_world=$2
	local set_user_group=$3
	case "${symbolic_perm}" in
		"r" ) echo "${owner_group_world}_READ";;
		"w" ) echo "${owner_group_world}_WRITE";;
		"x" ) echo "${owner_group_world}_EXECUTE";;
		"s" ) echo "${owner_group_world}_EXECUTE;${set_user_group}";;
		"S" ) echo "${set_user_group}";;
		"t" ) echo "${owner_group_world}_EXECUTE";;
		 *  ) echo "";;
	esac
}

function to_cmake_permissions() {
	local symbolic_perms=$1
	local -a cmake_perms_array
	for i in 0 1 2
	do
		cmake_perms_array[((0+$i))]=$(to_cmake_permission ${symbolic_perms:((1+$i)):1} "OWNER" "SETUID")
		cmake_perms_array[((3+$i))]=$(to_cmake_permission ${symbolic_perms:((4+$i)):1} "GROUP" "SETGID")
		cmake_perms_array[((6+$i))]=$(to_cmake_permission ${symbolic_perms:((7+$i)):1} "WORLD")
	done
	local cmake_perms=""
	for cmake_perm in ${cmake_perms_array[*]}
	do
		if [ -z $cmake_perms ]
		then
			cmake_perms=$cmake_perm
		else
			cmake_perms="$cmake_perms;$cmake_perm"
		fi
	done
	echo $cmake_perms
}

INPUT_OPTION=$1

# format timestamp according to ISO 8601
TIMESTAMP_FORMAT=%Y-%m-%dT%H:%M:%S%z

if [[ $OSTYPE == darwin* || $OSTYPE == freebsd* ]]
then
	# Mac OS X or FreeBSD
	if [ "$INPUT_OPTION" = "--size" ]
	then
		stat -f "%z" "$2"
	elif [ "$INPUT_OPTION" = "--timestamp" ]
	then
		date -r $(stat -f "%m" "$2") "+$TIMESTAMP_FORMAT"
	elif [ "$INPUT_OPTION" = "--current_timestamp" ]
	then
		date "+$TIMESTAMP_FORMAT"
	elif [ "$INPUT_OPTION" = "--user_name" ]
	then
		stat -f "%Su" "$2"
	elif [ "$INPUT_OPTION" = "--group_name" ]
	then
		stat -f "%Sg" "$2"
	elif [ "$INPUT_OPTION" = "--permissions" ]
	then
		to_cmake_permissions $(stat -f "%Sp" "$2")
	else
		exit 1
	fi
else
	# other Unices
	if [ "$INPUT_OPTION" = "--size" ]
	then
		stat -c "%s" "$2"
	elif [ "$INPUT_OPTION" = "--timestamp" ]
	then
		date -r "$2" "+$TIMESTAMP_FORMAT"
	elif [ "$INPUT_OPTION" = "--current_timestamp" ]
	then
		date "+$TIMESTAMP_FORMAT"
	elif [ "$INPUT_OPTION" = "--user_name" ]
	then
		stat -c "%U" "$2"
	elif [ "$INPUT_OPTION" = "--group_name" ]
	then
		stat -c "%G" "$2"
	elif [ "$INPUT_OPTION" = "--permissions" ]
	then
		to_cmake_permissions $(stat -c "%A" "$2")
	else
		exit 1
	fi
fi
