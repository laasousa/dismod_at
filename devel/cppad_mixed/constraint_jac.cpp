// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/cppad_mixed.hpp>

/*
$begin constraint_jac$$
$spell
	cppad
	eval
	vec
	const
	Cpp
	Jacobian
	jac
$$

$section cppad_mixed: Jacobian of Constraint w.r.t Fixed Effects$$

$head Syntax$$
$icode%mixed_object%.constraint_jac(
	%fixed_vec%, %row_out%, %col_out%, %val_out%
)%$$

$head mixed_object$$
We use $cref/mixed_object/cppad_mixed_derived_ctor/mixed_object/$$
to denote an object of a class that is
derived from the $code cppad_mixed$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/cppad_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which $latex c^{(1)} ( \theta )$$ is evaluated.

$head row_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %row_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same
as for a previous call to $code constraint_jac$$.
If it's input size is zero,
upon return it contains the row indices for the Jacobian elements
that are possibly non-zero.

$head col_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %col_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same as for
a previous call to $code constraint_jac$$.
If it's input size is zero,
upon return it contains the column indices for the Jacobian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).

$head val_out$$
This argument has prototype
$codei%
	CppAD::vector<double>& %val_out%
%$$
If the input size of this array is non-zero, it must have the same size
as for a previous call to $code constraint_jac$$.
Upon return, it contains the value of the Jacobian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).

$children%
	example/devel/cppad_mixed/private/constraint_jac_xam.cpp
%$$
$head Example$$
The file $cref constraint_jac_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void cppad_mixed::constraint_jac(
	const d_vector&        fixed_vec   ,
	CppAD::vector<size_t>& row_out     ,
	CppAD::vector<size_t>& col_out     ,
	d_vector&              val_out     )
{
	// make sure initilialize has been called
	if( ! initialize_done_  )
	{	std::string error_message =
		"cppad_mixed::initialize was not called before constraint_jac";
		fatal_error(error_message);
	}
	if( constraint_jac_.row.size() == 0 )
	{	// sparse Jacobian has no rows
		assert( row_out.size() == 0 );
		assert( col_out.size() == 0 );
		assert( constraint_jac_.row.size() == 0 );
		assert( constraint_jac_.col.size() == 0 );
		val_out.resize(0);
		return;
	}

	assert( row_out.size() == col_out.size() );
	assert( row_out.size() == val_out.size() );
	if( row_out.size() == 0 )
	{	row_out = constraint_jac_.row;
		col_out = constraint_jac_.col;
		val_out.resize( row_out.size() );
	}
# ifndef NDEBUG
	else
	{	size_t n_nonzero = constraint_jac_.row.size();
		assert( row_out.size() == n_nonzero );
		for(size_t k = 0; k < n_nonzero; k++)
		{	assert( row_out[k] == constraint_jac_.row[k] );
			assert( col_out[k] == constraint_jac_.col[k] );
		}
	}
# endif
	assert( row_out.size() != 0 );

	CppAD::vector< std::set<size_t> > not_used;
	constraint_fun_.SparseJacobianForward(
		fixed_vec       ,
		not_used        ,
		row_out         ,
		col_out         ,
		val_out         ,
		constraint_jac_.work
	);

	return;
}


} // END_DISMOD_AT_NAMESPACE