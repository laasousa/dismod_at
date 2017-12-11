var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_up0 = [
'dismod_at.htm',
'database.htm',
'input.htm',
'avgint_table.htm'
];
var list_down3 = [
'install_unix.htm',
'get_started.htm',
'user.htm',
'database.htm',
'model.htm',
'command.htm',
'python.htm',
'devel.htm',
'whats_new_2017.htm',
'wish_list.htm'
];
var list_down2 = [
'input.htm',
'data_flow.htm'
];
var list_down1 = [
'age_table.htm',
'avgint_table.htm',
'covariate_table.htm',
'data_table.htm',
'density_table.htm',
'integrand_table.htm',
'mulcov_table.htm',
'node_table.htm',
'nslist_pair_table.htm',
'nslist_table.htm',
'option_table.htm',
'prior_table.htm',
'rate_table.htm',
'smooth_grid_table.htm',
'smooth_table.htm',
'time_table.htm',
'weight_grid_table.htm',
'weight_table.htm'
];
var list_down0 = [
'avgint_table.py.htm'
];
var list_current0 = [
'avgint_table.htm#See Also',
'avgint_table.htm#Purpose',
'avgint_table.htm#Usage',
'avgint_table.htm#avgint_id',
'avgint_table.htm#integrand_id',
'avgint_table.htm#node_id',
'avgint_table.htm#weight_id',
'avgint_table.htm#age_lower',
'avgint_table.htm#age_upper',
'avgint_table.htm#time_lower',
'avgint_table.htm#time_upper',
'avgint_table.htm#Covariates',
'avgint_table.htm#Example'
];
function choose_across0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_across0[index-1];
}
function choose_up0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_up0[index-1];
}
function choose_down3(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down3[index-1];
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
function choose_current0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_current0[index-1];
}
