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
$begin put_table_row$$
$spell
	sqlite
	const
	std
	CppAD
	vec
$$

$section C++: Insert New Row at End of a Table$$

$head Syntax$$
$icode%table_name_id% = put_table_row(
	%db%, %table_name%, %col_name_vec%, %row_val_vec%
)%$$

$head db$$
This argument has prototype
$codei%
	sqlite3* %db%
%$$
and is the database we are putting information into.

$head table_name$$
This argument has prototype
$codei%
	const std::string& %table_name%
%$$
and is the name of the table we are putting information into.

$head col_name_vec$$
This argument has prototype
$codei%
	const CppAD::vector<std::string>& %col_name_vec%
%$$
and is a vector of the names for the column names in this table.
The size of this vector must be the number of columns in the table
minus one and the primary key column is not included.

$head row_val_vec$$
This argument has prototype
$codei%
	const CppAD::vector<std::string>& %row_val_vec%
%$$
and is a vector of the values we are inserting into the table.
It must have the same size, and be in the same order,
as $icode col_name_vec$$.
There cannot be any single quote characters $code '$$
in any of the values.

$head Primary Key$$
The primary key for this table name must have column name
$icode%table_name%_id%$$.
This key starts with zero, and increment by one between rows.

$head table_name_id$$
The return value has prototype
$codei%
	size_t %table_name_id%
%$$
It is the value of the primary key $icode%table_name%_id%$$ for this row.

$children%example/devel/table/put_table_row_xam.cpp
%$$
$head Example$$
The file $cref put_table_row_xam.cpp$$ contains an example that uses
this function.

$end
------------------------------------------------------------------------------
*/
# include <dismod_at/put_table_row.hpp>
# include <dismod_at/check_table_id.hpp>
# include <dismod_at/exec_sql_cmd.hpp>
# include <dismod_at/get_table_column.hpp>
# include <dismod_at/to_string.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

size_t put_table_row(
	sqlite3*                          db           ,
	const std::string&                table_name   ,
	const CppAD::vector<std::string>& col_name_vec ,
	const CppAD::vector<std::string>& row_val_vec  )
{
	size_t n_col = col_name_vec.size();
	size_t n_row = check_table_id(db, table_name);
	std::string table_name_id = to_string(n_row);
	//
	std::string name_list  = "(";
	std::string value_list = "(";
	name_list            += table_name + "_id";
	value_list           += table_name_id;
	for(size_t i = 0; i < n_col; i++)
	{	// check that there are not single quotes in the value
		assert( row_val_vec[i].find('\'') == std::string::npos );

		// add this column to name_list
		name_list += ",";
		name_list += col_name_vec[i];

		// add this column to value_list
		std::string column_type = get_table_column_type(
			db, table_name, col_name_vec[i]
		);
		if( column_type == "text" )
		{	value_list += ",'";
			value_list += row_val_vec[i];
			value_list += "'";
		}
		else
		{	value_list += ",";
			value_list += row_val_vec[i];
		}
	}
	name_list  += ")";
	value_list += ")";
	//
	std::string cmd = "insert into ";
	cmd            += table_name + " ";
	cmd            += name_list + " values ";
	cmd            +=  value_list + ";";
	//
	exec_sql_cmd(db, cmd);
	//
	return n_row;
}

} // END_DISMOD_AT_NAMESPACE
