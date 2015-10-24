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
$begin mixed_cppad_constraint_eval$$
$spell
	cppad
	eval
	vec
	const
	Cpp
$$

$section mixed_cppad: Evaluate Constraint Function$$

$head Syntax$$
$icode%c_vec% = %mixed_object%.constraint_eval(%fixed_vec%)%$$

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

$head c_vec$$
The return value has prototype
$codei%
	CppAD::vector<double> %c_vec%
%$$
and is the constraint function value
corresponding to the fixed effects; see
$cref/c_vec/mixed_cppad_constraint/c_vec/$$.

$children%
	example/devel/mixed_cppad/private/constraint_eval_xam.cpp
%$$
$head Example$$
The file $cref constraint_eval_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

CppAD::vector<double> mixed_cppad::constraint_eval(const d_vector& fixed_vec)
{
	// make sure initialize has been called
	if( ! initialize_done_ )
	{	std::string error_message =
		"mixed_cppad::initialize was not called before constraint_eval";
		fatal_error(error_message);
	}
	if( constraint_.size_var() == 0 )
	{	return CppAD::vector<double>(0); // empty vector
	}
	assert( constraint_.Domain() == n_fixed_ );
	return constraint_.Forward(0, fixed_vec);
}


} // END_DISMOD_AT_NAMESPACE