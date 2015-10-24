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
# include <dismod_at/configure.hpp>

/*
$begin mixed_cppad_record_hes_ran$$
$spell
	cppad
	hes hes
	vec
	const
	Cpp
	logdet
$$

$section Record Hessian of Random Negative Log-Likelihood w.r.t Random Effects$$

$head Syntax$$
$codei%record_hes_ran(%fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code mixed_cppad$$ class
and cannot be used by a derived
$cref/mixed_object/mixed_cppad_derived_ctor/mixed_object/$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/mixed_cppad/Random Effects, u/$$
vector $latex u$$ at which the recording is made.

$head hes_ran_row_, hes_ran_col_$$
The input value of the member variables
$codei%
	CppAD::vector<size_t> hes_ran_row_, hes_ran_col_
%$$
do not matter.
Upon return they contain the row indices and column indices
for the sparse Hessian;
see call to $code SparseHessian$$ below.
These indices are relative to both the fixed and random effects
with the fixed effects coming first.

$subhead Random Effects Index$$
To get the indices relative to just the random effects, subtract
$code n_fixed_$$; i.e.,
$codei%
	hes_ran_row_[%k%] - n_fixed_
	hes_ran_col_[%k%] - n_fixed_
%$$
are between zero and the $code n_random_$$ and
are the row and column indices for the Hessian element
that corresponds to the $th k$$ component of
$icode a1_val_out$$ in the call to $code SparseHessian$$ below.

$subhead Lower Triangle$$
The result are only for the lower triangle of the Hessian; i.e.,
$codei%
	hes_ran_row_[%k%] >= hes_ran_col_[%k%]
%$$

$subhead Order$$
The results are in column major order; i.e.,
$codei%
	hes_ran_col_[%k%] <= hes_ran_col_[%k+1%]
	if( hes_ran_col_[%k%] == hes_ran_col_[%k+1%] )
		hes_ran_row_[%k%] < hes_ran_row_[%k+1%]
%$$

$head hes_ran_0_$$
The input value of the member variables
$codei%
	CppAD::ADFun<double> hes_ran_0_
%$$
does not matter.
Upon return its zero order forward mode computes
the lower triangle of the sparse Hessian
$latex \[
	f_{uu}^{(2)} ( \theta , u )
\]$$

$head hes_ran_work_$$
The input value of the member variable
$codei%
	CppAD::sparse_hessian_work hes_ran_work_
%$$
does not matter.
Upon return it contains the necessary information so that
$codei%
	a1_ran_like_.SparseHessian(
		%a1_both_vec%,
		%a1_w%,
		%not_used%,
		hes_ran_row_,
		hes_ran_col_,
		%a1_val_out%,
		hes_ran_work_
	);
%$$
can be used to calculate the lower triangle of the sparse Hessian
$latex \[
	f_{uu}^{(2)} ( \theta , u )
\]$$
see $cref/f(theta, u)/
	mixed_cppad_theory/
	Random Negative Log-Likelihood, f(theta, u)
/$$.
Note that the matrix is symmetric and hence can be recovered from
its lower triangle.


$contents%devel/mixed_cppad/newton_step.cpp
	%example/devel/mixed_cppad/private/hes_ran_0_xam.cpp
%$$

$end
*/


namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void mixed_cppad::record_hes_ran(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{	assert( ! record_hes_ran_done_ );
	assert( fixed_vec.size() == n_fixed_ );
	assert( random_vec.size() == n_random_ );
	size_t i, j;

	// total number of variables
	size_t n_total = n_fixed_ + n_random_;

	// create a d_vector containing (theta, u)
	d_vector both(n_total);
	pack(fixed_vec, random_vec, both);

	// compute Jacobian sparsity corresponding to parital w.r.t. random effects
# if MIXED_CPPAD_SET_SPARSITY
	typedef CppAD::vector< std::set<size_t> > sparsity_pattern;
	sparsity_pattern r(n_total);
	for(i = n_fixed_; i < n_total; i++)
		r[i].insert(i);
# else
	typedef CppAD::vectorBool sparsity_pattern;
	sparsity_pattern r(n_total * n_total);
	for(i = 0; i < n_total; i++)
	{	for(j = 0; j < n_total; j++)
			r[i * n_total + j] = (i >= n_fixed_) && (i == j);
	}
# endif
	a0_ran_like_.ForSparseJac(n_total, r);

	// compute sparsity pattern corresponding to paritls w.r.t. (theta, u)
	// of partial w.r.t. u of f(theta, u)
	bool transpose = true;
	sparsity_pattern s(1), pattern;
# if MIXED_CPPAD_SET_SPARSITY
	assert( s[0].empty() );
	s[0].insert(0);
# else
	s[0] = true;
# endif
	pattern = a0_ran_like_.RevSparseHes(n_total, s, transpose);


	// determine row and column indices in lower triangle of Hessian
	// and set key for column major sorting
	CppAD::vector<size_t> row, col, key;
# if MIXED_CPPAD_SET_SPARSITY
	std::set<size_t>::iterator itr;
	for(i = n_fixed_; i < n_total; i++)
	{	for(itr = pattern[i].begin(); itr != pattern[i].end(); itr++)
		{	j = *itr;
			assert( j >= n_fixed_ );
			// only compute lower triangular part of Hessian w.r.t u only
			if( i >= j )
			{	row.push_back(i);
				col.push_back(j);
				key.push_back( i + j * n_total );
			}
		}
	}
# else
	for(i = n_fixed_; i < n_total; i++)
	{	for(j = n_fixed_; j < n_total; j++)
		{	if( pattern[i * n_total + j] && i >= j )
			{	// only compute lower triangular of Hessian w.r.t u only
				if( i >= j )
				{	row.push_back(i);
					col.push_back(j);
					key.push_back( i + j * n_total );
				}
			}
		}
	}
# endif
	// set hes_ran_row_ and hes_ran_col_ in colum major order
	size_t K = row.size();
	CppAD::vector<size_t> ind(K);
	CppAD::index_sort(key, ind);
	hes_ran_row_.resize(K);
	hes_ran_col_.resize(K);
	for(size_t k = 0; k < row.size(); k++)
	{	hes_ran_row_[k] = row[ ind[k] ];
		hes_ran_col_[k] = col[ ind[k] ];
	}

	// create a weighting vector
	d_vector w(1);
	w[0] = 1.0;

	// place where results go (not usd here)
	d_vector val_out(K);

	// compute the work vector
	a0_ran_like_.SparseHessian(
		both,
		w,
		pattern,
		hes_ran_row_,
		hes_ran_col_,
		val_out,
		hes_ran_work_
	);

	// now tape the same computation and store in hes_ran_0_
	CppAD::vector< std::set<size_t> > not_used(0);
	a1d_vector a1_both(n_total), a1_w(1), a1_val_out(K);
	for(size_t i = 0; i < n_total; i++)
		a1_both[i] = both[i];
	a1_w[0] = w[0];
	CppAD::Independent(a1_both);
	a1_ran_like_.SparseHessian(
		a1_both,
		a1_w,
		not_used,
		hes_ran_row_,
		hes_ran_col_,
		a1_val_out,
		hes_ran_work_
	);
	hes_ran_0_.Dependent(a1_both, a1_val_out);
	//
	record_hes_ran_done_ = true;
}


} // END_DISMOD_AT_NAMESPACE
