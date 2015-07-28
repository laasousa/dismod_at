// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin approx_constraint_xam.cpp$$
$spell
	hes
	eval
	interp
	xam
$$

$section C++ approx_mixed constraints: Example and Test$$

$head Model$$
$latex \[
	\B{p}( y_i | \theta , u ) \sim \B{N} ( u_i + \theta_i , 1 )
\] $$
$latex \[
	\B{p}( u_i | \theta ) \sim \B{N} ( 0 , 1 )
\] $$
$latex \[
	\B{p}( \theta ) \sim \B{U} ( - \infty , + \infty )
\] $$
where $latex \B{U} ( - \infty ,  + \infty )$$ is the improper uniform prior
on $latex [- \infty , + \infty ]$$.
It follows that the Laplace approximation is exact and
$latex \[
	\B{p}( y_i | \theta ) \sim \B{N} ( \theta_i , 2 )
\] $$
The corresponding objective for the fixed effects is equivalent to:
$latex \[
	\frac{1}{2} \sum_{i=0}^{N-1} ( y_i - \theta_i )^2
\] $$
For this problem we add the explicit constraint
$latex \[
	\frac{1}{2} \sum_i \theta_i^2 = 1;
\] $$
The corresponding Lagrangian is
$latex \[
L( \theta , \lambda ) =
	\frac{1}{2} \sum_{i=0}^{N-1} ( y_i - \theta_i )^2
		+ \lambda \left( \frac{1}{2} \sum_i \theta_i^2 - 1 \right)
\] $$

$code
$verbatim%example/devel/approx_mixed/user/approx_constraint_xam.cpp
	%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <cppad/cppad.hpp>
# include <dismod_at/approx_mixed.hpp>

namespace {
	using CppAD::vector;
	using CppAD::log;
	using CppAD::AD;

	class approx_derived : public dismod_at::approx_mixed {
	private:
		size_t                n_fixed_;
		const vector<double>& y_;
	public:
		// constructor
		approx_derived(
			size_t n_fixed                    ,
			size_t n_random                   ,
			const vector<double>& y           ) :
			dismod_at::approx_mixed(n_fixed, n_random) ,
			n_fixed_(n_fixed)                          ,
			y_(y)
		{}
	private:
		// implementation of joint_like
		template <class Float>
		vector<Float> implement_joint_like(
			const vector<Float>& theta  ,
			const vector<Float>& u      )
		{	assert( u.size() == y_.size() );
			assert( theta.size() == y_.size() );
			vector<Float> vec(1);

			// initialize part of log-density that is always smooth
			vec[0] = Float(0.0);

			// pi
			Float sqrt_2pi = Float( CppAD::sqrt(8.0 * CppAD::atan(1.0) ) );

			for(size_t i = 0; i < y_.size(); i++)
			{	Float mu     = u[i] + theta[i];
				Float sigma  = Float(1.0);
				Float res    = (y_[i] - mu) / sigma;

				// p(y_i | u, theta)
				vec[0] += log(sqrt_2pi * sigma) + res*res / Float(2.0);

				// p(u_i | theta)
				vec[0] += log(sqrt_2pi) + u[i] * u[i] / Float(2.0);
			}
			return vec;
		}
		// implementation of prior_like
		template <class Float>
		vector<Float> implement_prior_like(
			const vector<Float>& fixed_vec  )
		{	vector<Float> vec(1);

			// initialize part of log-density that is smooth
			vec[0] = Float(0.0);

			return vec;
		}
	public:
		// ------------------------------------------------------------------
		// User defined virtual functions
		virtual vector<a5_double> joint_like(
			const vector<a5_double>& fixed_vec  ,
			const vector<a5_double>& random_vec )
		{	return implement_joint_like(fixed_vec, random_vec); }
		//
		virtual vector<a1_double> prior_like(
			const vector<a1_double>& fixed_vec  )
		{	return implement_prior_like(fixed_vec); }
		//
		virtual vector<a1_double> constraint(
			const vector<a1_double>& fixed_vec  )
		{	vector<a1_double> ret_val(1);
			//
			ret_val[0] = 0.0;
			for(size_t i = 0; i < fixed_vec.size(); i++)
				ret_val[0] += fixed_vec[i] * fixed_vec[i];
			ret_val[0] /= 2.0;
			//
			return ret_val;
		}
		//
		virtual void fatal_error(const std::string& error_message)
		{	std::cerr << "Error: " << error_message << std::endl;
			std::exit(1);
		}
		//
		virtual void warning(const std::string& warning_message)
		{	std::cerr << "Warning: " << warning_message << std::endl;
		}
		// ------------------------------------------------------------------
	};
}

bool approx_constraint_xam(void)
{
	bool   ok = true;
	double inf = std::numeric_limits<double>::infinity();
	double tol = 1e-8;

	size_t n_data   = 3;
	size_t n_fixed  = n_data;
	size_t n_random = n_data;
	vector<double>
		fixed_lower(n_fixed), fixed_in(n_fixed), fixed_upper(n_fixed);
	for(size_t i = 0; i < n_fixed; i++)
	{	fixed_lower[i] = - inf;
		fixed_in[i]    = 0.1;
		fixed_upper[i] = inf;
	}
	//
	// explicit constriants (in addition to l1 terms)
	vector<double> constraint_lower(1), constraint_upper(1);
	constraint_lower[0] = 1.0;
	constraint_upper[0] = 1.0;
	//
	vector<double> data(n_data), random_in(n_random);
	for(size_t i = 0; i < n_data; i++)
	{	data[i]       = double(i + 1);
		random_in[i] = i / double(n_data);
	}

	// object that is derived from approx_mixed
	approx_derived approx_object(n_fixed, n_random, data);
	approx_object.initialize(fixed_in, random_in);

	// optimize the fixed effects
	std::string options =
		"Integer print_level               0\n"
		"String  sb                        yes\n"
		"String  derivative_test           second-order\n"
		"String  derivative_test_print_all yes\n"
		"Numeric tol                       1e-8\n"
	;
	vector<double> fixed_out = approx_object.optimize_fixed(
		options,
		fixed_lower,
		fixed_upper,
		constraint_lower,
		constraint_upper,
		fixed_in,
		random_in
	);

	// check constraint
	double sum = 0.0;
	for(size_t i = 0; i < n_fixed; i++)
		sum += fixed_out[i] * fixed_out[i];
	ok &= CppAD::abs( sum / 2.0 - 1.0 ) <= tol;

	// compute lagranges multiplier by averaging
	sum = 0.0;
	for(size_t i = 0; i < n_fixed; i++)
		sum += (fixed_out[i] - data[i]) / fixed_out[i];
	double lambda = sum / double(n_fixed);

	// check partials of Lagragian w.r.t fixed effects
	for(size_t i = 0; i < n_fixed; i++)
	{	double err	= data[i] - fixed_out[i] + lambda * fixed_out[i];
		ok         &= CppAD::abs(err) < tol;
	}
	return ok;
}
// END C++