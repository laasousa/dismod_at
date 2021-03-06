// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-19 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin get_mulcov_table$$
$spell
	mulcov
	sqlite
	struct
	cpp
	std
	covariate
	enum
$$

$section C++: Get the Covariate Multiplier Table Information$$

$head Syntax$$
$icode%mulcov_table% = get_mulcov_table(%db%)%$$

$head Purpose$$
To read the $cref mulcov_table$$ and return it as a C++ data structure.

$head db$$
The argument $icode db$$ has prototype
$codei%
	sqlite3* %db%
%$$
and is an open connection to the database.

$head mulcov_struct$$
This is a structure with the following fields
$table
Type $cnext Field $cnext Description
$rnext
$code mulcov_type_enum$$ $pre  $$ $cnext $code mulcov_type$$   $pre  $$ $cnext
	See the description of $code mulcov_type$$ below.
$rnext
$code int$$ $cnext $code rate_id$$     $pre  $$ $cnext
	The $cref/rate_id/mulcov_table/rate_id/$$
	for this multiplier
$rnext
$code int$$ $cnext $code integrand_id$$     $pre  $$ $cnext
	The $cref/integrand_id/mulcov_table/integrand_id/$$
	for this multiplier
$rnext
$code int$$ $cnext $code covariate_id$$     $pre  $$ $cnext
	The $cref/covariate_id/mulcov_table/covariate_id/$$
	for this multiplier
$rnext
$code int$$ $cnext $code group_id$$     $pre  $$ $cnext
	The $cref/group_id/mulcov_table/group_id/$$
	for this multiplier
$rnext
$code int$$ $cnext $code group_smooth_id$$     $pre  $$ $cnext
	The $cref/group_smooth_id/mulcov_table/group_smooth_id/$$
	for this multiplier
$rnext
$code int$$ $cnext $code subgroup_smooth_id$$     $pre  $$ $cnext
	The $cref/subgroup_smooth_id/mulcov_table/subgroup_smooth_id/$$
	for this multiplier
$tend

$subhead mulcov_type_enum$$
The $code mulcov_type_enum$$ possible values are
$code rate_value_enum$$,
$code meas_value_enum$$, and
$code meas_noise_enum$$.
These correspond to the values in the
$cref/mulcov_type/mulcov_table/mulcov_type/$$ column of the
$code mulcov_type$$ table.

$head mulcov_table$$
The return value $icode mulcov_table$$ has prototype
$codei%
	CppAD::vector<mulcov_struct>  %mulcov_table%
%$$
For each $cref/mulcov_id/mulcov_table/mulcov_id/$$,
$codei%
	%mulcov_table%[%mulcov_id%]
%$$
is the information for the corresponding multiplier.

$children%example/devel/table/get_mulcov_table_xam.cpp
%$$
$head Example$$
The file $cref get_mulcov_table_xam.cpp$$ contains an example that uses
this function.

$end
-----------------------------------------------------------------------------
*/

# include <cmath>


# include <dismod_at/get_mulcov_table.hpp>
# include <dismod_at/get_table_column.hpp>
# include <dismod_at/check_table_id.hpp>
# include <dismod_at/error_exit.hpp>
# include <dismod_at/log_message.hpp>
# include <dismod_at/null_int.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

CppAD::vector<mulcov_struct> get_mulcov_table(
	sqlite3* db                                           ,
	const CppAD::vector<subgroup_struct>& subgroup_table  )
{	using std::string;

	// determine the number of groups
	size_t n_subgroup = subgroup_table.size();
	size_t n_group    = size_t(subgroup_table[n_subgroup - 1].group_id + 1);

	string table_name   = "mulcov";
	size_t n_mulcov     = check_table_id(db, table_name);

	string column_name  = "mulcov_type";
	CppAD::vector<string>  mulcov_type;
	get_table_column(db, table_name, column_name, mulcov_type);
	assert( mulcov_type.size() == n_mulcov );

	column_name         = "rate_id";
	CppAD::vector<int>     rate_id;
	get_table_column(db, table_name, column_name, rate_id);
	assert( rate_id.size() == n_mulcov );

	column_name         = "integrand_id";
	CppAD::vector<int>     integrand_id;
	get_table_column(db, table_name, column_name, integrand_id);
	assert( integrand_id.size() == n_mulcov );

	column_name         = "covariate_id";
	CppAD::vector<int>     covariate_id;
	get_table_column(db, table_name, column_name, covariate_id);
	assert( covariate_id.size() == n_mulcov );

	column_name         = "group_id";
	CppAD::vector<int>    group_id;
	get_table_column(db, table_name, column_name, group_id);
	assert( group_id.size() == n_mulcov );

	column_name         = "group_smooth_id";
	CppAD::vector<int>     group_smooth_id;
	get_table_column(db, table_name, column_name, group_smooth_id);
	assert( group_smooth_id.size() == n_mulcov );
	//
	column_name         = "subgroup_smooth_id";
	CppAD::vector<int>     subgroup_smooth_id;
	get_table_column(db, table_name, column_name, subgroup_smooth_id);
	assert( subgroup_smooth_id.size() == n_mulcov );

	CppAD::vector<mulcov_struct>     mulcov_table(n_mulcov);
	for(size_t i = 0; i < n_mulcov; i++)
	{
		mulcov_table[i].rate_id             = rate_id[i];
		mulcov_table[i].integrand_id        = integrand_id[i];
		mulcov_table[i].covariate_id        = covariate_id[i];
		mulcov_table[i].group_id            = group_id[i];
		mulcov_table[i].group_smooth_id     = group_smooth_id[i];
		mulcov_table[i].subgroup_smooth_id  = subgroup_smooth_id[i];
		//
		// mulcov_type
		if( mulcov_type[i] == "rate_value" )
			mulcov_table[i].mulcov_type = rate_value_enum;
		else if( mulcov_type[i] == "meas_value" )
			mulcov_table[i].mulcov_type = meas_value_enum;
		else if( mulcov_type[i] == "meas_noise" )
			mulcov_table[i].mulcov_type = meas_noise_enum;
		else
		{	string message = "mulcov_type = '" + mulcov_type[i] + "'";
			message += " is not one of the following:\n"
				"'rate_value', 'meas_value', 'meas_noise'.";
			error_exit(message, table_name, i);
		}
		//
		// check group_id
		if( group_id[i] == DISMOD_AT_NULL_INT )
		{	string message = "group_id is null";
			error_exit(message, table_name, i);
		}
		if( size_t( group_id[i] ) >= n_group )
		{	string message =
				"group_id is not <= maximum group_id in subgroup table";
			error_exit(message, table_name, i);
		}
		//
		// check for subgroup meas_noise variables
		if( mulcov_type[i] == "meas_noise" &&
			subgroup_smooth_id[i] != DISMOD_AT_NULL_INT )
		{	string message =
			"mulcov_type = 'meas_noise' and subgroup_smooth_id is not null.";
			error_exit(message, table_name, i);
		}
	}
	return mulcov_table;
}

} // END DISMOD_AT_NAMESPACE
