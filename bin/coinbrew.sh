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
if [ $0 != 'bin/install_ipopt.sh' ]
then
	echo 'bin/install_ipopt.sh: must be executed from its parent directory'
	exit 1
fi
# -----------------------------------------------------------------------------
# bash function that echos and executes a command
echo_eval() {
	echo $*
	eval $*
}
# --------------------------------------------------------------------------
version="3.13.2"
coinbrew='https://raw.githubusercontent.com/coin-or/coinbrew/master/coinbrew'
# ---------------------------------------------------------------------------
# Get user configuration options from run_cmake.sh
#
# build_type
cmd=`grep '^build_type=' bin/run_cmake.sh`
eval $cmd
#
# ipopt_prefix
cmd=`grep '^ipopt_prefix=' bin/run_cmake.sh`
eval $cmd
#
# cmake_cxx_compiler
cmd=`grep '^cmake_cxx_compiler=' bin/run_cmake.sh`
eval $cmd
#
# cmake_libdir
cmd=`grep '^cmake_libdir=' bin/run_cmake.sh`
eval $cmd
# --------------------------------------------------------------------------
# set links for this build type
if echo "$ipopt_prefix" | grep '/dismod_at$' > /dev/null
then
	bin/build_type.sh install_ipopt $ipopt_prefix $build_type
fi
# --------------------------------------------------------------------------
if [ ! -e build/external ]
then
	mkdir -p build/external
fi
cd build/external
# -----------------------------------------------------------------------------
if [ -e coinbrew ]
then
	rm coinbrew
fi
echo_eval wget $coinbrew
echo_eval chmod +x coinbrew
# -----------------------------------------------------------------------------
# klugde necessary until coin or mumps fixes this problem
cat << EOF > junk.f
      program junk
      print*, "Hello World"
      end
EOF
if gfortran -c -fallow-argument-mismatch junk.f >& /dev/null
then
	echo 'Adding -fallow-argument-mismatch to Mumps fortran compiler flags'
	ADD_FCFLAGS='ADD_FCFLAGS=-fallow-argument-mismatch'
else
	ADD_FCFLAGS=''
fi
# -----------------------------------------------------------------------------
if [ "$build_type" == 'debug' ]
then
	debug_flags='--enable-debug --disable-shared'
else
	debug_flags=''
fi
# -----------------------------------------------------------------------------
echo_eval ./coinbrew build Ipopt@$version \
	--test \
	--no-prompt \
	--verbosity=4 \
	$debug_flags \
    --prefix=$ipopt_prefix \
	--libdir=$ipopt_prefix/$cmake_libdir \
	$ADD_FCFLAGS
# ----------------------------------------------------------------------------
echo 'install_ipopt.sh: OK'
exit 0
