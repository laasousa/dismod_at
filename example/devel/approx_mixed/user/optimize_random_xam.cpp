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
$begin optimize_random_xam.cpp$$
$spell
	interp
	xam
$$

$section C++ optimize_random: Example and Test$$

$code
$verbatim%example/devel/approx_mixed/user/optimize_random_xam.cpp
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
		const vector<double>& y_;
	public:
		// constructor
		approx_derived(
			size_t n_fixed                    ,
			size_t n_random                   ,
			const vector<double>& y           )
			:
			dismod_at::approx_mixed(n_fixed, n_random) ,
			y_(y)
		{ }
	private:
		// implementation of joint_like
		template <class Float>
		vector<Float> implement_joint_like(
			const vector<Float>& theta  ,
			const vector<Float>& u      )
		{	vector<Float> vec(1);

			// initialize part of log-density that is always smooth
			vec[0] = Float(0.0);

			// compute these factors once
			Float sqrt_2   = CppAD::sqrt( Float(2.0) );
			Float sqrt_2pi = Float( CppAD::sqrt( 8.0 * CppAD::atan(1.0) ) );

			for(size_t i = 0; i < y_.size(); i++)
			{	Float mu     = u[i];
				Float sigma  = theta[i];
				Float res    = (y_[i] - mu) / sigma;

				if( i % 2 )
				{	// This is a Gaussian term, so entire density is smooth
					vec[0]  += (sqrt_2pi * log(sigma) + res*res) / Float(2.0);
				}
				else
				{	// This term is Laplace distributed
					vec[0] += log(sqrt_2 * sigma);

					// part of the density that need absolute value
					vec.push_back(sqrt_2 * res);
				}
			}
			return vec;
		}
	public:
		// -------------------------------------------------------------------
		// User defined virtual functions
		virtual vector<a5_double> joint_like(
			const vector<a5_double>& fixed_vec  ,
			const vector<a5_double>& random_vec )
		{	return implement_joint_like(fixed_vec, random_vec); }
		//
		// improper constant prior
		virtual vector<a1_double> prior_like(
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
		// ------------------------------------------------------------------
	};
}

bool optimize_random_xam(void)
{
	bool   ok = true;

	size_t n_data = 10;
	vector<double> data(n_data), fixed_vec(n_data), random_in(n_data);

	for(size_t i = 0; i < n_data; i++)
	{	data[i]      = double(i + 1);
		fixed_vec[i] = 1.0;
		random_in[i] = 0.0;
	}

	// object that is derived from approx_mixed
	approx_derived approx_object(n_data, n_data, data);
	approx_object.initialize(fixed_vec, random_in);

	// determine the optimal random effects
	vector<double> random_out = approx_object.optimize_random(
		fixed_vec, random_in
	);

	// check the result
	for(size_t i = 0; i < n_data; i++)
	{	// debugging print out
		// std::cout << random_out[i] / data[i] - 1.0 << std::endl;
		ok &= CppAD::abs(random_out[i] / data[i] - 1.0) < 1e-10;
	}

	return ok;
}
// END C++