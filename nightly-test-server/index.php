BLAH BLAH BLAH<?php

include("NightlyTester.php");
$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm");
mysql_select_db("nightlytestresults");
?>

<html>
<head><title>LLVM Nightly Test Results Homepage</title></head>
<body>
<center><font size=+3 face=Verdana><b>LLVM Nightly Test Results Homepage</b></font></center><br>
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

<?
print "<p align=right><a href='locgraph.php?xsize=900&ysize=600'><img src='locgraph.php'  border=0 align=center height=250 width=400 alt='Lines of code graph'></a></p>\n";

/*
 * the following lists the tests in the last 24 hours
 */
print "Tests submitted in last 24 hours:<br>";
print "<table border=1 cellpadding=0 cellspacing=0>\n";
print "\t<tr bgcolor=#FFCC99>";
        print "\t\t<td>Date added</td>\n";
        print "\t\t<td>Machine name</td>\n";
        print "\t\t<td>Build status</td>\n";
	print "\t\t<td># of expected test passes</td>\n";
        print "\t\t<td># of unexpected test failures</td>\n";
        print "\t\t<td># of expected test failures</td>\n";
	print "\t\t<td></td>\n";
print "\t</tr>";

$result = getRecentTests("24",$mysql_link);
$line=1;
if(mysql_num_rows($result)==0){
	print "\t<tr bgcolor='white'>";
        print "\t\t<td>---";
        print "</td>\n";
	print "\t\t<td>---</td>\n";
        print "\t\t<td>---</td>\n";
	print "\t\t<td>---</td>\n";
        print "\t\t<td>---</td>\n";
        print "\t\t<td>---</td>\n";
	print "\t\t<td>---</a></td>\n";
	print "\t</tr>";
}
while($row = mysql_fetch_array($result)){
	$cur_machine_id = $row['machine'];
	$cur_night_id   = $row['id'];
	$cur_machine_row = getMachineInfo($cur_machine_id,$mysql_link);
	if(strcmp($cur_machine_row['nickname'], "")!=0){$machine_name = $cur_machine_row['nickname'];}
	else{$machine_name = $cur_machine_row['name'];}
        

	if($line % 2 == 0){
		print "\t<tr bgcolor=#DDDDDD>";		
	}
	else{
		print "\t<tr bgcolor='white'>";
	}
        print "\t\t<td>";
		preg_match("/(\d\d:\d\d:\d\d)/", $row['added'], $time);
                print $time[1];
        print "</td>\n";
	print "\t\t<td><a href=\"machine.php?machine=$cur_machine_id\">$machine_name</a></td>\n";
        print "\t\t<td>{$row['buildstatus']}</td>\n";
	print "\t\t<td>{$row['teststats_exppass']}</td>\n";
        print "\t\t<td>{$row['teststats_unexpfail']}</td>\n";
        print "\t\t<td>{$row['teststats_expfail']}</td>\n";
	print "\t\t<td><a href=\"test.php?machine=$cur_machine_id&night=$cur_night_id\">view details</a></td>\n";
	print "\t</tr>";
	$line++;

}//end while
print "</table>\n";

mysql_free_result($result);

mysql_close($mysql_link);
?>

                </td>
        </tr>
</table>

</body>
</html>