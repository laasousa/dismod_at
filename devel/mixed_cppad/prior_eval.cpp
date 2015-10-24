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
$begin mixed_cppad_prior_eval$$
$spell
	cppad
	eval
	vec
	const
	Cpp
$$

$section mixed_cppad: Evaluate Prior for Fixed Effects$$

$head Syntax$$
$icode%vec% = %mixed_object%.prior_eval(%fixed_vec%)%$$

$head mixed_object$$
We use $cref/mixed_object/mixed_cppad_derived_ctor/mixed_object/$$
to denote an object of a class that is
derived from the $code mixed_cppad$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$
vector $latex \theta$$ at which $latex g( \theta )$$ is evaluated.

$head vec$$
The return value has prototype
$codei%
	CppAD::vector<double> %vec%
%$$
and is a
$cref/negative log-density vector/mixed_cppad/Negative Log-Density Vector/$$
corresponding to the prior for the fixed effects
$cref/g(theta)/mixed_cppad_theory/Fixed Negative Log-Likelihood, g(theta)/$$.
To be specific;
$pre
	$$
$latex g( \theta ) = $$
$icode%vec%[0] + CppAD::abs(%vec%[1]) + %...% CppAD::abs(%vec%[%s%-1])
%$$
where $icode%s% = %vec%.size()%$$.

$children%
	example/devel/mixed_cppad/private/prior_eval_xam.cpp
%$$
$head Example$$
The file $cref prior_eval_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

CppAD::vector<double> mixed_cppad::prior_eval(const d_vector& fixed_vec)
{	assert( record_fix_like_done_ );
	if( fix_like_.size_var() == 0 )
	{	// empty vector case
		return CppAD::vector<double>(0);
	}
	assert( fix_like_.Domain() == n_fixed_ );
	return fix_like_.Forward(0, fixed_vec);
}


} // END_DISMOD_AT_NAMESPACE