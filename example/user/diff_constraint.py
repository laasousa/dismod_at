# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin user_diff_constraint.py$$ $newlinech #$$
# $spell
#	dage
#	dtime
# $$
#
# $section Constraints on Differences in Age and Time$$
#
# $code
# $verbatim%
#	example/user/diff_constraint.py
#	%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $$
# $end
# ---------------------------------------------------------------------------
# BEGIN PYTHON
import sys
import os
import distutils.dir_util
import subprocess
test_program = 'example/user/diff_constraint.py'
if sys.argv[0] != test_program  or len(sys.argv) != 1 :
	usage  = 'python3 ' + test_program + '\n'
	usage += 'where python3 is the python 3 program on your system\n'
	usage += 'and working directory is the dismod_at distribution directory\n'
	sys.exit(usage)
#
# import dismod_at
sys.path.append( os.getcwd() + '/python' )
import dismod_at
#
# change into the build/example/user directory
distutils.dir_util.mkpath('build/example/user')
os.chdir('build/example/user')
# ------------------------------------------------------------------------
def constant_weight_fun(a, t) :
	return 1.0
# note that the a, t values are not used for this case
def fun_pini_parent(a, t) :
	return ('prior_zero', 'prior_gauss_zero', 'prior_gauss_zero')
def fun_rate_child(a, t) :
	return ('prior_gauss_zero', 'prior_gauss_zero', 'prior_gauss_zero')
def fun_rate_parent(a, t) :
	return ('prior_value_parent', 'prior_diff_parent', 'prior_diff_parent')
# ------------------------------------------------------------------------
def example_db (file_name) :
	import copy
	import dismod_at
	# ----------------------------------------------------------------------
	# age table
	age_list    = [    0.0, 50.0,    100.0 ]
	#
	# time table
	time_list   = [ 1995.0, 2005.0, 2015.0 ]
	#
	# integrand table
	integrand_dict = [
		{ 'name':'prevalence',  'eta':1e-6 },
		{ 'name':'Sincidence',  'eta':1e-6 },
		{ 'name':'remission',   'eta':1e-6 },
		{ 'name':'mtexcess',    'eta':1e-6 },
		{ 'name':'mtother',     'eta':1e-6 }
	]
	#
	# node table: world -> north_america
	#             north_america -> (united_states, canada)
	node_dict = [
		{ 'name':'world',         'parent':'' },
		{ 'name':'north_america', 'parent':'world' },
		{ 'name':'united_states', 'parent':'north_america' },
		{ 'name':'canada',        'parent':'north_america' }
	]
	#
	# weight table: The constant function 1.0 (one age and one time point)
	fun = constant_weight_fun
	weight_dict = [
		{ 'name':'constant',  'age_id':[1], 'time_id':[1], 'fun':fun }
	]
	#
	# covariate table: no covriates
	covariate_dict = list()
	#
	# mulcov table
	mulcov_dict = list()
	# --------------------------------------------------------------------------
	# data table: same order as list of integrands
	data_dict = list()
	# values that are the same for all data rows
	row = {
		'node':        'world',
		'density':     'gaussian',
		'weight':      'constant',
		'hold_out':     False,
		'time_lower':   1995.0,
		'time_upper':   1995.0,
		'age_lower':    0.0,
		'age_upper':    0.0
	}
	# values that change between rows: (one data point for each integrand)
	for integrand_id in range( len(integrand_dict) ) :
		rate_id           = integrand_id
		meas_value        = 0.05
		meas_std          = 0.2 * meas_value
		integrand         = integrand_dict[integrand_id]['name']
		row['meas_value'] = meas_value
		row['meas_std']   = meas_std
		row['integrand']  = integrand
		# data_id = rate_id = integand_id
		data_dict.append( copy.copy(row) )
	# --------------------------------------------------------------------------
	# prior_table
	prior_dict = [
		{   # prior_zero
			'name':     'prior_zero',
			'density':  'uniform',
			'lower':    0.0,
			'upper':    0.0,
			'mean':     0.0,
			'std':      None,
			'eta':      None
		},{ # prior_one
			'name':     'prior_one',
			'density':  'uniform',
			'lower':    1.0,
			'upper':    1.0,
			'mean':     1.0,
			'std':      None,
			'eta':      None
		},{ # prior_gauss_zero
			'name':     'prior_gauss_zero',
			'density':  'gaussian',
			'lower':    None,
			'upper':    None,
			'mean':     0.0,
			'std':      0.01,
			'eta':      None
		},{ # prior_value_parent
			'name':     'prior_value_parent',
			'density':  'uniform',
			'lower':    0.01,
			'upper':    1.00,
			'mean':     0.1,
			'std':      None,
			'eta':      None
		},{ # prior_diff_parent
			'name':     'prior_diff_parent',
			'density':  'gaussian',
			'lower':    0.01,
			'upper':    None,
			'mean':     0.01,
			'std':      0.01,
			'eta':      None
		}
	]
	# --------------------------------------------------------------------------
	# smooth table
	middle_age_id  = 1
	middle_time_id = 1
	last_age_id    = 2
	last_time_id   = 2
	smooth_dict = [
		{   # smooth_rate_child
			'name':                     'smooth_rate_child',
			'age_id':                   [ 0, last_age_id ],
			'time_id':                  [ 0, last_time_id ],
			'mulstd_value_prior_name':  'prior_one',
			'mulstd_dage_prior_name':   'prior_one',
			'mulstd_dtime_prior_name':  'prior_one',
			'fun':                      fun_rate_child
		},{ # smooth_rate_parent
			'name':                     'smooth_rate_parent',
			'age_id':                   [ 0, last_age_id ],
			'time_id':                  [ 0, last_time_id ],
			'mulstd_value_prior_name':  'prior_one',
			'mulstd_dage_prior_name':   'prior_one',
			'mulstd_dtime_prior_name':  'prior_one',
			'fun':                       fun_rate_parent
		},{ # smooth_pini_parent
			'name':                     'smooth_pini_parent',
			'age_id':                   [ middle_age_id ],
			'time_id':                  [ middle_time_id ],
			'mulstd_value_prior_name':  'prior_one',
			'mulstd_dage_prior_name':   'prior_one',
			'mulstd_dtime_prior_name':  'prior_one',
			'fun':                       fun_pini_parent
		},{ # smooth_pini_child
			'name':                     'smooth_pini_child',
			'age_id':                   [ middle_age_id ],
			'time_id':                  [ middle_time_id ],
			'mulstd_value_prior_name':  'prior_one',
			'mulstd_dage_prior_name':   'prior_one',
			'mulstd_dtime_prior_name':  'prior_one',
			'fun':                       fun_rate_child
		}
	]
	# --------------------------------------------------------------------------
	# rate table
	rate_dict = [
		{
			'name':          'pini',
			'parent_smooth': 'smooth_pini_parent',
			'child_smooth':  'smooth_pini_child'
		},{
			'name':          'iota',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		},{
			'name':          'rho',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		},{
			'name':          'chi',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		},{
			'name':          'omega',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		}
	]
	# ------------------------------------------------------------------------
	# option_dict
	option_dict = [
		{ 'name':'parent_node_id',   'value':'0'            },
		{ 'name':'ode_step_size',    'value':'10.0'         },
		{ 'name':'random_seed',      'value':'0'            },
		{ 'name':'number_sample',    'value':'1'            },
		{ 'name':'rate_info',        'value':'chi_positive' },
		{ 'name':'tolerance',        'value':'1e-10'        },
		{ 'name':'max_num_iter',     'value':'100'          },
		{ 'name':'print_level',      'value':'0'            },
		{ 'name':'derivative_test',  'value':'second-order' }
	]
	# --------------------------------------------------------------------------
	# avg_case table: empty
	avg_case_dict = list()
	# --------------------------------------------------------------------------
	# create database
	dismod_at.create_database(
		file_name,
		age_list,
		time_list,
		integrand_dict,
		node_dict,
		weight_dict,
		covariate_dict,
		data_dict,
		prior_dict,
		smooth_dict,
		rate_dict,
		mulcov_dict,
		option_dict,
		avg_case_dict
	)
	# -----------------------------------------------------------------------
	n_smooth  = len( smooth_dict )
	rate_true = []
	for rate_id in range( len( data_dict ) ) :
		# for this particular example
		data_id    = rate_id
		meas_value = data_dict[data_id]['meas_value']
		rate_true.append(meas_value)
	#
	return
# ===========================================================================
file_name      = 'example.db'
example_db(file_name)
program        = '../../devel/dismod_at'
for command in [ 'init', 'start', 'fit' ] :
	cmd  = [ program, command, file_name ]
	print( ' '.join(cmd) )
	flag = subprocess.call( cmd )
	if flag != 0 :
		sys.exit('The dismod_at ' + command + ' command failed')
# -----------------------------------------------------------------------
# connect to database
new             = False
connection      = dismod_at.create_connection(file_name, new)
# -----------------------------------------------------------------------
# get parent rate variable values
var_dict     = dismod_at.get_table_dict(connection, 'var')
fit_var_dict = dismod_at.get_table_dict(connection, 'fit_var')
#
middle_age_id  = 1
middle_time_id = 1
last_age_id    = 2
last_time_id   = 2
parent_node_id = 0
n_rate         = 5
tol            = 1e-8
for rate_id in range(n_rate) :
	rate_value = dict()
	count      = 0
	for var_id in range( len(var_dict) ) :
		row   = var_dict[var_id]
		match = row['var_type'] == 'rate'
		match = match and row['rate_id'] == rate_id
		match = match and row['node_id'] == parent_node_id
		if match :
			age_id  = row['age_id']
			time_id = row['time_id']
			if age_id not in rate_value :
				rate_value[age_id] = dict()
			value = fit_var_dict[var_id]['fit_var_value']
			rate_value[age_id][time_id] = value
			count += 1
	if rate_id == 0 :
		# pini case
		assert count == 1
		assert abs( rate_value[middle_age_id][middle_time_id] ) < tol
	else :
		# other rates
		assert count == 4
		assert ( rate_value[0][0] / 0.05 - 1.0 ) < tol
		#
		diff  = rate_value[last_age_id][0] - rate_value[0][0]
		assert diff - 0.01 > - tol             # due to contraint
		assert abs(diff / 0.01 - 1.0) < 1e-3   # due to smoohting objective
		#
		diff  = rate_value[0][last_time_id] - rate_value[0][0]
		assert diff - 0.01 > - tol
		assert abs(diff / 0.01 - 1.0) < 1e-3
		#
		diff  = rate_value[last_age_id][0] - rate_value[0][0]
		assert diff - 0.01 > - tol
		assert abs(diff / 0.01 - 1.0) < 1e-3
		#
		diff  = rate_value[last_age_id][last_time_id] \
			- rate_value[last_age_id][0]
		assert diff - 0.01 > - tol
		assert abs(diff / 0.01 - 1.0) < 1e-3
# -----------------------------------------------------------------------------
print('diff_constraint.py: OK')
# -----------------------------------------------------------------------------
# END PYTHON