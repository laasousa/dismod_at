#! /bin/bash -e
# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-16 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
new_directories='
'
rename_files='
'
spell_files='
'
no_change_files='
'
#
rename_cmd='s|table.omh|database.omh|'
spell_cmd='s|^$spell|&\n\tcholeig|'
#
cat << EOF > junk.sed
s|size_t *( *DISMOD_AT_NULL_INT *)|DISMOD_AT_NULL_SIZE_T|g
EOF
# -----------------------------------------------------------------------------
if [ "$0" != "bin/batch_edit.sh" ]
then
	echo "bin/batch_edit.sh: must be executed from its parent directory"
	exit 1
fi
# -----------------------------------------------------------------------------
cp bin/batch_edit.sh $HOME/trash/batch_edit.sh
git reset --hard
# ----------------------------------------------------------------------------
# new directories
for dir in $new_directories
do
	if [ ! -e $dir ]
	then
		mkdir $dir
	fi
done
# ----------------------------------------------------------------------------
# rename files
for old in $rename_files
do
	new=`echo $old | sed -e "$rename_cmd"`
	echo_eval git mv $old $new
done
# ----------------------------------------------------------------------------
list_all=`bin/ls_files.sh`
for file in $list_all
do
	if [ "$file" != 'bin/batch_edit.sh' ]
	then
		echo_eval sed -f junk.sed -i $file
	fi
done
# ----------------------------------------------------------------------------
# files that have spelling changes
for file in $spell_files
do
	sed -e "$spell_cmd" -i $file
done
# ----------------------------------------------------------------------------
# files that should not change at all
list='
'
for file in $list
do
	echo_eval git checkout $file
done
# ----------------------------------------------------------------------------
# files that have hand edited and stored using 'git_new.sh to'
git_new='yes'
for name in add_list mod_list rm_list
do
	if [ ! -e new/$name ]
	then
		git_new='no'
	fi
done
if [ "$git_new" == 'yes' ]
then
	git_new.sh from
fi
# ----------------------------------------------------------------------------
cp $HOME/trash/batch_edit.sh bin/batch_edit.sh
echo 'batch_edit.sh: OK'
exit 0