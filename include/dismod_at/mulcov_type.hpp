// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-18 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_MULCOV_TYPE_HPP
# define DISMOD_AT_MULCOV_TYPE_HPP

namespace dismod_at {
	enum mulcov_type_enum {
		rate_value_enum,
		meas_value_enum,
		meas_noise_enum,
		number_mulcov_type_enum
	};
}

# endif
