<?php

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

$row = getMachineInfo($machine_id);
$today_row = getNightInfo($night_id);
$cur_date=$today_row['added'];

$today_query = getSuccessfulNightsHistory($machine_id,$night_id);
$today_row = mysql_fetch_array($today_query);
$yesterday_row = mysql_fetch_array($today_query);
mysql_free_result($today_query);
$previous_succesful_id = $yesterday_row['id'];

$email = htmlifyTestResults(getEmailReport($night_id, $previous_succesful_id));

?>
<html>
<head>
<title>Testing Email Report</title>
<STYLE TYPE="text/css">
<!--
  @import url(style.css);
-->
</STYLE>
</head>
<body>
<?php
print $email;
?>

</body>
<?php

mysql_close($mysql_link);
?>

