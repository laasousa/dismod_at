var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_up0 = [
'dismod_at.htm',
'python.htm',
'unicode_tuple.htm'
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
'create_connection.htm',
'create_table.htm',
'create_database.htm',
'db2csv_command.htm',
'get_name_type.htm',
'get_row_list.htm',
'get_table_dict.htm',
'metropolis.htm',
'modify_command.htm',
'unicode_tuple.htm',
'sql_command.htm'
];
var list_down0 = [
'unicode_tuple.py.htm'
];
var list_current0 = [
'unicode_tuple.htm#Syntax',
'unicode_tuple.htm#iterable',
'unicode_tuple.htm#quote_string',
'unicode_tuple.htm#None',
'unicode_tuple.htm#Infinity',
'unicode_tuple.htm#u_tuple',
'unicode_tuple.htm#Example'
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
