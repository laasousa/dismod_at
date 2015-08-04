// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/approx_mixed.hpp>
/*
$begin approx_mixed_ran_like_hes$$
$spell
	hes
	vec
	const
	Cpp
	xam
$$

$section Hessian of Random Negative Log-Likelihood Random Effects$$

$head Syntax$$
$icode%approx_object%.ran_like_hes(
	%fixed_vec%, %random_vec%, %row_out%, %col_out%, %val_out%
)%$$

$head Purpose$$
This routine computes the Hessian of the
negative log of the random negative log-likelihood
$cref/f(theta, u)
	/approx_mixed_theory/
	Random Negative Log-Likelihood, f(theta, u)
/$$
with respect to the random effects vector $latex u$$; i.e.
$latex \[
	f_{uu}^{(2)} ( \theta, u )
\] $$

$head approx_object$$
We use $cref/approx_object/approx_mixed_derived_ctor/approx_object/$$
to denote an object of a class that is
derived from the $code approx_mixed$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/approx_mixed/Random Effects, u/$$
vector $latex u$$.

$head row_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %row_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same
as for a previous call to $code ran_like_hes$$.
If it's input size is zero,
upon return it contains the row indices for the Hessian elements
that are possibly non-zero.

$head col_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %col_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same as for
a previous call to $code ran_like_hes$$.
If it's input size is zero,
upon return it contains the column indices for the Hessian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).
Note that only the lower triangle of the Hessian is computed and hence
$codei%
	%col_out%[%k%] <= %row_out%[%k%]
%$$
for all $icode%k% = 0 , %...%, %row_out%.size()-1%$$

$head val_out$$
This argument has prototype
$codei%
	CppAD::vector<a1_double>& %val_out%
%$$
If the input size of this array is non-zero, it must have the same size
as for a previous call to $code ran_like_hes$$.
Upon return, it contains the value of the Hessian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).

$children%
	example/devel/approx_mixed/private/ran_like_hes_xam.cpp
%$$
$head Example$$
The file $cref ran_like_hes_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

// ----------------------------------------------------------------------------
// ran_like_hes
void approx_mixed::ran_like_hes(
	const a1d_vector&        fixed_vec   ,
	const a1d_vector&        random_vec  ,
	CppAD::vector<size_t>&   row_out     ,
	CppAD::vector<size_t>&   col_out     ,
	a1d_vector&              val_out     )
{	assert( record_hes_ran_done_ );
	assert( n_fixed_  == fixed_vec.size() );
	assert( n_random_ == random_vec.size() );

	// size of outputs
	size_t n_nonzero = hes_ran_row_.size();
	if( n_nonzero == 0 )
	{	// special case where Hessian is zero.
		assert( row_out.size() == 0 );
		assert( col_out.size() == 0 );
		assert( val_out.size() == 0 );
		return;
	}
	// check recording
	assert( hes_ran_col_.size() == n_nonzero );

	// make sure outputs have proper dimension
	assert( row_out.size() == col_out.size() );
	assert( row_out.size() == val_out.size() );

	// check if this is first call
	if( row_out.size() == 0 )
	{	row_out.resize(n_nonzero);
		col_out.resize(n_nonzero);
		val_out.resize(n_nonzero);
		for(size_t k = 0; k < n_nonzero; k++)
		{	assert( hes_ran_row_[k] >= n_fixed_ );
			assert( hes_ran_col_[k] >= n_fixed_ );
			row_out[k] = hes_ran_row_[k] - n_fixed_;
			col_out[k] = hes_ran_col_[k] - n_fixed_;
		}
	}

	// create an a1d_vector containing (theta, u)
	a1d_vector a1_both_vec( n_fixed_ + n_random_ );
	pack(fixed_vec, random_vec, a1_both_vec);

	// create an a1d weight vector
	a1d_vector a1_w(1);
	a1_w[0] = 1.0;

	// First call to SparseHessian si suring record_hes_ran
	CppAD::vector< std::set<size_t> > not_used(0);

	// compute the sparse Hessian
	a1_ran_like_.SparseHessian(
		a1_both_vec,
		a1_w,
		not_used,
		hes_ran_row_,
		hes_ran_col_,
		val_out,
		hes_ran_work_
	);
# ifndef NDEUBG
	for(size_t k = 0; k < n_nonzero; k++)
	{	assert( row_out[k] + n_fixed_ == hes_ran_row_[k] );
		assert( col_out[k] + n_fixed_ == hes_ran_col_[k] );
	}
# endif
}


} // END_DISMOD_AT_NAMESPACE
