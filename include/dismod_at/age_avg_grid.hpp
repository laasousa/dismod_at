// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-18 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_AGE_AVG_GRID_HPP
# define DISMOD_AT_AGE_AVG_GRID_HPP

# include <string>
# include <cppad/utility/vector.hpp>

namespace dismod_at {

CppAD::vector<double> age_avg_grid(
	double                         ode_step_size  ,
	const std::string              age_avg_split  ,
	const CppAD::vector<double>&   age_table
);

}

# endif
