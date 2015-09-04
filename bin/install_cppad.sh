#! /bin/bash -e
# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# BEGIN USER_SETTINGS
# prefix below which cppad will be installed
cppad_prefix="$HOME/prefix/dismod_at"
# extra c++ flags used during compliation
extra_cxx_flags='-std=c++11 -Wall'
# ----------------------------------------------------------------------------
# setings for IHME cluster
# extra_cxx_flags='-Wall'
# END USER_SETTINGS
# ---------------------------------------------------------------------------
if [ $0 != 'bin/install_cppad.sh' ]
then
	echo 'bin/install_cppad.sh: must be executed from its parent directory'
	exit 1
fi
# -----------------------------------------------------------------------------
# bash function that echos and executes a command
echo_eval() {
	echo $*
	eval $*
}
# --------------------------------------------------------------------------
web_page='https://github.com/coin-or/CppAD.git'
hash_key='5967e213f6fd2be5adf4adef224b3c86f6532496'
version='20150904'
# --------------------------------------------------------------------------
if [ ! -e build/external ]
then
	mkdir -p build/external
fi
echo_eval cd build/external
# --------------------------------------------------------------------------
if [ ! -e cppad-$version ]
then
	echo_eval git clone $web_page cppad-$version
fi
#
echo_eval cd cppad-$version
echo_eval git checkout --quiet $hash_key
bin/version.sh set $version
bin/version.sh copy
#
if [ ! -e build ]
then
	mkdir build
fi
echo_eval cd build
# -----------------------------------------------------------------------------
if [ -e /usr/lib64 ]
then
	libdirs="'lib64;lib'"
else
	libdirs="'lib;lib64'"
fi
#
cmake_args="-D CMAKE_VERBOSE_MAKEFILE=0"
cmake_args="$cmake_args -D cmake_install_prefix=$cppad_prefix"
cmake_args="$cmake_args -D cmake_install_libdirs=$libdirs"
echo "cmake $cmake_args -D cppad_cxx_flags='$extra_cxx_flags' .."
cmake $cmake_args -D cppad_cxx_flags="$extra_cxx_flags" ..
#
echo_eval make install
