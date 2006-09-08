<?php
include("NightlyTester.php");

$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm"); 
if(!$mysql_link){
	die("$mysql_link,Could not connect to database ". mysql_error()."\n");
}
mysql_select_db("nightlytestresults");



?>
<html>
<head><title>List of LLVM Nightly Test Machines</title></head>
<body>
<center><font size=+3 face=Verdana><b>List of LLVM Nightly Test Machines</b></font></center><br>
<table cellspacing=5 cellpadding=3 border=0>
        <tr>
                <td valign=top>
                        <?
                        $machine = -1;
                        $night = -1;
                        include "sidebar.php";
                        ?>
                </td>
                <td>

<?php
/*
 * the following lists all the machines
 */

print "<br>List of all machines:<br>\n";
print "<table border=1 cellpadding=0 cellspacing=0>\n";
print "\t<tr bgcolor=#FFCC99>\n";
        print "\t\t<td>ID</td>\n";
        print "\t\t<td>Nickname</td>\n";
        print "\t\t<td>Machine name</td>\n";
        print "\t\t<td>Operating System</td>\n";
        print "\t\t<td>Hardware</td>\n";
        print "\t\t<td>Time of last test</td>\n";
        print "\t\t<td>Build status of last test</td>\n";
print "\t</tr>\n";

$result = getMachineResource();
$line=1;
while($row = mysql_fetch_array($result)){
	$query = getNightsResource($row['id']);
	$latest_test = mysql_fetch_array($query);
	mysql_free_result($query);      

	if($line %2 == 0){
		print "\t<tr bgcolor=#DDDDDD>\n";
	}
	else{
		print "\t<tr bgcolor='white'>\n";        
	}
	$line++;
	print "\t\t<td>{$row['id']}</td>\n";
        print "\t\t<td><a href=\"machine.php?machine={$row['id']}\">{$row['nickname']}</a></td>\n";
	print "\t\t<td><a href=\"machine.php?machine={$row['id']}\">{$row['name']}</a></td>\n";
        print "\t\t<td>{$row['os']}</td>\n";
        print "\t\t<td>{$row['hardware']}</td>\n";
        print "\t\t<td>{$latest_test['added']}</td>\n";
        print "\t\t<td>{$latest_test['buildstatus']}</td>\n";
        print "\t</tr>\n";
}
print "</table>\n";

mysql_free_result($result);
mysql_close($mysql_link);
?>

</td></tr></table>

</body>
</html>
