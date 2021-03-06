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
$begin open_connection$$
$spell
	sqlite
	const
	std
	bool
$$

$section Open An Sqlite3 Database Connection$$

$head Syntax$$
$icode%db% = open_connection(%file_name%, %new_file%)
%$$

$head file_name$$
This argument has prototype
$codei%
	const std::string& %file_name%
%$$
It is the name of the file where the data base is
(or will be) stored.

$head new_file$$
This argument has prototype
$codei%
	bool %new_file%
%$$
If it is true, and a database with the same name already exists,
the existing database is deleted before creating the connection.

$head db$$
The return value has prototype
$codei%
	sqlite3* %db%
%$$
and is the database connection.

$head close$$
When you are done with $icode db$$, you must call
$codei%
	sqlite3_close(%db%)
%$$

$end
-----------------------------------------------------------------------------
*/
# include <cstdlib>
# include <iostream>
# include <fstream>
# include <dismod_at/open_connection.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

sqlite3* open_connection(const std::string& file_name, bool new_file)
{
	if( new_file )
	{	// delete old version of database
		std::ifstream ifile( file_name.c_str() );
		if( ifile )
		{	ifile.close();
			std::remove( file_name.c_str() );
		}
	}
	// open a new database
	sqlite3* db;
	int rc = sqlite3_open(file_name.c_str(), &db);
	if( rc )
	{	std::string msg;
		if( new_file )
			msg = "Can't create a new database in file ";
		else
			msg = "Can't open the existing database in file ";
		std::cerr << msg << file_name << std::endl;
		sqlite3_close(db);
		std::exit(1);
	}
	return db;
}

} // END_DISMOD_AT_NAMESPACE
