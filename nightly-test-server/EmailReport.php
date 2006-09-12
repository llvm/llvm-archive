<html>
<body>
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

$query = "SELECT * FROM night WHERE machine = \"$machine_id\"";
$night_query = mysql_query($query) or die (mysql_error());
if ($row = mysql_fetch_array($night_query) or die (mysql_error())) {
  while ($value = current($row)) {
    $key_name = key($row);
    print "$key_name = $value<br>\n";
    next($row);
  }
}
mysql_free_result($night_query);

mysql_close($mysql_link);
?>
</body>
</html>

