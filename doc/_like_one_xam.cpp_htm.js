var list_up0 = [
'dismod_at.htm',
'devel.htm',
'devel_model.htm',
'devel_data_model.htm',
'data_model_like_one.htm',
'like_one_xam.cpp.htm'
];
var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_down4 = [
'devel_table.htm',
'devel_utility.htm',
'devel_model.htm'
];
var list_down3 = [
'devel_data_model.htm',
'prior_model.htm',
'fit_model.htm',
'depend.htm'
];
var list_down2 = [
'data_model_ctor.htm',
'set_eigne_ode2_case_number.htm',
'data_model_replace_like.htm',
'data_model_reference_ode.htm',
'data_model_sci_ode.htm',
'data_model_avg_no_ode.htm',
'data_model_avg_yes_ode.htm',
'data_model_like_one.htm',
'data_model_like_all.htm'
];
var list_down1 = [
'like_one_xam.cpp.htm'
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
