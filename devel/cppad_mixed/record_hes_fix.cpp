// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/cppad_mixed.hpp>

/*
$begin cppad_mixed_record_hes_fix$$
$spell
	cppad
	obj
	hes
	vec
	const
	Cpp
$$

$section Record Hessian of Random Likelihood w.r.t Fixed Effects$$

$head Syntax$$
$codei%record_hes_fix(%fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code cppad_mixed$$ class
and cannot be used by a derived
$cref/mixed_object/cppad_mixed_derived_ctor/mixed_object/$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/cppad_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the initial value for the
$cref/random effects/cppad_mixed/Random Effects, u/$$ optimization.


$head hes_fix_row_, hes_fix_col_$$
The input value of the member variables
$codei%
	CppAD::vector<size_t> hes_fix_row_, hes_fix_col_
%$$
do not matter.
Upon return the contain the row indices and column indices
for the sparse Hessian represented by $code hes_fix_$$; i.e.
$codei%hes_fix_row_[%i%]%$$ and $codei%hes_fix_col_[%i%]%$$
are the row and column indices for the Hessian element
that corresponds to the $th i$$ component of the function
corresponding to $code hes_fix_$$.

$head hes_fix_work_$$
The input value of the member variable
$codei%
	CppAD::sparse_hessian_work hes_fix_work_
%$$
does not matter.
Upon return it contains the necessary information so that
$codei%
	ran_obj_2_.SparseHessian(
		%beta_theta_u%,
		%w%,
		%not_used%,
		hes_fix_row_,
		hes_fix_col_,
		%val_out%,
		hes_fix_work_
	);
%$$
can be used to calculate the lower triangle of the sparse Hessian
$latex \[
	H_{\beta \beta}^{(2)} ( \beta, \theta , u )
\] $$
see
$cref/H(beta, theta, u)
	/cppad_mixed_theory
	/Differentiating Random Part of Objective
	/Approximate Random Objective, H(beta, theta, u)
/$$.
Note that the matrix is symmetric and hence can be recovered from
its lower triangle.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void cppad_mixed::record_hes_fix(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{	assert( ! record_hes_fix_done_ );
	assert( record_ran_obj_done_[2] );
	size_t i, j;

	// total number of variables in H
	size_t n_total = 2 * n_fixed_ + n_random_;

	//	create an a1d_vector containing (theta, theta , u)
	d_vector beta_theta_u(n_total);
	pack(fixed_vec, fixed_vec, random_vec, beta_theta_u);


	// compute Jacobian sparsity corresponding to partial w.r.t beta
	// of H(beta, beta, u)
	typedef CppAD::vector< std::set<size_t> > sparsity_pattern;
	sparsity_pattern r(n_total);
	for(i = 0; i < n_fixed_; i++)
		r[i].insert(i);
	ran_obj_2_.ForSparseJac(n_fixed_, r);

	// compute sparsity pattern corresponding to partial w.r.t (beta, theta, u)
	// of parital w.r.t beta of H(beta, theta, u)
	sparsity_pattern s(1);
	assert( s[0].empty() );
	s[0].insert(0);
	bool transpose = true;
	sparsity_pattern pattern =
		ran_obj_2_.RevSparseHes(n_fixed_, s, transpose);

	// determine row and column indices in lower triangle of Hessian
	hes_fix_row_.clear();
	hes_fix_col_.clear();
	std::set<size_t>::iterator itr;
	for(i = 0; i < n_fixed_; i++)
	{	for(
			itr  = pattern[i].begin();
			itr != pattern[i].end();
			itr++
		)
		{	j = *itr;
			// only compute lower triangular part
			if( i >= j )
			{	hes_fix_row_.push_back(i);
				hes_fix_col_.push_back(j);
			}
		}
	}

	// create a weighting vector
	d_vector w(1);
	w[0] = 1.0;

	// place where results go (not used here)
	d_vector val_out( hes_fix_row_.size() );

	// compute the work vector
	ran_obj_2_.SparseHessian(
		beta_theta_u,
		w,
		pattern,
		hes_fix_row_,
		hes_fix_col_,
		val_out,
		hes_fix_work_
	);
	//
	record_hes_fix_done_ = true;
}

} // END_DISMOD_AT_NAMESPACE