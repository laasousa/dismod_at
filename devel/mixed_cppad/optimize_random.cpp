// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <cppad/ipopt/solve.hpp>
# include <dismod_at/mixed_cppad.hpp>
/*
$begin mixed_cppad_optimize_random$$
$spell
	cppad
	vec
	const
	CppAD
	std
	Ipopt
$$

$section Optimize Random Effects$$

$head Syntax$$
$icode%random_out% =%$$
$icode%mixed_object%.optimize_random(%options%, %fixed_vec%, %random_in%)%$$

$head Purpose$$
This routine maximizes the
$cref/random negative log-likelihood/mixed_cppad_ran_like/$$
corresponding to the object $icode mixed_object$$.

$head mixed_object$$
We use $cref/mixed_object/mixed_cppad_derived_ctor/mixed_object/$$
to denote an object of a class that is
derived from the $code mixed_cppad$$ base class.

$head options$$
The argument $icode options$$ has prototype
$codei%
	const std::string& %options%
%$$
and is the $cref ipopt_options$$ for optimizing the random effects.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/mixed_cppad/Fixed Effects, theta/$$
vector $latex \theta$$.

$head random_in$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_in%
%$$
It specifies the initial value used for the optimization of the
$cref/random effects/mixed_cppad/Random Effects, u/$$
vector $latex u$$.

$head random_out$$
The return value  has prototype
$codei%
	CppAD::vector<double> %random_out%
%$$
It is the final value (obtained by optimization) of the
$cref/random effects/mixed_cppad/Random Effects, u/$$
vector $latex u$$.

$head Example$$
The file $cref optimize_random_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

// ----------------------------------------------------------------------------
// helper class used by optimize_random
class optimize_random_eval {
public:
	// same as mixed_cppad::d_vector
	typedef CppAD::vector<double>              Dvector;

	// same as mixed_cppad::a1d_vector
	typedef CppAD::vector< CppAD::AD<double> > ADvector;
private:
	const size_t     n_abs_;
	const size_t     n_fixed_;
	const size_t     n_random_;
	ADvector         fixed_vec_;
	mixed_cppad&    mixed_object_;
public:
	// constructor
	optimize_random_eval(
		size_t           n_abs           ,
		size_t           n_random        ,
		const Dvector&   fixed_vec       ,
		mixed_cppad&    mixed_object
	) :
	n_abs_        ( n_abs )            ,
	n_fixed_      ( fixed_vec.size() ) ,
	n_random_     ( n_random )         ,
	mixed_object_( mixed_object )
	{	fixed_vec_.resize( n_fixed_ );
		for(size_t i = 0; i < n_fixed_; i++)
			fixed_vec_[i] = fixed_vec[i];
	}

	// evaluate objective and constraints
	void operator()(ADvector& fg, const ADvector& x)
	{	assert( fg.size() == 1 + 2 * n_abs_ );

		// extract the random effects from x
		ADvector random_vec(n_random_), both_vec(n_fixed_ + n_random_);
		for(size_t j = 0; j < n_random_; j++)
			random_vec[j] = x[j];
		mixed_object_.pack(fixed_vec_, random_vec, both_vec);

		// compute log-density vector
		ADvector vec = mixed_object_.a1_ran_like_.Forward(0, both_vec);

		// initialize smooth part of negative log-likelihood
		size_t k = 0;
		fg[k++]  = vec[0];

		// terms corresponding to data likelihood absolute values
		size_t n_abs   = vec.size() - 1;
		for(size_t j = 0; j < n_abs; j++)
		{	// x[ n_random_ + j] >= abs(log_den[1 + j]
			fg[k++] = x[ n_random_ + j] - vec[1 + j];
			fg[k++] = x[ n_random_ + j] + vec[1 + j];
			//
			// smooth contribution to log-likelihood
			fg[0]  += x[ n_random_ + j];
		}
	}
};

// ----------------------------------------------------------------------------
// optimize_random
CppAD::vector<double> mixed_cppad::optimize_random(
	const std::string& options         ,
	const d_vector&    fixed_vec       ,
	const d_vector&    random_in       )
{
	// make sure initialize has been called
	if( ! initialize_done_ )
	{	std::string error_message =
		"mixed_cppad::initialize was not called before optimize_random";
		fatal_error(error_message);
	}
	if( ! record_ran_like_done_ )
	{	std::string error_message =
		"mixed_cppad::optimize_random there are no random effects";
		fatal_error(error_message);
	}

	// number of fixed and random effects
	assert( n_fixed_  == fixed_vec.size() );
	assert( n_random_ == random_in.size() );

	// determine initial density vector
	d_vector both_vec(n_fixed_ + n_random_);
	pack(fixed_vec, random_in, both_vec);
	d_vector vec = a0_ran_like_.Forward(0, both_vec);

	// number of absolute value terms in objective
	size_t n_abs = vec.size() - 1;

	// number of independent variable is number of random effects
	// plus number of log-density terms that require absolute values
	size_t nx = n_random_ + n_abs;

	// set initial x vector
	d_vector xi(nx);
	for(size_t j = 0; j < n_random_; j++)
		xi[j] = random_in[j];
	for(size_t j = 0; j < n_abs; j++)
		xi[n_random_ + j] = CppAD::abs( vec[j] );

	// ipopts default value for infinity (use options to change it)
	double ipopt_infinity = 1e19;

	// set lower and upper limits for x
	d_vector xl(nx), xu(nx);
	for(size_t j = 0; j < nx; j++)
	{	xl[j] = - ipopt_infinity;
		xu[j] = + ipopt_infinity;
	}

	// set limits for abs contraint representation
	d_vector gl(2*n_abs), gu(2*n_abs);
	for(size_t j = 0; j < 2*n_abs; j++)
	{	gl[j] = 0.0;
		gu[j] = + ipopt_infinity;
	}

	// construct fg_eval  object
	optimize_random_eval fg_eval(n_abs, n_random_, fixed_vec, *this);

	// optimizer options
	assert( options[ options.size() - 1] == '\n' );
	std::string solve_options = options + "Sparse  true  reverse \n";

	// return solution
	CppAD::ipopt::solve_result<d_vector> solution;

	// solve the optimization problem
	CppAD::ipopt::solve(
		solve_options, xi, xl, xu, gl, gu, fg_eval, solution
	);

	return solution.x;
}


} // END_DISMOD_AT_NAMESPACE
