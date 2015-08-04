// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */

# include <cppad/cppad.hpp>
# include <gsl/gsl_randist.h>
# include <gsl/gsl_linalg.h>
# include <dismod_at/approx_mixed.hpp>
# include <dismod_at/manage_gsl_rng.hpp>

// J. Andrew Royle, Biometrics 60, 108-115 March 2004,
// N-Mixture Models for Estimating Population Size
// from Spatially Replicated Counts.


namespace { // BEGIN_EMPTY_NAMESPACE

using CppAD::vector;
using std::exp;
using std::log;

// simulate covariates, x, and data, y
void simulate(
	size_t                 I     ,
	size_t                 T     ,
	const vector<double>&  theta ,
	vector<size_t>&        y     )
{	assert( theta.size() == 2 );
	assert( y.size() == I * T );
	//
	gsl_rng* rng = dismod_at::get_gsl_rng();
	//
	// simulate population sizes and number of captures
	double p      = theta[0];
	double lambda = theta[1];
	for(size_t i = 0; i < I; i++)
	{	size_t N = gsl_ran_poisson(rng, lambda);
		for(size_t t = 0; t < T; t++)
			y[ i * T + t ]   = gsl_ran_binomial(rng, p, N);
	}
	return;
}

// approx_mixed derived class
class approx_derived : public dismod_at::approx_mixed {
private:
	const size_t          K_;      // maximum size of population
	const size_t          I_;      // number of locations
	const size_t          T_;      // number of measurements at each location
	const vector<size_t>& y_;      // reference to data values
	//
	// effectively const
	vector<size_t>        M_;      // maximum of y[ i*T + t] .w.r.t t
	vector<double>        logfac_; // logfac_[k] = log( k ! )
// ------------------------------------------------------------------------
public:
	// constructor
	approx_derived(size_t K, size_t I, size_t T, vector<size_t>&  y)
		:
		dismod_at::approx_mixed(2, 0) , // n_fixed = 2, n_random = 0
		K_(K)   ,
		I_(I)   ,
		T_(T)   ,
		y_(y)
	{	assert( y.size() == I * T );
		// set M_
		M_.resize(I);
		for(size_t i = 0; i < I; i++)
		{	M_[i] = 0;
			for(size_t t = 0; t < T; t++)
				M_[i] = std::max( M_[i], y[ i * T + t] );
			assert( M_[i] < K );
		}
		logfac_.resize(K+1);
		logfac_[0]  = 0.0;
		logfac_[1]  = 0.0;
		for(size_t k = 2; k <= K; k++)
			logfac_[k] = log( double(k) ) + logfac_[k-1];
	}
	// implementaion of fix_like
	template <class Float>
	vector<Float> implement_fix_like(const vector<Float>&  theta)
	{	vector<Float> vec(1);
		Float eps( 10.0 * std::numeric_limits<double>::epsilon() );
		//  ------------------------------------------------------------
		// log [ p(y | theta) ]
		//  ------------------------------------------------------------
		Float p      = theta[0];
		Float lambda = theta[1];
		vec[0]       = Float(0.0);
		for(size_t i = 0; i < I_; i++)
		{	Float sum_prob = Float(0.0);
			for(size_t k = M_[i]; k < K_; k++)
			{	// log probability for k given lambda
				Float sum_log = log( lambda + eps ) * Float(k) - lambda;
				sum_log      -= logfac_[k];
				//
				for(size_t t = 0; t < T_; t++)
				{	// data at loation i and time t
					size_t yit = y_[ i * T_ + t];
					//
					// log (k choose yit) - log(k!)
					sum_log += Float(
						logfac_[k] - logfac_[yit] - logfac_[k - yit]
					);
					//
					// log ( p^yit )
					sum_log += Float( yit ) * log(p + eps);
					//
					// log [ ( 1 - p^yit )^(N - yit) ]
					sum_log += Float(k - yit) * log( Float(1.0 + eps) - p );
					//
				}
				sum_prob += exp( sum_log );
			}
			vec[0] -= log( sum_prob );
		}
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
	{	std::cerr << "Error: " << error_message << std::endl;
		std::exit(1);
	}
	//
	virtual void warning(const std::string& warning_message)
	{	std::cerr << "Warning: " << warning_message << std::endl;
	}
};
} // END_EMPTY_NAMESPACE

bool n_mixture(void)
{	bool ok = true;
	size_t n_fixed = 2;
	size_t random_seed = dismod_at::new_gsl_rng(0);

	// simulation parameters
	size_t I = 300; // I = 50, T =  10 in paper
	size_t T = 30;
	vector<double> theta_sim(n_fixed);
	theta_sim[0] =   0.25;      // probability of capture
	theta_sim[1] =   5.;        // mean population size

	// simulate data
	vector<size_t> y(I * T);
	simulate(I, T, theta_sim, y);

	// maximum population size
	double lambda = theta_sim[1];
	double sigma  = std::sqrt( lambda );
	size_t K      = size_t( lambda + 5.0 * sigma) + 1;

	// create derived object
	approx_derived approx_object(K, I, T, y);

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
	// limits on mean population size
	theta_lower[1] = 0.0;
	theta_upper[1] = K;

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
	// simulated and fit values are very different
	for(size_t j = 0; j < n_fixed; j++)
	{	// std::cout << theta_out[j] / theta_sim[j] - 1.0 << std::endl;
		ok &= std::fabs( theta_out[j] / theta_sim[j] - 1.0 ) < 1e-1;
	}
	//
	if( ! ok )
		std::cout << "random_seed = " << random_seed << std::endl;
	//
	dismod_at::free_gsl_rng();
	return ok;
}