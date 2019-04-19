#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-19 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin user_cascade.py$$ $newlinech #$$
# $spell
#	covariate
#	covariates
#	cv
#	da
#	Sincidence
#	misspecification
# $$
#
# $section Generating Priors For Next Level Down The Node Tree$$
#
# $head Under Construction$$
# So far the following steps have been implemented:
# $cref/fit n1/user_cascade.py/Procedure/Fit n1/$$,
# $cref/sample data/user_cascade.py/Procedure/Simulate Data/$$,
# $cref/sample posterior/user_cascade.py/Procedure/Sample Posterior/$$.
#
# $head Node Table$$
# The following is a diagram of the $cref node_table$$:
# $pre
#                n1
#          /-----/\-----\
#        n11            n12
#      /     \        /     \
#    n111   n112    n121   n122
# $$
#
# $head Problem$$
# Given the information for a fit with $icode n1$$ as the parent
# with corresponding data $icode y1$$,
# pass down summary information for a fit with $icode n11$$ as the parent
# with corresponding data $icode y11$$.
#
# $head Procedure$$
#
# $subhead Fit n1$$
# Use $cref/fit both/fit_command/variables/both/$$
# to fit with $icode n1$$ as the parent to obtain
# $icode e1$$ the corresponding estimate for the $cref model_variables$$
#
# $subhead Simulate Data$$
# Set the $cref truth_var_table$$ equal to the estimate $icode e1$$
# and then use the $cref simulate_command$$ to simulate $icode N$$ data sets.
#
# $subhead Sample Posterior$$
# Use the sample command with the
# $cref/simulate/sample_command/method/simulate/$$ method
# to create $icode N$$ samples of the model variables.
# Call these samples $icode s1_1, ... , s1_N$$.
#
# $subhead n11 Predictions$$
# Use the predict command with the
# $cref/sample/predict_command/source/sample/$$
# to create $icode N$$ predictions for the
# model variable corresponding to fit with $icode n11$$ as the parent.
# Call these predictions $icode p11_1, ... , p11_N$$.
#
# $subhead n11 Priors$$
# Use the predictions $icode p11_1, ... , p11_N$$ to create a priors
# for the model variables corresponding to fitting $icode n11$$
# with data $icode y11$$.
# In this process account for the fact that the data $icode y11$$ is a subset
# of $icode y1$$ which was used to obtain the predictions.
#
# $subhead Fit n11$$
# Use $cref/fit both/fit_command/variables/both/$$
# to fit with $icode n11$$ as the parent to obtain
# $icode e1$$ corresponding estimate for the model variables.
#
# $head Problem Parameters$$
# The following parameters, used in this example, can be changed:
# $srcfile%example/user/cascade.py%
#   0%# begin problem parameters%# end problem parameters%1
# %$$
#
# $head Age and Time Values$$
# The time values do not matter for this problem
# because all the functions are constant with respect to time.
# The $cref age_table$$ for this problem is given by
# $srcfile%example/user/cascade.py%
#	0%# age_table%	# time_table%1
# %$$
# We use $icode n_age$$ to denote the length of this table.
#
# $head Rate Table$$
# The only rate in this problem is $icode iota$$. There are $icode n_age$$
# $cref/parent rate/model_variables/Fixed Effects, theta/Parent Rates/$$
# values for $icode iota$$, one for each point in the age table.
# There are two $icode iota$$
# $cref/child rate effects
#	/model_variables
#	/Random Effects, u
#	/Child Rate Effects
# /$$,
# one for each child node.
# (Note that there are two children when fitting $icode n1$$ as the parent
# and when fitting $icode n11$$ as the parent.)
#
# $head Covariates$$
# There are two $cref/covariates/covariate_table/$$ for this example.
# One covariate has the constant one and reference zero.
# The other covariate is income and uses the average for its reference.
# The average income is different depending on whether
# $icode n1$$ or $icode n11$$
# is the parent.
# There are two
# $cref/covariate multipliers
#	/model_variables
#	/Fixed Effects, theta
#	/Covariate Multipliers
# /$$.
# One multiples the constant one and models the unknown variation
# in the data (sometimes referred to as model misspecification).
# We call this covariate multiplier
# $cref/gamma/data_like/Measurement Noise Covariates/gamma_j/$$.
# The other multiplies income and affects $icode iota$$.
# We call this covariate multiplier
# $cref/alpha
#	/avg_integrand
#	/Rate Functions
#	/Rate Covariate Multiplier, alpha_jk
# /$$.
#
# $head Data Table$$
# For this example, all the data is
# $cref/Sincidence/avg_integrand/Integrand, I_i(a,t)/Sincidence/$$.
# There are $icode data_per_leaf$$ data point for each leaf node; i.e.,
# $icode n111, n112, n121, n122$$.
# Income is varies within each leaf node so the random effect
# can be separated from the income effect.
# Normally there is much more data, so we compensate by using
# a small coefficient of variation for the measurement values
# $icode meas_cv$$.
# The simulation value of $icode iota$$, corresponding to $icode n1$$, is
# a function of age and defined by $codei%iota_true(%age%)%$$.
# Each data point corresponds to a leaf node.
# The total effect for a data point is
# the random effect for the leaf node,
# plus the random effect for parent of the leaf,
# plus the income effect.
# Each data point is for a specific age and the corresponding mean
# is $icode%iota_true(%age%)%$$ times the exponential of the total effect.
# The standard deviation of the data is $icode meas_cv$$ times its mean.
# A Gaussian with this mean and standard deviation is used to simulate
# each data point.
#
# $head Source Code$$
# $srcfile%
#	example/user/cascade.py
#	%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $end
#
# $end
# ----------------------------------------------------------------------------
# BEGIN PYTHON
# begin problem parameters
def iota_true(age) :
	return 0.01 + 0.01 * age / 100.0 # must be non-decreasing with age
data_per_leaf =  10    # number of simulated data points for each leaf node
meas_cv       =  0.10  # coefficient of variation for each data point
alpha_true    = -0.10  # rate_value covariate multiplier used to simulate data
random_seed   =  0     # if zero, seed off the clock
number_sample =  10    # number of simulated data sets and posterior samples
#
random_effect = dict()
random_effect['n11']  =  0.2
random_effect['n12']  = -random_effect['n11']
random_effect['n111'] =  0.1
random_effect['n112'] = -random_effect['n111']
random_effect['n121'] =  0.1
random_effect['n122'] = -random_effect['n121']
#
average_income = dict()
average_income['n111'] = 1.0
average_income['n112'] = 2.0
average_income['n121'] = 3.0
average_income['n122'] = 4.0
# end problem parameters
# ----------------------------------------------------------------------------
import time
if random_seed == 0 :
	random_seed = int( time.time() )
#
import sys
import os
import distutils.dir_util
import subprocess
import copy
import random
import math
import numpy
test_program = 'example/user/cascade.py'
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
# ----------------------------------------------------------------------------
def system_command(command) :
	print( ' '.join(command) )
	flag = subprocess.call( command )
	if flag != 0 :
		sys.exit('command failed: flag = ' + str(flag))
	return
# ----------------------------------------------------------------------------
def constant_weight_fun(a, t) :
	return 1.0
def fun_iota_n1(a, t) :
	return ('prior_iota_n1_value', 'prior_iota_n1_dage', None)
def fun_iota_child(a, t) :
	return ('prior_iota_child', None, None)
def fun_alpha(a, t) :
	return ('prior_alpha', None, None)
def fun_gamma(a, t) :
	return ('prior_gamma', None, None)
#
def example_db (file_name) :
	# node_table
	node_table = [
		{ 'name':'n1',      'parent':''    },
		{ 'name':'n11',     'parent':'n1'  },
		{ 'name':'n12',     'parent':'n1'  },
		{ 'name':'n111',    'parent':'n11' },
		{ 'name':'n112',    'parent':'n11' },
		{ 'name':'n121',    'parent':'n12' },
		{ 'name':'n122',    'parent':'n12' },
	]
	# age_table
	age_list = [ 0.0, 20.0, 40.0, 60.0, 80.0, 100.0 ]
	# time_table
	time_list = [ 1990.0, 2020.0 ]
	# rate_table
	rate_table = [ {
		'name':          'iota',
		'parent_smooth': 'smooth_iota_n1',
		'child_smooth':  'smooth_iota_child',
	} ]
	# covariate_table
	reference_income = sum( average_income.values() ) / len( average_income )
	covariate_table = [
		{ 'name':'one',    'reference':0.0 },
		{ 'name':'income', 'reference':reference_income },
	]
	# mulcov_table
	mulcov_table = [ {
		'covariate': 'one',
		'type':      'meas_noise',
		'effected':  'Sincidence',
		'smooth':    'smooth_gamma'
		},{
		'covariate': 'income',
		'type':      'rate_value',
		'effected':  'iota',
		'smooth':    'smooth_income'
	} ]
	# prior_table
	prior_table = [
		{   # prior_iota_n1_value
			'name':    'prior_iota_n1_value',
			'density': 'uniform',
			'lower':   iota_true(0)   / 10.0,
			'upper':   iota_true(100) * 10.0,
			'mean':    iota_true(50)
		},{ # prior_iota_n1_dage
			'name':    'prior_iota_n1_dage',
			'density': 'log_gaussian',
			'mean':     0.0,
			'std':      0.5,
			'eta':      iota_true(0) / 10.0
		},{ # prior_iota_child
			'name':    'prior_iota_child',
			'density': 'gaussian',
			'mean':     0.0,
			'std':      1.0,
		},{ # prior_alpha
			'name':    'prior_alpha',
			'density': 'uniform',
			'mean':     0.0,
			'lower':   -abs(alpha_true) * 10.0,
			'upper':    abs(alpha_true) * 10.0,
		},{ # prior_gamma
			'name':    'prior_gamma',
			'density': 'uniform',
			'mean':     0.0,
			'std':      1.0,
			'lower':    0.0,
			'upper':   10.0,
		}
	]
	# smooth_table
	smooth_table = [
		{   # smooth_iota_n1
			'name' :    'smooth_iota_n1',
			'age_id':   range(len(age_list)),
			'time_id':  [0],
			'fun':      fun_iota_n1
		},{ # smooth_iota_child
			'name' :    'smooth_iota_child',
			'age_id':   [0],
			'time_id':  [0],
			'fun':      fun_iota_child
		},{ # smooth_income
			'name':     'smooth_income',
			'age_id':   [0],
			'time_id':  [0],
			'fun':      fun_alpha
		},{ # smooth_gamma
			'name':     'smooth_gamma',
			'age_id':   [0],
			'time_id':  [0],
			'fun':      fun_gamma
		}
	]
	# weight_table
	fun = constant_weight_fun
	weight_table = [
		{ 'name':'constant',  'age_id':[1], 'time_id':[1], 'fun':fun }
	]
	# nslist_table
	nslist_table = dict()
	# option_table
	option_table = [
		{ 'name':'parent_node_name',      'value':'n1'},
		{ 'name':'rate_case',             'value':'iota_pos_rho_zero'},
		{ 'name': 'zero_sum_random',      'value':'iota'},
		{ 'name': 'meas_noise_effect',    'value':'add_var_scale_all'},
		{ 'name':'quasi_fixed',           'value':'false'},
		{ 'name':'max_num_iter_fixed',    'value':'100'},
		{ 'name':'print_level_fixed',     'value':'0'},
		{ 'name':'tolerance_fixed',       'value':'1e-12'},
	]
	# integrand_table
	integrand_table = [ {'name':'Sincidence'} ]
	# ------------------------------------------------------------------------
	# data_table
	data_table = list()
	# values that are the same for all data points
	row = {
		'integrand':   'Sincidence',
		'density':     'gaussian',
		'weight':      'constant',
		'hold_out':    False,
		'time_lower':  2000.0,
		'time_upper':  2000.0,
		'one':         1.0,
	}
	assert covariate_table[1]['name'] == 'income'
	income_reference = covariate_table[1]['reference']
	random.seed(random_seed)
	for age_id in range( len(age_list) ) :
		age       = age_list[age_id]
		for node in average_income :
			for i in range(data_per_leaf) :
				income = i * average_income[node] * 2.0 / (data_per_leaf - 1)
				total_effect  = alpha_true * (income - income_reference)
				total_effect += random_effect[node]
				if node.startswith('n11') :
					total_effect += random_effect['n11']
				else :
					total_effect += random_effect['n12']
				#
				iota       = iota_true(age) * math.exp(total_effect)
				meas_std   = iota * meas_cv
				meas_value = random.gauss(iota, meas_std)
				row['node']       = node
				row['meas_value'] = meas_value
				row['meas_std']   = meas_std
				row['age_lower']  = age
				row['age_upper']  = age
				row['income']     = income
				data_table.append( copy.copy(row) )
	# ------------------------------------------------------------------------
	# avgint_table
	avgint_table = list()
	# values that are the same for all data points
	income = (average_income['n111'] + average_income['n112']) / 2.0
	row = {
		'node':        'n11',
		'integrand':   'Sincidence',
		'weight':      'constant',
		'hold_out':    False,
		'time_lower':  2000.0,
		'time_upper':  2000.0,
		'income':      income,
		'one':         1.0,
	}
	for age_id in range( len(age_list) ) :
		age  = age_list[age_id]
		row['age_lower']  = age
		row['age_upper']  = age
		avgint_table.append( copy.copy(row) )
	# ----------------------------------------------------------------------
	# create database
	dismod_at.create_database(
		file_name,
		age_list,
		time_list,
		integrand_table,
		node_table,
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
# ---------------------------------------------------------------------------
# create database
file_name  = 'example.db'
skip2predict = False
if not skip2predict :
	example_db(file_name)
#
# init
program = '../../devel/dismod_at'
if not skip2predict :
	system_command( [ program, file_name, 'init' ] )
#
# obtain e1, estimate of model variables with n1 as the parent node
if not skip2predict :
	system_command( [ program, file_name, 'fit', 'both' ] )
#
# check e1
new              = False
connection       = dismod_at.create_connection(file_name, new)
rate_table       = dismod_at.get_table_dict(connection, 'rate')
node_table       = dismod_at.get_table_dict(connection, 'node')
age_table        = dismod_at.get_table_dict(connection, 'age')
var_table        = dismod_at.get_table_dict(connection, 'var')
fit_var_table    = dismod_at.get_table_dict(connection, 'fit_var')
covariate_table  = dismod_at.get_table_dict(connection, 'covariate')
n_var            = len(var_table)
max_rel_err      = 0.0
for var_id in range(n_var) :
	var_type     = var_table[var_id]['var_type']
	age_id       = var_table[var_id]['age_id']
	rate_id      = var_table[var_id]['rate_id']
	node_id      = var_table[var_id]['node_id']
	covariate_id = var_table[var_id]['covariate_id']
	value    = fit_var_table[var_id]['fit_var_value']
	truth    = None
	if var_type == 'rate' :
		age  = age_table[age_id]['age']
		node = node_table[node_id]['node_name']
		rate = rate_table[rate_id]['rate_name']
		assert rate == 'iota'
		if node == 'n1' :
			truth = iota_true(age)
		else :
			truth = random_effect[node]
	elif var_type == 'mulcov_rate_value' :
		rate      = rate_table[rate_id]['rate_name']
		covariate = covariate_table[covariate_id]['covariate_name']
		assert rate == 'iota'
		assert covariate == 'income'
		truth = alpha_true
	else :
		covariate = covariate_table[covariate_id]['covariate_name']
		assert var_type == 'mulcov_meas_noise'
		assert covariate == 'one'
	if truth != None :
		rel_err = (1.0 - value / truth)
		if abs(rel_err) >= abs(max_rel_err) :
			max_rel_err = rel_err
			max_var_id  = var_id
if abs(max_rel_err) > 2e-1 :
	print('fit value: max_rel_err = ', max_rel_err)
	print("random_seed = ",  random_seed)
	assert False
#
# obtain s1_1, ... , s1_N
N_str = str(number_sample)
if not skip2predict :
	system_command([ program, file_name, 'set', 'truth_var', 'fit_var' ])
	system_command([ program, file_name, 'simulate', N_str ])
	system_command([ program, file_name, 'sample', 'simulate', N_str ])
#
# check posterior distribution of variables
connection.close()
connection   = dismod_at.create_connection(file_name, new)
sample_table = dismod_at.get_table_dict(connection, 'sample')
sample_array = numpy.zeros( (number_sample, n_var), dtype = numpy.double )
for sample_id in range( len(sample_table) ) :
	sample_index = sample_table[sample_id]['sample_index']
	var_id       = sample_table[sample_id]['var_id']
	var_value    = sample_table[sample_id]['var_value']
	assert sample_id == sample_index * n_var + var_id
	sample_array[sample_index, var_id] = var_value
sample_mean = numpy.mean(sample_array, axis=0)
sample_std  = numpy.std(sample_array, axis=0, ddof = 1)
for var_id in range(n_var) :
	var_type     = var_table[var_id]['var_type']
	age_id       = var_table[var_id]['age_id']
	node_id      = var_table[var_id]['node_id']
	covariate_id = var_table[var_id]['covariate_id']
	mean         = sample_mean[var_id]
	std          = sample_std[var_id]
	if var_type == 'rate' :
		age  = age_table[age_id]['age']
		node = node_table[node_id]['node_name']
		if node == 'n1' :
			truth = iota_true(age)
		else :
			truth = random_effect[node]
	elif var_type == 'mulcov_rate_value' :
		truth = alpha_true
	else :
		assert var_type == 'mulcov_meas_noise'
		gamma_fit = fit_var_table[var_id]['fit_var_value']
	if var_type != 'mulcov_meas_noise' :
		rel_err = (mean - truth) / std
		if abs(rel_err) >= abs(max_rel_err) :
			max_rel_err = rel_err
			max_var_id  = var_id
# We expand the std by a factor 1 + gamma because the correponding
# noise is probably not independent.
max_rel_err = max_rel_err / (1.0 + gamma_fit )
if abs(max_rel_err) > 3.0 :
	print('std coverage : max_rel_err = ', max_rel_err)
	print("random_seed = ",  random_seed)
	assert False
#
# obtain p11_1, p_11_2, ...
system_command([ program, file_name, 'predict', 'sample' ])
avgint_table    = dismod_at.get_table_dict(connection, 'avgint')
predict_table   = dismod_at.get_table_dict(connection, 'predict')
n_avgint        = len( avgint_table )
n_predict       = len( predict_table )
n_subset        = int( n_predict / number_sample )
assert n_predict == n_subset * number_sample
predict_array = numpy.zeros( (number_sample, n_avgint), dtype = numpy.double )
predict_found = n_avgint * [False]
for predict_id in range( n_predict ) :
	sample_index  = predict_table[predict_id]['sample_index']
	avgint_id     = predict_table[predict_id]['avgint_id']
	avg_integrand = predict_table[predict_id]['avg_integrand']
	predict_array[sample_index, avgint_id] = avg_integrand
	predict_found[avgint_id] = True
predict_mean = numpy.mean(predict_array, axis=0)
predict_std  = numpy.std(predict_array, axis=0, ddof = 1)
for avgint_id in range( n_avgint ) :
	assert predict_found[avgint_id]
	age  = avgint_table[avgint_id]['age_lower']
	mean = predict_mean[avgint_id]
	std  = predict_std[avgint_id]
	print("age, mean, std = ", age, mean, std)
# ----------------------------------------------------------------------------
os.chdir('../../..')
file_name = 'build/example/user/' + file_name
program   = 'bin/dismodat.py'
system_command( [ program, file_name, 'db2csv' ] )
print('cascade.py: OK')
# END PYTHON