<?
/******************************
 *
 * Checking input variables
 *
 ******************************/
if(!isset($HTTP_GET_VARS['machine']) || !is_numeric($HTTP_GET_VARS['machine'])){
	print "Error: Incorrect URL for machine.php!\n";
        die();
}
$machine = $HTTP_GET_VARS['machine'];
		
if(!(include "NightlyTester.php")){
	print "Error: could not load necessary files!\n";
        die();
}

$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$row = getMachineInfo($machine,$mysql_link);
$machine_name = $row['name'];
if(strcmp($row['nickname'],"")!=0){
	$machine_name=$row['nickname'];
}


?>

<html>
<head>
<title>LLVM Machine Test Results For <?php print $machine_name ?></title>
<script type="text/javascript" src="popup.js"></script>
<STYLE TYPE="text/css">
<!--
  @import url(style.css);
-->
</STYLE>
</head>
<body>

<center><font size=+3 face=Verdana><b>LLVM Machine Test Results For <?php print $machine_name ?></b></font></center><br>

<table cellspacing=5 cellpadding=5 border=0>
        <tr>
                <td valign=top>
                        <? 
			$machine = $HTTP_GET_VARS['machine'];
			$night = -1;
			include "sidebar.php"; 
			?>
                </td>
                <td valign=top>

<?

print "<table border=0 cellpadding=0 cellspacing=5>\n";
print "<tr>\n";
print "<td><b>Nickname:</b></td>";
print "<td>{$row['nickname']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>uname:</b></td>";
//print $row['uname'];
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


$result = getNightsResource($machine,$mysql_link);
$recent=mysql_fetch_array($result);
$recent_id=$recent['id'];
$cur_date = $recent['added'];
$old=mysql_fetch_array($result);
$old_id=$old['id'];
mysql_free_result($result);

include("ProgramResults.php");
if(is_numeric($recent_id) && is_numeric($old_id)){
	$today_results = GetDayResults($recent_id, $category_array, $mysql_link);
	$yesterday_results = GetDayResults($old_id, $category_array, $mysql_link);
	$percent_difference = CalculateChangeBetweenDays($yesterday_results, $today_results);

	/* note: $medium_change, $large_change, and $crazy_change are defined in ProgramResult.php */
	print "<table border=0><tr><td>\n";
	
	print "<table border=0>\n";
	Print "<tr><td>$medium_change % differences from yesterday:</td><td> $medium_number</td></tr>\n";
	Print "<tr><td>$large_change % differences from yesterday:</td><td> $large_number</td></tr>\n";
	Print "<tr><td>$crazy_change % differences from yesterday:</td><td> $crazy_number</td></tr>\n";
	print "</table>\n";
	
	print "</td><td valign=top>";

	print "<span style=\"position:relative;\">\n";
	print "<span id=\"differences\" class=\"popup\">\n";
	print "<pre>The number of measurements that signifigantly<br>changed from the previous test.</pre>\n";
	print "</span><a href=\"javascript:void(0);\" onClick=\"TogglePop('differences');\">?</a></span>\n";

	print "</td></tr></table>\n";
}

print "<form method=GET action=\"machinegraph.php\">\n";
print "<input type=hidden name=machine value=\"$machine\">\n";
print "<input type=hidden name=night value=\"$recent_id\">\n";
print "<input type=hidden name=end value=\"$cur_date\">\n";
print "<input type=hidden name=start value=\"2000-01-01 01:01:01\">\n";

echo "<table border=1 cellpadding=0 cellspacing=0>\n";
echo "<tr bgcolor=#FFCC99>";
	echo "<td>";
		echo "Date added";
	echo "</td>";
	
	echo "<td align=center>";
		echo "CVS checkout time";
		print "<br><input type=checkbox name=\"measure[]\" value=\"getcvstime_wall\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Configure time cpu";
		print "<br><input type=checkbox name=\"measure[]\" value=\"configuretime_cpu\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Configure time wall";
		print "<br><input type=checkbox name=\"measure[]\" value=\"configuretime_wall\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Build time cpu";
		print "<br><input type=checkbox name=\"measure[]\" value=\"buildtime_cpu\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Build time wall";
		print "<br><input type=checkbox name=\"measure[]\" value=\"buildtime_wall\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Dejagnu time cpu";
		print "<br><input type=checkbox name=\"measure[]\" value=\"dejagnutime_cpu\">\n";
	echo "</td>";
	echo "<td align=center>";
		echo "Dejagnu time wall";
		print "<br><input type=checkbox name=\"measure[]\" value=\"dejagnutime_wall\">\n";
	echo "</td>";
	echo "<td align=center>";
                echo "# of expected test passes";
		print "<br><input type=checkbox name=\"measure[]\" value=\"teststats_exppass\">\n";
        echo "</td>";
        echo "<td align=center>";
                echo "# of unexpected test failures";
		print "<br><input type=checkbox name=\"measure[]\" value=\"teststats_unexpfail\">\n";
        echo "</td>";
        echo "<td align=center>";
                echo "# of expected test failures";
		print "<br><input type=checkbox name=\"measure[]\" value=\"teststats_expfail\">\n";
        echo "</td>";
	echo "<td align=center>";
		echo "# of warnings";
	echo "</td>";
	echo "<td>";
	echo "</td>";
echo "</tr>";

$result = getNightsResource($machine,$mysql_link);
$line=1;
$row = mysql_fetch_array($result);
$x=0;
while($x<10 && $prev_row = mysql_fetch_array($result)){


$warnings ="";
if(strcmp($row['warnings'],"")!=0){
	$warnings=$row['warnings'];
}

$num_warnings = preg_match_all('/warning/', $warnings, $match);	
	
	
	if(strpos($row['buildstatus'],"OK")===FALSE){
		print "\t<tr bgcolor=#FFCC33>\n";
		$build_ok=0;
	}
	else if($line % 2 ==0){
		print "\t<tr bgcolor=#DDDDDD>\n";
		$build_ok=1;
	}
	else{
		print "\t<tr bgcolor='white'>\n";
		$build_ok=1;
	}	
	$line++;

		/*~~~~~~~~~~~~ Date of test ~~~~~~~~~~~~*/

		echo "<td>";
			$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$row['added']);
			echo "$date";
		echo "</td>";

		/*~~~~~~~~~~~~ Get CVS Wall Time ~~~~~~~~~~~~*/

	
		echo "<td>";
			echo $row['getcvstime_wall'];
		echo "</td>";

		/*~~~~~~~~~~~~ Configure Time CPU ~~~~~~~~~~~~*/
		if($prev_row['configuretime_cpu']!=0 && $build_ok){
			$delta = round(( ($prev_row['configuretime_cpu'] - $row['configuretime_cpu']) / $prev_row['configuretime_cpu'] ) * 100,2);
		        if ($delta > 20){
				print "<td bgcolor=#CCFFCC>{$row['configuretime_cpu']}</td>";
			}
			else if ($delta < -20){
				print "<td bgcolor=#FFAAAA>{$row['configuretime_cpu']}</td>";
			}
			else{
				print "<td>{$row['configuretime_cpu']}</td>";
			}
		}
		else{
			print "<td>{$row['configuretime_cpu']}</td>";
		}

		/*~~~~~~~~~~~~ Configure Time Wall ~~~~~~~~~~~~*/

		echo "<td>";
			echo $row['configuretime_wall'];
		echo "</td>";

		/*~~~~~~~~~~~~ Build Time CPU ~~~~~~~~~~~~*/

		if($prev_row['buildtime_cpu']!=0 && $build_ok){
			$delta = round(( ($prev_row['buildtime_cpu'] - $row['buildtime_cpu']) / $prev_row['buildtime_cpu'] ) * 100,2);
		        if ($delta > 10){
				print "<td bgcolor=#CCFFCC>{$row['buildtime_cpu']}</td>";
			}
			else if ($delta < -10){
				print "<td bgcolor=#FFAAAA>{$row['buildtime_cpu']}</td>";
			}
			else{
				print "<td>{$row['buildtime_cpu']}</td>";
			}
		}
		else{
			print "<td>{$row['buildtime_cpu']}</td>";
		}

		/*~~~~~~~~~~~~ Build Time Wall ~~~~~~~~~~~~*/

		echo "<td>";
			echo $row['buildtime_wall'];
		echo "</td>";
	
		/*~~~~~~~~~~~~ Dejagnu Time CPU ~~~~~~~~~~~~*/

		if($prev_row['dejagnutime_cpu']!=0 && $build_ok){
			$delta = round( ( ($prev_row['dejagnutime_cpu'] - $row['dejagnutime_cpu']) / $prev_row['dejagnutime_cpu'] ) * 100,2);
		        if ($delta > 10){
				print "<td bgcolor=#CCFFCC>{$row['dejagnutime_cpu']}</td>";
			}
			else if ($delta < -10){
				print "<td bgcolor=#FFAAAA>{$row['dejagnutime_cpu']}</td>";
			}
			else{
				print "<td>{$row['dejagnutime_cpu']}</td>";
			}
		}
		else{
			print "<td>{$row['dejagnutime_cpu']}</td>";
		}
		

		/*~~~~~~~~~~~~ Dejagnu Time Wall ~~~~~~~~~~~~*/

		echo "<td>";
			echo $row['dejagnutime_wall'];
		echo "</td>";

		/*~~~~~~~~~~~~ # of expected passes ~~~~~~~~~~~~*/

	        if ($row['teststats_exppass'] > $prev_row['teststats_exppass'] && $build_ok){
			$color="#CCFFCC";
		}
		else if ($row['teststats_exppass'] < $prev_row['teststats_exppass'] && $build_ok){
			$color="#FFAAAA";	
		}
		else{
			$color="white";
		}
		if(!$build_ok){
			print "<td>{$row['teststats_exppass']}</td>";		
		}
		else{
			print "<td bgcolor=$color>{$row['teststats_exppass']}</td>";		
		}
		/*~~~~~~~~~~~~ # of unexpected failures ~~~~~~~~~~~~*/

	        if ($row['teststats_unexpfail'] < $prev_row['teststats_unexpfail'] && $build_ok){
			$color="#CCFFCC";
		}
		else if ($row['teststats_unexpfail'] > $prev_row['teststats_unexpfail'] && $build_ok){
			$color="#FFAAAA";	
		}
		else{
			$color="white";
		}

		if(!$build_ok){
                        print "<td>{$row['teststats_unexpfail']}</td>";
                }
		else if($row['teststats_exppass']!=0){
			print "<td bgcolor=$color><a href=\"test.php?machine=$machine&night={$row['id']}#unexpfail_tests\">{$row['teststats_unexpfail']}</a></td>";
		}
		else{
			print "<td bgcolor=$color>{$row['teststats_unexpfail']}</td>";		
		}

		/*~~~~~~~~~~~~ # of expected failures ~~~~~~~~~~~~*/

	        if ($row['teststats_expfail'] < $prev_row['teststats_expfail'] && $build_ok){
			print "<td bgcolor=#CCFFCC>{$row['teststats_expfail']}</td>";
		}
		else if ($row['teststats_expfail'] > $prev_row['teststats_expfail'] && $build_ok){
			print "<td bgcolor=#FFAAAA>{$row['teststats_expfail']}</td>";
		}
		else{
			print "<td>{$row['teststats_expfail']}</td>";
		}

		/*~~~~~~~~~~~~ Number Of Warnings ~~~~~~~~~~~~*/
	
		if($num_warnings>0){
			print "\t<td><a href=\"test.php?machine=$machine&night={$row['id']}#warnings\">$num_warnings</a></td>\n";
		}	
		else{
			print "\t<td>$num_warnings</td>\n";
		}

		/*~~~~~~~~~~~~ Link to test page ~~~~~~~~~~~~*/
	
		echo "<td>";
			print "<a href=\"test.php?machine=$machine&night={$row['id']}\">View details</a>\n";
		echo "</td>";
	echo "</tr>";
	$row = $prev_row;
	$x++;
} #end while

mysql_free_result($result);

/*******************************
 *
 * Taking care of last row
 *******************************/

if($line % 2 ==0){
	print "\t<tr bgcolor=#DDDDDD>\n";
}
else{
	print "\t<tr bgcolor='white'>\n";
}	
$line++;
echo "<td>";
	$date = preg_replace("/\s\d\d:\d\d:\d\d/","",$row['added']);
	echo $date;
echo "</td>";
echo "<td>";
	echo $row['getcvstime_wall'];
echo "</td>";
echo "<td>";
	echo $row['configuretime_cpu'];
echo "</td>";
echo "<td>";
	echo $row['configuretime_wall'];
echo "</td>";
echo "<td>";
	echo $row['buildtime_cpu'];
echo "</td>";
echo "<td>";
	echo $row['buildtime_wall'];
echo "</td>";
echo "<td>";
	echo $row['dejagnutime_cpu'];
echo "</td>";
echo "<td>";
	echo $row['dejagnutime_wall'];
echo "</td>";
echo "<td>";
	echo $row['teststats_exppass'];
echo "</td>";
echo "<td>";
       	echo $row['teststats_unexpfail'];
echo "</td>";
echo "<td>";
     	echo $row['teststats_expfail'];
echo "</td>";
echo "<td>";
	if(isset($num_warnings)){
		echo $num_warnings;
	}
	else{
		echo "-";
	}
echo "</td>";
echo "<td>";
	print "<a href=\"test.php?machine=$machine&night={$row['id']}\">View details</a>\n";
echo "</td>";
echo "</tr>";

echo "</table>";


print "<input type=submit name=action value=\"Graph Column\"> | ";
print "<input type=reset>\n";
print "<span style=\"position:relative;\">\n";
print "<span id=\"graph\" class=\"popup2\">\n";
print "<pre>Produces a graph of the selected columns over<br>time.</pre>\n";
print "</span><a href=\"javascript:void(0);\" onClick=\"TogglePop('graph');\">?</a></span>\n";
print "</form>\n";

mysql_close($mysql_link);
?>

                </td>
        </tr>
</table>

</body>
</html>