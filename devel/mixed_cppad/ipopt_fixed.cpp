// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/configure.hpp>
# include <dismod_at/ipopt_fixed.hpp>


namespace {

# if MIXED_CPPAD_NEWTON
	// merge two (row, col) sparsity patterns into one
	void merge_sparse(
		const CppAD::vector<size_t>& row_one      , // first sparsity pattern
		const CppAD::vector<size_t>& col_one      ,
		const CppAD::vector<size_t>& row_two      , // second sparsity pattern
		const CppAD::vector<size_t>& col_two      ,
		const CppAD::vector<size_t>& row_three    , // third sparsity pattern
		const CppAD::vector<size_t>& col_three    ,
		CppAD::vector<size_t>&       row_out      , // merged sparsity pattern
		CppAD::vector<size_t>&       col_out      ,
		CppAD::vector<size_t>&       one_2_out    , // maps first into merged
		CppAD::vector<size_t>&       two_2_out    , // maps second into merged
		CppAD::vector<size_t>&       three_2_out  ) // maps third into merged
	{	assert( row_out.size() == 0 );
		assert( col_out.size() == 0 );
		//
		assert( row_one.size() == col_one.size() );
		assert( row_one.size() == one_2_out.size() );
		//
		assert( row_two.size() == col_two.size() );
		assert( row_two.size() == two_2_out.size() );
		//
		assert( row_three.size() == col_three.size() );
		assert( row_three.size() == three_2_out.size() );
		//
		size_t n_one   = row_one.size();
		size_t n_two   = row_two.size();
		size_t n_three = row_three.size();
		//
		// compute maximum column index
		size_t max_col = 0;
		for(size_t k = 0; k < n_one; k++)
			max_col = std::max( col_one[k], max_col );
		for(size_t k = 0; k < n_two; k++)
			max_col = std::max( col_two[k], max_col );
		for(size_t k = 0; k < n_three; k++)
			max_col = std::max( col_three[k], max_col );
		//
		// keys for sorting and maximum key value
		CppAD::vector<size_t>
			key_one(n_one), key_two(n_two), key_three(n_three);
		size_t key_max = 0;
		for(size_t k = 0; k < n_one; k++)
		{	key_one[k] = row_one[k] * max_col + col_one[k];
			key_max    = std::max(key_max, key_one[k]);
		}
		for(size_t k = 0; k < n_two; k++)
		{	key_two[k] = row_two[k] * max_col + col_two[k];
			key_max    = std::max(key_max, key_two[k]);
		}
		for(size_t k = 0; k < n_three; k++)
		{	key_three[k] = row_three[k] * max_col + col_three[k];
			key_max      = std::max(key_max, key_three[k]);
		}
		//
		// sort all three
		CppAD::vector<size_t>
			ind_one(n_one), ind_two(n_two), ind_three(n_three);
		CppAD::index_sort(key_one,   ind_one);
		CppAD::index_sort(key_two,   ind_two);
		CppAD::index_sort(key_three, ind_three);
		//
		// now merge into row_out and col_out
		size_t k_one = 0, k_two = 0, k_three = 0;
		while( k_one < n_one || k_two < n_two || k_three < n_three )
		{	size_t key_next = key_max + 1;
			size_t n_out    = row_out.size();
			if( k_one < n_one )
				key_next = std::min(key_next, key_one[k_one]);
			if( k_two < n_two )
				key_next = std::min(key_next, key_two[k_two]);
			if( k_three < n_three )
				key_next = std::min(key_next, key_three[k_three]);
			assert( key_next <= key_max );
			//
			size_t found = false;
			if( k_one < n_one && key_one[k_one] == key_next )
			{	found = true;
				row_out.push_back( row_one[k_one] );
				col_out.push_back( col_one[k_one] );
				//
				one_2_out[k_one] = n_out;
				k_one++;
			}
			if( k_two < n_two && key_two[k_two] == key_next )
			{	if( found )
				{	assert( row_two[k_two] == row_out[n_out] );
					assert( col_two[k_two] == col_out[n_out] );
					two_2_out[k_two] =n_out;
				}
				else
				{	found = true;
					row_out.push_back( row_two[k_two] );
					col_out.push_back( col_two[k_two] );
					two_2_out[k_two] = n_out;
				}
				k_two++;
			}
			if( k_three < n_three && key_three[k_three] == key_next )
			{	if( found )
				{	assert( row_three[k_three] == row_out[n_out] );
					assert( col_three[k_three] == col_out[n_out] );
					three_2_out[k_three] = n_out;
				}
				else
				{	found = true;
					row_out.push_back( row_three[k_three] );
					col_out.push_back( col_three[k_three] );
					three_2_out[k_three] = n_out;
				}
				k_three++;
			}
			assert(found);
		}
		return;
	}
# endif // MIXED_CPPAD_NEWTON

	// --------------------------------------------------------------------
	bool check_in_limits(double lower, double x, double upper, double tol)
	{	bool flag = true;
		if( upper >= 0.0 )
			flag &= x <= (1.0 + tol) * upper;
		else
			flag &= x <= (1.0 - tol) * upper;
		//
		if( lower >= 0.0 )
			flag &= (1.0 - tol) * lower <= x;
		else
			flag &= (1.0 + tol) * lower <= x;
		//
		return flag;
	}
}

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE
/* $$
$begin ipopt_fixed_ctor$$
$spell
	cppad
	obj
	hes
	vec
	eval
	ipopt
	const
	CppAD
	nnz_jac
	Jacobian
	std
	tol
$$

$section Ipopt Example: Constructor and Destructor$$

$head Syntax$$
$codei%ipopt_fixed %ipopt_object%(
	%random_options%,
	%fixed_tolerance%,
	%fixed_lower%,
	%fixed_upper%,
	%constraint_lower%,
	%constraint_upper%,
	%fixed_in%,
	%random_in%,
	%mixed_object%
)%$$

$head Prototype$$
The arguments has prototypes
$codei%
	const std::string&           %random_options%
	const double&                %fixed_tolerance%
	const CppAD::vector<double>& %fixed_lower%
	const CppAD::vector<double>& %fixed_in%
	const CppAD::vector<double>& %fixed_upper%
	const CppAD::vector<double>& %random_in%
	mixed_cppad&                %mixed_object%
%$$

$head References$$
The values of the arguments are stored by reference and hence
the arguments must not be deleted while $icode ipopt_object$$
is still being used.

$head random_options$$
This argument has prototype
$codei%
	const std::string& %random_options%
%$$
and is the $cref ipopt_options$$ for optimizing the random effects.

$head fixed_tolerance$$
Is the relative convergence criteria used by Ipopt for optimize fixed effects.
This only informs ipopt_fixed,
the IpoptApplication must be informed separately using
$codei%
	%app%->Options()->SetNumericValue("tol", %fixed_tolerance%)
%$$

$head fixed_lower$$
specifies the lower limits for the
$cref/fixed_effects/model_variable/Fixed Effects, theta/$$.
Note that
$code%
	- std::numeric_limits<double>::infinity()
%$$
is used for minus infinity; i.e., no lower limit.

$head fixed_upper$$
specifies the upper limits for the fixed effects.
Note that
$code%
	std::numeric_limits<double>::infinity()
%$$
is used for plus infinity; i.e., no upper limit.

$head constraint_lower$$
specifies the lower limits for the
$cref/constraints/mixed_cppad_constraint/$$.
Note that
$code%
	- std::numeric_limits<double>::infinity()
%$$
is used for minus infinity; i.e., no lower limit.

$head constraint_upper$$
specifies the upper limits for the constraints.
Note that
$code%
	std::numeric_limits<double>::infinity()
%$$
is used for plus infinity; i.e., no upper limit.

$head fixed_in$$
specifies the initial value (during optimization) for the fixed effects.
It must hold for each $icode j$$ that
$codei%
	%fixed_lower%[%j%] <= %fixed_in%[%j%] <= %fixed_upper%[%j%]
%$$

$head random_in$$
specifies the initial value (for initial optimization) of the random effects.

$head mixed_object$$
The argument $icode mixed_object$$ is an object of a class that is
derived from the $code mixed_cppad$$ base class.

$head Non-Const Member Variables$$
The following member variables are set by the constructor
and should not be modified.

$subhead fix_like_n_abs_$$
number of absolute value terms in the
$cref/fix_like/mixed_cppad_fix_like/$$.

$head prior_nnz_jac_$$
number of non-zeros in the Jacobian of the fixed negative log-likelihood.

$head lag_hes_row_$$
The row indices for the sparse representation of the Hessian
of the Lagrangian (for any Lagrange multiplier values).

$head lag_hes_col_$$
The column indices for the sparse representation of the Hessian
of the Lagrangian (for any Lagrange multiplier values).


$end
*/
ipopt_fixed::ipopt_fixed(
	const std::string&  random_options     ,
	const double&       fixed_tolerance    ,
	const d_vector&     fixed_lower        ,
	const d_vector&     fixed_upper        ,
	const d_vector&     constraint_lower   ,
	const d_vector&     constraint_upper   ,
	const d_vector&     fixed_in           ,
	const d_vector&     random_in          ,
	mixed_cppad&       mixed_object      ) :
random_options_    ( random_options )          ,
fixed_tolerance_   ( fixed_tolerance  )        ,
n_fixed_           ( fixed_in.size()  )        ,
n_random_          ( random_in.size() )        ,
n_constraint_      ( constraint_lower.size() ) ,
fixed_lower_       ( fixed_lower      )        ,
fixed_upper_       ( fixed_upper      )        ,
constraint_lower_  ( constraint_lower      )   ,
constraint_upper_  ( constraint_upper      )   ,
fixed_in_          ( fixed_in         )        ,
random_in_         ( random_in        )        ,
mixed_object_     ( mixed_object    )
{
	//
	// -----------------------------------------------------------------------
	// set nlp_lower_bound_inf_, nlp_upper_bound_inf_
	// -----------------------------------------------------------------------
	nlp_lower_bound_inf_ = - 1e19;
	nlp_upper_bound_inf_ = + 1e19;
	double inf           = std::numeric_limits<double>::infinity();
	for(size_t j = 0; j < n_fixed_; j++)
	{	if( fixed_lower[j] != - inf ) nlp_lower_bound_inf_ =
				std::min(nlp_lower_bound_inf_, 1.1 * fixed_lower[j] );
		//
		if( fixed_upper[j] != inf ) nlp_upper_bound_inf_ =
				std::max(nlp_upper_bound_inf_, 1.1 * fixed_upper[j] );
	}
	for(size_t j = 0; j < n_constraint_; j++)
	{	if( constraint_lower[j] != - inf ) nlp_lower_bound_inf_ =
				std::min(nlp_lower_bound_inf_, 1.1 * constraint_lower[j] );
		//
		if( constraint_upper[j] != inf ) nlp_upper_bound_inf_ =
				std::max(nlp_upper_bound_inf_, 1.1 * constraint_upper[j] );
	}
	// -----------------------------------------------------------------------
	// set fix_like_n_abs_
	// -----------------------------------------------------------------------
	// fixed negative log-likelihood at the initial fixed effects vector
	d_vector fix_like_vec = mixed_object_.prior_eval(fixed_in);
	if( fix_like_vec.size() == 0 )
		fix_like_n_abs_ = 0;
	else
		fix_like_n_abs_ = fix_like_vec.size() - 1;
	// -----------------------------------------------------------------------
	// set fix_like_jac_row_, fix_like_jac_col_, fix_like_jac_val_
	// constraint_jac_row_, constraint_jac_col_, constraint_jac_val_
	// -----------------------------------------------------------------------
	mixed_object.fix_like_jac(
		fixed_in, fix_like_jac_row_, fix_like_jac_col_, fix_like_jac_val_
	);
	mixed_object.constraint_jac(
		fixed_in, constraint_jac_row_, constraint_jac_col_, constraint_jac_val_
	);
	// -----------------------------------------------------------------------
	// set nnz_jac_g_
	// -----------------------------------------------------------------------
	nnz_jac_g_ = 0;
	for(size_t k = 0; k < fix_like_jac_row_.size(); k++)
	{	if( fix_like_jac_row_[k] != 0 )
		{	 // this is an absolute value term
			nnz_jac_g_ += 2;
		}
	}
	// derivative w.r.t auxillary variables
	nnz_jac_g_ += 2 * fix_like_n_abs_;
	// derivative of the constraints
	nnz_jac_g_ += constraint_jac_row_.size();
	// -----------------------------------------------------------------------
	// set lag_hes_row_, lag_hes_col_, ran_obj_2_lag_, fix_like2lag_
	// -----------------------------------------------------------------------
# if ! MIXED_CPPAD_NEWTON
	nnz_h_lag_ = 0;
# else
	// row and column indices for contribution from random part of objective
	if( n_random_ > 0 ) mixed_object.ran_obj_hes_fix(
		fixed_in, random_in,
		ran_obj_hes_row_, ran_obj_hes_col_, ran_obj_hes_val_
	);
	// row and column indices for contribution from prior
	d_vector weight( 1 + fix_like_n_abs_ );
	for(size_t i = 0; i < weight.size(); i++)
		weight[i] = 1.0;
	mixed_object.fix_like_hes(
		fixed_in, weight, fix_like_hes_row_, fix_like_hes_col_, fix_like_hes_val_
	);
	// row and column indices for contribution from constraint
	weight.resize( n_constraint_ );
	for(size_t i = 0; i < weight.size(); i++)
		weight[i] = 1.0;
	mixed_object.constraint_hes(
		fixed_in,
		weight,
		constraint_hes_row_,
		constraint_hes_col_,
		constraint_hes_val_
	);
	//
	// merge to form sparsity for Lagrangian
	ran_obj_2_lag_.resize( ran_obj_hes_row_.size() );
	fix_like2lag_.resize( fix_like_hes_row_.size() );
	constraint_2_lag_.resize( constraint_hes_row_.size() );
	merge_sparse(
		ran_obj_hes_row_      ,
		ran_obj_hes_col_      ,
		fix_like_hes_row_        ,
		fix_like_hes_col_        ,
		constraint_hes_row_   ,
		constraint_hes_col_   ,
		lag_hes_row_          ,
		lag_hes_col_          ,
		ran_obj_2_lag_        ,
		fix_like2lag_          ,
		constraint_2_lag_
	);
# ifndef NDEBUG
	for(size_t k = 0; k < ran_obj_hes_row_.size(); k++)
		assert( ran_obj_2_lag_[k] < lag_hes_row_.size() );
	//
	for(size_t k = 0; k < fix_like_hes_row_.size(); k++)
		assert( fix_like2lag_[k] < lag_hes_row_.size() );
	//
	for(size_t k = 0; k < constraint_hes_row_.size(); k++)
		assert( constraint_2_lag_[k] < lag_hes_row_.size() );
# endif
	// -----------------------------------------------------------------------
	// set nnz_h_lag_
	// -----------------------------------------------------------------------
	nnz_h_lag_ = lag_hes_row_.size();
	assert( nnz_h_lag_ == lag_hes_col_.size() );
# endif // MIXED_CPPAD_NEWTON
	// -----------------------------------------------------------------------
	// set size of temporary vectors
	// -----------------------------------------------------------------------
	fixed_tmp_.resize( n_fixed_ );
	random_tmp_.resize( n_random_ );
	c_vec_tmp_.resize( n_constraint_ );
	H_beta_tmp_.resize( n_fixed_ );
	w_fix_like_tmp_.resize( fix_like_n_abs_ + 1 );
	w_constraint_tmp_.resize( n_constraint_ );
	assert( fix_like_vec_tmp_.size() == 0 );
	if( fix_like_vec.size() > 0 )
		fix_like_vec_tmp_.resize( fix_like_n_abs_ + 1 );
	// -----------------------------------------------------------------------
}
ipopt_fixed::~ipopt_fixed(void)
{ }
/* $$
$end
------------------------------------------------------------------------------
$begin ipopt_fixed_get_nlp_info$$
$spell
	cppad
	obj
	ipopt_fixed_get_nlp_info
	nnz_jac
	Jacobian
	bool
	Enum
	bool
	nlp
$$

$section Return Information About Problem Sizes$$

$head Syntax$$
$icode%ok% = get_nlp_info(%n%, %m%, %nnz_jac_g%, %nnz_h_lag%, %index_style%)%$$

$head n$$
is set to the number of variables in the problem (dimension of x).

$head m$$
is set to the number of constraints in the problem (dimension of g(x)).

$head nnz_jac_g$$
is set to the number of nonzero entries in the Jacobian of g(x).

$head nnz_h_lag$$
is set to the number of nonzero entries in the Hessian of the Lagrangian
$latex f(x) + \lambda^\R{T} g(x)$$.

$head index_style$$
is set to the numbering style used for row/col entries in the sparse matrix
format (C_STYLE: 0-based, FORTRAN_STYLE: 1-based).

$head ok$$
If set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_nlp_info(
	Index&          n            ,  // out
	Index&          m            ,  // out
	Index&          nnz_jac_g    ,  // out
	Index&          nnz_h_lag    ,  // out
	IndexStyleEnum& index_style  )  // out
{
	n           = n_fixed_ + fix_like_n_abs_;
	m           = 2 * fix_like_n_abs_ + n_constraint_;
	nnz_jac_g   = nnz_jac_g_;
	nnz_h_lag   = nnz_h_lag_;
	index_style = C_STYLE;
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_get_bounds_info$$
$spell
	cppad
	obj
	ipopt
	bool
$$

$section Return Optimization Bounds$$

$head Syntax$$
$icode%ok% = get_bounds_info(%n%, %x_l%, %x_u%, %m%, %g_l%, %g_u%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x_l$$
set to the lower bounds for $icode x$$ (has size $icode n$$).

$head x_u$$
set to the upper bounds for $icode x$$ (has size $icode n$$).

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head g_l$$
set to the lower bounds for $icode g(x)$$ (has size $icode m$$).

$head g_u$$
set to the upper bounds for $icode g(x)$$ (has size $icode m$$).

$head ok$$
If set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_bounds_info(
		Index       n        ,   // in
		Number*     x_l      ,   // out
		Number*     x_u      ,   // out
		Index       m        ,   // in
		Number*     g_l      ,   // out
		Number*     g_u      )   // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );

	for(size_t j = 0; j < n_fixed_; j++)
	{	// map infinity to crazy value required by ipopt
		if( fixed_lower_[j] == - std::numeric_limits<double>::infinity() )
			x_l[j] = nlp_lower_bound_inf_;
		else
			x_l[j] = fixed_lower_[j];
		//
		if( fixed_upper_[j] == std::numeric_limits<double>::infinity() )
			x_u[j] = nlp_upper_bound_inf_;
		else
			x_u[j] = fixed_upper_[j];
	}
	// auxillary varibles for absolute value terms
	for(size_t j = 0; j < fix_like_n_abs_; j++)
	{	x_l[n_fixed_ + j] = nlp_lower_bound_inf_;
		x_u[n_fixed_ + j] = nlp_upper_bound_inf_;
	}
	//
	// constraints for absolute value terms
	for(size_t j = 0; j < 2 * fix_like_n_abs_; j++)
	{	g_l[j] = 0.0;
		g_u[j] = nlp_upper_bound_inf_;
	}
	//
	// explicit constraints
	for(size_t j = 0; j < n_constraint_; j++)
	{	g_l[2 * fix_like_n_abs_ + j] = constraint_lower_[j];
		g_u[2 * fix_like_n_abs_ + j] = constraint_upper_[j];
	}
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_get_starting_point$$
$spell
	cppad
	obj
	init
	ipopt
	bool
$$

$section Return Initial Values Where Optimization is Started$$

$head Syntax$$
$icode%ok% = get_starting_point(
	%n%, %init_x%, %x%, %init_z%, %z_L%, %z_U%, %m%, %init_lambda%, %lambda%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head init_x$$
assumed true which means the ipopt options specify that the this routine
will provide an initial value for $icode x$$.

$head x$$
if $icode init_x$$ is true,
set to the initial value for the primal variables (has size $icode n$$).

$head init_z$$
assumes $icode init_z$$ is false.
If it were true, the ipopt options specify that the this routine
will provide an initial value for $icode x$$ upper and lower bound
multipliers.

$head z_L$$
if $icode init_z$$ is true,
set to the initial value for the lower bound multipliers (has size $icode n$$).

$head z_U$$
if $icode init_z$$ is true,
set to the initial value for the upper bound multipliers (has size $icode n$$).

$head init_lambda$$
assumes $icode init_lambda$$ is false.
If it were true, the ipopt options specify that the this routine
will provide an initial value for $icode g(x)$$ upper and lower bound
multipliers.

$head lambda$$
if $icode init_lambda$$ is true,
set to the initial value for the $icode g(x)$$ multipliers
(has size $icode m$$).

$head ok$$
If set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_starting_point(
	Index           n            ,  // in
	bool            init_x       ,  // in
	Number*         x            ,  // out
	bool            init_z       ,  // in
	Number*         z_L          ,  // out
	Number*         z_U          ,  // out
	Index           m            ,  // in
	bool            init_lambda  ,  // in
	Number*         lambda       )  // out
{
	assert( init_x == true );
	assert( init_z == false );
	assert( init_lambda == false );
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );

	// fixed negative log-likelihood at the initial fixed effects vector
	if( fix_like_vec_tmp_.size() == 0 )
		assert( mixed_object_.prior_eval(fixed_in_).size() == 0 );
	else
	{	fix_like_vec_tmp_ = mixed_object_.prior_eval(fixed_in_);
		assert( fix_like_vec_tmp_.size() == 1 + fix_like_n_abs_ );
	}

	for(size_t j = 0; j < n_fixed_; j++)
		x[j] = fixed_in_[j];
	for(size_t j = 0; j < fix_like_n_abs_; j++)
		x[n_fixed_ + j] = CppAD::abs( fix_like_vec_tmp_[1 + j] );

	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_f$$
$spell
	cppad
	obj
	ipopt
	bool
	eval
	obj
	const
$$

$section Compute Value of Objective$$

$head Syntax$$
$icode%ok% = eval_f(%n%, %x%, %new_x%, %obj_value%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the objective
f(x) is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head obj_val$$
set to the initial value of the objective function f(x).

$head ok$$
If set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_f(
	Index           n         ,  // in
	const Number*   x         ,  // in
	bool            new_x     ,  // in
	Number&         obj_value )  // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	//
	// initialize random effects
	if( random_h_.size() == 0 )
		random_tmp_ = random_in_;
	else
		random_tmp_ = random_h_;
	//
	// value of fixed effects corresponding to this x
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = double( x[j] );
	//
	// random part of objective
	double H = Number( 0.0 );
	if( n_random_ > 0 )
	{	//
		// compute the optimal random effects corresponding to fixed effects
		if( new_x )
		random_cur_ = mixed_object_.optimize_random(
			random_options_, fixed_tmp_, random_tmp_
		);
# if ! MIXED_CPPAD_NEWTON
		H = mixed_object_.h_ran_like(fixed_tmp_, random_cur_);
# else
		//
		// compute random part of the objective
		H = mixed_object_.ran_obj_eval(
			fixed_tmp_, fixed_tmp_, random_cur_
		);
# endif
	}
	obj_value = Number(H);
	if( fix_like_vec_tmp_.size() == 0 )
		assert( mixed_object_.prior_eval(fixed_tmp_).size() == 0 );
	else
	{
		// fixed part of objective
		// (2DO: cache fix_like_vec_tmp_ for eval_g with same x)
		fix_like_vec_tmp_ = mixed_object_.prior_eval(fixed_tmp_);
		//
		// only include smooth part of prior in objective
		obj_value += Number( fix_like_vec_tmp_[0] );
		//
		// use contraints to represent absolute value part
		for(size_t j = 0; j < fix_like_n_abs_; j++)
			obj_value += x[n_fixed_ + j];
	}
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_grad_f$$
$spell
	cppad
	obj
	ipopt
	bool
	eval
	const
$$

$section Compute Gradient of the Objective$$

$head Syntax$$
$icode%ok% = eval_grad_f(%n%, %x%, %new_x%, %grad_f%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the gradient
$latex \nabla f(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head grad_f$$
is set to the value for the gradient $latex \nabla f(x)$$
(has size $icode m$$).

$head ok$$
If set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_grad_f(
	Index           n         ,  // in
	const Number*   x         ,  // in
	bool            new_x     ,  // in
	Number*         grad_f    )  // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	//
	// fixed effects
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = double( x[j] );
	//
	// initialize random effects
	if( random_h_.size() == 0 )
		random_tmp_ = random_in_;
	else
		random_tmp_ = random_h_;
	//
	// random part of objective
	assert( H_beta_tmp_.size() == n_fixed_ );
	for(size_t j = 0; j < n_fixed_; j++)
		H_beta_tmp_[j] = Number(0.0);

	d_vector r_fixed(n_fixed_);
	if( n_random_ > 0 )
	{
		// compute the optimal random effects corresponding to fixed effects
		if( new_x )
		random_cur_ = mixed_object_.optimize_random(
			random_options_, fixed_tmp_, random_tmp_
		);
		// Jacobian for random part of the Lalpace objective
# if ! MIXED_CPPAD_NEWTON
		mixed_object_.d_ran_like(
			fixed_tmp_, random_cur_, H_beta_tmp_
		);
# else
		H_beta_tmp_ = mixed_object_.ran_obj_beta(
			fixed_tmp_, fixed_tmp_, random_cur_
		);
# endif
	}
	//
	// Jacobian of fixed part of objective
	// (2DO: do not revaluate when eval_jac_g has same x)
	mixed_object_.fix_like_jac(
		fixed_tmp_, fix_like_jac_row_, fix_like_jac_col_, fix_like_jac_val_
	);

	//
	// set grad_f
	for(size_t j = 0; j < n_fixed_; j++)
	{	assert( j < size_t(n) );
		grad_f[j] = Number( H_beta_tmp_[j] );
	}
	for(size_t j = 0; j < fix_like_n_abs_; j++)
	{	assert( n_fixed_ + j < size_t(n) );
		grad_f[n_fixed_ + j] = Number( 1.0 );
	}
	for(size_t k = 0; k < fix_like_jac_row_.size(); k++)
	{	if( fix_like_jac_row_[k] == 0 )
		{	size_t j = fix_like_jac_col_[k];
			assert( j < size_t(n) );
			grad_f[j] += Number( fix_like_jac_val_[k] );
		}
	}
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_g$$
$spell
	cppad
	obj
	ipopt
	bool
	const
	eval
$$

$section Compute Value of Constraint Functions$$

$head Syntax$$
$icode%ok% = eval_g(%n%, %x%, %new_x%, %m%, %g%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the constraints
$latex g(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head g$$
is set to the value for the constraint functions (has size $icode m$$).

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_g(
	Index           n        ,  // in
	const Number*   x        ,  // in
	bool            new_x    ,  // in
	Index           m        ,  // in
	Number*         g        )  // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );
	//
	// fixed effects
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = double( x[j] );
	//
	// fixed part of objective
	// (2DO: cache fix_like_vec_tmp_ for eval_f with same x)
	fix_like_vec_tmp_ = mixed_object_.prior_eval(fixed_tmp_);
	//
	// convert absolute value terms to constraints
	for(size_t j = 0; j < fix_like_n_abs_; j++)
	{	// x[n_fixed_ + j] >= fix_like_vec_tmp_[1 + j];
		assert( 2 * j + 1 < size_t(m) );
		g[2 * j] = Number(x[n_fixed_ + j] - fix_like_vec_tmp_[1 + j]); // >= 0
		// x[n_fixed_ + j] >= - fix_like_vec_tmp_[1 + j]
		g[2*j+1] = Number(x[n_fixed_ + j] + fix_like_vec_tmp_[1 + j]); // >= 0
	}
	//
	// include explicit constraints
	c_vec_tmp_ = mixed_object_.constraint_eval(fixed_tmp_);
	assert( c_vec_tmp_.size() == n_constraint_ );
	for(size_t j = 0; j < n_constraint_; j++)
	{	assert( 2 * fix_like_n_abs_ + j < size_t(m) );
		g[2 * fix_like_n_abs_ + j] = c_vec_tmp_[j];
	}
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_jac_g$$
$spell
	cppad
	obj
	ipopt
	bool
	eval
	const
	nele_jac
	Jacobian
	nnz
$$

$section Compute Jacobian of Constraint Functions$$

$head Syntax$$
$icode%ok% = eval_jac_g(
	%n%, %x%, %new_x%, %m%, %nele_jac%, %iRow%, %jCol%, %values%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the Jacobian
of the constraints $latex \nabla g(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head nele_jac$$
is the number of non-zero elements in the Jacobian of $icode g(x)$$; i.e.,
the same as
$cref/nnz_jac_g/ipopt_fixed_get_nlp_info/nnz_jac_g/$$.

$head iRow$$
If $icode values$$ is $code NULL$$,
$icode iRow$$ has size $icode nele_jac$$ and is set to the
row indices for the non-zero entries in the Jacobian of the constraints
$latex g^{(1)} (x)$$.

$head jCol$$
If $icode values$$ is $code NULL$$,
$icode jCol$$ has size $icode nele_jac$$ and is set to the
column indices for the non-zero entries in the Jacobian of the constraints
$latex g^{(1)} (x)$$.

$head values$$
If $icode values$$ is not $code NULL$$,
it has size $icode nele_jac$$ and $icode%values%[%k%]%$$
is set to the value of element of the Jacobian $latex g^{(1)} (x)$$
with row index $icode%iRow%[%k%]%$$
and column index $icode%jCol%[%k%]%$$.

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_jac_g(
	Index           n        ,  // in
	const Number*   x        ,  // in
	bool            new_x    ,  // in
	Index           m        ,  // in
	Index           nele_jac ,  // in
	Index*          iRow     ,  // out
	Index*          jCol     ,  // out
	Number*         values   )  // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );
	assert( size_t(nele_jac) == nnz_jac_g_ );
	//
	if( values == NULL )
	{	// just return row and column indices for l1 constraints
		size_t ell = 0;
		for(size_t k = 0; k < fix_like_jac_row_.size(); k++)
		{	if( fix_like_jac_row_[k] != 0 )
			{	assert( ell + 1 < nnz_jac_g_ );
				iRow[ell+1] = iRow[ell] = Index( fix_like_jac_row_[k] );
				jCol[ell+1] = jCol[ell] = Index( fix_like_jac_col_[k] );
				ell += 2;
			}
		}
		// auxillary variables for l1 constraints
		for(size_t j = 0; j < fix_like_n_abs_; j++)
		{	assert( ell + 1 < nnz_jac_g_ );
			iRow[ell] = Index( 2 * j );
			jCol[ell] = Index( n_fixed_ + j);
			ell++;
			iRow[ell] = Index( 2 * j + 1);
			jCol[ell] = Index(n_fixed_ + j);
			ell++;
		}
		// explicit constaints
		for(size_t k = 0; k < constraint_jac_row_.size(); k++)
		{	assert( ell < nnz_jac_g_ );
			iRow[ell] = Index( constraint_jac_row_[k] );
			jCol[ell] = Index( constraint_jac_col_[k] );
			ell++;
		}
		assert( ell == nnz_jac_g_ );
		return true;
	}
	//
	// fixed effects
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = double( x[j] );
	//
	// Jacobian of fixed part of objective
	// (2DO: do not revaluate when eval_grad_f had same x)
	mixed_object_.fix_like_jac(
		fixed_tmp_, fix_like_jac_row_, fix_like_jac_col_, fix_like_jac_val_
	);
	size_t ell = 0;
	for(size_t k = 0; k < fix_like_jac_row_.size(); k++)
	{	if( fix_like_jac_row_[k] != 0 )
		{	assert( ell + 1 < nnz_jac_g_ );
			values[ell] = Number( fix_like_jac_val_[k] );
			ell++;
			values[ell] = Number( - fix_like_jac_val_[k] );
			ell++;
		}
	}
	for(size_t j = 0; j < fix_like_n_abs_; j++)
	{	assert( ell + 1 < nnz_jac_g_ );
		values[ell+1] = values[ell] = Number(1.0);
		ell += 2;
	}
	//
	// Jacobian of explicit constraints
	mixed_object_.constraint_jac(
		fixed_tmp_,
		constraint_jac_row_,
		constraint_jac_col_,
		constraint_jac_val_
	);
	for(size_t k = 0; k < constraint_jac_row_.size(); k++)
	{	assert( ell < nnz_jac_g_ );
		values[ell++] = Number( constraint_jac_val_[k] );
	}
	assert( ell == nnz_jac_g_ );
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_h$$
$spell
	cppad
	obj
	ipopt
	bool
	eval
	const
	obj
	nele_hess
	nnz
$$

$section Compute the Hessian of the Lagrangian$$

$head Syntax$$
$icode%ok% = eval_h(
	%n%, %x%, %new_x%,%obj_factor%, %m%, %lambda%, %new_lambda%,%$$
$icode%nele_hess%, %iRow%, %jCol%, %values%
)%$$

$head Lagrangian$$
The Lagrangian is defined to be
$latex \[
	L(x) = \alpha f(x) + \sum_{i=0}^{m-1} \lambda_i g_i (x)
\] $$

$head ! MIXED_CPPAD_NEWTON$$
It is assumed that this preprocessor symbol is false (zero).

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the
Hessian of the Lagrangian is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head obj_factor$$
is the factor $latex \alpha$$ that multiplies the objective f(x)
in the definition of the Lagrangian.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head lambda$$
is the value of the constraint multipliers $latex \lambda$$
at which the Hessian is to be evaluated (has size $icode m$$).

$head new_lambda$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode lambda$$.

$head nele_hess$$
is the number of non-zero elements in the Hessian $latex L^{(2)} (x)$$; i.e.,
the same as
$cref/nnz_h_lag/ipopt_fixed_get_nlp_info/nnz_h_lag/$$.

$head iRow$$
If $icode values$$ is $code NULL$$,
$icode iRow$$ has size $icode nele_hess$$ and is set to the
row indices for the non-zero entries in the Hessian
$latex L^{(2)} (x)$$.

$head jCol$$
If $icode values$$ is $code NULL$$,
$icode jCol$$ has size $icode nele_hess$$ and is set to the
column indices for the non-zero entries in the Hessian
$latex L^{(2)} (x)$$.

$head values$$
If $icode values$$ is not $code NULL$$,
it has size $icode nele_hess$$ and $icode%values%[%k%]%$$
is set to the value of element of the Hessian $latex L^{(2)} (x)$$
with row index $icode%iRow%[%k%]%$$
and column index $icode%jCol%[%k%]%$$.

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_h(
	Index         n              ,  // in
	const Number* x              ,  // in
	bool          new_x          ,  // in
	Number        obj_factor     ,  // in
	Index         m              ,  // in
	const Number* lambda         ,  // in
	bool          new_lambda     ,  // in
	Index         nele_hess      ,  // in
	Index*        iRow           ,  // out
	Index*        jCol           ,  // out
	Number*       values         )  // out
{
	assert( MIXED_CPPAD_NEWTON );
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );
	assert( size_t(nele_hess) == nnz_h_lag_ );
	if( values == NULL )
	{	for(size_t k = 0; k < nnz_h_lag_; k++)
		{	iRow[k] = Index( lag_hes_row_[k] );
			jCol[k] = Index( lag_hes_col_[k] );
		}
		return true;
	}
	//
	// fixed effects
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = double( x[j] );
	//
	// initialize random effects
	if( random_h_.size() == 0 )
		random_tmp_ = random_in_;
	else
		random_tmp_ = random_h_;
	//
	// initialize return value
	for(size_t k = 0; k < nnz_h_lag_; k++)
		values[k] = Number( 0.0 );
	//
	// random part of objective
	if( n_random_ > 0 )
	{
		// compute the optimal random effects corresponding to fixed effects
		if( new_x )
		random_cur_ = mixed_object_.optimize_random(
			random_options_, fixed_tmp_, random_tmp_
		);
		//
		random_h_   = random_cur_;
		//
		// compute Hessian of random part w.r.t. fixed effects
		mixed_object_.ran_obj_hes_fix(
			fixed_tmp_, random_cur_,
			ran_obj_hes_row_, ran_obj_hes_col_, ran_obj_hes_val_
		);
		for(size_t k = 0; k < ran_obj_hes_row_.size(); k++)
			values[ ran_obj_2_lag_[k] ] +=
				obj_factor * Number( ran_obj_hes_val_[k] );
	}
	//
	// Hessian of Lagrangian of weighted prior w.r.t. fixed effects
	w_fix_like_tmp_[0] = obj_factor;
	for(size_t j = 0; j < fix_like_n_abs_; j++)
		w_fix_like_tmp_[1 + j] = lambda[2 * j + 1] - lambda[2 * j];
	mixed_object_.fix_like_hes(
		fixed_tmp_,
		w_fix_like_tmp_,
		fix_like_hes_row_,
		fix_like_hes_col_,
		fix_like_hes_val_
	);
	for(size_t k = 0; k < fix_like_hes_row_.size(); k++)
	{	assert( fix_like2lag_[k] < nnz_h_lag_ );
		values[ fix_like2lag_[k] ] += Number( fix_like_hes_val_[k] );
	}
	//
	// Hessian of Lagrangian of weighted explicit constraints
	for(size_t j = 0; j < n_constraint_; j++)
		w_constraint_tmp_[j] = lambda[2 * fix_like_n_abs_ + j];
	mixed_object_.constraint_hes(
		fixed_tmp_,
		w_constraint_tmp_,
		constraint_hes_row_,
		constraint_hes_col_,
		constraint_hes_val_
	);
	for(size_t k = 0; k < constraint_hes_row_.size(); k++)
	{	assert( constraint_2_lag_[k] < nnz_h_lag_ );
		values[ constraint_2_lag_[k] ] += Number( constraint_hes_val_[k] );
	}
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_finalize_solution$$
$spell
	cppad
	obj
	ipopt
	bool
	eval
	const
	obj
	ip
	cq
	namespace
	infeasibility
	doesn't
	Inf
	naninf
	std
	cout
	endl
	fabs
	tol
$$

$section Get Solution Results$$

$head Syntax$$
$codei%finalize_solution(
	%status%, %n%, %x%, %z_L%, %z_U%, %m%, %g%,%$$
$icode%lambda%, %obj_value%, %ip_data%, %ip_cq%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the final value (best value found) for the primal variables
(has size $icode n$$).

$head z_L$$
is the final value for the $icode x$$ lower bound constraint multipliers
(has size $icode n$$).

$head z_U$$
is the final value for the $icode x$$ upper bound constraint multipliers
(has size $icode n$$).

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head lambda$$
is the final value for the g(x) constraint multipliers $latex \lambda$$.

$head obj_value$$
is the value of the objective f(x) at the final $icode x$$ value.

$head ip_data$$
Unspecified; i.e., not part of the Ipopt user API.

$head ip_cq$$
Unspecified; i.e., not part of the Ipopt user API.

$head status$$
These status values are in the $code Ipopt$$ namespace; e.g.,
$code SUCCESS$$ is short for $code Ipopt::SUCCESS$$:

$subhead SUCCESS$$
Algorithm terminated successfully at a locally optimal point,
satisfying the convergence tolerances (can be specified by options).

$subhead MAXITER_EXCEEDED$$
Maximum number of iterations exceeded (can be specified by an option).

$subhead CPUTIME_EXCEEDED$$
Maximum number of CPU seconds exceeded (can be specified by an option).

$subhead STOP_AT_TINY_STEP$$
Algorithm proceeds with very little progress.

$subhead STOP_AT_ACCEPTABLE_POINT$$
Algorithm stopped at a point that was converged, not to desired
tolerances, but to acceptable tolerances (see the acceptable-... options).

$subhead LOCAL_INFEASIBILITY$$
Algorithm converged to a point of local infeasibility. Problem may be
infeasible.

$subhead USER_REQUESTED_STOP$$
A user call-back function returned false, i.e.,
the user code requested a premature termination of the optimization.

$subhead DIVERGING_ITERATES$$
It seems that the iterates diverge.

$subhead RESTORATION_FAILURE$$
Restoration phase failed, algorithm doesn't know how to proceed.

$subhead ERROR_IN_STEP_COMPUTATION$$
An unrecoverable error occurred while Ipopt tried to compute
the search direction.

$subhead INVALID_NUMBER_DETECTED$$
Algorithm received an invalid number (such as NaN or Inf) from
the NLP; see also option check_derivatives_for_naninf.

$end
-------------------------------------------------------------------------------
*/
void ipopt_fixed::finalize_solution(
	Ipopt::SolverReturn               status    ,  // in
	Index                             n         ,  // in
	const Number*                     x         ,  // in
	const Number*                     z_L       ,  // in
	const Number*                     z_U       ,  // in
	Index                             m         ,  // in
	const Number*                     g         ,  // in
	const Number*                     lambda    ,  // in
	Number                            obj_value ,  // in
	const Ipopt::IpoptData*           ip_data   ,  // in
	Ipopt::IpoptCalculatedQuantities* ip_cq     )  // in
{	bool ok = true;
	//
	assert( n > 0 && size_t(n) == n_fixed_ + fix_like_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * fix_like_n_abs_ + n_constraint_ );
	assert( fixed_opt_.size() == 0 );
	//
	//
	// relaxed verison of tolerance
	double tol = fixed_tolerance_;
	//
	// check that x is within its limits
	fixed_opt_.resize(n_fixed_);
	for(size_t j = 0; j < n_fixed_; j++)
	{	fixed_opt_[j] = x[j];
		ok &= check_in_limits(fixed_lower_[j], x[j], fixed_upper_[j], tol);
	}
	//
	// check that the bound multipliers are feasible
	for(size_t j = 0; j < n_fixed_ + fix_like_n_abs_; j++)
	{	ok &= 0.0 <= z_L[j];
		ok &= 0.0 <= z_U[j];
	}
	//
	// fixed negative log-likelihood at the final fixed effects vector
	if( fix_like_vec_tmp_.size() == 0 )
		assert( mixed_object_.prior_eval(fixed_opt_).size() == 0 );
	else
	{	fix_like_vec_tmp_ = mixed_object_.prior_eval(fixed_opt_);
		assert( fix_like_vec_tmp_.size() == 1 + fix_like_n_abs_ );

		// check constraints corresponding to l1 terms
		for(size_t j = 0; j < fix_like_n_abs_; j++)
		{	double check = double( x[n_fixed_ + j] );
			ok &= check - fix_like_vec_tmp_[j + 1] + 1e2 * tol >= 0;
			ok &= check + fix_like_vec_tmp_[j + 1] + 1e2 * tol >= 0;
		}
	}
	//
	// explicit constraints at the final fixed effects vector
	c_vec_tmp_ = mixed_object_.constraint_eval(fixed_opt_);
	assert( c_vec_tmp_.size() == n_constraint_ );

	// check explicit constraints
	for(size_t j = 0; j < n_constraint_; j++)
	{	ok &= check_in_limits(
			constraint_lower_[j], c_vec_tmp_[j], constraint_upper_[j], tol
		);
	}
	// Evaluate gradient of f w.r.t x
	CppAD::vector<Number> grad_f(n);
	bool new_x = true;
	eval_grad_f(n, x, new_x, grad_f.data() );

	// Evaluate gradient of g w.r.t x
	CppAD::vector<Index> iRow(nnz_jac_g_), jCol(nnz_jac_g_);
	iRow.data();
	eval_jac_g(
		n, x, new_x, m, nnz_jac_g_,
		iRow.data(), jCol.data(), NULL
	);
	CppAD::vector<Number> values(nnz_jac_g_);
	eval_jac_g(
		n, x, new_x, m, nnz_jac_g_,
		iRow.data(), jCol.data(), values.data()
	);

	// Check the partial of the Lagrangian w.r.t fixed effects
	double average = 0.0;
	for(size_t j = 0; j < n_fixed_; j++)
	{	Number sum = grad_f[j];
		for(size_t k = 0; k < nnz_jac_g_; k++)
		{	if( jCol[k] == Index(j) )
			{	Index  i = iRow[k];
				sum   += lambda[i] * values[k];
			}
		}
		// sum += z_U[j] - z_L[j]; does not work because
		// Ipopt does not seem to set z_U[j] and z_L[j] accuractely
		double scale = CppAD::abs( (1.0 + tol) * x[j] );
		bool at_lower = x[j] - fixed_lower_[j] <= scale;
		if( at_lower )
			sum = std::min(sum, 0.0);
		bool at_upper = fixed_upper_[j] - x[j] <= scale;
		if( at_upper )
			sum = std::max(sum, 0.0);
		//
		average += CppAD::abs(sum) / double(n_fixed_);
	}
	// needed to relax tolerance for bfgs method (Newton is more accurate)
	ok &= average <= 30. * tol;

	// Check the partial of the Lagrangian w.r.t auxillary variables
	average = 0.0;
	for(size_t j = n_fixed_; j < n_fixed_ + fix_like_n_abs_; j++)
	{	Number sum = grad_f[j];
		for(size_t k = 0; k < nnz_jac_g_; k++)
		{	if( jCol[k] == Index(j) )
			{	Index  i = iRow[k];
				sum   += lambda[i] * values[k];
			}
		}
		average += CppAD::abs(sum) / double(n_fixed_);
	}
	ok &= average <= tol;

	// set member variable finalize_solution_ok_
	finalize_solution_ok_ = ok;
}
} // END_DISMOD_AT_NAMESPACE