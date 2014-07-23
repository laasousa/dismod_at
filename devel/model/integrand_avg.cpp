// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-14 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the 
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin integrand$$

$section Integrand Average For Each Data Point$$

$head Syntax$$
$codei%dismod_at::integrand_avg %avg_int%(
	%wg_vec%, 
	%data_table%, 
	%integrand_table%,
	%age_table%,
	%time_table%,
	%n_age_ode%,
	%n_time_ode%,
	%ode_step_size%
)%$$
$codei%
%data_mean% = avg_int%.compute(
	%data_id%, %iota%, %rho%, %chi%, %omega%, %S%, %C%
)%$$

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$, $code CppAD::AD<double>$$

$head integrand_avg$$
This constructs an object that can evaluate the 
$cref/average integrand/model_data_mean/Average Integrand$$ 
for each data point. 



$end
-----------------------------------------------------------------------------
*/
# include <dismod_at/dismod_at.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

integrand_avg::integrand_avg(
	const CppAD::vector<weight_grid>&    wg_vec          ,                 
	const CppAD::vector<data_struct>&    data_table      ,
	const CppAD::vector<integrand_enum>& integrand_table ,
	const CppAD::vector<double>&         age_table       ,
	const CppAD::vector<double>&         time_table      ,
	size_t                               n_age_ode       ,
	size_t                               n_time_ode      ,
	double                               ode_step_size   )
:
n_age_ode_(n_age_ode)         ,
n_time_ode_(n_time_ode)       ,
ode_step_size_(ode_step_size)
{	using std::string;
	size_t i, j, k;
	assert( n_age_ode > 1 );
	assert( n_time_ode > 1 );

	// dimenson data_info_
	size_t n_data = data_table.size();
	data_info_.resize( n_data );

	string table_name = "data";
	//
	double age_min    = age_table[0];
	double age_max    = age_table[ age_table.size() - 1 ];
	double time_min   = time_table[0];
	double time_max   = time_table[ time_table.size() - 1 ];
	//
	assert( age_max  <= age_min  + n_age_ode * ode_step_size );
	assert( time_max <= time_min + n_time_ode * ode_step_size );
	//
	for(size_t data_id = 0; data_id < n_data; data_id++)
	{
		// integrand
		size_t  integrand_id     = data_table[data_id].integrand_id;
		integrand_enum integrand = integrand_table[integrand_id];
		// age limits
		double age_lower  = data_table[data_id].age_lower;
		double age_upper  = data_table[data_id].age_upper;
		if( age_lower < age_min )
		{	string msg = "age_lower is less than minimum age in age table";
			table_error_exit(table_name, data_id, msg);
		}
		if( age_upper < age_lower )
		{	string msg = "age_upper is less than age_lower";
			table_error_exit(table_name, data_id, msg);
		}
		if( age_max < age_upper )
		{	string msg = "age_upper is greater than maximum age in age table";
			table_error_exit(table_name, data_id, msg);
		}

		// time limits
		double time_lower  = data_table[data_id].time_lower;
		double time_upper  = data_table[data_id].time_upper;
		if( time_lower < time_min )
		{	string msg = "time_lower is less than minimum time in time table";
			table_error_exit(table_name, data_id, msg);
		}
		if( time_upper < time_lower )
		{	string msg = "time_upper is less than time_lower";
			table_error_exit(table_name, data_id, msg);
		}
		if( time_max < time_upper )
		{	string msg = 
				"time_upper is greater than maximum time in time table";
			table_error_exit(table_name, data_id, msg);
		}

		// determine minimum age index and number of age indices
		size_t i_min = 0;
		while(age_min + i_min * ode_step_size < age_lower )
			i_min++;
		i_min = std::min(i_min, n_time_ode - 2);
		size_t n_age = 0;
		while( age_min + (i_min + n_age) * ode_step_size < age_upper )
			n_age++;
		n_age = std::max(n_age, size_t(2));
		assert( i_min + n_age <= n_age_ode );

		// determine minimum time index and number of time indices
		size_t j_min = 0;
		while(time_min + j_min * ode_step_size < time_lower )
			j_min++;
		j_min  = std::min(j_min, n_age_ode - 2);
		size_t n_time = 0;
		while( time_min + (j_min + n_time) * ode_step_size < time_upper )
			n_time++;
		n_time = std::max(n_time, size_t(2));
		assert( j_min + n_time <= n_time_ode );

		// sum of coefficients for each ode grid point within limits
		CppAD::vector<double> c_sum(n_age * n_time);

		// weight grid for this data point
		const weight_grid& wg( wg_vec[ data_table[data_id].weight_id ] );
		size_t i_wg = 0;
		size_t j_wg = 0;

		// loop over all the ode rectangles that are within limits
		CppAD::vector<double> w(4), c(4);
		std::pair<double, double> w_pair, c_pair;
		for(i = 0; i < n_age-1; i++)
		{	double a1 = age_min + (i_min + i) * ode_step_size;
			double a2 = a1 + ode_step_size;
			//
			double b1 = std::max(a1, age_lower);
			double b2 = std::min(a2, age_upper);
			//
			std::pair<double, double> a_pair(a1, a2);
			std::pair<double, double> b_pair(b1, b2);
			for(j = 0; j < n_time-1; j++)
			{	double t1 = time_min + (j_min + j) * ode_step_size;
				double t2 = t1 + ode_step_size;
				//
				double s1 = std::max(t1, time_lower);
				double s2 = std::min(t2, time_upper);
				//
				std::pair<double, double> t_pair(t1, t2);
				std::pair<double, double> s_pair(s1, s2);
				//
				if( age_lower == age_upper && time_lower == time_upper )
				{	assert( n_age = 2 && n_time == 2 );
					double a = age_lower;
					double t = time_lower;
					double d = (a2 - a1) * (t2 - t1);
					c[0]     = (a2 - a)*(t2 - t) / d;
					c[1]     = (a2 - a)*(t - t1) / d;
					c[2]     = (a - a1)*(t2 - t) / d;
					c[3]     = (a - a1)*(t - t1) / d;
				}
				else if( age_lower == age_upper )
				{	assert( n_age == 2 );
					double a = age_lower;
					double d = (a2 - a1);
					w_pair.first = interp_weight(
						a, t1, wg, age_table, time_table, i_wg, j_wg
					);
					w_pair.second = interp_weight(
						a, t2, wg, age_table, time_table, i_wg, j_wg
					);
					c_pair = integrate_1d(t_pair, s_pair, w_pair);
					c[0]   = c_pair.first  * (a2 - a) / d;
					c[1]   = c_pair.second * (a2 - a) / d;
					c[2]   = c_pair.first  * (a - a1) / d;
					c[3]   = c_pair.second * (a - a1) / d;
				}
				else if( time_lower == time_upper )
				{	assert( n_time == 2 );
					double t = time_lower;
					double d = (t2 - t1);
					w_pair.first = interp_weight(
						a1, t, wg, age_table, time_table, i_wg, j_wg
					);
					w_pair.second = interp_weight(
						a2, t, wg, age_table, time_table, i_wg, j_wg
					);
					c_pair = integrate_1d(a_pair, b_pair, w_pair);
					c[0]   = c_pair.first  * (t2 - t) / d;
					c[1]   = c_pair.first  * (t - t2) / d;
					c[2]   = c_pair.second * (t2 - t) / d;
					c[3]   = c_pair.second * (t - t2) / d;
				}
				else 
				{	w[0] = interp_weight(
						a1, t1, wg, age_table, time_table, i_wg, j_wg);
					w[1] = interp_weight(
						a1, t2, wg, age_table, time_table, i_wg, j_wg);
					w[2] = interp_weight(
						a2, t1, wg, age_table, time_table, i_wg, j_wg);
					w[3] = interp_weight(
						a2, t2, wg, age_table, time_table, i_wg, j_wg);
					//
					c = integrate_2d(a_pair, t_pair, b_pair, s_pair, w);
				}
				// add this rectangle contribution
				c_sum[     i * n_time + j   ]  += c[0];
				c_sum[     i * n_time + j+1 ]  += c[1];
				c_sum[ (i+1) * n_time + j   ]  += c[2];
				c_sum[ (i+1) * n_time + j+1 ]  += c[3];
			}
		}
		double sum = 0.0;
		for(k = 0; i < n_age * n_time; k++)
			sum += c_sum[k];

		// set the information for this data point
		data_info_[data_id].integrand = integrand;
		data_info_[data_id].i_min     = i_min;
		data_info_[data_id].j_min     = j_min;
		data_info_[data_id].n_age     = n_age; 
		data_info_[data_id].n_time    = n_time; 
		//
		data_info_[data_id].c.resize(n_age * n_time);
		for(k = 0; k < n_age * n_time; k++)
			data_info_[data_id].c[k] = c_sum[k] / sum;
	}
}

template <class Float>
CppAD::vector<Float> integrand_avg::compute(
		const CppAD::vector<size_t>&  data_id  ,
		const CppAD::vector<Float>&   iota     ,
		const CppAD::vector<Float>&   rho      ,
		const CppAD::vector<Float>&   chi      ,
		const CppAD::vector<Float>&   omega    ,
		const CppAD::vector<Float>&   S        ,
		const CppAD::vector<Float>&   C
	) const
{	size_t i, j, k;

	size_t n_ode = n_age_ode_ * n_time_ode_;
	assert( iota.size()  == n_ode );
	assert( rho.size()   == n_ode );
	assert( chi.size()   == n_ode );
	assert( omega.size() == n_ode );
	assert( S.size()     == n_ode );
	assert( C.size()     == n_ode );

	// prevalence for each ode point
	CppAD::vector<Float> P(n_ode);
	for(k = 0; k < n_ode; k++)
		P[k] = C[k] / ( S[k] + C[k]);

	size_t n_avg = data_id.size();
	CppAD::vector<Float> avg(n_avg);

	for(size_t i_avg = 0; i_avg < n_avg; i_avg++)
	{	// constructor information for this data point
		const ode_point& info = data_info_[ data_id[i_avg] ];

		Float sum = 0.0;
		for(i = 0; i < info.n_age; i++)
		{	size_t i_ode = info.i_min + i;
			for(j = 0; j < info.n_time; j++)
			{	size_t j_ode = info.j_min + j;
				k            = i_ode * n_time_ode_ + j_ode;
				double I_k;
				switch( info.integrand )
				{
					case incidence_enum:
					I_k = iota[k]; 
					break;

					case remission_enum:
					I_k = rho[k]; 
					break;

					case mtexcess_enum:
					I_k = chi[k]; 
					break;

					case mtother_enum:
					I_k = omega[k]; 
					break;

					case prevalence_enum:
					I_k = P[k]; 
					break;

					case mtspecific_enum:
					I_k = chi[k] * P[k]; 
					break;

					case mtall_enum:
					I_k = omega[k] + chi[k] * P[k]; 
					break;

					case mtstandard_enum:
					I_k = (omega[k] + chi[k]) / (omega[k] + chi[k] * P[k]); 
					break;

					case mtstandard_enum:
					I_k = (omega[k] + chi[k]) / omega[k];
					break;

					default:
					assert(false);
				}
				sum += info.c[ i * info.n_time + j] * I_k;
			}
		}
		avg[i_avg] = sum;
	}
	return avg;
}

} // END DISMOD_AT_NAMESPACE
