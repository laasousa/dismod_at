// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-17 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin check_child_nslist$$
$spell
	sqlite
	std
	const
	CppAD
	struct
	nslist
$$

$section Check Assumptions About Lists of Child Node Smmothing Pairs$$

$head syntax$$
$codei%check_child_nslist(
	%db%,
	%option_table%,
	%rate_table%,
	%node_table%,
	%smooth_table%,
	%nslist_table%,
	%nslist_pair%
)%$$

$head Purpose$$
Checks the assumptions related to the list specifed by each
$cref/child_nslist_id/rate_table/child_nslist/id/$$.

$head db$$
This argument has prototype
$codei%
	sqlite3* %db%
%$$
and is the database connection for $cref/logging/log_message/$$ errors.

$head option_table$$
This argument has prototype
$codei%
	const CppAD::vector<option_struct>& %option_table%
%$$
and it is the
$cref/option_table/get_option_table/option_table/$$.

$head rate_table$$
This argument has prototype
$codei%
	const CppAD::vector<rate_struct>& %rate_table%
%$$
and it is the
$cref/rate_table/get_rate_table/rate_table/$$.

$head node_table$$
This argument has prototype
$codei%
	const CppAD::vector<node_struct>& %node_table%
%$$
and it is the
$cref/node_table/get_node_table/node_table/$$.

$head smooth_table$$
This argument has prototype
$codei%
	const CppAD::vector<smooth_struct>& %smooth_table%
%$$
and it is the
$cref/smooth_table/get_smooth_table/smooth_table/$$.

$head nslist_table$$
This argument has prototype
$codei%
	const CppAD::vector<std::string>& %nslist_table%
%$$
and it is the
$cref/nslist_table/get_nslist_table/nslist_table/$$.


$head nslist_pair$$
This argument has prototype
$codei%
	const CppAD::vector<nslist_pair_struct>& %nslist_pair%
%$$
and it is the
$cref/nslist_pair/get_nslist_pair/nslist_pair/$$.

$end
*/
# include <dismod_at/check_child_nslist.hpp>
# include <dismod_at/null_int.hpp>
# include <dismod_at/error_exit.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

void check_child_nslist(
	sqlite3*                                 db             ,
	const CppAD::vector<option_struct>&      option_table   ,
	const CppAD::vector<rate_struct>&        rate_table     ,
	const CppAD::vector<node_struct>&        node_table     ,
	const CppAD::vector<std::string>&        nslist_table   ,
	const CppAD::vector<nslist_pair_struct>& nslist_pair    )
{	assert( rate_table.size()   == number_rate_enum );
	using std::string;
	string msg;
	//
	// parent_node_id
	int parent_node_id = DISMOD_AT_NULL_INT;
	for(size_t i = 0; i < option_table.size(); i++)
	{	if( option_table[i].option_name == "parent_node_id" )
			parent_node_id = std::atoi( option_table[i].option_value.c_str() );
	}
	assert( parent_node_id != DISMOD_AT_NULL_INT );
	//
	// child_node_id
	CppAD::vector<size_t> child_node_id(0);
	for(size_t node_id = 0; node_id < node_table.size(); node_id++)
	{	if( node_table[node_id].parent == parent_node_id )
			child_node_id.push_back( node_id );
	}
	size_t n_child = child_node_id.size();
	//
	// child_found
	//
	for(size_t rate_id = 0; rate_id < rate_table.size(); rate_id++)
	if(	rate_table[rate_id].child_nslist_id != DISMOD_AT_NULL_INT )
	{	int nslist_id  = rate_table[rate_id].child_nslist_id;
		//
		// initialize child_found for this rate
		CppAD::vector<bool> child_found(n_child);
		for(size_t i = 0; i < n_child; i++)
			child_found[i] = false;
		//
		// run thourgh the entries in this list
		for(size_t pair_id = 0; pair_id < nslist_pair.size(); pair_id++)
		if( nslist_pair[pair_id].nslist_id == nslist_id )
		{	size_t node_id        = nslist_pair[pair_id].node_id;
			string table_name     = "nslist_pair";
			if( node_table[node_id].parent != parent_node_id )
			{	msg  = node_table[node_id].node_name;
				msg += " is not a child of the parent node in option table ";
				msg += node_table[parent_node_id].node_name;
				error_exit(msg, table_name, pair_id);
			}
			for(size_t i = 0; i < n_child; i++)
			{	if( child_node_id[i] == node_id )
				{	if( child_found[i] )
					{	msg += "node " + node_table[node_id].node_name;
						msg += " found multiple times in the list ";
						msg += nslist_table[nslist_id];
						error_exit(msg, table_name, pair_id);
					}
					child_found[i] = true;
				}
			}
		}
		for(size_t i = 0; i < n_child; i++) if( ! child_found[i] )
		{	size_t node_id = child_node_id[i];
			string table_name = "nslist";
			msg += "child node " + node_table[node_id].node_name;
			msg += " not found in the list ";
			msg += nslist_table[nslist_id];
			error_exit(msg, table_name, nslist_id);
		}
	}
}

} // END DISMOD_AT_NAMESPACE
