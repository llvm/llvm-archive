<?php

if ($argc != 3) {
  print "Not enough arguments\n";
  die();
}

$machine_id = $argv[1];
if(!is_numeric($machine_id)) {
  print "Incorrect machine id\n";
  die();
}

$night_id = $argv[2];
if(!is_numeric($night_id)) {
  print "Incorrect night id\n";
  die();
}


if(!(include "NightlyTester.php")) {
  print "Error: could not load necessary files!\n";
  die();
}

if(!(include"ProgramResults.php")) {
  print "Error: could not load necessary files!\n";
  die();
}

$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$row = getMachineInfo($machine_id);
$today_row = getNightInfo($night_id);
$cur_date=$today_row['added'];

$today_query = getSuccessfulNightsHistory($machine_id,$night_id);
$today_row = mysql_fetch_array($today_query);
$yesterday_row = mysql_fetch_array($today_query);
mysql_free_result($today_query);
$previous_succesful_id = $yesterday_row['id'];

$new_schema_id = 684;

$email = getEmailReport($night_id, $previous_succesful_id);

print $email;

mysql_close($mysql_link);
?>

