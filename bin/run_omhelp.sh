#! /bin/bash -e
# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
# 	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
if [ "$0" != 'bin/run_omhelp.sh' ]
then
	echo 'bin/run_omhelp.sh must be run from its parent directory'
	exit 1
fi
# -----------------------------------------------------------------------------
# bash function that echos and executes a command
echo_eval() {
	echo $*
	eval $*
}
if [ "$1" != 'htm' ] && [ "$1" != 'xml' ]
then
	echo "usage: bin/run_omhelp.sh (htm|xml) [printable]"
	exit 1
fi
if [ "$2" != '' ] && [ "$2" != 'printable' ]
then
	echo "usage: bin/run_omhelp.sh (htm|xml) [printable]"
	exit 1
fi
ext="$1"
if [ "$2" == 'printable' ]
then
	printable='yes'
else
	printable='no'
fi
# -----------------------------------------------------------------------------
if [ ! -e 'doc' ]
then
	echo_eval mkdir doc
fi
# -----------------------------------------------------------------------------
version=`bin/version.sh get`
if [ ! -e "doc/dismod_at-$version.tgz" ]
then
	echo_eval rm -rf "doc/dismod_at-$version"
	echo_eval mkdir "doc/dismod_at-$version"
	for file in `git ls-files`
	do
		sub_dir=`echo $file | sed -e 's|/[^/]*$||'`
		if [ "$sub_dir" != "$file" ]
		then
			if [ ! -e doc/dismod_at-$version/$sub_dir ]
			then
				mkdir -p doc/dismod_at-$version/$sub_dir
			fi
		fi
		cp $file doc/dismod_at-$version/$file
	done
	echo_eval cd doc
	echo_eval tar -czf dismod_at-$version.tgz dismod_at-$version
	echo_eval rm -r dismod_at-$version
else
	echo_eval cd doc
fi
# -----------------------------------------------------------------------------
#
flags=''
if [ "$ext" == 'xml' ]
then
	flags="$flags -xml"
fi
if [ "$printable" == 'yes' ]
then
	flags="$flags -printable"
fi
cmd="omhelp ../doc.omh -debug -noframe $flags"
echo "$cmd > omhelp.$ext.log"
if ! $cmd  > ../omhelp.$ext.log
then
	cat ../omhelp.$ext.log
	exit 1
fi
#
if grep '^OMhelp Warning:' ../omhelp.$ext.log
then
	exit 1
fi
echo 'run_omhelp.sh: OK'
exit 0
