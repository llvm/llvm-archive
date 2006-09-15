<?php
$user = $_POST["User"];
$password = $_POST["Password"];
$query = $_POST["Query"];
$was_query = isset($query);
if (!isset($user)) $user = "";
if (!isset($password)) $password = "";
if (!isset($query)) $query = "";
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
		<TD>Query:</TD> <TD><INPUT NAME="Query" ID="Query" TYPE="text" VALUE="$query" SIZE="100"><BR></TD>
	</TR>
</TABLE>
<BUTTON TYPE="submit" ID="Send" NAME="Send" VALUE="Send">Query</BUTTON><BR>
</FORM>
EOD;

if ($was_query) {
  $mysql_link = mysql_connect("127.0.0.1", $user, $password) or die("Error: could not connect to database!\n");
  mysql_select_db("nightlytestresults");
  
  $query = $_POST["Query"];
  $my_query = mysql_query($query) or die (mysql_error());
  
  print "<TABLE>\n";
  $heading = false;
  
  while ($row = mysql_fetch_assoc($my_query)) {
    if (!$heading) {
      print "  <TR STYLE=\"font-weight: bold;\">\n";
      foreach ($row as $key => $value) {
        print "    <TD>$key</TD>\n";
      }
      print "  </TR>\n";
      $heading = true;
    }
    
    print "  <TR>\n";
    foreach ($row as $key => $value) {
      print "    <TD>$value</TD>\n";
    }
    print "  </TR>\n";
  }
  
  print "</TABLE>\n";

  mysql_free_result($my_query);
  
  mysql_close($mysql_link);
}

?>

</BODY>
</HTML>

