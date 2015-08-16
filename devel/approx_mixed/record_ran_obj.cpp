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
$begin approx_mixed_record_ran_obj$$
$spell
	obj
	vec
	const
	Cpp
$$

$section Record Approximation for Random Part of Objective$$

$head Syntax$$
$codei%record_ran_obj(%order%, %fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code approx_mixed$$ class
and cannot be used by a derived
$cref/approx_object/approx_mixed_derived_ctor/approx_object/$$.

$head order$$
This argument has prototype
$icode%
	size_t %order%
%$$
It specifies the order of accuracy when approximating
$cref/u^(theta)/approx_mixed_theory/Objective/u^(theta)/$$
and must be zero, one, or two.
The zero order recording is shortest, but can only compute
$cref/r(theta)
	/approx_mixed_theory
	/Objective
	/Random Part of Objective, r(theta)
/$$.
The first order recording is longer, and can compute
both $latex r( \theta )$$ and its derivative.
The second order recording is longest, and can compute
$latex r( \theta )$$, and its derivative, and its Hessian.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/approx_mixed/Random Effects, u/$$
vector $latex u$$ at which the recording is made.

$head ran_obj_order_$$
The input value of the member variable
$codei%
	CppAD::ADFun<double> ran_obj_%order%_
%$$
does not matter.
Upon return it contains a an recording for the
approximate random part of the objective; see
$cref/H(beta, theta, u)
	/approx_mixed_theory
	/Differentiating Random Part of Objective
	/Approximate Random Objective, H(beta, theta, u)
/$$.

$end
*/
# include <Eigen/Sparse>
# include <dismod_at/approx_mixed.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void approx_mixed::record_ran_obj(
	size_t          order      ,
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{	assert( order <= 2 );
	assert( ! record_ran_obj_done_[order] );

	// declare eigen matrix types
	using Eigen::Dynamic;
	typedef Eigen::Matrix<a1_double, Dynamic, Dynamic> dense_matrix;
	typedef Eigen::SparseMatrix<a1_double>             sparse_matrix;

	//	create an a1d_vector containing (beta, theta, u)
	a1d_vector beta_theta_u( 2 * n_fixed_ + n_random_ );
	pack(fixed_vec, fixed_vec, random_vec, beta_theta_u);

	// start recording a1_double operations
	CppAD::Independent( beta_theta_u );

	// split back out to beta, theta, u
	a1d_vector beta(n_fixed_), theta(n_fixed_), u(n_random_);
	unpack(beta, theta, u, beta_theta_u);

	// evaluate the hessian f_{uu}^{(2)} (theta, u)
	a1d_vector both(n_fixed_ + n_random_), val_out( hes_ran_row_.size() );
	pack(theta, u, both);
	ran_like_hes(both, val_out);

	// create a lower triangular eigen sparse matrix representation of Hessian
	sparse_matrix hessian(n_random_, n_random_);
	size_t K = hes_ran_row_.size();
	for(size_t k = 0; k < K; k++)
	{	assert( n_fixed_        <= hes_ran_col_[k]  );
		assert( hes_ran_col_[k] <= hes_ran_row_[k] );
		size_t row = hes_ran_row_[k] - n_fixed_;
		size_t col = hes_ran_col_[k] - n_fixed_;
		hessian.insert(row, col) = val_out[k];
	}

	// compute an LDL^T Cholesky factorization of f_{uu}^{(2)}(theta, u)
	Eigen::SimplicialLDLT<sparse_matrix, Eigen::Lower> chol;
	chol.analyzePattern(hessian);
	chol.factorize(hessian);

	// First order section: Compute U -----------------------------------------
	a1d_vector U(n_random_), grad(n_random_);
	dense_matrix rhs(n_random_, 1), newton_step(n_random_, 1);
	if( order < 1 )
	{	for(size_t j = 0; j < n_random_; j++)
			U[j] = u[j];
	}
	else
	{	// evaluate gradient f_u^{(1)} (beta , u )
		grad = ran_like_grad(beta, u);

		// newton_step = f_{uu}^{(2)} (theta , u)^{-1} f_u^{(1)} (beta, u)
		for(size_t j = 0; j < n_random_; j++)
			rhs(j) = grad[j];
		newton_step = chol.solve(rhs);

		// compute U(beta, theta, u)
		for(size_t j = 0; j < n_random_; j++)
			U[j] = u[j] - newton_step(j);

		// evaluate hessian f_{uu}^{(2)}(beta, U) and compute its factorization
		pack(beta, U, both);
		ran_like_hes(both, val_out);
		for(size_t k = 0; k < K; k++)
		{	size_t row = hes_ran_row_[k] - n_fixed_;
			size_t col = hes_ran_col_[k] - n_fixed_;
			hessian.coeffRef(row, col) = val_out[k];
		}
		chol.factorize(hessian);
	}
	// Second order section: Compute W ----------------------------------------
	a1d_vector W(n_random_);
	if( order < 2 )
	{	for(size_t j = 0; j < n_random_; j++)
			W[j] = U[j];
	}
	else
	{	// evaluate gradient f_u^{(1)} (beta , U )
		grad = ran_like_grad(beta, U);

		// newton_step = f_{uu}^{(2)} (beta , U)^{-1} f_u^{(1)} (beta, U)
		for(size_t j = 0; j < n_random_; j++)
			rhs(j) = grad[j];
		newton_step = chol.solve(rhs);

		// compute W(beta, theta, u)
		for(size_t j = 0; j < n_random_; j++)
			W[j] = U[j] - newton_step(j);

		// evaluate hessian f_{uu}^{(2)}(beta, W) and compute its factorization
		pack(beta, W, both);
		ran_like_hes(both, val_out);
		for(size_t k = 0; k < K; k++)
		{	size_t row = hes_ran_row_[k] - n_fixed_;
			size_t col = hes_ran_col_[k] - n_fixed_;
			hessian.coeffRef(row, col) = val_out[k];
		}
		chol.factorize(hessian);
	}
	// Back to all orders ----------------------------------------------------
	//
	// logdet f_{uu}^{(2)} (beta, W)]
	dense_matrix diag = chol.vectorD();
	a1_double logdet = a1_double(0.0);
	for(size_t j = 0; j < n_random_; j++)
		logdet += log( diag(j) );

	// f[beta, W(beta, theta, u)]
	pack(beta, W, both);
	a1d_vector vec = a1_ran_like_.Forward(0, both);
	a1_double sum = vec[0];
	size_t n_abs = vec.size() - 1;
	for(size_t i = 0; i < n_abs; i++)
		sum += abs( vec[1 + i] );

	// constant term
	double pi   = CppAD::atan(1.0) * 4.0;
	double constant_term = CppAD::log(2.0 * pi) * double(n_random_) / 2.0;

	// H(beta, theta, u)
	a1d_vector H(1);
	H[0] = logdet / 2.0 + sum - constant_term;

	// complete recording of H(beta, theta, u)
	if( order == 0 )
	{	ran_obj_0_.Dependent(beta_theta_u, H);
		ran_obj_0_.optimize();
	}
	else if( order == 1 )
	{	ran_obj_1_.Dependent(beta_theta_u, H);
		ran_obj_1_.optimize();
	}
	else
	{	assert(order == 2 );
		ran_obj_2_.Dependent(beta_theta_u, H);
		ran_obj_2_.optimize();
	}
	record_ran_obj_done_[order] = true;
}

} // END_DISMOD_AT_NAMESPACE
