# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-20 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin user_jump_at_age.py$$ $newlinech #$$
# $spell
#	init
#	Sincidence
#	pos
# $$
#
# $section Zero Rate Until a Jump at a Known Age$$
#
# $head Purpose$$
# Usually the prior for the rate is smooth.
# This requires lots of data, at a fine age spacing,
# to resolve a jump in a rate at an unknown age.
# If the age at which a jump occurs is known, it is possible to resolve
# the jump with much less data by specifying a prior that has this knowledge.
#
# $head Parameters$$
# The following values are used to simulate the data and define the priors
# and can be changed:
# $srccode%py%
iota_near_zero  = 1e-10
iota_after_20   = 1e-1
iota_eta        = 1e-5
time_table      = [ 2020.0, 2000.0 ]
age_table       = [ 100.0, 70.0, 40.0, 20.0, 0.0 ]
# %$$
#
# $subhead iota_near_zero$$
# This is the true value of $icode iota$$ up to and including age 20.
# Note that it is close to zero, but not equal to zero, so that we can
# use the rate case
# $cref/iota_pos_rho_zero/option_table/rate_case/iota_pos_rho_zero/$$.
#
# $subhead iota_after_20$$
# This is the true value of $icode iota$$ for ages greater than 20.
#
# $subhead iota_eta$$
# Offset in log transformation used for values of eta.
#
# $subhead age_table$$
# The $cref age_table$$ does not need to be monotone increasing.
# For this example, it is the same as the table of ages at which
# $icode iota$$ is modeled .
# You can changed the order of $code age_table$$
# and it will not affect the results.
#
# $subhead time_table$$
# The $cref time_table$$ does not need to be monotone increasing.
# You can changed the order of $code time_table$$
# and it will not affect the results.
#
# $head Model Variables$$
# This example's variables are all
# $cref/parent rates/model_variables/Fixed Effects, theta/Parent Rates/$$
# for $cref/iota/rate_table/rate_name/iota/$$.
# The value of $icode iota$$ is modeled at each age in the $code age_table$$.
# The prior for the value of $icode iota$$ up to age 20 is a constant equal to
# $code iota_near_zero$$.
# After age 20 it is uniform with lower limit $code iota_near_zero$$,
# upper limit 1 and mean $code iota_after_20 / 4.0$$
# (The mean is only used for the initial value and scaling the optimization.)
# The prior for the forward age differences in $icode iota$$ at age 20
# is uniform, and above age 20 it is a Log-Gaussian with mean 0 and
# standard deviation 0.1 (about 10 percent coefficient of variation).
#
# $head Truth$$
# For this example the rate $icode iota$$ is constant
# with value $code iota_near_zero$$ for ages less than or equal 20,
# and $code iota_after_20$$ for ages greater than 20.
#
# $head Simulated Data$$
# For this example, the simulated data is all
# $cref/Sincidence/avg_integrand/Integrand, I_i(a,t)/Sincidence/$$; i.e.
# direct measurements of the value of $icode iota$$.
# There is no noise simulated with the data; i.e., it is equal to the
# 'true' value of $icode iota$$.
# On the other hand, its is modeled as if there is a 10% coefficient
# of variation in the data; i.e., as if there were measurement noise with
# standard deviation equal to 10% of the measurement value.
# There is a measured value for each age in the $code age_table$$
# that is greater than 20.
#
# $head Source Code$$
# $srcthisfile%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $end
# ---------------------------------------------------------------------------
# BEGIN PYTHON
# ------------------------------------------------------------------------
import sys
import os
import distutils.dir_util
import copy
test_program = 'example/user/jump_at_age.py'
if sys.argv[0] != test_program  or len(sys.argv) != 1 :
	usage  = 'python3 ' + test_program + '\n'
	usage += 'where python3 is the python 3 program on your system\n'
	usage += 'and working directory is the dismod_at distribution directory\n'
	sys.exit(usage)
print(test_program)
#
# import dismod_at
local_dir = os.getcwd() + '/python'
if( os.path.isdir( local_dir + '/dismod_at' ) ) :
	sys.path.insert(0, local_dir)
import dismod_at
#
# change into the build/example/user directory
distutils.dir_util.mkpath('build/example/user')
os.chdir('build/example/user')
# ------------------------------------------------------------------------
def iota_true(age) :
	if age <= 20.0 :
		return iota_near_zero
	else :
		return iota_after_20
# ------------------------------------------------------------------------
# Note that the a, t values are used for this example
def example_db (file_name) :
	#
	def fun_iota_parent(a, t) :
		if a <= 20.5 :
			return ('prior_up_to_20', 'prior_none', 'prior_difference')
		else :
			return ('prior_after_20', 'prior_difference', 'prior_difference')
	# ----------------------------------------------------------------------
	# age table (in age_list above)
	age_list = age_table
	#
	# time table
	time_list   = time_table
	#
	# integrand table
	integrand_table = [
		{ 'name':'Sincidence' }
	]
	#
	# node table: world
	node_table = [ { 'name':'world',         'parent':'' } ]
	#
	# weight table:
	weight_table = list()
	#
	# covariate table:
	covariate_table = list()
	#
	# mulcov table
	mulcov_table = list()
	#
	# avgint table: empty
	avgint_table = list()
	#
	# nslist_table:
	nslist_table = dict()
	# ----------------------------------------------------------------------
	# data table:
	data_table = list()
	#
	# values that are the same for all data rows
	row = {
		'node':        'world',
		'subgroup':    'world',
		'density':     'log_gaussian',
		'weight':      '',
		'hold_out':     False,
		'time_lower':   time_list[0],
		'time_upper':   time_list[0]
	}
	# Sincidence data
	data_age_list  = [ age for age in age_table if age > 20.0 ]
	for age in data_age_list :
		meas_value = iota_true(age)
		row['age_lower']    = age
		row['age_upper']    = age
		row['integrand']    = 'Sincidence'
		row['meas_value']   = meas_value
		row['meas_std']     = meas_value * 0.1
		row['eta']          = iota_eta
		data_table.append( copy.copy(row) )
	#
	# ----------------------------------------------------------------------
	# prior_table
	prior_table = [
		{	# prior_none
			'name':     'prior_none',
			'density':  'uniform',
			'lower':    -1.0,
			'upper':    +1.0,
			'mean':     0.0,
		},{ # prior_difference
			'name':     'prior_difference',
			'density':  'log_gaussian',
			'mean':     0.0,
			'std':      0.1,
			'eta':      iota_eta
		},{ # prior_up_to_20
			'name':     'prior_up_to_20',
			'density':  'uniform',
			'lower':    iota_near_zero,
			'upper':    iota_near_zero,
			'mean':     iota_near_zero,
		},{ # prior_after_20
			'name':     'prior_after_20',
			'density':  'uniform',
			'lower':    iota_near_zero,
			'upper':    1.0,
			'mean':     iota_after_20 / 4.0,
			'eta':      iota_eta,
		}
	]
	# ----------------------------------------------------------------------
	# smooth table
	#
	smooth_table = [
		{ # smooth_iota_parent
			'name':                     'smooth_iota_parent',
			'age_id':                   range( len(age_table) ),
			'time_id':                  [0],
			'fun':                      fun_iota_parent
		}
	]
	# ----------------------------------------------------------------------
	# rate table
	rate_table = [
		{
			'name':          'iota',
			'parent_smooth': 'smooth_iota_parent',
		}
	]
	# ----------------------------------------------------------------------
	# option_table
	option_table = [
		{ 'name':'parent_node_name',       'value':'world'             },
		{ 'name':'ode_step_size',          'value':'1.0'               },
		{ 'name':'random_seed',            'value':'0'                 },
		{ 'name':'rate_case',              'value':'iota_pos_rho_zero' },

		{ 'name':'quasi_fixed',            'value':'true'              },
		{ 'name':'derivative_test_fixed',  'value':'first-order'       },
		{ 'name':'max_num_iter_fixed',     'value':'200'               },
		{ 'name':'print_level_fixed',      'value':'0'                 },
		{ 'name':'tolerance_fixed',        'value':'1e-10'             },

		{ 'name':'derivative_test_random', 'value':'second-order'      },
		{ 'name':'max_num_iter_random',    'value':'100'               },
		{ 'name':'print_level_random',     'value':'0'                 },
		{ 'name':'tolerance_random',       'value':'1e-10'             }
	]
	# ----------------------------------------------------------------------
	# subgroup_table
	subgroup_table = [ { 'subgroup':'world', 'group':'world' } ]
	# ----------------------------------------------------------------------
	# create database
	dismod_at.create_database(
		file_name,
		age_list,
		time_list,
		integrand_table,
		node_table,
		subgroup_table,
		weight_table,
		covariate_table,
		avgint_table,
		data_table,
		prior_table,
		smooth_table,
		nslist_table,
		rate_table,
		mulcov_table,
		option_table
	)
	# ----------------------------------------------------------------------
# ===========================================================================
file_name = 'example.db'
example_db(file_name)
#
program = '../../devel/dismod_at'
dismod_at.system_command_prc([ program, file_name, 'init' ])
dismod_at.system_command_prc([ program, file_name, 'fit', 'both' ])
# -----------------------------------------------------------------------
# connect to database
new             = False
connection      = dismod_at.create_connection(file_name, new)
# -----------------------------------------------------------------------
# Results for fitting with no noise
var_table     = dismod_at.get_table_dict(connection, 'var')
fit_var_table = dismod_at.get_table_dict(connection, 'fit_var')
#
# check rates values
iota_rate_id      = 1
max_err           = 0.0
for var_id in range( len(var_table) ) :
	row     = var_table[var_id]
	rate_id = row['rate_id']
	assert row['var_type'] == 'rate'
	assert row['node_id']  == 0
	assert rate_id == iota_rate_id
	#
	age        = age_table[ row['age_id'] ]
	value      = fit_var_table[var_id]['fit_var_value']
	value_true = iota_true(age)
	rate       = 'iota'
	max_err = max(max_err, abs( value / value_true - 1.0 ) )
	if( abs(value / value_true - 1.0) > 1e-7 ) :
		print(rate, age, value / value_true - 1.0 )
assert max_err <= 1e-7
# -----------------------------------------------------------------------------
print('jump_at_age.py: OK')
# -----------------------------------------------------------------------------
# END PYTHON
