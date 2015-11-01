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
$begin residual_density$$
$spell
	mu
	enum
	const
	struct
	wres
	logden
	fabs
	bool
$$

$section Compute Weighted Residual and Log-Density$$

$head Syntax$$
$icode%residual% = residual_density(
	%z%, %y%, %mu%, %delta%, %eta%, %d%, %difference%
)%$$

$head d$$
This argument has prototype
$codei%
	density_enum %d%
%$$
It specifies the $cref/density/get_density_table/density_enum/$$.

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$, $code AD<double>$$, or $cref a2_double$$.

$head z$$
This argument has prototype
$codei%
	const %Float%& z
%$$
If $icode difference$$ is true,
it specifies the first random variable in the difference.
This argument is not used when $icode difference$$ is false.

$head y$$
This argument has prototype
$codei%
	const %Float%& y
%$$
If $icode difference$$ is true,
it specifies the second random variable in the difference.
Otherwise it specifies the only random variable.

$head mu$$
This argument has prototype
$codei%
	const %Float%& mu
%$$
If $icode difference$$ is true, it is the central value for the difference.
Otherwise, it is the central value for $icode y$$.

$head delta$$
This argument has prototype
$codei%
	const %Float%& delta
%$$
It is either the standard deviation or a parameter in the standard deviation;
see below.

$head eta$$
If the density $icode d$$ is
$code log_gaussian_enum$$ or $code log_laplace_enum$$,
it specifies the offset in the log transformation.
Otherwise it is not used.

$head difference$$
This argument has prototype
$codei%
	bool %difference%
%$$
If $icode difference$$ is true,
this calculation is for the difference of the
random variables $latex z$$ and $latex y$$.
Otherwise it is just for the random variable $latex y$$.

$head residual$$
The return value has prototype
$codei%
	residual_struct<%Float%> %residual%
%$$

$subhead residual_struct$$
This structure has the following fields:
$table
Type $cnext Field $cnext Description $rnext
$icode Float$$ $cnext
	$code wres$$ $cnext
	$cref/weighted residual/statistic/Weighted Residual Function, R/$$
$rnext
$icode Float$$ $cnext
	$icode logden_smooth$$ $cnext
	this smooth term is in
	$cref/log-density/statistic/Log-Density Function, D/$$
$rnext
$icode Float$$ $cnext
	$icode logden_sub_abs$$ $cnext
	absolute value of this smooth term is in log-density
$rnext
$code density_enum$$ $cnext
	$icode density$$ $cnext
	type of density function; see
	$cref/density_enum/get_density_table/density_enum/$$
$rnext
$tend
The $cref/log-density function
	/statistic
	/Log-Density Function, D
/$$

$subhead wres$$
If $icode difference$$ is false, $icode wres$$ is the value of
$latex \[
	R(y, \mu, \delta, \eta, d)
\]$$
see $cref/weighted residual function
	/statistic
	/Weighted Residual Function, R
/$$.
If $icode difference$$ is true, $icode wres$$ is the value of
$latex \[
	R(z, y, \mu, \delta, \eta, d)
\]$$

$subhead logden$$
If $icode difference$$ is false, the log-density function
$latex \[
	D(y, \mu, \delta, \eta, d)
\]$$
is equal to
$codei%
    %logden_smooth% - fabs(%logden_sub_abs)%)
%$$
see $cref/log-density function
	/statistic
	/Log-Density Function, D
/$$.
If $icode difference$$ is true, the log-density function
$latex \[
	D(z, y, \mu, \delta, \eta, d)
\]$$
is equal to
$codei%
    %logden_smooth% - fabs(%logden_sub_abs)%)
%$$
This expresses the log-density
in terms of smooth functions (for optimization purposes).
In the case of the uniform density,
both $icode logden_smooth$$ and $icode logden_sub_abs$$ will be zero.
In the case of the gaussian and log-gaussian densities,
$icode logden_sub_abs$$ will be zero.

$end
*/
# include <cppad/cppad.hpp>
# include <dismod_at/residual_density.hpp>
# include <dismod_at/a2_double.hpp>

namespace {
	template <class Float>
	void print_forward_if_not_positive(
		const char* name    ,
		const Float& value  )
	{	std::string lable = "residual_density: ";
		lable += name;
		lable += " = ";
		CppAD::PrintFor(value, lable.c_str(), value, "\n");
	}
	void print_forward_if_not_positive(
		const char* name    ,
		const double& value )
	{ }
}


namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

template <class Float>
residual_struct<Float> residual_density(
	const Float&       z          ,
	const Float&       y          ,
	const Float&       mu         ,
	const Float&       delta      ,
	const Float&       eta        ,
	density_enum       d          ,
	bool               difference )
{	Float nan(std::numeric_limits<double>::quiet_NaN());
	Float tiny( 10.0 / std::numeric_limits<double>::max() );

	Float wres = nan;
	Float sigma = nan;
	switch( d )
	{
		case uniform_enum:
		wres = 0.0;
		break;

		case gaussian_enum:
		case laplace_enum:
		print_forward_if_not_positive("delta", delta);
		assert( delta > 0.0 );
		sigma = delta;
		if( difference )
			wres  = ( z - y - mu) / sigma;
		else
			wres = (y - mu) / sigma;
		break;

		case log_gaussian_enum:
		case log_laplace_enum:
		print_forward_if_not_positive("delta", delta);
		print_forward_if_not_positive("z", z + tiny);
		print_forward_if_not_positive("mu", mu + tiny);
		assert( delta > 0.0 );
		if( difference )
		{	sigma = delta;
			wres  = ( log( z + eta ) - log( y + eta ) - mu ) / sigma;
		}
		else
		{	sigma = log( 1.0 + delta / (y + eta) );
			wres  = ( log( y + eta ) - log( mu + eta ) ) / sigma;
		}
		break;

		default:
		assert(false);
	}
	Float logden_smooth = nan;
	Float logden_sub_abs = nan;
	switch( d )
	{
		case uniform_enum:
		logden_smooth  = 0.0;
		logden_sub_abs = 0.0;
		break;

		case gaussian_enum:
		case log_gaussian_enum:
		{	double pi2 = 8.0 * std::atan(1.0);
			logden_smooth  = - log( sigma * sqrt( pi2 ) ) - wres * wres/ 2.0;
			logden_sub_abs = 0.0;
		}
		break;

		case laplace_enum:
		case log_laplace_enum:
		{	double r2   = sqrt(2.0);
			logden_smooth  = - log( sigma * r2 );
			logden_sub_abs = r2 * wres;
		}
		break;

		default:
		assert(false);
	}
	//
	residual_struct<Float> residual;
	residual.wres           = wres;
	residual.logden_smooth  = logden_smooth;
	residual.logden_sub_abs = logden_sub_abs;
	residual.density        = d;
	return residual;
}

// instantiation macro
# define DISMOD_AT_INSTANTIATE_RESIDUAL_DENSITY(Float)        \
	template residual_struct<Float> residual_density(         \
		const Float&       z            ,                     \
		const Float&       y            ,                     \
		const Float&       mu           ,                     \
		const Float&       delta        ,                     \
		const Float&       eta          ,                     \
		density_enum       density      ,                     \
		bool               difference                         \
	);

// instantiations
DISMOD_AT_INSTANTIATE_RESIDUAL_DENSITY(double)
DISMOD_AT_INSTANTIATE_RESIDUAL_DENSITY( CppAD::AD<double> )
DISMOD_AT_INSTANTIATE_RESIDUAL_DENSITY( a2_double )

} // END DISMOD_AT_NAMESPACE
