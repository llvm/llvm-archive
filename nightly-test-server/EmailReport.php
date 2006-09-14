<html>
<body>
<?php

$mysql_link = mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$query = $_POST["Query"];
$query = preg_replace("/\n/", " ", $query);
$my_query = mysql_query($query) or die (mysql_error());

while ($row = mysql_fetch_array($my_query)) {
  foreach ($row as $key => $value) {
    print "$key => $value, ";
  }
  print "<br>\n";
}

mysql_free_result($my_query);

mysql_close($mysql_link);

?>
</body>
</html>

