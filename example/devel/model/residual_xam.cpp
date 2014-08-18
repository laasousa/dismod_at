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
$begin residual_xam.cpp$$
$spell
	interp
	xam
$$

$section C++ residual: Example and Test$$
$index example, C++ residual$$
$index residual, C++ example$$

$code
$verbatim%example/devel/model/residual_xam.cpp%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <limits>
# include <dismod_at/include/data_model.hpp>

bool residual_xam(void)
{	bool   ok = true;
	size_t i, j, k;
	using CppAD::abs;	
	using CppAD::vector;	
	using std::cout;
	typedef CppAD::AD<double> Float;
	Float eps = CppAD::numeric_limits<Float>::epsilon() * 100;
	//
	// ode_step_size
	double ode_step_size = 30.0;
	//
	// age_table 
	// (make sure that ode grid lands on last age table point)
	double age = 0.0;
	vector<double> age_table;
	age_table.push_back(age);
	while( age < 100. )
	{	age += ode_step_size;
		age_table.push_back(age);
	}
	size_t n_age_table = age_table.size();
	double age_min     = age_table[0];
	double age_max     = age_table[n_age_table - 1];
	//
	// time_table 
	// (make sure that ode grid lands on last time table point)
	double time = 1980.0;
	vector<double> time_table;
	time_table.push_back(time);
	while( time < 2020.0 )
	{	time += ode_step_size;
		time_table.push_back(time);
	}
	size_t n_time_table = time_table.size();
	double time_min     = time_table[0];
	double time_max     = time_table[n_time_table - 1];

	// age and time smoohting grid indices
	size_t n_age_si   = 3;
	size_t n_time_si  = 2;
	vector<size_t> age_id(n_age_si), time_id(n_time_si);
	age_id[0]   = 0;
	age_id[1]   = n_age_table / 2;
	age_id[2]   = n_age_table - 1;
	time_id[0]  = 0;
	time_id[1]  = n_time_table - 1;
	//
	// w_info_vec
	// weight value should not matter when constant
	size_t n_si = n_age_si * n_time_si;
	vector<double> weight(n_si);
	for(k = 0; k < n_si; k++)
		weight[k] = 0.5;
	dismod_at::weight_info w_info(age_id, time_id, weight);
	vector<dismod_at::weight_info> w_info_vec(1);
	w_info_vec[0] = w_info;
	//
	// s_info_vec
	vector<dismod_at::smooth_info> s_info_vec(2);
	size_t mulstd_value = 1, mulstd_dage = 1, mulstd_dtime = 1;
	for(size_t smooth_id = 0; smooth_id < 2; smooth_id++)
	{	vector<size_t> age_id_tmp;
		if( smooth_id == 0 )
		{	n_si       = n_age_si * n_time_si;
			age_id_tmp = age_id;
		}
		else
		{	n_si = n_time_si;
			age_id_tmp.resize(1);
			age_id_tmp[0] = 0;
		}
		//
		vector<size_t> value_like_id(n_si), 
			dage_like_id(n_si), dtime_like_id(n_si);
		dismod_at::smooth_info s_info(
			age_id_tmp, time_id, value_like_id, dage_like_id, dtime_like_id,
			mulstd_value, mulstd_dage, mulstd_dtime
		);
		s_info_vec[smooth_id] = s_info;
	}
	//
	// integrand_table
	size_t n_integrand = dismod_at::number_integrand_enum;
	vector<dismod_at::integrand_struct> integrand_table(n_integrand);
	for(i = 0; i < n_integrand; i++)
	{	integrand_table[i].integrand = dismod_at::integrand_enum(i);
		integrand_table[i].eta       = 1e-4;
	}
	//
	// n_age_ode
	size_t n_age_ode     =  1;
	while( age_min + (n_age_ode-1) * ode_step_size < age_max )
			n_age_ode++; 
	//
	// n_time_ode
	size_t n_time_ode     =  1;
	while( time_min + (n_time_ode-1) * ode_step_size < time_max )
			n_time_ode++; 
	//
	// node_table:    
	CppAD::vector<dismod_at::node_struct> node_table(1);
	node_table[0].parent = -1;
	//
	// parent_node_id
	size_t parent_node_id = 0;
	//
	// data_table
	vector<dismod_at::data_struct> data_table(2);
	//
	// parent node, time and age integrantion.
	vector<double> x(0);
	for(size_t data_id = 0; data_id < 2; data_id++) 
	{
		data_table[data_id].integrand_id = dismod_at::mtother_enum;
		data_table[data_id].node_id    = 0;
		data_table[data_id].weight_id  = 0;
		data_table[data_id].age_lower  = 35.0;
		data_table[data_id].age_upper  = 55.0;
		data_table[data_id].time_lower = 1990.0;
		data_table[data_id].time_upper = 2000.0;
		data_table[data_id].meas_value = ( 50. * 1995) / (100.0 * 2000.0);
		data_table[data_id].meas_std   = data_table[data_id].meas_value / 10.;
		if( data_id == 0 )
			data_table[data_id].density_id = dismod_at::gaussian_enum;
		else
			data_table[data_id].density_id = dismod_at::log_gaussian_enum;
		data_table[data_id].x          = x;
	}
	//
	// data_model
	dismod_at::data_model dm(
		parent_node_id,
		n_age_ode,
		n_time_ode,
		ode_step_size,
		age_table,
		time_table,
		integrand_table,
		node_table,
		data_table,
		w_info_vec,
		s_info_vec
	);
	//
	// smooth_table
	size_t n_child        = 0;
	size_t pini_smooth_id = 1; // only one age 
	vector<dismod_at::smooth_struct> smooth_table(s_info_vec.size());
	for(size_t smooth_id = 0; smooth_id < s_info_vec.size(); smooth_id++)
	{	smooth_table[smooth_id].n_age  = s_info_vec[smooth_id].age_size();
		smooth_table[smooth_id].n_time = s_info_vec[smooth_id].time_size();
	}
	// mulcov_table
	vector<dismod_at::mulcov_struct> mulcov_table(0);
	// rate_table
	vector<dismod_at::rate_struct>   rate_table(dismod_at::number_rate_enum);
	for(size_t rate_id = 0; rate_id < rate_table.size(); rate_id++)
	{	rate_table[rate_id].parent_smooth_id = 0;
		rate_table[rate_id].child_smooth_id = 0;
	}
	// var_info
	dismod_at::pack_var var_info(
		n_integrand, n_child, pini_smooth_id,
		smooth_table, mulcov_table, rate_table
	);
	//
	// var_vec
	vector<Float> var_vec( var_info.size() );
	dismod_at::pack_var::subvec_info info;
	for(size_t child_id = 0; child_id <= n_child; child_id++)
	{	info = var_info.rate_info(dismod_at::omega_enum, child_id);
		dismod_at::smooth_info& s_info = s_info_vec[info.smooth_id];
		for(i = 0; i < s_info.age_size(); i++)
		{	double age = age_table[ s_info.age_id(i) ];
			for(j = 0; j < s_info.time_size(); j++)
			{	double time    = time_table[ s_info.time_id(j) ];
				size_t index   = info.offset + i * s_info.time_size() + j; 
				var_vec[index] = age * time / (age_max*time_max);
			}
		}
	}
	// check results
	for(size_t data_id = 0; data_id < data_table.size(); data_id++)
	{	Float avg         = dm.avg_no_ode(data_id, var_info, var_vec);
		Float wres        = dm.residual(data_id, var_info, var_vec, avg);
		double delta      = data_table[data_id].meas_std;
		double y          = data_table[data_id].meas_value;
		double eta        = 1e-4;
		size_t density_id = data_table[data_id].density_id;
		bool log_density = density_id == dismod_at::log_gaussian_enum;
		log_density     |= density_id == dismod_at::log_laplace_enum;
		Float check;
		if( log_density )
			check = (log(y+eta) - log(avg+eta)) / log(1.0 + delta/(y+eta));
		else
			check = (y - avg) / delta;
		ok  &= abs( 1.0 - wres / check ) <= eps;
		/*
		if( data_id == 0 )
			cout << "Debugging" << std::endl; 
		cout << "wres = " << wres; 
		cout << ", check = " << check; 
		cout << ", relerr    = " << 1.0 - wres / check  << std::endl;
		*/
	}

	return ok;
}
// END C++
