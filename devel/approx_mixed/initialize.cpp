// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/approx_mixed.hpp>

/*
$begin approx_mixed_initialize$$
$spell
	vec
	const
	Cpp
	var
	hes
$$

$section approx_mixed: Initialization After Constructor$$

$head Syntax$$
$icode%approx_object%.initialize(%fixed_vec%, %random_vec%)%$$

$head Purpose$$
Some of the $code approx_mixed$$ initialization requires calling the
derived class version of the
$cref/joint_density/approx_mixed_joint_density/$$ function.
Hence this initialization cannot be done until
after the $cref/derived constructor/approx_mixed_derived_ctor/$$ completes.

$head approx_object$$
We use $cref/approx_object/approx_mixed_derived_ctor/approx_object/$$
to denote an object of a class that is
derived from the $code approx_mixed$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which certain $code CppAD::ADFun$$
objects are recorded.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/approx_mixed/Random Effects, u/$$
vector $latex u$$ at which certain $code CppAD::ADFun$$
objects are recorded.

$head Member Variables$$
The following $code approx_mixed$$ member variables are
$code CppAD::ADFun$$ functions that are assumed to be empty when
$code initialize$$ is called; i.e.,
the corresponding $code size_var()$$ is zero.
They will contain the corresponding recordings when $code initialize$$ returns:
$list number$$
For $icode%k% = 0%, ... ,% 4%$$,
the member variable $codei%a%k%_joint_density_%$$ is the corresponding
$cref/joint_density_/approx_mixed_private/joint_density_/$$.
$lnext
The member variable
$cref/grad_ran_/approx_mixed_private/grad_ran_/$$.
$lnext
The member variable
$cref/hes_ran_/approx_mixed_private/hes_ran_/$$.
$lnext
For $icode%k% = 0%, ... ,% 2%$$,
the member variable $codei%laplace_%k%_%$$ is the $th k$$ order
$cref/Laplace approximation/approx_mixed_private/laplace_k_/$$.
$lnext
The member variable
$cref/hes_fix_/approx_mixed_private/hes_fix_/$$.
$lnext
The member variable
$cref/prior_density_/approx_mixed_private/prior_density_/$$.
$lend

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void approx_mixed::initialize(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{	if( grad_ran_.size_var() != 0 )
	{	std::cerr << "approx_mixed::initialize was called twice" << std::endl;
		exit(1);
	}
	//
	// joint_density_
	assert( a2_joint_density_.size_var() == 0 );
	assert( a1_joint_density_.size_var() == 0 );
	assert( a0_joint_density_.size_var() == 0 );
	record_joint(fixed_vec, random_vec);
	assert( a2_joint_density_.size_var() > 0 );
	assert( a1_joint_density_.size_var() > 0 );
	assert( a0_joint_density_.size_var() > 0 );
	//
	// grad_ran_
	assert( grad_ran_.size_var() == 0 );
	record_grad_ran(fixed_vec, random_vec);
	assert( grad_ran_.size_var() > 0 );
	//
	// hes_ran_
	assert( hes_ran_.size_var() == 0 );
	record_hes_ran(fixed_vec, random_vec);
	assert( hes_ran_.size_var() > 0 );
	//
	// laplace_0_
	assert( laplace_0_.size_var() == 0 );
	record_laplace(0, fixed_vec, random_vec);
	assert( laplace_0_.size_var() > 0 );
	//
	// laplace_1_
	assert( laplace_1_.size_var() == 0 );
	record_laplace(1, fixed_vec, random_vec);
	assert( laplace_1_.size_var() > 0 );
	//
	// laplace_0_
	assert( laplace_2_.size_var() == 0 );
	record_laplace(2, fixed_vec, random_vec);
	assert( laplace_2_.size_var() > 0 );
	//
	// hes_fix_
	assert( hes_fix_.size_var() == 0 );
	record_hes_fix(fixed_vec, random_vec);
	assert( hes_fix_.size_var() > 0 );
	//
	// prior_density_
	assert( prior_density_.size_var() == 0 );
	record_prior(fixed_vec);
	assert( prior_density_.size_var() > 0 );
}

} // END_DISMOD_AT_NAMESPACE
