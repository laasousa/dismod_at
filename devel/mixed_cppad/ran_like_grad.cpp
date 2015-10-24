// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/mixed_cppad.hpp>
/*
$begin mixed_cppad_ran_like_grad$$
$spell
	cppad
	vec
	const
	Cpp
	xam
$$

$section Gradient of Random Negative Log-Likelihood With Respect to Random Effects$$

$head Syntax$$
$icode%grad% = %mixed_object%.ran_like_grad( %fixed_vec%, %random_vec%)%$$

$head Purpose$$
This routine computes the gradient of the negative log of the random negative log-likelihood
$cref/f(theta, u)/mixed_cppad_theory/Random Negative Log-Likelihood, f(theta, u)/$$
with respect to the random effects vector $latex u$$; i.e.
$latex \[
	f_u^{(1)} ( \theta, u )
\] $$

$head mixed_object$$
We use $cref/mixed_object/mixed_cppad_derived_ctor/mixed_object/$$
to denote an object of a class that is
derived from the $code mixed_cppad$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$
vector $latex \theta$$.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/mixed_cppad/Random Effects, u/$$
vector $latex u$$.

$head grad$$
The return value has prototype
$codei%
	CppAD::vector<a1_double>& %grad%
%$$
It contains the gradient $latex f_u^{(1)} ( \theta , u )$$.

$children%
	example/devel/mixed_cppad/private/ran_like_grad_xam.cpp
%$$
$head Example$$
The file $cref ran_like_grad_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

// ----------------------------------------------------------------------------
// ran_like_grad
CppAD::vector<mixed_cppad::a1_double> mixed_cppad::ran_like_grad(
	const a1d_vector&        fixed_vec   ,
	const a1d_vector&        random_vec  )
{	assert( record_ran_like_done_ );

	// number of fixed and random effects
	assert( n_fixed_  == fixed_vec.size() );
	assert( n_random_ == random_vec.size() );

	// create an a1d_vector containing (theta, u)
	a1d_vector both_vec( n_fixed_ + n_random_ );
	pack(fixed_vec, random_vec, both_vec);

	// zero order forward mode
	a1_ran_like_.Forward(0, both_vec);

	// first order reverse f_{theta,u}^{(1) ( theta , u )
	assert( a1_ran_like_.Range() == 1);
	a1d_vector a1_w(1), grad_both(n_fixed_ + n_random_);
	a1_w[0] = 1.0;
	grad_both = a1_ran_like_.Reverse(1, a1_w);

	// extract u part of the gradient
	a1d_vector grad_ran(n_random_);
	for(size_t j = 0; j < n_random_; j++)
		grad_ran[j] = grad_both[n_fixed_ + j];
	//
	return grad_ran;
}


} // END_DISMOD_AT_NAMESPACE
