# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-18 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin db2csv_command.py$$ $newlinech #$$
# $spell
#	init
#	dismod
# $$
#
# $section init Command: Example and Test$$
#
# $code
# $srcfile%
#	example/get_started/db2csv_command.py
#	%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $$
# $end
# ---------------------------------------------------------------------------
# BEGIN PYTHON
import sys
import os
import copy
import subprocess
import distutils.dir_util
import csv
import math
# ---------------------------------------------------------------------------
# check execution is from distribution directory
example = 'example/get_started/db2csv_command.py'
if sys.argv[0] != example  or len(sys.argv) != 1 :
	usage  = 'python3 ' + example + '\n'
	usage += 'where python3 is the python 3 program on your system\n'
	usage += 'and working directory is the dismod_at distribution directory\n'
	sys.exit(usage)
#
# distribution directory
dist_dir  = os.getcwd()
#
# directory where test files are stored
test_dir  = dist_dir + '/build/example/get_started'
#
# import sandbox version of dismod_at
local_dir = dist_dir + '/python'
if( os.path.isdir( local_dir + '/dismod_at' ) ) :
	sys.path.insert(0, local_dir)
import dismod_at
#
# import get_started_db example
sys.path.append( os.getcwd() + '/example/get_started' )
import get_started_db
#
# change into the build/example/get_started directory
distutils.dir_util.mkpath(test_dir)
os.chdir(test_dir)
# ---------------------------------------------------------------------------
# create get_started.db
get_started_db.get_started_db()
# -----------------------------------------------------------------------
program        = '../../devel/dismod_at'
file_name      = 'get_started.db'
for command in [ 'init', 'fit' ] :
	cmd = [ program, file_name, command ]
	if command == 'fit' :
		variables = 'both'
		cmd.append(variables)
	print( ' '.join(cmd) )
	flag = subprocess.call( cmd )
	if flag != 0 :
		sys.exit('The dismod_at ' + command + ' command failed')
#
# change into distribution directory to run sandbox version of dismodat.py
# return to test_dir when done.
os.chdir(dist_dir)
print( os.getcwd() )
program        = 'bin/dismodat.py'
file_name      = test_dir + '/get_started.db'
command        = 'db2csv'
cmd            = [ program, file_name, command ]
#
print( ' '.join(cmd) )
flag = subprocess.call( cmd )
if flag != 0 :
	sys.exit('The dismod_at db2csv command failed')
os.chdir(test_dir)
# ---------------------------------------------------------------------------
# get_table
def get_table(name) :
	file_name = name + '.csv'
	file_ptr  = open(file_name, 'r')
	table     = list()
	reader    = csv.DictReader(file_ptr)
	for row in reader :
		table.append(row)
	file_ptr.close()
	return table
#
# near_equal
def near_equal(x, y, eps) :
	if y == 0.0 :
		return float(x) < eps
	return abs( float(x) / y - 1.0 ) <= eps
# --------------------------------------------------------------------------
omega_world       = 1e-2
income_multiplier = -1e-3
adjusted_omega    = omega_world * math.exp(income_multiplier * 1000.0)
meas_value        = math.exp( - adjusted_omega * 50.0 )
meas_std          = meas_value / 20.
# --------------------------------------------------------------------------
# data.csv
data_table = get_table('data')
#
# check data.csv correspond to the single row of data
assert len(data_table) == 1
row = data_table[0]
assert row['data_info']          == 'd1'
assert row['integrand']          == 'susceptible'
assert row['weight']             == 'constant_one'
assert row['density']            == 'gaussian'
assert row['eta']                == ''
assert row['nu']                 == ''
assert row['Delta']              == row['meas_std']
assert int( row['data_id'] )     == 0
assert int(row['out'])           == 0
assert float(row['age_lo'])      == 50.0
assert float(row['age_up'])      == 50.0
assert float(row['time_lo'])     == 2000.0
assert float(row['time_up'])     == 2000.0
assert float(row['income'])      == 1000.0
#
assert near_equal(row['residual'],   0.0,        1e-5)
assert near_equal(row['meas_value'], meas_value, 1e-5)
assert near_equal(row['avgint'],     meas_value, 1e-5)
assert near_equal(row['meas_std'],   meas_std,   1e-5)
# -----------------------------------------------------------------------
# variable.csv
variable_table = get_table('variable')
#
# res_value is empty because value prior is uniform
# sim_v, sim_a, sim_t are empty because have not run simulate command
always_empty  = [ 'integrand', 'depend', 'truth', 'sam_avg', 'sam_std' ]
always_empty += [ 'res_value', 'res_dage', 'res_dtime' ]
always_empty += [ 'std_v', 'eta_v', 'nu_v' ]
always_empty += [ 'sim_v', 'sim_a', 'sim_t' ]
for prefix in [ 'lower', 'upper', 'mean', 'std', 'eta', 'nu', 'density' ] :
	for suffix in ['_a', '_t'] :
		always_empty.append( prefix + suffix )
for row in variable_table :
	empty_field = copy.copy( always_empty )
	if row['var_type'] != 'rate' and row['var_type'] != 'mulcov_rate_value' :
		empty_field.append('rate')
	if row['var_type'].startswith('mulcov_') :
		empty_field.append('node')
	else :
		empty_field.append('covariate')
	for field in row :
		if field in empty_field :
			assert row[field] == ''
		else :
			assert row[field] != ''
	#
	assert row['rate']               == 'omega'
	assert row['fixed']              == 'true'
	assert row['density_v']          == 'uniform'
	assert float( row['age'] )       == 0.0
	assert float( row['time'] )      == 1995.0
	assert float( row['lag_dage'] )  == 0.0
	assert float( row['lag_dtime'] ) == 0.0
	assert near_equal(row['lag_value'], 0.0, 1e-5)
	if row['var_type'] == 'rate' :
		assert row['covariate']   == ''
		assert row['node']        == 'world'
		assert int( row['s_id'] ) == 0 # smooth_omega_parent
		assert near_equal(row['fit_value'], omega_world, 1e-5)
	else :
		assert row['var_type'] == 'mulcov_rate_value'
		assert row['covariate']   == 'income'
		assert row['node']        == ''
		assert int( row['s_id'] ) == 1 # smooth_income_multiplier
		assert near_equal(row['fit_value'], income_multiplier, 1e-5)
# -----------------------------------------------------------------------
print('db2csv_command: OK')
# END PYTHON