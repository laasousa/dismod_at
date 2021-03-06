// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-17 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_LOG_MESSAGE_HPP
# define DISMOD_AT_LOG_MESSAGE_HPP

# include <sqlite3.h>
# include <string>
# include <ctime>

namespace dismod_at {
	extern std::time_t log_message(
		sqlite3*           db           ,
		std::ostream*      os           ,
		const std::string& message_type ,
		const std::string& message
	);
	extern std::time_t log_message(
		sqlite3*           db           ,
		std::ostream*      os           ,
		const std::string& message_type ,
		const std::string& message      ,
		const std::string& table_name   ,
		const size_t&      row_id
	);
}

# endif
