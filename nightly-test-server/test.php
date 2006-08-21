<?php
/******************************
 *
 * Checking input variables
 *
 ******************************/
if(!isset($HTTP_GET_VARS['machine']) || !is_numeric($HTTP_GET_VARS['machine'])){
        print "Error: Incorrect URL!\n";
        die();
}
$machine_id = $HTTP_GET_VARS['machine'];

if(!isset($HTTP_GET_VARS['night']) || !is_numeric($HTTP_GET_VARS['night'])){
        print "Error: Incorrect URL!\n";
        die();
}
$night_id = $HTTP_GET_VARS['night'];


if(!(include "NightlyTester.php")){
        print "Error: could not load necessary files!\n";
        die();
}

if(!(include"ProgramResults.php")){
        print "Error: could not load necessary files!\n";
        die();
}

$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$row = getMachineInfo($machine_id,$mysql_link);
$today_row = getNightInfo($night_id,$mysql_link);
$cur_date=$today_row['added'];

$today_query = getSuccessfulNightsHistory($machine_id,$mysql_link,$night_id);
$today_row = mysql_fetch_array($today_query);
$yesterday_row = mysql_fetch_array($today_query);
$oldday_row = mysql_fetch_array($today_query);
mysql_free_result($today_query);
$previous_succesful_id = $yesterday_row['id'];
?>

<html>
<head>
<title>LLVM Nightly Test Results For <?php print $cur_date; ?></title>
<STYLE TYPE="text/css">
<!--
  @import url(style.css);
-->
</STYLE>
<script type="text/javascript" src="sorttable.js"></script>
<script type="text/javascript" src="popup.js"></script>
</head>
<body>

<center><font size=+3 face=Verdana><b>LLVM Nightly Test Results For <?php print $cur_date; ?></b></font></center><br>

<table cellspacing=0 cellpadding=0 border=0>
  <tr>
    <td valign=top width="180">
      <? 
      $machine = $HTTP_GET_VARS['machine'];
      $night = $HTTP_GET_VARS['night'];
      include 'sidebar.php'; 
      ?>      
    </td>
    <td>
<?php

/*****************************************************
 *
 * Printing machine information
 *
 ******************************************************/
print "<table border=1 cellpadding=0 cellspacing=0>\n";
print "<tr>\n";
print "<td><b>Nickname:</b></td>";
print "<td>{$row['nickname']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>uname:</b></td>";
print "<td>{$row['uname']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Hardware:</b></td>";
print "<td>{$row['hardware']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>OS:</b></td>";
print "<td>{$row['os']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Hostname:</b></td>";
print "<td>{$row['name']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>GCC:</b></td>";
print "<td>{$row['gcc']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Machine ID:</b></td>";
print "<td>{$row['id']}</td>\n";
print "</tr>\n";
print "</table>\n<br>\n";

/*****************************************************
 *
 * Printing link to build log
 *
 ******************************************************/
print"<h4><a href=\"fulltest.php?machine=$machine_id&night=$night_id\">See Full Test Results</a></h4>\n";

$buildfile=str_replace(" ", "_", $cur_date);
if(file_exists("machines/$machine_id/$buildfile-Build-Log.txt")){
  print "<h4><a href=\"machines/$machine_id/$buildfile-Build-Log.txt\">".
      "View Build Log</a></h4>\n";
}

/*****************************************************
 *
 * Printing the build status
 *
 ******************************************************/
if(strpos($today_row['buildstatus'], "OK")===FALSE){
  $disp="";
  $sign="(+)";
}
else{
  $disp="none";
  $sign="(-)";
}

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer".
    "('buildStatus');\", id=\"buildStatus_\">$sign Build Status</a></font>\n";
print "<div id=\"buildStatus\" style=\"display: $disp;\" class=\"hideable\">\n";
print "<h3><u>Build Status </u></h3></p>";
print "<font color=red>{$today_row['buildstatus']}</font><br>\n";
print "</div><br><br>\n";

/*****************************************************
 *
 * Printing changes in test suite
 *
 ******************************************************/
$new_tests=getNewTests($night_id, $previous_succesful_id, $mysql_link);
if(strcmp($new_tests,"")===0){
  $new_tests="None";
}
$removed_tests=getRemovedTests($night_id, $previous_succesful_id, $mysql_link);
if(strcmp($removed_tests,"")===0){
  $removed_tests="None";
}
$newly_passing_tests=getFixedTests($night_id, $previous_succesful_id, $mysql_link);
if(strcmp($newly_passing_tests,"")===0){
  $newly_passing_tests="None";
}
$newly_failing_tests=getBrokenTests($night_id, $previous_succesful_id, $mysql_link);
if(strcmp($newly_failing_tests,"")===0){
  $newly_failing_tests="None";
}

if(strpos($new_tests, "None")!==FALSE &&
   strpos($removed_tests, "None")!==FALSE &&
   strpos($newly_passing_tests, "None")!==FALSE &&
   strpos($newly_failing_tests, "None")!==FALSE ){
  $disp="none";
  $sign="(-)";
}
else{
  $disp="";
  $sign="(+)";
}

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('testSuite');\", id=\"testSuite_\">$sign Test Suite Changes</a></font>\n";
print "<div id=\"testSuite\" style=\"display: $disp;\" class=\"hideable\">\n";
print"<h3><u>Test suite changes:</u></h3>\n";
print"<b>New tests:</b><br>\n";
print "$new_tests<br><br>\n";
print"<b>Removed tests:</b><br>\n";
print "$removed_tests<br><br>\n";
print"<b>Newly passing tests:</b><br>\n";
print "$newly_passing_tests<br><br>\n";
print"<b>Newly failing tests:</b><br>\n";
print "$newly_failing_tests<br><br>\n";
print "</div><br><br>\n";

/*****************************************************
 *
 * Printing failures in test suite
 *
 ******************************************************/
$failing_tests=getFailures($night_id, $previous_succesful_id, $mysql_link);
if(strcmp($failing_tests,"")===0){
  $newly_failing_tests="None";
}
$disp="none";
$sign="(-)";
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('testSuiteFailures');\", id=\"testSuite_\">$sign Test Suite Failures</a></font>\n";
print "<div id=\"testSuiteFailures\" style=\"display: $disp;\" class=\"hideable\">\n";
print"<h3><u>Test suite failures:</u></h3>\n";
print"<b>Failing tests:</b><br>\n";
print "$failing_tests<br><br>\n";
print "</div><br><br>\n";

/*****************************************************
 *
 * Dejagnu result go here
 *
 ******************************************************/
$delta_exppass = $today_row['teststats_exppass']-$yesterday_row['teststats_exppass'];
$delta_expfail = $today_row['teststats_expfail']-$yesterday_row['teststats_expfail'];
$delta_unexpfail = $today_row['teststats_unexpfail']-$yesterday_row['teststats_unexpfail'];
$unexpected_failures = getUnexpectedFailures($night_id, $mysql_link);

if($delta_exppass==0 && $delta_expfail==0 && 
   $delta_unexpfail==0 && strcmp($unexpected_failures,"")===0){
  $disp="none";
        $sign="(-)";
}
else{
  $disp="";
  $sign="(+)";
}


if(isset($today_row['teststats_exppass'])){
  $exp_pass=$today_row['teststats_exppass'];
}
else{
  $exp_pass=0;
}
if(isset($today_row['teststats_unexpfail'])){
  $unexp_fail=$today_row['teststats_unexpfail'];
}
else{
  $unexp_fail=0;
}
if(isset($today_row['teststats_expfail'])){
  $exp_fail=$today_row['teststats_expfail'];  
}
else{
  $exp_fail=0;

}

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('dejagnuTests');\", id=\"dejagnuTests_\">$sign Dejagnu Tests</a></font>\n";
print "<div id=\"dejagnuTests\" style=\"display: $disp;\" class=\"hideable\">\n";

print"<h3><u>Dejagnu tests:</u></h3><br>\n";

print "<table>\n";
print "\t<tr>\n";
print "\t\t<td></td><td># of tests</td><td>Change from yesterday</td>\n";
print "\t</tr>\n";
$delta = $today_row['teststats_exppass']-$yesterday_row['teststats_exppass'];
print "\t\t<td>Expected Passes:</td><td align=center>$exp_pass</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
$delta = $today_row['teststats_unexpfail']-$yesterday_row['teststats_unexpfail'];
print "\t\t<td>Unexpected Failures:</td><td align=center>$unexp_fail</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
$delta = $today_row['teststats_expfail']-$yesterday_row['teststats_expfail'];
print "\t\t<td>Expected Failures:</td><td align=center>$exp_fail</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "</table><br><br>\n";

print"<a name=\"unexpfail_tests\"><b>Unexpected test failures:</b></a><br>\n";
print "$unexpected_failures<br><br>\n";

print "</div><br><br>\n";


/*****************************************************
 *
 * Printing warning information
 *
 ******************************************************/
if((strpos($today_row['warnings_added'], "none")===FALSE &&
   strpos($today_row['warnings_removed'], "none")===FALSE) &&
   (strcmp($today_row['warnings_added'], "")!=0 &&
   strcmp($today_row['warnings_removed'], "")!=0)){
  $disp=" ";
  $sign="(+)";
}
else{
  $disp="none";
  $sign="(-)";
}
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('warningsChanges');\", id=\"warningsChanges_\">$sign Warnings Information</a></font>\n";
print "<div id=\"warningsChanges\" style=\"display: $disp;\" class=\"hideable\">\n";
print"<h3><u>Changes to warnings during the build:</u></h3>\n";
print"<b>New Warnings:</b><br>\n";
print "{$today_row['warnings_added']}<br><br>\n";
print"<b>Removed Warnings:</b><br>\n";
print "{$today_row['warnings_removed']}<br>\n";
print"<a name=\"warnings\"><h3><u>Warnings during the build:</u></h3></a><tt>{$today_row['warnings']}</tt><br>\n";
print "</div><br><br>\n";


/*****************************************************
 *
 * Printing execution
 *
 ******************************************************/

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('executionTimes');\", id=\"executionTimes_\">(-) Execution Times</a></font>\n";
print "<div id=\"executionTimes\" style=\"display: none;\" class=\"hideable\">\n";
print"<h3><u>Execution times in seconds:</u></h3><br>\n";
print "<table border=1 cellpadding=0 cellspacing=0>\n";

print "\t<tr>\n";
print "\t\t<td></td>\n";
print "\t\t<td>CVS cpu</td>\n";
print "\t\t<td>CVS wall</td>\n";
print "\t\t<td>Configure cpu</td>\n";
print "\t\t<td>Configure wall</td>\n";
print "\t\t<td>Build cpu</td>\n";
print "\t\t<td>Build wall</td>\n";
print "\t\t<td>Dejagnu cpu</td>\n";
print "\t\t<td>Dejagnu wall</td>\n";
print "\t</tr>\n";

print "\t<tr>\n";
print "\t\t<td>$cur_date</td>\n";
print "\t\t<td>{$today_row['getcvstime_cpu']}</td>\n";
print "\t\t<td>{$today_row['getcvstime_wall']}</td>\n";
print "\t\t<td>{$today_row['configuretime_cpu']}</td>\n";
print "\t\t<td>{$today_row['configuretime_wall']}</td>\n";
print "\t\t<td>{$today_row['buildtime_cpu']}</td>\n";
print "\t\t<td>{$today_row['buildtime_wall']}</td>\n";
print "\t\t<td>{$today_row['dejagnutime_cpu']}</td>\n";
print "\t\t<td>{$today_row['dejagnutime_wall']}</td>\n";
print "\t</tr>\n";

if( isset($yesterday_row) ){
  print "\t<tr>\n";
  print "\t\t<td>Previous nightly test ({$yesterday_row['added']})</td>\n";
  print "\t\t<td>{$yesterday_row['getcvstime_cpu']}</td>\n";
  print "\t\t<td>{$yesterday_row['getcvstime_wall']}</td>\n";
  print "\t\t<td>{$yesterday_row['configuretime_cpu']}</td>\n";
  print "\t\t<td>{$yesterday_row['configuretime_wall']}</td>\n";
  print "\t\t<td>{$yesterday_row['buildtime_cpu']}</td>\n";
  print "\t\t<td>{$yesterday_row['buildtime_wall']}</td>\n";
  print "\t\t<td>{$yesterday_row['dejagnutime_cpu']}</td>\n";
  print "\t\t<td>{$yesterday_row['dejagnutime_wall']}</td>\n";
  print "\t</tr>\n";


  print "\t<tr>\n";
  print "\t\t<td>% change</td>\n";
  
  if($yesterday_row['getcvstime_cpu']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['getcvstime_cpu'] - $yesterday_row['getcvstime_cpu'])/$yesterday_row['getcvstime_cpu']) * 100,2);  
    $color=DetermineColor($delta, "white");
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }  

  $color="white";
  if($yesterday_row['getcvstime_wall']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['getcvstime_wall'] - $yesterday_row['getcvstime_wall'])/$yesterday_row['getcvstime_wall']) * 100,2);
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }
  
  $color="white";
  if($yesterday_row['configuretime_cpu']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['configuretime_cpu'] - $yesterday_row['configuretime_cpu'])/$yesterday_row['configuretime_cpu']) * 100,2);
          $color=DetermineColor($delta, "white");
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }
  
  $color="white";

  if($yesterday_row['configuretime_wall']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['configuretime_wall'] - $yesterday_row['configuretime_wall'])/$yesterday_row['configuretime_wall']) * 100,2);
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }

  $color="white";
  if($yesterday_row['buildtime_cpu']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['buildtime_cpu'] - $yesterday_row['buildtime_cpu'])/$yesterday_row['buildtime_cpu']) * 100,2);
          $color=DetermineColor($delta, "white");
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }

  $color="white";
  if($yesterday_row['buildtime_wall']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['buildtime_wall'] - $yesterday_row['buildtime_wall'])/$yesterday_row['buildtime_wall']) * 100,2);
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }
  
  $color="white";
  if($yesterday_row['dejagnutime_cpu']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['dejagnutime_cpu'] - $yesterday_row['dejagnutime_cpu'])/$yesterday_row['dejagnutime_cpu']) * 100,2);
    $color=DetermineColor($delta, "white");
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }

  $color="white";
  if($yesterday_row['dejagnutime_wall']==0){
    print "\t\t<td>-</td>\n";
  }
  else{
    $delta = round((($today_row['dejagnutime_wall'] - $yesterday_row['dejagnutime_wall'])/$yesterday_row['dejagnutime_wall']) * 100,2);
    print "\t\t<td bgcolor=$color>$delta</td>\n";
  }
  
  print "\t</tr>\n";
}

print "</table><br>\n";

print "</div><br><br>\n";

/*****************************************************
 *
 * Printing CVS information
 *
 ******************************************************/
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('CVSInformation');\", id=\"CVSInformation_\">(-) CVS Information</a></font>\n";
print "<div id=\"CVSInformation\" style=\"display: none;\" class=\"hideable\">\n";

print"<h3><u>CVS information:</u></h3><br>\n";

$row = getNightInfo($night_id,$mysql_link);
$com_users = $row['cvs_usersadd'];
$co_users  = $row['cvs_usersco'];
$com_users = str_replace("\n","<br>",$com_users);
$co_users = str_replace("\n","<br>",$co_users);
print "<table border=1 cellspacing=0 cellpadding=0>\n";
print "\t<tr>\n";
print "\t\t<td>Users who commited</td><td>Users who checked out</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
print "\t\t<td valign=top>$com_users</td><td valign=top>$co_users</td>\n";
print "\t</tr>\n";
print "</table><br><br>\n";

print"<b>Added files:</b><br>\n";
$added_files  = $row['cvs_added'];
if(strcmp($added_files,"")!=0){
  $added_files = str_replace("\n","<br>",$added_files);
  print "<table>\n";
  print "\t<tr>\n";
  print "\t\t<td>$added_files</td>\n";
  print "\t</tr>\n";
  print "</table><br><br>\n";
}
else{
  print "No removed files<br><br>\n";
}

print"<b>Removed files:</b><br>\n";
$removed_files  = $row['cvs_removed'];
if(strcmp($removed_files,"")!=0){
  $removed_files = str_replace("\n","<br>",$removed_files);
  print "<table>\n";
  print "\t<tr>\n";
  print "\t\t<td>$removed_files</td>\n";
  print "\t</tr>\n";
  print "</table><br><br>\n";
}
else{
  print "No removed files<br><br>\n";
}

print"<b>Modified files:</b><br>\n";
$modified_files  = $row['cvs_modified'];
if(strcmp($modified_files,"")!=0){
  $modified_files = str_replace("\n","<br>",$modified_files);
  print "<table>\n";
  print "\t<tr>\n";
  print "\t\t<td>$modified_files</td>\n";
  print "\t</tr>\n";
  print "</table><br>\n";
}
else{
  print "No removed files<br>\n";
}
print "</div><br><br>\n";

/*****************************************************
 *
 * ending sidebar table here
 *
 ******************************************************/
print "</td></tr></table>\n";

/*****************************************************
 *
 * Printing file size information
 *
 ******************************************************/
$all_data=buildFileSizeTable($mysql_link, $machine_id, $night_id);

$num_sig_changes=0;
foreach (array_keys($all_data) as $d){
  if( ($all_data["$d"][1]>$medium_change && $all_data["$d"][2]>$byte_threshold) ||
      ($all_data["$d"][1]<($medium_change*-1) && $all_data["$d"][2]<($byte_threshold*-1)) ) {
    $num_sig_changes++;
  }
}
if($num_sig_changes==0){
    $disp="none";
    $sign="(-)";
  }
  else{
    $disp="";
    $sign="(+)";
  }
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('file_sizes_layer');\", id=\"file_sizes_layer_\">$sign $num_sig_changes Significant Changes in File Size</a></font>\n";
print "<div id=\"file_sizes_layer\" style=\"display: $disp;\" class=\"hideable\">\n";

print "<form method=GET action=\"individualfilesizegraph.php\">\n";
print "<input type=hidden name=machine value=\"$machine_id\">\n";
print "<input type=hidden name=night value=\"$night_id\">\n";
print "<input type=hidden name=end value=\"$cur_date\">\n";


$unformatted_num=number_format($all_data['Total Sum'][0],0,".",",");
print "<b>Total size</b>: $unformatted_num bytes<br>\n";
print "<b>Percent difference from previous test</b>: {$all_data['Total Sum'][1]}<br>\n";
print "<b>Percent difference from five tests ago</b>: {$all_data['Total Sum'][2]}<br><br>\n";

print "<b>Significant Changes in File Size<br></b>";
print "<table border='0' cellspacing='0' cellpadding='2'><tr><td bgcolor=#000000>\n"; #creating the black border
print "<table class=\"sortable\" id=\"file_sizes\" border='1' cellspacing='0' cellpadding='0'>\n";
print "\t<tr bgcolor=#FFCC99>\n";
print "\t\t<td>File</td>\n";
print "\t\t<td>File Size in Bytes</td>\n";
print "\t\t<td>% difference from previous test</td>\n";
print "\t\t<td>Byte difference from previous test</td>\n";
print "\t\t<td>% difference from five tests ago</td>\n";
print "\t\t<td>Byte difference from five tests ago</td>\n";
print "\t</tr>\n";

print "\t<tr>\n";
print "<td></td>\n";
print "<td></td>\n";
print "<td></td>\n";
print "<td></td>\n";
print "<td></td>\n";
print "\t</tr>\n";

$row_color=0;
foreach (array_keys($all_data) as $d){
  if( ($all_data["$d"][1]>$medium_change && $all_data["$d"][2]>$byte_threshold) || 
      ($all_data["$d"][1]<($medium_change*-1) && $all_data["$d"][2]<($byte_threshold*-1)) ) {
   
    if($row_color % 2 == 0){
      $def_color="white";
    } else{
      $def_color="#DDDDDD";
    }
    $row_color++;

    print "\t<tr bgcolor=\"$def_color\">\n";
    if(strcmp($d, "Total Sum")!=0){
      print "\t\t<td><input type=checkbox name=files[] multiple=\"multiple\" value=\"$d\" >\n";
    }
    else{
      print "\t\t<td>\n";
    }
    print "\t\t$d</td>\n";
    print "\t\t<td>{$all_data["$d"][0]}</td>\n";

    $color="bgcolor=\"".DetermineColor($all_data["$d"][1], "$def_color")."\"";
    print "\t\t<td $color>{$all_data["$d"][1]}</td>\n";
    print "\t\t<td $color>{$all_data["$d"][2]}</td>\n";

    $color="bgcolor=\"".DetermineColor($all_data["$d"][3], "$def_color")."\"";
    print "\t\t<td $color>{$all_data["$d"][3]}</td>\n";
    print "\t\t<td $color>{$all_data["$d"][4]}</td>\n";

    print "\t</tr>\n";
  }
}

print "</table>\n";
print "</td></tr></table><br>\n"; #ending black border around table  
print "<input type=submit name=action value=\"Compare values\"> | ";
print "<input type=button value=\"Check all\" onClick=\"this.value=check(this.form.elements)\"> | \n"; 
print "<input type=reset>\n";
print "</form>\n";
print "</div><br><br>\n";

/*****************************************************
 *
 * Finding big changes in results table
 *
 ******************************************************/

$today_results = GetDayResults($today_row['id'], $category_array, $mysql_link);
if(isset($yesterday_row['id'])){
  $yesterday_results = GetDayResults($yesterday_row['id'], $category_array, $mysql_link);
  $percent_difference = CalculateChangeBetweenDays($yesterday_results, $today_results, .2);
}
if(isset($oldday_row['id'])){
  $oldday_results = GetDayResults($oldday_row['id'], $category_array, $mysql_link);
  $twoday_difference = CalculateChangeBetweenDays($oldday_results, $today_results, .01);
}


if(isset($percent_difference) && isset($twoday_difference)){
  $big_changes = getThreeDaySignifigantChanges($today_results, $yesterday_results, $oldday_results, $percent_difference, $twoday_difference, $category_print_array);
  sortSignifigantChangeArray($big_changes, 3);
}
else if(isset($percent_difference) && !isset($twoday_difference)){
  $big_changes = getTwoDaySignifigantChanges($today_results, $yesterday_results, $percent_difference, $category_print_array);
        sortSignifigantChangeArray($big_changes, 3);
}

/********************** Regressions table **********************/

if(!isset($big_changes)){
  print "Cannot compare today's results to previous results. Reason: there are no previous results!<br>\n";
}
else{

$row_color=1;
$count=0;
for($y=0; $y<sizeof($category_print_array_ordered); $y++){
  print "<form method=GET action=\"resultsgraph.php\">\n";
  print "<input type=hidden name=machine value=\"$machine_id\">\n";
  print "<input type=hidden name=night value=\"$night_id\">\n";
  print "<input type=hidden name=end value=\"$cur_date\">\n";
  print "<input type=hidden name=measure[] value=\"{$category_print_array_ordered[$y]}\">\n";
  

  /* testing to see if we should show this table */
  $measure_number=0;
  for($pdj = 0; $pdj < sizeof($category_print_array); $pdj++){
    if(strcmp($category_print_array[$pdj],$category_print_array_ordered[$y])==0){
      $measure_number=$pdj;
    }
  }
  $num_changes = CountSignifigantDifferences($percent_difference, $measure_number, $medium_change);
  if($num_changes==0){
    $disp="none";
    $sign="(-)";
  }
  else{
    $disp="";
    $sign="(+)";
  }

  print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('{$category_print_array_ordered[$y]}');\", id=\"{$category_print_array_ordered[$y]}_\">$sign $num_changes Tests Significantly Changed for {$category_print_array_ordered[$y]}</a></font>\n";
  print "<div id=\"{$category_print_array_ordered[$y]}\" style=\"display: $disp;\" class=\"hideable\">\n";
  print "<b>Significant Changes for {$category_print_array_ordered[$y]}</b>";
  print "<span style=\"position:relative;\">\n";
  print "<span id=\"$y\" class=\"popup2\">\n";
  print "<pre>$category_print_array_ordered_description[$y]</pre>\n";
  print "</span><a href=\"javascript:void(0);\" onClick=\"TogglePop('$y');\">?</a></span>\n";
  print "<br>\n";
  print "<table border='0' cellspacing='0' cellpadding='2'><tr><td bgcolor=#000000>\n"; #creating the black borde
  print "<table class=\"sortable\" id=\"multisource_tests\" border='1' cellspacing='0' cellpadding='0'>\n";
  print "\t<tr bgcolor=#FFCC99>\n";
  print "\t\t<th>Program</th>\n";
  print "\t\t<th>% Change from yesterday</th>\n";
  print "\t\t<th>% Change from two days ago</th>\n";
  print "\t\t<th>Previous day's test value</th>\n";
  print "\t\t<th>Current day's test value</th>\n";
  print "\t</tr>\n";
  print "\t<tr><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n";

  foreach (array_keys($big_changes) as $x){
    if(strcmp($big_changes[$x][1],$category_print_array_ordered[$y])==0){
      if($row_color % 2 == 0){
        $def_color="white";
      } else{
        $def_color="#DDDDDD";
      }
      print "\t<tr bgcolor='$def_color'>\n";
      print "\t\t<td><input type=checkbox name=program[] multiple=\"multiple\" value=\"{$big_changes[$x][0]}\">{$big_changes[$x][2]}/{$big_changes[$x][0]}</td>\n";
      $color=DetermineColor($big_changes[$x][3], "#FFFFFF");
      print "\t\t<td bgcolor=\"$color\">{$big_changes[$x][3]}</td>\n";
      $color=DetermineColor($big_changes[$x][4], "#FFFFFF");
      print "\t\t<td bgcolor=\"$color\">{$big_changes[$x][4]}</td>\n";
      print "\t\t<td>{$big_changes[$x][5]}</td>\n";
      print "\t\t<td>{$big_changes[$x][6]}</td>\n";
                       
  
      /*for($y=0; $y<sizeof($big_changes[$x]); $y++){
                          print "\t\t<td>{$big_changes[$x][$y]}</td>\n";
                   }*/
                  print "\t</tr>\n";
                  $row_color++;
                  if($row_color > 4){
      $row_color=1;
                  }
                   $count++;
          }//end if strcmp
  }
  print "</table>\n";
  print "</td></tr></table><br>\n"; #ending black border around table  
  print "<input type=submit name=action value=\"Examine Longterm Results\"> | ";
  print "<input type=button value=\"Check all\" onClick=\"this.value=check(this.form.elements)\"> | \n"; 
  print "<input type=reset>\n";
  print "</form>\n";
  print "</div><br><br>\n";
}


}//end foreach


mysql_close($mysql_link);
?>



</body>
</html>


