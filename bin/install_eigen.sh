#! /bin/bash -e
# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-20 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
if [ $0 != 'bin/install_eigen.sh' ]
then
	echo 'bin/install_eigen.sh: must be executed from its parent directory'
	exit 1
fi
# -----------------------------------------------------------------------------
# bash function that echos and executes a command
echo_eval() {
	echo $*
	eval $*
}
# ---------------------------------------------------------------------------
version='3.2.9'
web_page='https://gitlab.com/libeigen/eigen.git'
# --------------------------------------------------------------------------
# Get user configuration options from run_cmake.sh
#
# build_type
cmd=`grep '^build_type=' bin/run_cmake.sh`
eval $cmd
#
# eigen_prefix
cmd=`grep '^eigen_prefix=' bin/run_cmake.sh`
eval $cmd
# --------------------------------------------------------------------------
dismod_at_dir=`echo $eigen_prefix | sed -e 's|/dismod_at/eigen$|/dismod_at|'`
if echo "$dismod_at_dir" | grep '/dismod_at$' > /dev/null
then
	bin/build_type.sh install_eigen $dismod_at_dir $build_type
fi

if echo "$eigen_prefix" | grep '/dismod_at$' > /dev/null
then
	bin/build_type.sh install_eigen $eigen_prefix $build_type
fi
# --------------------------------------------------------------------------
if [ ! -e build/external ]
then
	mkdir -p build/external
fi
cd build/external
# --------------------------------------------------------------------------
if [ ! -e eigen.git ]
then
	echo_eval git clone $web_page eigen.git
fi
echo_eval cd eigen.git
echo_eval git checkout $version
#
if [ ! -e build ]
then
	echo_eval mkdir build
fi
echo_eval cd build
# --------------------------------------------------------------------------
echo_eval cmake \
	-Wno-dev \
	-DCMAKE_INSTALL_PREFIX=$eigen_prefix \
	-DCMAKE_BUILD_TYPE=$build_type \
	..
echo_eval make install
# --------------------------------------------------------------------------
include_dir="$eigen_prefix/include"
if [ ! -h $include_dir/Eigen ]
then
	echo_eval ln -s $include_dir/eigen3/Eigen $include_dir/Eigen
fi
# -----------------------------------------------------------------------------
echo 'install_eigen.sh: OK'
exit 0
