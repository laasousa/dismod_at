// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_MIXED_PACK_HPP
# define DISMOD_AT_MIXED_PACK_HPP
# include <dismod_at/mixed_cppad.hpp>
# include <dismod_at/a2_double.hpp>

/*
$begin mixed_cppad_pack$$
$spell
	cppad
	vec
	const
	Cpp
	dismod
	hpp
$$

$section mixed_cppad: Pack Fixed Effect and Random Effects Into One Vector$$

$head Syntax$$
$codei%pack(%fixed_one%, %random_vec%, %both_vec%)
%$$
$codei%pack(%fixed_one%, %fixed_two%, %random_vec%, %three_vec%)
%$$

$head Private$$
This function is $code private$$ to the $code mixed_cppad$$ class
and cannot be used by a derived
$cref/mixed_object/mixed_cppad_derived_ctor/mixed_object/$$.

$head Float_unpack$$
This can be any type.

$head Float_pack$$
If $icode x$$ has type $icode Float_unpack$$,
the syntax $icode%Float_pack%(%x%)%$$ must convert $icode x$$
to the type $icode Float_pack$$.

$head fixed_one$$
This argument has prototype
$codei%
	const CppAD::vector<%Float_unpack%>& %fixed_one%
%$$
It specifies the a value for the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$.
The size of this vector must be equal to
$cref/n_fixed_/mixed_cppad_private/n_fixed_/$$.

$head fixed_two$$
This argument has prototype
$codei%
	const CppAD::vector<%Float_unpack%>& %fixed_two%
%$$
If present, it also specifies the a value for the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$.
The size of this vector must be equal to
$cref/n_fixed_/mixed_cppad_private/n_fixed_/$$.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float_unpack%>& %random_vec%
%$$
It specifies a value for the
$cref/random effects/mixed_cppad/Random Effects, u/$$.
The size of this vector must be equal to
$cref/n_fixed_/mixed_cppad_private/n_random_/$$.

$head both_vec$$
This argument has prototype
$codei%
	CppAD::vector<%Float_pack%>& %both_vec%
%$$
If present, the size of this vector must be equal to
$icode%n_fixed_% + %n_random_%$$.
The input value of its elements does not matter.
Upon return, it contains the values in
$icode fixed_one$$ and $icode random_vec$$ as one vector in that order;
i.e., $icode fixed_one$$ comes first and then $icode random_vec$$.

$head three_vec$$
This argument has prototype
$codei%
	CppAD::vector<%Float_pack%>& %three_vec%
%$$
If present, the size of this vector must be equal to
$codei%2*%n_fixed_% + %n_random_%$$.
The input value of its elements does not matter.
Upon return, it contains the values in
$icode fixed_one$$, $icode fixed_two$$, and $icode random_vec$$ as one vector.
The order of the result is unspecified.

$end
*/
# include <dismod_at/mixed_cppad.hpp>
# include <dismod_at/a2_double.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

template <class Float_unpack, class Float_pack>
void mixed_cppad::pack(
	const CppAD::vector<Float_unpack>& fixed_one  ,
	const CppAD::vector<Float_unpack>& random_vec ,
	CppAD::vector<Float_pack>&         both_vec   ) const
{
	assert( fixed_one.size() == n_fixed_ );
	assert( random_vec.size() == n_random_ );
	assert( both_vec.size() == n_fixed_ + n_random_ );
	for(size_t j = 0; j < n_fixed_; j++)
		both_vec[j] = Float_pack( fixed_one[j] );
	for(size_t j = 0; j < n_random_; j++)
		both_vec[n_fixed_ + j] = Float_pack( random_vec[j] );
}
template <class Float_unpack, class Float_pack>
void mixed_cppad::pack(
	const CppAD::vector<Float_unpack>& fixed_one  ,
	const CppAD::vector<Float_unpack>& fixed_two  ,
	const CppAD::vector<Float_unpack>& random_vec ,
	CppAD::vector<Float_pack>&         three_vec  ) const
{
	assert( fixed_one.size() == n_fixed_ );
	assert( fixed_two.size() == n_fixed_ );
	assert( random_vec.size() == n_random_ );
	assert( three_vec.size() == 2 * n_fixed_ + n_random_ );
	for(size_t j = 0; j < n_fixed_; j++)
	{	three_vec[j] = Float_pack( fixed_one[j] );
		three_vec[n_fixed_ + j] = Float_pack( fixed_two[j] );
	}
	for(size_t j = 0; j < n_random_; j++)
		three_vec[2 * n_fixed_ + j] = Float_pack( random_vec[j] );
}


} // END_DISMOD_AT_NAMESPACE

# endif
