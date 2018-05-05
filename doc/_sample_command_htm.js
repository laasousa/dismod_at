var list_up0 = [
'dismod_at.htm',
'command.htm',
'sample_command.htm'
];
var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_current0 = [
'sample_command.htm#Syntax',
'sample_command.htm#database',
'sample_command.htm#Purpose',
'sample_command.htm#number_sample',
'sample_command.htm#method',
'sample_command.htm#method.simulate',
'sample_command.htm#method.asymptotic',
'sample_command.htm#data_sim_table',
'sample_command.htm#prior_sim_table',
'sample_command.htm#fit_var_table',
'sample_command.htm#sample_table',
'sample_command.htm#Bounds',
'sample_command.htm#Example'
];
var list_down2 = [
'install_unix.htm',
'get_started.htm',
'user.htm',
'database.htm',
'model.htm',
'command.htm',
'python.htm',
'devel.htm',
'whats_new_2018.htm',
'wish_list.htm'
];
var list_down1 = [
'set_command.htm',
'init_command.htm',
'depend_command.htm',
'fit_command.htm',
'simulate_command.htm',
'sample_command.htm',
'predict_command.htm'
];
var list_down0 = [
'sample_command.py.htm'
];
function choose_up0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_up0[index-1];
}
function choose_across0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_across0[index-1];
}
function choose_current0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_current0[index-1];
}
function choose_down2(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down2[index-1];
}
function choose_down1(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down1[index-1];
}
function choose_down0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down0[index-1];
}