<?php

$meaningfull_names = array('getcvstime_wall' => 'CVS Checkout Wall Time',
                        'getcvstime_cpu' => 'CVS Checkout CPU Time',
                        'configuretime_cpu' => 'Configure CPU Time',
                        'configuretime_wall' => 'Configure Wall Time',
                        'buildtime_cpu' => 'Build CPU Time',
                        'buildtime_wall' => 'Build Wall Time',
                        'dejagnutime_wall' => 'Dejagnu Wall Time',
                        'dejagnutime_cpu' => 'Dejagnu CPU Time',
                        'teststats_exppass' => 'Expected Test Passes',
                        'teststats_unexpfail' => 'Unexpected Test Failures',
                        'teststats_expfail' => 'Expected Test Failurs');


/*****************************************************
 *
 * Purpose: Get information about a certain machine
 * Returns: A hash of a table row with the keys
 *          being the column names 
 *
 *****************************************************/
function getMachineInfo($machine_id, $mysql_link){
	$query="SELECT * FROM machine WHERE id=$machine_id";
	$machine_query = mysql_query($query, $mysql_link) or die ("$night failed! " . mysql_error());
	$row = mysql_fetch_array($machine_query);
	mysql_free_result($machine_query);
	return $row;
}

/*****************************************************
 *
 * Purpose: Get information about all machines
 * Returns: A mysql query resource of machine table
 *          rows.
 *
 *****************************************************/
function getMachineResource($mysql_link){
	$machine_query = mysql_query("SELECT * FROM machine ORDER BY nickname ASC") or die (mysql_error());
	return $machine_query;
}

/*****************************************************
 *
 * Purpose: Get information about machines that have 
 *          submitted a test in the last month
 * Returns: A mysql query resource of machine table
 *          rows.
 *
 *****************************************************/
function getRecentMachineResource($mysql_link){
	$night_query = mysql_query("SELECT machine FROM night where DATE_SUB(NOW(), INTERVAL 1 MONTH) <= added") or die (mysql_error());
	$machines="where id=0";
	$machine_arr = array();
	while($row = mysql_fetch_array($night_query)){
		if(!isset($machine_arr["{$row['machine']}"])){
			$machine_arr["{$row['machine']}"]=1;
			$machines.=" or id={$row['machine']}";
		}
	}
	mysql_free_result($night_query);
	$machine_query = mysql_query("SELECT * FROM machine $machines ORDER BY nickname ASC") or die (mysql_error());

	return $machine_query;
}

/*****************************************************
 *
 * Purpose: Get information about machines that have not 
 *          submitted a test in the last month
 * Returns: A mysql query resource of machine table
 *          rows.
 *
 *****************************************************/
function getOldMachineResource($mysql_link){
	$night_query = mysql_query("SELECT machine FROM night where DATE_SUB(NOW(), INTERVAL 1 MONTH) > added") or die (mysql_error());
	$machines="where id=0";
	$machine_arr = array();
	while($row = mysql_fetch_array($night_query)){
		if(!isset($machine_arr["{$row['machine']}"])){
			$machine_arr["{$row['machine']}"]=1;
			$machines.=" or id={$row['machine']}";
		}
	}
	mysql_free_result($night_query);
	$machine_query = mysql_query("SELECT * FROM machine $machines ORDER BY nickname ASC") or die (mysql_error());

	return $machine_query;
}

/*****************************************************
 *
 * Purpose: Get information about a certain night
 * Returns: A hash of a table row with the keys
 *          being the column names 
 *
 *****************************************************/
function getNightInfo($night_id, $mysql_link){
	$query="SELECT * FROM night WHERE id=$night_id";
	$today_query = mysql_query("SELECT * FROM night WHERE id=$night_id") or die ("$query failed! " . mysql_error());
	$today_row = mysql_fetch_array($today_query);
	mysql_free_result($today_query);	
	return $today_row;
}

/*****************************************************
 *
 * Purpose: Get the nights associated with a specific machine
 * Returns: A mysql query resource. Basically something you cal
 *          mysql_fetch_array on.
 *
 *****************************************************/
function getNightsResource($machine_id, $mysql_link, $start="2000-01-01 01:01:01", $end="2020-12-30 01:01:01", $order="DESC"){
	$query = mysql_query("SELECT * FROM night WHERE machine=$machine_id and added<=\"$end\" and added>=\"$start\" order by added $order") or die (mysql_error());
	return $query;
}

/*****************************************************
 *
 * Purpose: Get all the tests in the last $hours hours
 * Returns: A mysql query resource.
 *
 *****************************************************/
function getRecentTests($hours="24", $mysql_link){
	$result = mysql_query("select * from night where DATE_SUB(NOW(),INTERVAL $hours HOUR)<=added ORDER BY added DESC") or die (mysql_error());
	return $result;
}

/*****************************************************
 *
 * Purpose: Calculate a date in the past given a length
 *          and an origin date.
 * Returns: A string formatted as "Year-month-day H:M:S" 
 *
 *****************************************************/
function calculateDate($mysql_link, $time_frame="1 YEAR", $origin_date="CURDATE()"){
	if(strpos($origin_date, "CURDATE")===false){
		$query=mysql_query("SELECT \"$origin_date\" - INTERVAL $time_frame") or die(mysql_error());
	}
	else{
		$query=mysql_query("SELECT $origin_date - INTERVAL $time_frame") or die(mysql_error());
	}
	$row = mysql_fetch_array($query);
	mysql_free_result($query);
	$time = $row[0];
	return $time;	
}

/*****************************************************
 *
 * Purpose: get file size for a specific file
 * Returns: An array with the first element being the
 * name of the file, then the file size, then night id,
 * then build type.
 *
 *****************************************************/
function get_file($mysql_link, $file, $night_id){
	$query = mysql_query("select * from file WHERE file=\"$file\" and night=$night_id") or die (mysql_error());
	$file = mysql_fetch_array($query);
	$result = array("{$file['file']}","{$file['size']}","{$file['night']}","{$file['type']}");
	mysql_free_result($query);
	return $result;
}

/*****************************************************
 *
 * Purpose: Get a list of all sizes measured on a 
 * particular machine for a specific file
 * Returns: an array with the key being the date and
 * the value being an array containing file name, size
 * night, and build type
 *
 *****************************************************/
function get_file_history($mysql_link, $machine_id, $file_name){
        $nights_select = "select id, added from night WHERE machine=$machine_id ".
	                 "order by added desc";
	$nights_query = mysql_query($nights_select) 
  				 or die (mysql_error());
	$result = array();
	while($row = mysql_fetch_array($nights_query)){
		$file_select = "select * from file where night={$row['id']} and ".
			       "file=\"$file_name\"";
		$file_query = mysql_query($file_select);
		$file_array = mysql_fetch_array($file_query);
		if(isset($file_array['file'])){
		  array_unshift($file_array['size'], "{$row['added']}");
		  array_push($result, $file_array['size']);
		}//end if
		mysql_free_result($file_query);
	}//end while
	mysql_free_result($nights_query);
	return $result;
}

/*****************************************************
 *
 * Example uses of each function
 *
 *****************************************************/
/*$mysql_link = mysql_connect("127.0.0.1","llvm","ll2002vm");
mysql_select_db("nightlytestresults");

$machine_id = 8;
$file="./test/Regression/Archive/xpg4.a";

$files = get_file_history($mysql_link, $machine_id, $file);

foreach (array_keys($files) as $f){
	print "$f = > {$files["$f"][0]}<br>\n";
}

$machine_info = getMachineInfo(21, $mysql_link);
foreach (array_keys($machine_info) as $key){
	print "$key => {$machine_info["$key"]}<br>\n";
}
print "<br><br><br>\n";

$night_id=-1;
$night_resource = getNightsResource($machine_info['id'], $mysql_link);
	while($row = mysql_fetch_array($night_resource)){
	print "added => {$row['added']}<br>\n";
	$night_id=$row['id'];
}
mysql_free_result($night_resource);
print "<br><br><br>\n";


$night_info = getNightInfo($night_id, $mysql_link);
print "buildstatus => {$night_info['buildstatus']}<br>\n";
print "<br><br><br>\n";

$recent_resource = getRecentTests($machine_info['id'], $mysql_link);
	while($row = mysql_fetch_array($recent_resource)){
	print "added => {$row['added']}<br>\n";
	$night_id=$row['id'];
}
mysql_free_result($recent_resource);
print "<br><br><br>\n";

$my_day = calculateDate($mysql_link,"30 WEEK");
print "today's date - 30 weeks = $my_day<br>\n";
print "<br><br><br>\n";
*/


?>
