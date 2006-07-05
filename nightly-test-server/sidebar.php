<?php

/*****************************************************
 *
 * Prints out a sidebar for results graph page
 *
 ******************************************************/
if(isset($measure_arr) && isset($program_arr)){
	$machine_row=getMachineInfo($machine,$mysql_link);
	$today_row = getNightInfo($night,$mysql_link);
	$cur_date = $today_row['added'];

	print "<a href=\"index.php\">Homepage</a><br><br>\n";
	if(isset($machine_row['nickname'])){
		print "Machine: <a href=\"machine.php?machine=$machine\">{$machine_row['nickname']}</a><br><br>\n";
	}
	else{
		print "Machine: <a href=\"machine.php?machine=$machine\">{$machine_row['name']}</a><br><br>\n";
	}
	print "Test dates:<br>\n<ul>\n";
	
	$measure_link="";
	foreach ($measure_arr as $x){
		$measure_link.="&measure[]=$x";
	}
	$program_link="";
	foreach ($program_arr as $x){
		$program_link.="&program[]=$x";
	}


	/********************** Creating list to future and past tests **********************/
	
	$next_stack = array();
	$next_query = getNightsResource($machine,$mysql_link,$cur_date,"2020-12-30 01:01:01");
	$x=0;
	while($x<7 && $x<mysql_affected_rows()-1 && $next = mysql_fetch_array($next_query)){
		array_push($next_stack, $next);
		$x++;
	}
	foreach ($next_stack as $x){
		$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$x['added']);
		print "\t<li><a href=\"test.php?machine=$machine&night={$x['id']}\">$date</a>\n";
		
	}
	mysql_free_result($next_query);	

	$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$today_row['added']);
	print "\t<li><h3><a href=\"test.php?machine=$machine&night=$night\">$date</a></h3>\n"; 
	
	$previous_query = getNightsResource($machine,$mysql_link,"2000-01-01 01:01:01","$cur_date");
	$x=0;
	$prev=mysql_fetch_array($previous_query); //eliminates current date
	while($x<7 && $prev=mysql_fetch_array($previous_query)){
		$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$prev['added']);
		print "\t<li><a href=\"resultsgraph.php?machine=$machine&night={$prev['id']}$end_url$start_url$measure_link$program_link\">$date</a>\n";
		$x++;
	}
	print "</ul>\n";
	mysql_free_result($previous_query);

}

/*****************************************************
 *
 * sidebar for test page
 *
 * print out link to homepage, then find links to the next 7 
 * days and the previous 7 dates. Also 
 * link to machine page.
 *
 ******************************************************/
else if($machine!=-1 && $night !=-1){
	$machine_row=getMachineInfo($machine,$mysql_link);
	$today_row = getNightInfo($night,$mysql_link);
	$cur_date = $today_row['added'];

	print "<a href=\"index.php\">Homepage</a><br><br>\n";
	if(isset($machine_row['nickname'])){
		print "Machine: <a href=\"machine.php?machine=$machine\">{$machine_row['nickname']}</a><br><br>\n";
	}
	else{
		print "Machine: <a href=\"machine.php?machine=$machine\">{$machine_row['name']}</a><br><br>\n";
	}
	print "Test dates:<br>\n<ul>\n";
	

	/********************** Creating list to future and past tests **********************/
	
	$next_stack = array();
	$next_query = getNightsResource($machine,$mysql_link,$cur_date,"2020-12-30 01:01:01","ASC");
	$next = mysql_fetch_array($next_query);
	$x=0;
	while($x<7 && $x<mysql_affected_rows()-1 && $next = mysql_fetch_array($next_query)){
		array_unshift($next_stack, $next);
		$x++;
	}
	foreach ($next_stack as $x){
		$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$x['added']);
		print "\t<li><a href=\"test.php?machine=$machine&night={$x['id']}\">$date</a>\n";
		
	}
	mysql_free_result($next_query);	

	$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$today_row['added']);
	print "\t<li><h3><a href=\"test.php?machine=$machine&night=$night\">$date</a></h3>\n"; 
	
	$previous_query = getNightsResource($machine,$mysql_link,"2000-01-01 01:01:01","$cur_date");
	$x=0;
	$prev=mysql_fetch_array($previous_query); //eliminates current date
	while($x<7 && $prev=mysql_fetch_array($previous_query)){
		$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$prev['added']);
		print "\t<li><a href=\"test.php?machine=$machine&night={$prev['id']}\">$date</a>\n";
		$x++;
	}
	print "</ul>\n";
	mysql_free_result($previous_query);
	
	$next_query = getNightsResource($machine,$mysql_link);
	print "<form method=GET action=\"test.php\">\n";
        print "<input type=hidden name=\"machine\" value=\"$machine\">\n";
	print "<select name=night>\n";
	while ($next = mysql_fetch_array($next_query)){
		print "<option value={$next['id']}>{$next['added']}\n";
	}
	
	print "</select><br>\n";
	print "<input type=submit value=\"Jump to Date\">\n";
	print "</form>\n";
	mysql_free_result($next_query);

}



/*****************************************************
 *
 * This is the sidebar for the machine page
 *
 * Print out a link to the homepage then print
 * out links to the last 20 tests
 *
 ******************************************************/
elseif($machine != -1 && $night == -1){
	print "<a href=\"index.php\">Homepage</a><br><br>\n";
	print "Test dates:<br>\n<ul>\n";
	$x=0;
	$machine_nights = getNightsResource($machine, $mysql_link); 
	while($x<20 && $temp_row=mysql_fetch_array($machine_nights)){
		$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$temp_row['added']);
		print "<li><a href=\"test.php?machine=$machine&night={$temp_row['id']}\">$date</a>\n";
		$x++;
	}	
	print "</ul>\n";
	mysql_free_result($machine_nights);	

	$next_query = getNightsResource($machine,$mysql_link);
        print "<form method=GET action=\"test.php\">\n";
        print "<input type=hidden name=machine value=\"$machine\">\n";
        print "<select name=night>\n";
        while ($next = mysql_fetch_array($next_query)){
                print "<option value={$next['id']}>{$next['added']}\n";
        }
        print "</select><br>\n";
        print "<input type=submit value=\"Jump to Date\">\n";
        print "</form>\n";
        mysql_free_result($next_query);
}



/*****************************************************
 *
 * This is the sidebar for the index
 *
 * Print out link to homepage, then a link to machine page, 
 * then link to every machine.
 *
 ******************************************************/
else{ /*if($machine ==-1 && night ==-1)*/

	print "<a href=\"index.php\">Homepage</a><br><br>\n";
	$list_o_machines = getMachineResource($mysql_link);
	print "<a href='testers.php'>Test machines:</a><br>\n<ul>\n";
	while($temp_row = mysql_fetch_array($list_o_machines)){
		if(strcmp($temp_row['nickname'],"")==0){
			print "<li><a href=\"machine.php?machine={$temp_row['id']}\">{$temp_row['name']}</a>\n";
		}
		else{
			print "<li><a href=\"machine.php?machine={$temp_row['id']}\">{$temp_row['nickname']}</a>\n";	
		}
	}
	print "</ul>\n";	

	mysql_free_result($list_o_machines);
}


?>
