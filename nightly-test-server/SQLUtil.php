<HTML>
<HEAD>
<STYLE TYPE="text/css">
BODY
{
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
</STYLE>
</HEAD>
<BODY>

<?php

$mysql_link = mysql_connect("127.0.0.1", "llvm", "ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$query = "SELECT * FROM tests WHERE measure=\"dejagnu\"";
if ($get_query = mysql_query($query)) {
  while ($row = mysql_fetch_assoc($get_query)) {
    $old = $row['program'];
    $subpatterns = array();
    if (preg_match("/(.*)\/llvm\/test\/(.*)/", $old, $subpatterns)) {
      list($ignore, $before, $after) = $subpatterns;
      $new = "test/".$after;
      $result = $row['result'];
      $night =  $row['night'];
      $query = "UPDATE tests SET program=\"$new\" WHERE night=$night AND program=\"$old\" AND result=\"$result\" AND measure=\"dejagnu\"";
      $set_query = mysql_query($query);
      mysql_free_result($set_query);
    }
  }

  mysql_free_result($get_query);
} else {
  $error = mysql_error();
  print "<B>$error</B><BR>\n";
}
  print "<B>DONE</B><BR>\n";

mysql_close($mysql_link);

?>

</BODY>
</HTML>
