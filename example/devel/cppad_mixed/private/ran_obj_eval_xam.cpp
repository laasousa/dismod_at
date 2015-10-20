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
$begin ran_obj_eval_xam.cpp$$
$spell
	cppad
	obj
	eval
	interp
	xam
$$

$section C++ ran_obj_eval: Example and Test$$

$head Private$$
This example is not part of the
$cref/cppad_mixed public API/cppad_mixed_public/$$.

$head Model$$
$latex \[
	\B{p}( y_i | \theta , u ) \sim \B{N} ( u_i + \theta_0 , \theta_1^2 )
\] $$
$latex \[
	\B{p}( u_i | \theta ) \sim \B{N} ( 0 , 1 )
\] $$
It follows that the Laplace approximation is exact and
$latex \[
	\B{p}( y_i | \theta ) \sim \B{N} \left( \theta_0 , 1 + \theta_1^2 \right)
\] $$

$code
$verbatim%example/devel/cppad_mixed/private/ran_obj_eval_xam.cpp
	%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <cppad/cppad.hpp>
# include <dismod_at/cppad_mixed.hpp>

namespace {
	using CppAD::vector;
	using CppAD::log;
	using CppAD::AD;

	class mixed_derived : public dismod_at::cppad_mixed {
	private:
		const vector<double>& y_;
	public:
		// constructor
		mixed_derived(
			size_t n_fixed                    ,
			size_t n_random                   ,
			const vector<double>& y           )
			:
			dismod_at::cppad_mixed(n_fixed, n_random) ,
			y_(y)
		{	assert( n_fixed == 2);
		}
	private:
		// implementation of ran_like
		template <class Float>
		vector<Float> implement_ran_like(
			const vector<Float>& theta  ,
			const vector<Float>& u      )
		{	vector<Float> vec(1);

			// initialize part of log-density that is always smooth
			vec[0] = Float(0.0);

			// pi
			Float sqrt_2pi = Float( CppAD::sqrt(8.0 * CppAD::atan(1.0) ) );

			for(size_t i = 0; i < y_.size(); i++)
			{	Float mu     = u[i] + theta[0];
				Float sigma  = theta[1];
				Float res    = (y_[i] - mu) / sigma;

				// p(y_i | u, theta)
				vec[0] += log(sqrt_2pi * sigma) + res*res / Float(2.0);

				// p(u_i | theta)
				vec[0] += log(sqrt_2pi) + u[i] * u[i] / Float(2.0);
			}
			return vec;
		}
	public:
		//
		virtual vector<a2_double> ran_like(
			const vector<a2_double>& fixed_vec  ,
			const vector<a2_double>& random_vec )
		{	return implement_ran_like(fixed_vec, random_vec); }
		//
		virtual vector<a1_double> fix_like(
			const vector<a1_double>& fixed_vec  )
		{	a1d_vector vec(1);
			vec[0] = 0.0;
			return vec;
		}
		//
		virtual vector<a1_double> constraint(
			const vector<a1_double>& fixed_vec  )
		{	return vector<a1_double>(0); } // empty vector
		//
		virtual void fatal_error(const std::string& error_message)
		{	std::cerr << "Error: " << error_message << std::endl;
			std::exit(1);
		}
		//
		virtual void warning(const std::string& warning_message)
		{	std::cerr << "Warning: " << warning_message << std::endl;
		}
	};
}

bool ran_obj_eval_xam(void)
{
	bool   ok = true;
	double eps = 100. * std::numeric_limits<double>::epsilon();
	double sqrt_2pi = CppAD::sqrt(8.0 * CppAD::atan(1.0) );

	size_t n_data   = 10;
	size_t n_fixed  = 2;
	size_t n_random = n_data;
	vector<double> data(n_data), fixed_vec(n_fixed), random_vec(n_random);
	vector<double> beta(n_fixed), theta(n_fixed), uhat(n_random);

	fixed_vec[0] = 2.0;
	fixed_vec[1] = 1.0;
	for(size_t i = 0; i < n_data; i++)
	{	data[i]       = double(i + 1);
		random_vec[i] = i / double(n_data);
	}
	for(size_t j = 0; j < n_fixed; j++)
		beta[j] = theta[j] = fixed_vec[j];

	// object that is derived from cppad_mixed
	mixed_derived mixed_object(n_fixed, n_random, data);
	mixed_object.initialize(fixed_vec, random_vec);

	// optimize the random effects
	std::string options;
	options += "Integer print_level 0\n";
	options += "String  sb          yes\n";
	options += "String  derivative_test second-order\n";
	uhat = mixed_object.optimize_random(options, fixed_vec, random_vec);

	// compute random part of Laplace approximation
	double H = mixed_object.ran_obj_eval(beta, theta, uhat);

	// For this case the Laplace approximation is exactly equal the integral
	// p(y | theta ) = integral of p(y | theta , u) p(u | theta) du
	double mu    = fixed_vec[0];
	double sum   = 0.0;
	double sigma  = fixed_vec[1];
	double delta  = CppAD::sqrt( sigma * sigma + 1.0 );
	for(size_t i = 0; i < n_data; i++)
	{	double res    = (data[i] - mu) / delta;
		sum          += CppAD::log(sqrt_2pi * delta) + res*res / 2.0;
	}
	ok &= abs( H / sum - 1.0 ) < eps;

	return ok;
}
// END C++