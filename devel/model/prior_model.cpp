// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-18 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin prior_model_ctor$$
$spell
	pack_vec
	CppAD
	const
	subvectors
	std
	struct
	fabs
	var
$$

$section Construct Fixed Negative Log-Likelihood Object$$

$head Syntax$$
$codei%prior_model %prior_object%(
	%pack_object%, %var2prior%, %age_table%, %time_table%, %prior_table%, %s_info_vec%
)%$$

$head prior_object$$
This object has prototype
$codei%
	prior_model %prior_object%
%$$

$head pack_object$$
This argument has prototype
$codei%
	const pack_info& %pack_object%
%$$
and is the $cref pack_info$$ information corresponding to the
$cref model_variables$$.

$head var2prior$$
This argument has prototype
$codei%
	const pack_prior& %var2prior%
%$$
and is the $cref pack_prior$$ information corresponding to the
$cref model_variables$$.

$head age_table$$
This argument has prototype
$codei%
	const CppAD::vector<double>&  %age_table%
%$$
and is the $cref/age_table/get_age_table/age_table/$$.

$head time_table$$
This argument has prototype
$codei%
	const CppAD::vector<double>&  %time_table%
%$$
and is the $cref/time_table/get_time_table/time_table/$$.

$head prior_table$$
This argument has prototype
$codei%
	const CppAD::vector<prior_struct>&  %prior_table%
%$$
and is the $cref/prior_table/get_prior_table/prior_table/$$.
Only to following fields are used
$cref/density_id/prior_table/density_id/$$,
$cref/mean/prior_table/mean/$$,
$cref/std/prior_table/std/$$,
$cref/eta/prior_table/eta/$$,
$cref/nu/prior_table/nu/$$.

$head s_info_vec$$
This argument has prototype
$codei%
	const CppAD::vector<smooth_info>& %s_info_vec%
%$$
For each $cref/smooth_id/smooth_table/smooth_id/$$,
$codei%
	%s_info_vec%[ %smooth_id% ]
%$$
is the corresponding $cref smooth_info$$ information.

$end
*/

# include <utility>
# include <dismod_at/prior_model.hpp>
# include <dismod_at/residual_density.hpp>
# include <dismod_at/prior_model.hpp>
# include <dismod_at/smooth_info.hpp>
# include <dismod_at/ad_types.hpp>
# include <dismod_at/null_int.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

prior_model::prior_model(
	const pack_info&                       pack_object     ,
	const pack_prior&                      var2prior       ,
	const CppAD::vector<double>&           age_table       ,
	const CppAD::vector<double>&           time_table      ,
	const CppAD::vector<prior_struct>&     prior_table     ,
	const CppAD::vector<smooth_info>&      s_info_vec      )
:
pack_object_(pack_object)  ,
var2prior_(var2prior)      ,
age_table_(age_table)      ,
time_table_(time_table)    ,
prior_table_(prior_table)  ,
s_info_vec_(s_info_vec)
{ }

// private
template <class Float>
residual_struct<Float> prior_model::log_prior(
	const prior_struct&   prior      ,  // prior for this residual
	const Float&          mulstd     ,  // multiplies prior std
	const Float&          z          ,  // first random variable
	const Float&          y          ,  // second random variable
	size_t                index      ,  // the index for this residual
	bool                  difference ) const // is this a difference residual
{	assert ( 0 <= prior.density_id  );
	assert ( prior.density_id < number_density_enum );

	density_enum density = density_enum(prior.density_id);
	Float        mu      = Float(prior.mean);
	Float        delta   = mulstd * Float(prior.std);
	Float        eta     = Float(prior.eta);
	Float        nu      = Float(prior.nu);
	return residual_density(
		z, y, mu, delta, density, eta, nu, index, difference
	);
}

/*
------------------------------------------------------------------------------
$begin prior_fixed_effect$$
$spell
	vec
	Cpp
	struct
	const
	var
$$

$section Evaluate Fixed Negative Log-Likelihood for the Fixed Effects$$

$head Syntax$$
$icode%residual_vec% = %prior_object%.fixed(%pack_vec%)%$$

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$ or one of the $cref ad_types$$.

$head prior_object$$
This object has prototype
$codei%
	const prior_model %prior_object%
%$$
see $cref/prior_object constructor/prior_model_ctor/prior_object/$$.

$head pack_vec$$
This vector has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is all the $cref model_variables$$ in the order
specified by $cref pack_info$$.

$head residual_vec$$
The return value has prototype
$codei%
	CppAD::vector< residual_struct<%Float%> > %residual_vec%
%$$
The size of $icode residual$$ is not equal to the number of fixed effects
because there are priors on smoothing differences as well as values.
The order of the residuals is unspecified.

$subhead index$$
The $cref/index/residual_density/index/$$ for each residual is
$codei%3 * %var_id% + %k%$$ where
$cref/var_id/var_table/var_id/$$ is the variable index,
$icode%k% = 0%$$ for value priors,
$icode%k% = 1%$$ for age difference priors, and
$icode%k% = 2%$$ for time difference priors.

$head p(theta)$$
The log of the fixed negative log-likelihood for the
$cref/fixed effects/model_variables/Fixed Effects, theta/$$,
$latex \log \; \B{p}( \theta )$$,
is the sum of all the log densities corresponding to the
$cref/residuals/residual_density/$$.


$children%example/devel/model/prior_fixed_xam.cpp
%$$
$head Example$$
The file $cref prior_fixed_xam.cpp$$ contains an example and test
of using this routine.

$end
*/
template <class Float>
CppAD::vector< residual_struct<Float> >
prior_model::fixed(const CppAD::vector<Float>& pack_vec ) const
{	Float nan = Float( std::numeric_limits<double>::quiet_NaN() );
	//
	// initialize the log of the fixed negative log-likelihood as zero
	CppAD::vector< residual_struct<Float> > residual_vec;
	assert( residual_vec.size() == 0 );
	//
	// for computing one residual
	residual_struct<Float> residual;

	// n_var
	assert( pack_vec.size()   == pack_object_.size() );
	assert( var2prior_.size() == pack_object_.size() );
	size_t n_var = pack_vec.size();

	for(size_t var_id = 0; var_id < n_var; ++var_id)
	{	// This variable value
		Float y = pack_vec[var_id];
		//
		// prior information
# ifndef NDEBUG
		double const_value    = var2prior_.const_value(var_id);
# endif
		size_t smooth_id      = var2prior_.smooth_id(var_id);
		size_t value_prior_id = var2prior_.value_prior_id(var_id);
		size_t dage_prior_id  = var2prior_.dage_prior_id(var_id);
		size_t dtime_prior_id = var2prior_.dtime_prior_id(var_id);
		bool   fixed_effect   = var2prior_.fixed_effect(var_id);
		//
		if( smooth_id == DISMOD_AT_NULL_SIZE_T )
		{	// standard deviation multipliers are fixed effects and do not
			// have a smoothing, hence the following
			assert( fixed_effect );
			assert( std::isnan(const_value) );
			assert( dage_prior_id  == DISMOD_AT_NULL_SIZE_T );
			assert( dtime_prior_id == DISMOD_AT_NULL_SIZE_T );
			//
			// value prior
			const prior_struct& prior = prior_table_[value_prior_id];
			//
			// no standard deviation multiplier for this prior
			Float mulstd = Float(1.0);
			//
			// this is not a difference prior
			bool  difference  = false;
			Float z           = nan;
			//
			// index used for value priors
			size_t index              = 3 * var_id + 0;
			//
			// negative log-likelihood of value prior for this multiplier
			residual  = log_prior(prior, mulstd, z, y, index, difference);
			if( residual.density != uniform_enum )
				residual_vec.push_back(residual);
		}
		else if( fixed_effect )
		{	if( value_prior_id != DISMOD_AT_NULL_SIZE_T )
			{	const prior_struct& prior = prior_table_[value_prior_id];
				bool   difference = false;
				Float  z          = nan;
				size_t k          = 0;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
			if( dage_prior_id != DISMOD_AT_NULL_SIZE_T )
			{	const prior_struct& prior = prior_table_[dage_prior_id];
				bool   difference = true;
				Float  z          = pack_vec[var2prior_.dage_var_id(var_id)];

				size_t k          = 1;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
			if( dtime_prior_id != DISMOD_AT_NULL_SIZE_T )
			{	const prior_struct& prior = prior_table_[dtime_prior_id];
				bool   difference = true;
				Float  z          = pack_vec[var2prior_.dtime_var_id(var_id)];
				size_t k          = 2;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
		}
	}
	return residual_vec;
}
/*
------------------------------------------------------------------------------
$begin prior_random_effect$$
$spell
	vec
	Cpp
	struct
	const
	var
$$

$section Evaluate Fixed Negative Log-Likelihood for the Random Effects$$

$head Syntax$$
$icode%residual_vec% = %prior_object%.random(%pack_vec%)%$$

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$ or one of the $cref ad_types$$.

$head prior_object$$
This object has prototype
$codei%
	const prior_model %prior_object%
%$$
see $cref/prior_object constructor/prior_model_ctor/prior_object/$$.

$head pack_vec$$
This vector has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is all the $cref model_variables$$ in the order
specified by $cref pack_info$$.

$head residual_vec$$
The return value has prototype
$codei%
	CppAD::vector< residual_struct<%Float%> > %residual_vec%
%$$
The size of $icode residual$$ is not equal to the number of random effects
because there are priors on smoothing differences as well as values.
The order of the residuals is unspecified (at this time).

$subhead index$$
The $cref/index/residual_density/index/$$ for each residual is
$codei%3 * %var_id% + %k%$$ where
$cref/var_id/var_table/var_id/$$ is the variable index,
$icode%k% = 0%$$ for value priors,
$icode%k% = 1%$$ for age difference priors, and
$icode%k% = 2%$$ for time difference priors.

$head p(u|theta)$$
The log of the fixed negative log-likelihood for the
and $cref/random effects/model_variables/Random Effects, u/$$
given the fixed effects,
$latex \log \; \B{p}( u | \theta )$$,
is the sum of the all the log densities corresponding to the
$cref/residuals/residual_density/$$.


$children%example/devel/model/prior_random_xam.cpp
%$$
$head Example$$
The file $cref prior_random_xam.cpp$$ contains an example and test
of using this routine.

$end
*/
template <class Float>
CppAD::vector< residual_struct<Float> >
prior_model::random(const CppAD::vector<Float>& pack_vec ) const
{	Float nan = Float( std::numeric_limits<double>::quiet_NaN() );
	//
	// initialize the log of the fixed negative log-likelihood as zero
	CppAD::vector< residual_struct<Float> > residual_vec;
	assert( residual_vec.size() == 0 );
	//
	// for computing one residual
	residual_struct<Float> residual;

	// n_var
	assert( pack_vec.size()   == pack_object_.size() );
	assert( var2prior_.size() == pack_object_.size() );
	size_t n_var = pack_vec.size();

	for(size_t var_id = 0; var_id < n_var; ++var_id)
	{	// This variable value
		Float y = pack_vec[var_id];
		//
		// prior information
		size_t smooth_id      = var2prior_.smooth_id(var_id);
		size_t value_prior_id = var2prior_.value_prior_id(var_id);
		size_t dage_prior_id  = var2prior_.dage_prior_id(var_id);
		size_t dtime_prior_id = var2prior_.dtime_prior_id(var_id);
		bool   fixed_effect   = var2prior_.fixed_effect(var_id);
		//
		if( ! fixed_effect )
		{	assert( smooth_id != DISMOD_AT_NULL_SIZE_T );
			//
			if( value_prior_id != DISMOD_AT_NULL_SIZE_T )
			{
# ifndef NDEBUG
				double const_value    = var2prior_.const_value(var_id);
				assert( std::isnan(const_value)  );
# endif
				const prior_struct& prior = prior_table_[value_prior_id];
				bool   difference = false;
				Float  z          = nan;
				size_t k          = 0;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
			if( dage_prior_id != DISMOD_AT_NULL_SIZE_T )
			{	const prior_struct& prior = prior_table_[dage_prior_id];
				bool   difference = true;
				Float  z          = pack_vec[var2prior_.dage_var_id(var_id)];

				size_t k          = 1;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
			if( dtime_prior_id != DISMOD_AT_NULL_SIZE_T )
			{	const prior_struct& prior = prior_table_[dtime_prior_id];
				bool   difference = true;
				Float  z          = pack_vec[var2prior_.dtime_var_id(var_id)];
				size_t k          = 2;
				size_t index      = 3 * var_id + k;
				size_t offset     = pack_object_.mulstd_offset(smooth_id, k);
				Float mulstd      = Float(1.0);
				if( offset != DISMOD_AT_NULL_SIZE_T )
					mulstd        = pack_vec[offset];
				//
				residual = log_prior(prior, mulstd, z, y, index, difference);
				if( residual.density != uniform_enum )
					residual_vec.push_back(residual);
			}
		}
	}
	return residual_vec;
}

# define DISMOD_AT_INSTANTIATE_PRIOR_DENSITY(Float)                       \
	template                                                              \
	CppAD::vector< residual_struct<Float> > prior_model::fixed<Float>(  \
		const CppAD::vector<Float>&   pack_vec                            \
	) const;                                                              \
	template                                                              \
	CppAD::vector< residual_struct<Float> > prior_model::random<Float>( \
		const CppAD::vector<Float>&   pack_vec                            \
	) const;

// instantiations
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( double )
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( a1_double )
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( a2_double )
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( a3_double )

} // BEGIN_DISMOD_AT_NAMESPACE
