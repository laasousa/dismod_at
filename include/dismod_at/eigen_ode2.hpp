// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_EIGEN_ODE2_HPP
# define DISMOD_AT_EIGEN_ODE2_HPP

# include <cppad/utility/vector.hpp>

namespace dismod_at {
	template <class Float>
	extern CppAD::vector<Float> eigen_ode2(
		size_t                       case_number ,
		const CppAD::vector<Float>&  b           ,
		const CppAD::vector<Float>&  yi          ,
		const Float&                 tf
	);
}
# endif
