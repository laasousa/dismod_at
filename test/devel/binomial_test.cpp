// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */

# include <gsl/gsl_randist.h>
# include <cppad/vector.hpp>
# include <dismod_at/approx_mixed.hpp>
# include <dismod_at/manage_gsl_rng.hpp>

namespace { // BEGIN_EMPTY_NAMESPACE

using CppAD::vector;
using std::exp;
using std::log;
using std::endl;

// simulate covariates, x, and data, y
void simulate(
	size_t                 N     ,
	size_t                 I     ,
	const vector<double>&  theta ,
	vector<size_t>&        y     )
{	assert( theta.size() == 1 );
	assert( y.size() == I );
	//
	gsl_rng* rng = dismod_at::get_gsl_rng();
	//
	// simulate population sizes
	double p  = theta[0];
	for(size_t i = 0; i < I; i++)
		y[ i ] = gsl_ran_binomial(rng, p, N);
	return;
}

// approx_mixed derived class
class approx_derived : public dismod_at::approx_mixed {
private:
	const size_t          N_;      // size of population
	const vector<size_t>& y_;      // reference to data values
	vector<double>        logfac_; // logfac_[k] = log( k ! )
// ------------------------------------------------------------------------
public:
	// constructor
	approx_derived(size_t N, vector<size_t>&  y)
		:
		dismod_at::approx_mixed(1, 0) , // n_fixed = 1, n_random = 0
		N_(N)                         ,
		y_(y)
	{	logfac_.resize(N+1);
		logfac_[0]  = 0.0;
		logfac_[1]  = 0.0;
		for(size_t k = 2; k <= N; k++)
			logfac_[k] = log( double(k) ) + logfac_[k-1];
	}
	// implementaion of ran_like
	template <class Float>
	vector<Float> implement_fix_like(const vector<Float>&  theta)
	{	vector<Float> vec(1);
		Float eps( 10.0 * std::numeric_limits<double>::epsilon() );
		//  ------------------------------------------------------------
		// log [ p(y | theta) ]
		//  ------------------------------------------------------------
		Float p      = theta[0];
		size_t I     = y_.size();
		Float sumlog = Float(0.0);
		Float term;
		for(size_t i = 0; i < I; i++)
		{	double yi = y_[i];
			// log (N choose yi)
			sumlog += Float( logfac_[N_] - logfac_[yi] - logfac_[N_ - yi] );
			// log ( p^yi )
			sumlog += Float( yi ) * log(p + eps);
			// log [ ( 1 - p^yi )^(N - yi) ]
			sumlog += Float(N_ - yi) * log( Float(1.0 + eps) - p );
		}
		vec[0] = - sumlog;
		return vec;
	}
// ------------------------------------------------------------------------
public:
	virtual vector<a2_double> ran_like(
		const vector<a2_double>& fixed_vec  ,
		const vector<a2_double>& random_vec )
	{	return vector<a2_double>(0); } // empty vector
	//
	virtual vector<a1_double> fix_like(
		const vector<a1_double>& fixed_vec  )
	{	return implement_fix_like<a1_double>(fixed_vec); }
	//
	virtual vector<a1_double> constraint(
		const vector<a1_double>& fixed_vec  )
	{	return vector<a1_double>(0); } // empty vector
	//
	virtual void fatal_error(const std::string& error_message)
	{	std::cerr << "Error: " << error_message << endl;
		std::exit(1);
	}
	//
	virtual void warning(const std::string& warning_message)
	{	std::cerr << "Warning: " << warning_message << endl;
	}
};
} // END_EMPTY_NAMESPACE

bool binomial_test(void)
{	bool ok = true;
	size_t n_fixed = 1;
	size_t random_seed = dismod_at::new_gsl_rng(0);

	// simulation parameters
	size_t N = 25;
	size_t I = 5000;
	vector<double> theta_sim(n_fixed);
	theta_sim[0] =   0.25;      // constant term in covariate model

	// simulate data
	vector<size_t> y(I);
	simulate(N, I, theta_sim, y);

	// create derived object
	approx_derived approx_object(N, y);

	// initialize point to start optimization at
	vector<double> theta_in( n_fixed ), u_in(0);
	for(size_t j = 0; j < n_fixed; j++)
		theta_in[j] = theta_sim[j];
	approx_object.initialize(theta_in, u_in);

	// lower and upper limits
	vector<double> constraint_lower, constraint_upper;
	vector<double> theta_lower(n_fixed), theta_upper(n_fixed);
	// limits on p
	theta_lower[0] = 0.0;
	theta_upper[0] = 1.0;

	// optimize the fixed effects
	std::string options =
		"Integer print_level               0\n"
		"String  sb                        yes\n"
		"String  derivative_test           second-order\n"
		"String  derivative_test_print_all yes\n"
		"Numeric tol                       1e-8\n"
	;
	vector<double> theta_out = approx_object.optimize_fixed(
		options,
		theta_lower,
		theta_upper,
		constraint_lower,
		constraint_upper,
		theta_in,
		u_in
	);
	for(size_t j = 0; j < n_fixed; j++)
	{	// std::cout << endl << theta_out[j] / theta_sim[j] - 1.0 << endl;
		ok &= std::fabs( theta_out[j] / theta_sim[j] - 1.0) < 1e-2;
	}
	//
	if( ! ok )
		std::cout << endl << "random_seed = " << random_seed << endl;
	dismod_at::free_gsl_rng();
	return ok;
}