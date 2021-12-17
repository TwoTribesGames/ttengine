#!/bin/bash

# Verify command line arguments
if [ -z $1 ] ; then
	echo "No working copy path specified."
	exit 1
fi

if [ ! -d $1 ] ; then
	echo "Specified working copy '$1' is not a directory (or does not exist)."
	exit 1
fi

if [ -z $2 ] ; then
	echo "No target directory for revision info file specified."
	exit 1
fi

if [ ! -d $2 ] ; then
	echo "Specified target directory for revision info '$2' is not a directory (or does not exist)."
	exit 1
fi

# TTDEV_ROOT environment variable is required
if [ -z $TTDEV_ROOT ] ; then
	echo "Environment variable TTDEV_ROOT is not set (must point to the root of the TTdev working copy)."
	exit 1
fi

if [ ! -d $TTDEV_ROOT ] ; then
	echo "Environment variable TTDEV_ROOT ('$TTDEV_ROOT') does not point to a directory."
	exit 1
fi

# Check which revision template file to use (if revision template file is specified, use that)
REV_TEMPLATE_FILENAME=$TTDEV_ROOT/tools/windows/revision_info/__revision_template.h
if [ ! -z $3 ] ; then
	REV_TEMPLATE_FILENAME=$3
fi

if [ ! -f $REV_TEMPLATE_FILENAME ] ; then
	echo "Revision template filename '$REV_TEMPLATE_FILENAME' does not exist."
	exit 1
fi

# Get a temporary filename
# FIXME: This should be something unique for each run
REV_TEMPFILE=/tmp/__revision_autogen.h

# Retrieve the revision information for the specified working copy
REVISION_NUMBER=`svn info $1 | grep 'Last Changed Rev: ' | cut -c 19-`

# Parse the revision info template file to a temp file
sed s/\\\$WCREV\\\$/$REVISION_NUMBER/ < $REV_TEMPLATE_FILENAME > $REV_TEMPFILE

# Only update the target file if there was actually a change (prevent unnecessary rebuilds)
if ! cmp -s $REV_TEMPFILE $2/__revision_autogen.h ; then
	cp -f -p $REV_TEMPFILE $2/__revision_autogen.h
fi

# Clean up after ourselves
rm -f $REV_TEMPFILE
