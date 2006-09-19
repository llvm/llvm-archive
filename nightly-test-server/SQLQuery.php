<?php
$user = $_POST["User"];
$password = $_POST["Password"];
$queries = $_POST["Queries"];
$was_query = isset($queries);
if (!isset($user)) $user = "";
if (!isset($password)) $password = "";
if (!isset($queries)) $queries = "";
?>

<HTML>
<HEAD>
<STYLE TYPE="text/css">
TD
{
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
}
</STYLE>
</HEAD>
<BODY>

<?php
print <<<EOD
<FORM ACTION="http://llvm.org/nightlytest/SQLQuery.php" METHOD="post" ID="MyForm">
<TABLE>
	<TR>
		<TD>User:</TD> <TD><INPUT NAME="User" ID="User" TYPE="text" VALUE="$user" SIZE="30"><BR></TD>
	</TR>
	<TR>
		<TD>Password:</TD> <TD><INPUT NAME="Password" ID="Password" TYPE="password" VALUE="$password" SIZE="30"><BR></TD>
	</TR>
	<TR>
		<TD>Query:</TD> <TD><TEXTAREA ID="Queries" NAME="Queries" ROWS="10" COLS="100">$queries</TEXTAREA><BR></TD>
	</TR>
</TABLE>
<BUTTON TYPE="submit" ID="Send" NAME="Send" VALUE="Send">Query</BUTTON><BR>
</FORM>
EOD;

if ($was_query) {
  $queries = split("\n", $queries);

  $mysql_link = mysql_connect("127.0.0.1", $user, $password) or die("Error: could not connect to database!\n");
  mysql_select_db("nightlytestresults");
  
  foreach ($queries as $query) {
    $query = rtrim($query);
    if (strlen($query) == 0) continue;
    
    if ($my_query = mysql_query($query)) {
      print "<TABLE>\n";
      $heading = false;
      
      while ($row = mysql_fetch_assoc($my_query)) {
        if (!$heading) {
          print "  <TR STYLE=\"font-weight: bold;\">";
          foreach ($row as $key => $value) {
            print "    <TD>$key</TD>";
          }
          print "  </TR>\n";
          $heading = true;
        }
        
        print "  <TR>";
        foreach ($row as $key => $value) {
          print "    <TD>$value</TD>";
        }
        print "  </TR>\n";
      }
      
      print "</TABLE><BR><BR><BR>\n";
  
      mysql_free_result($my_query);
    } else {
      $error = mysql_error();
      print "<B>$error</B><BR><BR><BR>\n";
    }
  }
  
  mysql_close($mysql_link);
}

?>

</BODY>
</HTML>

