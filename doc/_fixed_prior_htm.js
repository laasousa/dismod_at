var list_up0 = [
'dismod_at.htm',
'model.htm',
'fixed_prior.htm'
];
var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_current0 = [
'fixed_prior.htm#theta',
'fixed_prior.htm#theta.lambda',
'fixed_prior.htm#theta.beta',
'fixed_prior.htm#theta.theta',
'fixed_prior.htm#Value Constraints',
'fixed_prior.htm#Value Constraints.theta_k',
'fixed_prior.htm#Value Constraints.L_k^v',
'fixed_prior.htm#Value Constraints.U_k^v',
'fixed_prior.htm#Age Difference Limits',
'fixed_prior.htm#Age Difference Limits.a_i(k)',
'fixed_prior.htm#Age Difference Limits.Delta^a',
'fixed_prior.htm#Age Difference Limits.L_k^a',
'fixed_prior.htm#Age Difference Limits.U_k^a',
'fixed_prior.htm#Time Difference Limits',
'fixed_prior.htm#Theta',
'fixed_prior.htm#Normalization Constant, C_theta',
'fixed_prior.htm#p(theta)'
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
'model_variables.htm',
'avg_integrand.htm',
'data_like.htm',
'fixed_value.htm',
'fixed_diff.htm',
'fixed_prior.htm',
'random_value.htm',
'random_diff.htm',
'random_prior.htm',
'statistic.htm',
'bilinear.htm',
'ode_grid.htm',
'posterior.htm',
'prev_dep.htm'
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