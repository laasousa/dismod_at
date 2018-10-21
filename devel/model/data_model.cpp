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
$begin data_model_ctor$$
$spell
	avgint
	logden
	std
	vec
	const
	enum
	struct
	Cpp
	obj
	covariate
	covariates
	cov
	cv
	eigen
$$

$section Data Model: Constructor$$

$head Syntax$$
$codei%data_model %data_object%(
	%rate_case%,
	%bound_random%,
	%n_covariate%,
	%ode_step_size%,
	%avg_age_grid%,
	%age_table%,
	%time_table%,
	%integrand_table%,
	%prior_table%,
	%subset_object%,
	%subset_cov_value%,
	%w_info_vec%,
	%s_info_vec%,
	%pack_object%,
	%child_object%
)%$$

$head Prototype$$
$srcfile%devel/model/data_model.cpp%
	0%// BEGIN_DATA_MODEL_PROTOTYPE%// END_DATA_MODEL_PROTOTYPE%1
%$$

$head data_object$$
This is the $code data_model$$ object being constructed.

$head rate_case$$
This is the value of
$cref/rate_case/option_table/rate_case/$$ in the option table.

$head bound_random$$
This is the
$cref/bound_random/option_table/Random Only/bound_random/$$.

$head n_covariate$$
This is the number of covariates; i.e., number or rows in
$cref covariate_table$$.

$head ode_step_size$$
This is the $cref/ode_step_size/option_table/ode_step_size/$$.

$head avg_age_grid$$
This is the $cref/average age grid/option_table/avg_age_split/Average Age Grid/$$.

$head age_table$$
This is the $cref/age_table/get_age_table/age_table/$$.

$head time_table$$
This is the $cref/time_table/get_time_table/time_table/$$.

$head integrand_table$$
This is the $cref/integrand_table/get_integrand_table/integrand_table/$$.

$head subset_object$$
This is the sub-sampled version of the data or avgint table; see
$cref/data_subset_obj/data_subset/data_subset_obj/$$,
$cref/avgint_subset_obj/avgint_subset/avgint_subset_obj/$$.

$head subset_cov_value$$
This is the sub-sampled version of the covariates; see
$cref/data_subset_cov_value/data_subset/data_subset_cov_value/$$,
$cref/avgint_subset_cov_value/avgint_subset/avgint_subset_cov_value/$$.
A reference to $icode subset_cov_value$$ is used by $icode data_object$$
(so $icode age_table$$ cannot be deleted for as long as
$icode data_object$$ is used).

$head w_info_vec$$
For each $cref/weight_id/weight_table/weight_id/$$,
$codei%
	%w_info_vec%[ %weight_id% ]
%$$
is the corresponding $cref weight_info$$ information.

$head s_info_vec$$
For each $cref/smooth_id/smooth_table/smooth_id/$$,
$codei%
	%s_info_vec%[ %smooth_id% ]
%$$
is the corresponding $cref smooth_info$$ information.
For each $icode%s_info_vec%[%smooth_id%]%$$ object,
only the following functions are used:
$code age_size$$, $code time_size$$, $code age_id$$, $code time_id$$.

$head pack_object$$
This is the $cref pack_info$$ information corresponding to
the $cref model_variables$$.
A reference to $icode pack_object$$ is used by $icode data_object$$.

$head child_object$$
This is the $cref child_info$$ information corresponding to
parent node, node table, and data table.

$head n_covariate_$$
set to $icode n_covariate$$.

$head ode_step_size_$$
set to $icode ode_step_size$$.

$head pack_object_$$
set to $icode pack_object$$.

$head minimum_meas_cv_$$
Set to a vector with the same size as $icode integrand_table$$ and
$codei%
	%minimum_meas_cv_[%integrand_id%] =
		%integrand_table%[%integrand_id%].minimum_meas_cv
%$$

$head replace_like_called_$$
initialize as false.

$head n_child_$$
set to number of children.

$head data_subset_obj_$$
for each $icode subset_id$$, set $codei%data_subset_obj_[%subset_id%]%$$
fields that are command both data_subset and avgint_subset.

$head child_ran_zero_$$
for each $icode%child% < n_child_%$$,
$codei%child_ran_zero_%[%child%]%$$ is true if all the random effects
for this child are constrained to be zero.

$head data_info_$$
for each $icode subset_id$$, set $codei%data_info_[%subset_id%]%$$
is extra information for each data point.

$head avgint_obj_$$
The $code avg_integrand$$ $cref/constructor/avg_integrand_ctor/$$
is used to create this argument.
The arguments to this constructor are a subset of the
arguments to the $code data_model$$ constructor and are passed through
(with the same name) to the $code avg_integrand$$ constructor.

$end
-----------------------------------------------------------------------------
*/
# include <cppad/mixed/exception.hpp>
# include <dismod_at/min_max_vector.hpp>
# include <dismod_at/data_model.hpp>
# include <dismod_at/child_info.hpp>
# include <dismod_at/get_rate_table.hpp>
# include <dismod_at/solve_ode.hpp>
# include <dismod_at/residual_density.hpp>
# include <dismod_at/a1_double.hpp>
# include <dismod_at/avgint_subset.hpp>
# include <dismod_at/null_int.hpp>
# include <dismod_at/error_exit.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

// destructor
data_model::~data_model(void)
{ }

// BEGIN_DATA_MODEL_PROTOTYPE
template <class SubsetStruct>
data_model::data_model(
	const std::string&                       rate_case       ,
	double                                   bound_random    ,
	size_t                                   n_covariate     ,
	double                                   ode_step_size   ,
	const CppAD::vector<double>&             avg_age_grid    ,
	const CppAD::vector<double>&             age_table       ,
	const CppAD::vector<double>&             time_table      ,
	const CppAD::vector<integrand_struct>&   integrand_table ,
	const CppAD::vector<prior_struct>&       prior_table     ,
	const CppAD::vector<SubsetStruct>&       subset_object   ,
	const CppAD::vector<double>&             subset_cov_value,
	const CppAD::vector<weight_info>&        w_info_vec      ,
	const CppAD::vector<smooth_info>&        s_info_vec      ,
	const pack_info&                         pack_object     ,
	const child_info&                        child_object    )
// END_DATA_MODEL_PROTOTYPE
:
// const
n_covariate_       (n_covariate)                   ,
ode_step_size_     (ode_step_size)                 ,
n_child_           ( child_object.child_size() )   ,
subset_cov_value_  (subset_cov_value)              ,
pack_object_       (pack_object)                   ,
avgint_obj_(
	ode_step_size,
	rate_case,
	avg_age_grid,
	age_table,
	time_table,
	integrand_table,
	w_info_vec,
	s_info_vec,
	pack_object
),
avgstd_obj_(
	ode_step_size,
	avg_age_grid,
	age_table,
	time_table,
	integrand_table,
	w_info_vec,
	s_info_vec,
	pack_object
)
{	assert( bound_random >= 0.0 );
	assert( n_child_ == pack_object.child_size() );
	// ------------------------------------------------------------------------
	//
	using std::string;
	//
	// minimum_meas_cv_
	minimum_meas_cv_.resize( integrand_table.size() );
	for(size_t i = 0; i < integrand_table.size(); ++i)
		minimum_meas_cv_[i] = integrand_table[i].minimum_meas_cv;
	//
	// replace_like_called_: initialize
	replace_like_called_ = false;
	//
	// -----------------------------------------------------------------------
	// data_subset_obj_
	//
	// only set the fileds that are common to data_subset and avgint_subset
	size_t n_subset = subset_object.size();
	data_subset_obj_.resize(n_subset);
	assert( subset_cov_value.size() == n_covariate * n_subset );
	for(size_t i = 0; i < n_subset; i++)
	{	data_subset_obj_[i].original_id  = subset_object[i].original_id;
		data_subset_obj_[i].integrand_id = subset_object[i].integrand_id;
		data_subset_obj_[i].node_id      = subset_object[i].node_id;
		data_subset_obj_[i].weight_id    = subset_object[i].weight_id;
		data_subset_obj_[i].age_lower    = subset_object[i].age_lower;
		data_subset_obj_[i].age_upper    = subset_object[i].age_upper;
		data_subset_obj_[i].time_lower   = subset_object[i].time_lower;
		data_subset_obj_[i].time_upper   = subset_object[i].time_upper;
	}
	// -----------------------------------------------------------------------
	// child_ran_zero_
	double inf = std::numeric_limits<double>::infinity();
	child_ran_zero_.resize(n_child_);
	for(size_t child = 0; child < n_child_; ++child)
	{	child_ran_zero_[child] = true;
		for(size_t rate_id = 0; rate_id < number_rate_enum; ++rate_id)
		{	// check if any random effects for this rate are not constant
			size_t smooth_id = pack_object.rate_info(rate_id, child).smooth_id;
			if( smooth_id != DISMOD_AT_NULL_SIZE_T )
			{	const smooth_info& s_info = s_info_vec[smooth_id];
				size_t             n_age  = s_info.age_size();
				size_t             n_time = s_info.time_size();
				for(size_t i = 0; i < n_age; i++)
				{	for(size_t j = 0; j < n_time; j++)
					{	size_t prior_id    = s_info.value_prior_id(i, j);
						// if prior_id is null then const_value is not null
						if( prior_id != DISMOD_AT_NULL_SIZE_T )
						{
# ifndef NDEBUG
							double lower = prior_table[prior_id].lower;
# endif
							double upper = prior_table[prior_id].upper;
							assert( upper == inf  || upper == lower );
							assert( lower == -inf || upper == lower );
							bool zero    = upper == 0.0;
							zero        |= upper == inf && bound_random == 0.0;
							if( ! zero )
								child_ran_zero_[child] = false;
						}
					}
				}
			}
		}
	}
	// -----------------------------------------------------------------------
	// data_ode_info_
	//
	// has same size as data_subset_obj
	data_info_.resize( n_subset );
	//
	for(size_t subset_id = 0; subset_id < n_subset; subset_id++)
	{	// information for this data point
		size_t original_id = subset_object[subset_id].original_id;

		// integrand as an enum instead of integrand_id
		size_t  integrand_id     = subset_object[subset_id].integrand_id;
		integrand_enum integrand = integrand_table[integrand_id].integrand;

		// child of parent node that this data is associated with
		size_t  child            = child_object.table_id2child(original_id);

		// set the information for this data point
		data_info_[subset_id].integrand = integrand;
		data_info_[subset_id].child     = child;

		// Does this data point depend on the random effects
		// that do not have equal bounds
		bool bound_ran_neq = false;
		if( child < n_child_ && bound_random > 0.0 )
		{	CppAD::vector<size_t> rate_id;
			switch( integrand )
			{	case Sincidence_enum:
				rate_id.push_back( size_t(iota_enum) );
				break;

				case remission_enum:
				rate_id.push_back( size_t(rho_enum) );
				break;

				case mtexcess_enum:
				rate_id.push_back( size_t(chi_enum) );
				break;

				case mtother_enum:
				rate_id.push_back( size_t(omega_enum) );
				break;

				case mtwith_enum:
				case relrisk_enum:
				rate_id.push_back( size_t(chi_enum) );
				rate_id.push_back( size_t(omega_enum) );
				break;

				default:
				for(size_t ell = 0; ell < number_rate_enum; ell++)
					rate_id.push_back( ell );
				break;
			}
			for(size_t ell = 0; ell < rate_id.size(); ell++)
			{	// check if any random effects for this rate are not constant
				size_t smooth_id =
					pack_object.rate_info(rate_id[ell], child).smooth_id;
				if( smooth_id != DISMOD_AT_NULL_SIZE_T )
				{	const smooth_info& s_info = s_info_vec[smooth_id];
					size_t             n_a  = s_info.age_size();
					size_t             n_t = s_info.time_size();
					for(size_t i = 0; i < n_a; i++)
					{	for(size_t j = 0; j < n_t; j++)
						{	size_t prior_id    = s_info.value_prior_id(i, j);
							// if prior_id is null then const_value is not null
							if( prior_id != DISMOD_AT_NULL_SIZE_T )
							{	double lower = prior_table[prior_id].lower;
								double upper = prior_table[prior_id].upper;
								if( lower != upper )
									bound_ran_neq = true;
							}
						}
					}
				}
			}
		}
		data_info_[subset_id].bound_ran_neq = bound_ran_neq;
	}
}
/*
$begin data_model_replace_like$$
$spell
	avgint
	const
	CppAD
	struct
	std
	obj
	cv
$$

$section Set Value Necessary for Likelihood (not for Average Integrand)$$

$head Syntax$$
$icode%data_object%.replace_like(%data_subset_obj%)%$$

$head Purpose$$
The values
$cref/density_id/data_table/density_id/$$,
$cref/hold_out/data_table/hold_out/$$,
$cref/meas_value/data_table/meas_value/$$,
$cref/meas_std/data_table/meas_std/$$ are not necessary to calculate
However, the are necessary to use the functions
$cref/data_object.like_one/data_model_like_one/$$ and
$cref/data_object.like_all/data_model_like_all/$$.

$head data_object$$
This object has prototype
$codei%
	data_model %data_object%
%$$
see $cref/data_object constructor/data_model_ctor/data_object/$$.

$subhead subset_object$$
We use $icode subset_object$$
for the $cref/subset_object/data_model_ctor/$$ in the $icode data_object$$
constructor.

$subhead n_subset$$
We use the notation
$codei%
	%n_subset% = %subset_object%.size()
%$$

$subhead subset_id$$
This an index between zero and $icode%n_subset% - 1%$$.
It is used to refer to the corresponding element of
$icode subset_object$$.

$head data_subset_obj$$
This argument has prototype
$codei%
	const CppAD::vector<data_subset_struct>& %data_subset_obj%
%$$
and has size $icode n_subset$$.
For each $icode subset_id$$,
and for $icode%field% = density_id%,% hold_out%,% meas_value%,% meas_std%$$,
$icode%data_subset_obj%[%subset_id%].%field%$$,
is used as a replacement for
$icode%subset_object[%subset_id%]%.%field%$$.

$head Laplace Density$$
The Laplace density is only allowed if the model for the
$cref avg_integrand$$ does not depend on the value of the random effects.
If this assumption is violated, an error messages is generated and the
program exits.

$end
*/
void data_model::replace_like(
		const CppAD::vector<data_subset_struct>&  data_subset_obj )
{
	// n_subset
	size_t n_subset = data_subset_obj_.size();
	assert( data_subset_obj.size() == n_subset );
	//
	// replace density_id, hold_out, meas_value, meas_std, eta, nu
	for(size_t subset_id = 0; subset_id < n_subset; subset_id++)
	{	data_subset_obj_[subset_id].density_id =
			data_subset_obj[subset_id].density_id;
		data_subset_obj_[subset_id].hold_out =
			data_subset_obj[subset_id].hold_out;
		data_subset_obj_[subset_id].meas_value =
			data_subset_obj[subset_id].meas_value;
		data_subset_obj_[subset_id].meas_std =
			data_subset_obj[subset_id].meas_std;
		data_subset_obj_[subset_id].eta =
			data_subset_obj[subset_id].eta;
		data_subset_obj_[subset_id].nu =
			data_subset_obj[subset_id].nu;
		//
		data_info_[subset_id].density = density_enum(
			data_subset_obj[subset_id].density_id
		);
		//
		bool laplace = data_info_[subset_id].density == laplace_enum;
		laplace     |= data_info_[subset_id].density == log_laplace_enum;
		if( laplace && data_info_[subset_id].bound_ran_neq )
		{	std::string msg, table_name;
			size_t data_id = data_subset_obj_[subset_id].original_id;
			table_name = "data";
			msg  = "density_id corresponds to laplace or log_laplace and\n";
			msg += "model depends on random effects that are not constrained";
			error_exit(msg, table_name, data_id);
		}
	}
	replace_like_called_ = true;
	return;
}

/*
-----------------------------------------------------------------------------
$begin data_model_average$$

$spell
	vec
	const
	CppAD
$$
$section Data Model: Compute One Average Integrand$$

$head Syntax$$
$icode%avg% = %data_object%.average(%subset_id%, %pack_vec%)%$$

$head data_object$$
This object has prototype
$codei%
	data_model %data_object%
%$$
see $cref/data_object constructor/data_model_ctor/data_object/$$.
The object $icode data_object$$ is effectively const.

$head Float$$
The type $icode Float$$ must be $code double$$ or
$cref a1_double$$.

$head subset_id$$
This argument has prototype
$codei%
	size_t %subset_id%
%$$
and is the $cref/subset_id/data_subset/data_subset_obj/subset_id/$$
we are computing the average integrand for.

$head pack_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is all the $cref model_variables$$ in the order
specified by $cref pack_info$$.

$head avg$$
The return value has prototype
$codei%
	%Float% avg
%$$
This is the
$cref/average integrand/avg_integrand/Average Integrand, A_i/$$
for the specified data point.

$children%example/devel/model/avg_no_ode_xam.cpp
	%example/devel/model/avg_yes_ode_xam.cpp
%$$
$head Example$$
The files
$cref avg_no_ode_xam.cpp$$ and $cref avg_yes_ode_xam.cpp$$
contain examples using this routine.

$end
*/
template <class Float>
Float data_model::average(
	size_t                        subset_id ,
	const CppAD::vector<Float>&   pack_vec  )
{
	// arguments to avg_integrand::rectangle
	double age_lower    = data_subset_obj_[subset_id].age_lower;
	double age_upper    = data_subset_obj_[subset_id].age_upper;
	double time_lower   = data_subset_obj_[subset_id].time_lower;
	double time_upper   = data_subset_obj_[subset_id].time_upper;
	size_t weight_id    = size_t( data_subset_obj_[subset_id].weight_id );
	size_t integrand_id = size_t( data_subset_obj_[subset_id].integrand_id );
	size_t child        = size_t( data_info_[subset_id].child );
	CppAD::vector<double> x(n_covariate_);
	for(size_t j = 0; j < n_covariate_; j++)
		x[j] = subset_cov_value_[subset_id * n_covariate_ + j];
	//
	// compute average integrand
	Float result = avgint_obj_.rectangle(
		age_lower,
		age_upper,
		time_lower,
		time_upper,
		weight_id,
		integrand_id,
		n_child_,
		child,
		x,
		pack_vec
	);
	//
	return result;
}

/*
-----------------------------------------------------------------------------
$begin data_model_like_one$$

$spell
	avgint
	Sincidence
	Tincidence
	struct
	logden
	fabs
	logden
	std
	Integrands
	wres
	var
	vec
	const
	CppAD
	subvectors
	mtspecific
	mtall
	mtstandard
	mtexcess
	mtwith
	mtother
	relrisk
	xam
$$
$section One Weighted Residual and Log-Likelihood for any Integrands$$

$head Syntax$$
$icode%residual% = %data_object%.like_one(
	%subset_id%, %pack_vec%, %avg%, %delta_out%
)%$$

$head Requirement$$
One must call $cref/replace_like/data_model_replace_like/$$
before calling this function.

$head Log-likelihood$$
We use $cref/y_i/data_like/Data Table Notation/y_i/$$ to denote the
$cref/meas_value/data_table/meas_value/$$ corresponding
to this $cref/subset_id/data_subset/data_subset_obj/subset_id/$$.
The log-likelihood computed by $code like_one$$ is the mapping
$latex \[
	\ell (u, \theta) = C + \log [ \B{p} ( y_i | u , \theta ) ]
\] $$
where $latex u$$ are the random effects,
$latex \theta$$ are the fixed effects, and
$latex C$$ is a constant that does
not depend on $latex ( u , \theta )$$.

$head data_object$$
This object has prototype
$codei%
	data_model %data_object%
%$$
see $cref/data_object constructor/data_model_ctor/data_object/$$.
It is effectively const
(some internal arrays are used for temporary work space and kept between
calls to avoid re-allocating memory).

$head Float$$
The type $icode Float$$ must be $code double$$ or
$cref a1_double$$.

$head subset_id$$
This argument has prototype
$codei%
	size_t %subset_id%
%$$
and is the $cref/subset_id/data_subset/data_subset_obj/subset_id/$$
we are computing the weighted residual and log-likelihood for.

$head pack_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is all the $cref model_variables$$ in the order
specified by $cref pack_info$$; i.e.,
$latex (u , \theta)$$.
Only the $cref pack_info_mulcov_meas$$ subvectors of $icode pack_vec$$ are used
by $code like_one$$ (note that other components of $latex (u, \theta )$$
are used to compute $icode avg$$ documented below).

$head avg$$
This argument has prototype
$codei%
	const %Float%& %avg%
%$$
and is the
$cref/average integrand/avg_integrand/Average Integrand, A_i/$$,
$latex A_i ( u , \theta )$$, for the specified data point.
This can be calculated using $cref data_model_average$$.

$head delta_out$$
The input value of $icode delta_out$$ does not matter.
Upon return it is the adjusted standard deviation
$cref/delta/data_like/Adjusted Standard Deviation, delta_i/$$
for this data point.

$head residual$$
The return value has prototype
$codei%
	residual_struct<%Float%> %residual%
%$$
see $cref/residual_struct/residual_density/residual/residual_struct/$$.
It contains the weighted residual and the corresponding log-density.

$subhead index$$
Note that the $cref/index/residual_density/index/$$ field is set to
the value $icode subset_id$$.

$children%example/devel/model/like_one_xam.cpp
%$$
$head Example$$
The file $cref like_one_xam.cpp$$ contains an example and test
of using this routine.

$end
*/
template <class Float>
residual_struct<Float> data_model::like_one(
	size_t                        subset_id ,
	const CppAD::vector<Float>&   pack_vec  ,
	const Float&                  avg       ,
	Float&                        delta_out )
{
	assert( pack_object_.size() == pack_vec.size() );
	assert( replace_like_called_ );

	// covariate information for this data point
	CppAD::vector<double> x(n_covariate_);
	for(size_t j = 0; j < n_covariate_; j++)
		x[j] = subset_cov_value_[subset_id * n_covariate_ + j];
	double eta           = data_subset_obj_[subset_id].eta;
	double nu            = data_subset_obj_[subset_id].nu;
	double meas_value    = data_subset_obj_[subset_id].meas_value;
	double meas_std      = data_subset_obj_[subset_id].meas_std;
	double age_lower    = data_subset_obj_[subset_id].age_lower;
	double age_upper    = data_subset_obj_[subset_id].age_upper;
	double time_lower   = data_subset_obj_[subset_id].time_lower;
	double time_upper   = data_subset_obj_[subset_id].time_upper;
	size_t weight_id    = size_t( data_subset_obj_[subset_id].weight_id );
	size_t integrand_id = size_t( data_subset_obj_[subset_id].integrand_id );
	//
	assert( meas_std  > 0.0 );
	double meas_cv       = minimum_meas_cv_[integrand_id];
	double Delta = std::max(meas_std, meas_cv * std::fabs(meas_value) );
	//

	// average standard deviation effect
	Float std_effect = avgstd_obj_.rectangle(
		age_lower,
		age_upper,
		time_lower,
		time_upper,
		weight_id,
		integrand_id,
		x,
		pack_vec
	);
	//
	// Compute the adusted standard deviation
	density_enum density = data_info_[subset_id].density;
	if( density == log_gaussian_enum
	||  density == log_laplace_enum
	||  density == log_students_enum
	)
		delta_out  = Delta * (1.0  + std_effect);
	else
		delta_out  = Delta + std_effect;
	//
	Float not_used;
	bool difference = false;
	struct residual_struct<Float> residual = residual_density(
		not_used,
		avg,
		Float(meas_value),
		delta_out,
		density,
		Float(eta),
		Float(nu),
		subset_id,
		difference
	);
	//
	// switch the sign of the residual because it is computed as
	// model minus data (to avoid having model scale the variance)
	residual.wres = - residual.wres;
	//
	return residual;
}
/*
-------------------------------------------------------------------------------
$begin data_model_like_all$$
$spell
	avgint
	bool
	enum
	vec
	CppAD
	const
	struct
$$

$section All the Weighted Residuals and Log-Likelihoods$$

$head Syntax$$
$icode%residual_vec% = %data_object%.like_all(
	%hold_out%, %random_depend%, %pack_vec%
)%$$

$head Requirement$$
One must call $cref/replace_like/data_model_replace_like/$$
before calling this function.

$head data_object$$
This object has prototype
$codei%
	data_model %data_object%
%$$
see $cref/data_object constructor/data_model_ctor/data_object/$$.
It is effectively const
(some internal arrays are used for temporary work space and kept between
calls to avoid re-allocating memory).

$head Float$$
The type $icode Float$$ must be $code double$$ or
$cref a1_double$$.

$head hold_out$$
This argument has prototype
$codei%
	bool %hold_out%
%$$
If it is true, the flagged data will be held out,
otherwise it will not.

$head random_depend$$
This argument has prototype $codei%
	bool %random_depend%
%$$
If it is true (false), only residuals that depend on the random effects
that have non-equal (equal) bounds, are included.

$head pack_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is all the $cref model_variables$$ in the order
specified by $cref pack_info$$; i.e., $latex (u , \theta)$$.
If $icode random_depend$$ is false,
only the components of $icode u$$ that have equal lower and upper
bounds are defined (the other components are unspecified).

$head residual_vec$$
The return value has prototype
$codei%
	CppAD::vector< residual_struct<%Float%> > %residual_vec%
%$$

$subhead index$$
For each element of $icode residual_vec$$,
the residual $cref/index/residual_density/index/$$
is set to the $cref/data_subset_id/data_subset_table/data_subset_id/$$
for the corresponding residual.

$subhead Include Hold Outs$$
If $icode hold_out$$ is false,
the size of $icode residual$$ is equal the number of subset data values
$cref/n_subset/data_subset/data_subset_obj/n_subset/$$.
The order of the residuals is the same as in
$cref/subset_object/data_model_ctor/subset_object/$$ in the
$icode data_object$$ constructor.

$subhead No Hold Outs$$
If $icode hold_out$$ is true,
the size of $icode residual$$ is equal the number of subset data values
$cref/n_subset/data_subset/data_subset_obj/n_subset/$$,
minus the number for which
$cref/hold_out/data_table/hold_out/$$ is one.
The order of the residuals is not specified in this case.

$head Log Density$$
The log of the density
$latex \B{p}( y | u , \theta )$$,
for the data that is included,
is the sum of the log of the densities corresponding to all the
$cref/residuals/residual_density/$$ in $icode residual_vec$$.

$end
*/
template <class Float>
CppAD::vector< residual_struct<Float> > data_model::like_all(
	bool                        hold_out      ,
	bool                        random_depend ,
	const CppAD::vector<Float>& pack_vec      )
{	assert( replace_like_called_ );
	//
	// loop over the subsampled data
	CppAD::vector< residual_struct<Float> > residual_vec;
	for(size_t subset_id = 0; subset_id < data_subset_obj_.size(); subset_id++)
	{	bool keep = hold_out == false;
		keep     |= data_subset_obj_[subset_id].hold_out == 0;
		if( random_depend )
			keep &= data_info_[subset_id].bound_ran_neq == true;
		else
			keep &= data_info_[subset_id].bound_ran_neq == false;
		assert( data_info_[subset_id].child <= n_child_ );
		if( keep )
		{	Float avg = average(subset_id, pack_vec);

			// compute its residual and log likelihood
			Float not_used;
			residual_struct<Float> residual =
				like_one(subset_id, pack_vec, avg, not_used);
			residual_vec.push_back( residual );
		}
	}
	return residual_vec;
}

// ------------------------------------------------------------------------
# define DISMOD_AT_INSTANTIATE_DATA_MODEL_CTOR(SubsetStruct)   \
template data_model::data_model(                                \
	const std::string&                       rate_case       ,  \
	double                                   bound_random    ,  \
	size_t                                   n_covariate     ,  \
	double                                   ode_step_size   ,  \
	const CppAD::vector<double>&             avg_age_grid    ,  \
	const CppAD::vector<double>&             age_table       ,  \
	const CppAD::vector<double>&             time_table      ,  \
	const CppAD::vector<integrand_struct>&   integrand_table ,  \
	const CppAD::vector<prior_struct>&       prior_table     ,  \
	const CppAD::vector<SubsetStruct>&       subset_object   ,  \
	const CppAD::vector<double>&             subset_cov_value,  \
	const CppAD::vector<weight_info>&        w_info_vec      ,  \
	const CppAD::vector<smooth_info>&        s_info_vec      ,  \
	const pack_info&                         pack_object     ,  \
	const child_info&                        child_object       \
);
DISMOD_AT_INSTANTIATE_DATA_MODEL_CTOR(data_subset_struct)
DISMOD_AT_INSTANTIATE_DATA_MODEL_CTOR(avgint_subset_struct)
// ------------------------------------------------------------------------
# define DISMOD_AT_INSTANTIATE_DATA_MODEL(Float)            \
	template Float data_model::average(                     \
		size_t                        subset_id,            \
		const CppAD::vector<Float>&   pack_vec              \
	);                                                      \
	template residual_struct<Float>                         \
	data_model::like_one(                                   \
		size_t                        subset_id,            \
		const CppAD::vector<Float>&   pack_vec ,            \
		const Float&                  avg      ,            \
		Float&                        delta_out             \
	);                                                      \
	template CppAD::vector< residual_struct<Float> >        \
	data_model::like_all(                                   \
		bool                          hold_out ,            \
		bool                          parent   ,            \
		const CppAD::vector<Float>&   pack_vec              \
	);                                                      \

// instantiations
DISMOD_AT_INSTANTIATE_DATA_MODEL( double )
DISMOD_AT_INSTANTIATE_DATA_MODEL( a1_double )


} // END DISMOD_AT_NAMESPACE
