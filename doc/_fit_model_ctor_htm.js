var list_up0 = [
'dismod_at.htm',
'devel.htm',
'devel_model.htm',
'fit_model.htm',
'fit_model_ctor.htm'
];
var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_down4 = [
'math_abstract.htm',
'install_unix.htm',
'get_started.htm',
'user.htm',
'database.htm',
'model.htm',
'command.htm',
'python.htm',
'devel.htm',
'whats_new_2019.htm',
'wish_list.htm'
];
var list_down3 = [
'devel_table.htm',
'devel_utility.htm',
'devel_model.htm'
];
var list_down2 = [
'devel_data_model.htm',
'prior_model.htm',
'fit_model.htm',
'depend.htm',
'adj_integrand.htm',
'devel_avg_integrand.htm',
'devel_avg_std_effect.htm'
];
var list_down1 = [
'fit_model_ctor.htm',
'fit_model_run_fit.htm',
'fit_model_get_solution.htm',
'fit_model_sample_posterior.htm'
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
function choose_down4(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down4[index-1];
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
