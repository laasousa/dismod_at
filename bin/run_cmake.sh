# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
# 
# This program is distributed under the terms of the 
# 	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
verbose='0'
if [ ! -e build ]
then
	mkdir build
fi
cd build
cmake \
	-D CMAKE_VERBOSE_MAKEFILE=$verbose \
	-D extra_cxx_flags='-std=c++11 -Wall -g' \
	-D cppad_prefix=$HOME/prefix/cppad \
	-D ipopt_prefix=$HOME/prefix/ipopt \
	-D suitesparse_prefix=$HOME/prefix/suitesparse \
	..
