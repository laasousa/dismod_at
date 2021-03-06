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
url_list=( \
	'https://github.com/coin-or-tools/ThirdParty-ASL' \
	'https://github.com/coin-or-tools/ThirdParty-Mumps' \
	'https://github.com/coin-or/Ipopt' \
)
version_list=( \
	'stable/2.0' \
	'stable/2.1' \
	'releases/3.13.2' \
)
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
# do work in build/external
if [ ! -e build/external ]
then
	mkdir -p build/external
fi
cd build/external
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
	add_mumps_fcflags='ADD_FCFLAGS=-fallow-argument-mismatch'
else
	add_mumps_fcflags=''
fi
# -----------------------------------------------------------------------------
for i in {0..2}
do
	url=${url_list[$i]}
	version=${version_list[$i]}
	name=$(echo $url | sed -e 's|.*/||' -e 's|ThirdParty-||')
	if [ ! -e $name.git ]
	then
		echo_eval git clone $url.git $name.git
	fi
	echo_eval cd $name.git
	if [ -e "get.$name" ] && [ ! -e "get.$name.done" ]
	then
		echo_eval ./get.$name
		touch ./get.$name.done
	fi
	echo_eval git checkout --quiet $version
	if [ "$name" == 'Ipopt' ]
	then
		patch='IpIpoptApplication'
		if [ ! -e "$patch.sed" ]
			then
cat << EOF > $patch.sed
# -----------------------------------------------------------------------
# Kluge: Add reference counter for argument to SetJournalist
/^static.*dbg_verbosity *= /! b one
s|\$|\\
static SmartPtr<Journalist> smart_jnlst(NULL);|
b end
# -----------------------------------------------------------------------
: one
# Kludge: If this is the first time SetJournalList gets called
# then the raw pointer gets saved and used. You have to read SetJourlist
# and know this is the only call to SetJournalist to know this works.
/DebugJournalistWrapper::SetJournalist(/! b end
s|$|\\
        if( GetRawPtr(smart_jnlst) == NULL )\\
            smart_jnlst = jnlst_;|
# -----------------------------------------------------------------------
: end
EOF
			echo_eval sed -i src/Interfaces/$patch.cpp -f $patch.sed
		fi
	fi
	if [ ! -e build ]
	then
		echo_eval mkdir build
	fi
	echo_eval cd build
	#
	if [ "$build_type" == 'debug' ]
	then
		debug_flags='--enable-debug --disable-shared'
		if [ "$name" == 'Ipopt' ]
		then
			debug_flags="$debug_flags --with-ipopt-verbosity"
		fi
	else
		debug_flags=''
	fi
	if [ "$name" == 'Mumps' ]
	then
		add_fcflags="$add_mumps_fcflags"
	else
		add_fcflags=''
	fi
	../configure \
		--disable-dependency-tracking \
		--prefix=$ipopt_prefix \
		--libdir=$ipopt_prefix/$cmake_libdir \
		$debug_flags \
		$add_fcflags
	echo_eval make install
    #
    # back to build/external
    echo_eval cd ../..
done
# ----------------------------------------------------------------------------
echo 'install_ipopt.sh: OK'
exit 0
