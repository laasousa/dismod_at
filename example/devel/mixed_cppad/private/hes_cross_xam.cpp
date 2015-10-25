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
$begin hes_cross_xam.cpp$$
$spell
	cppad
	hes
	interp
	xam
$$

$section C++ Hessian Cross Terms: Example and Test$$


$head Private$$
This example is not part of the
$cref/mixed_cppad public API/mixed_cppad_public/$$.

$code
$verbatim%example/devel/mixed_cppad/private/hes_cross_xam.cpp
	%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <cppad/cppad.hpp>
# include <dismod_at/mixed_cppad.hpp>
# include <dismod_at/mixed_pack.hpp>

namespace {
	using CppAD::vector;
	using CppAD::log;
	using CppAD::AD;

	class mixed_derived : public dismod_at::mixed_cppad {
	private:
		const vector<double>& y_;
	public:
		// constructor
		mixed_derived(
			size_t n_fixed                    ,
			size_t n_random                   ,
			const vector<double>& y           )
			:
			// quasi_fixed = false
			dismod_at::mixed_cppad(n_fixed, n_random, false) ,
			y_(y)
		{ }
	private:
		// implementation of ran_like
		template <class Float>
		vector<Float> implement_ran_like(
			const vector<Float>& theta  ,
			const vector<Float>& u      )
		{	vector<Float> vec(1);

			// initialize part of log-density that is always smooth
			vec[0] = Float(0.0);

			for(size_t i = 0; i < y_.size(); i++)
			{	Float mu     = u[i];
				Float sigma  = theta[i];
				Float res    = (y_[i] - mu) / sigma;

				// (do not need 2*pi inside of log)
				vec[0]  += (log(sigma) + res*res) / Float(2.0);
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

bool hes_cross_xam(void)
{
	bool   ok = true;
	double eps = 100. * std::numeric_limits<double>::epsilon();

	size_t n_data   = 10;
	size_t n_fixed  = n_data;
	size_t n_random = n_data;
	vector<double> data(n_data);
	vector<double> theta(n_fixed), u(n_random);
	vector<double> fixed_vec(n_fixed), random_vec(n_random);

	for(size_t i = 0; i < n_data; i++)
	{	data[i]      = double( (i + 1) * (i + 1) );
		fixed_vec[i] = theta[i] = std::sqrt( double(i + 1) );
		random_vec[i] = u[i] = 0.0;
	}

	// object that is derived from mixed_cppad
	mixed_derived mixed_object(n_fixed, n_random, data);
	mixed_object.initialize(theta, u);

	// number of non-zeros in Hessian cross terms
	ok &= mixed_object.hes_cross_row_.size() == n_random;
	ok &= mixed_object.hes_cross_col_.size() == n_random;

	// compute Hessian cross terms
	vector<double> w(1), val_out(n_random), both(n_fixed + n_random);
	w[0] = 1.0;
	CppAD::vector< std::set<size_t> > not_used;
	mixed_object.pack(fixed_vec, random_vec, both);
	mixed_object.ran_like_fun_.SparseHessian(
		both,
		w,
		not_used,
		mixed_object.hes_cross_row_,
		mixed_object.hes_cross_col_,
		val_out,
		mixed_object.hes_cross_work_
	);

	CppAD::vector<size_t>& row(mixed_object.hes_cross_row_);
	CppAD::vector<size_t>& col(mixed_object.hes_cross_col_);

	// check Hessian
	for(size_t k = 0; k < n_random; k++)
	{	ok     &= row[k] >= n_fixed;
		ok     &= col[k] < n_fixed;
		size_t i = row[k] - n_fixed;
		size_t j = col[k];
		ok      &= (i == j);
		//
		double sigma3 = fixed_vec[i] * fixed_vec[i] * fixed_vec[i];
		double check  = 2.0 * (data[i] - random_vec[i])  / sigma3;
		ok              &= abs( val_out[k] / check - 1.0) <= eps;
	}
	for(size_t k = 1; k < n_random; k++)
	{	ok &= col[k-1] <= col[k];
		if( col[k-1] == col[k] )
			ok &= row[k-1] < row[k];
	}
	return ok;
}
// END C++
