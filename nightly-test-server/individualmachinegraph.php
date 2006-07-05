<?php
/*if(!(include "jpgraph.php")){
        die("Error: could not load necessary files!\n");
}
if(!(include "jpgraph_line.php")){
        die("Error: could not load necessary files!\n");
}
if(!(include "jpgraph_utils.inc")){
        die("Error: could not load necessary files!\n");
}
if(!(include "jpgraph_date.php")){
        die("Error: could not load necessary files!\n");
}*/
if(!(include "NightlyTester.php")){
        die("Error: could not load necessary files!\n");
}

$DEBUG=0;

/********************************
 *
 * printing the appropriate error
 * image if necessary
 *
 ********************************/
function printErrorMsg( $error_string, $DEBUG=0 ){
        if($DEBUG){
                print "$error_string";
        }
        $xsize=500;
        $ysize=300;

        if(!$DEBUG){
                $error_image = ImageCreate($xsize, $ysize);
                $white = imagecolorallocate($error_image, 255, 255, 255);
                $black = imagecolorallocate($error_image, 0,0,0);
                ImageFill($error_image, 0, 0, $white);
                imagerectangle($error_image,0,0,$xsize-1,$ysize-1,$black);
                imagestring($error_image, 14, 5, 5, $error_string, $black);
                header("Content-type: image/png");
                imagepng($error_image);
                imagedestroy($error_image);
        }
        exit(0);
}

/********************************
 *
 * gathering url information and creating query
 * If url is invalid an error image will be
 * returned.
 *
 ********************************/
$URL_ERROR=0;
$error_msg="";

$mysql_link = mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

if($mysql_link==FALSE){
	$URL_ERROR=1;
	$error_msg="Could not connect to database $!\n";
}                        

if(isset($HTTP_GET_VARS['measure'])){
        $measure_arr=$HTTP_GET_VARS["measure"];
	$measure=$measure_arr[0];
	$measure_url="";
	$measure_url.="&measure[]=$measure";
}
else{$URL_ERROR=1;$error_msg="no value for measure";}

if(isset($HTTP_GET_VARS['xsize'])){
        $xsize=$HTTP_GET_VARS['xsize'];
}
else{$xsize=800;}

if(isset($HTTP_GET_VARS['ysize'])){
        $ysize = $HTTP_GET_VARS['ysize'];
}else{$ysize=500;}

if(isset($HTTP_GET_VARS['name'])){
        $name = $HTTP_GET_VARS['name'];
}
else{$name = "Results Graph";}

if(isset($HTTP_GET_VARS['machine'])){
        $machine_id=$HTTP_GET_VARS['machine'];
	$machine=$machine_id;
}
else{$URL_ERROR=1;$error_msg="no value for machine";}

if(isset($HTTP_GET_VARS['start'])){
	if(preg_match("/\d\d\d\d\-\d\d\-\d\d \d\d:\d\d:\d\d/", "{$HTTP_GET_VARS['start']}")>0){
	        $start = $HTTP_GET_VARS['start'];
        	$start_query = "and added >= \"$start\"";
		$start_url = "&start=$start";
	}
	else{
		print "Error: Incorrect URL!\n";
                die();
	}
}
else{
	$result = mysql_query("SELECT * FROM night WHERE machine=$machine ORDER BY added ASC") or die (mysql_error());
	$recent=mysql_fetch_array($result);
	$start = $recent['added'];
	mysql_free_result($result);

	$start_url="&start=$start";
        $start_query = " and added >= \"$start\"";
}

if(isset($HTTP_GET_VARS['end'])){
	if(preg_match("/\d\d\d\d\-\d\d\-\d\d \d\d:\d\d:\d\d/", "{$HTTP_GET_VARS['end']}")>0){
        	$end = $HTTP_GET_VARS['end'];
        	$end_query = "and added <= \"$end\"";
	}
	else{
                print "Error: Incorrect URL!\n";
                die();
        }
}
else{
	$result = mysql_query("SELECT * FROM night WHERE machine=$machine ORDER BY added DESC") or die (mysql_error());
	$recent=mysql_fetch_array($result);
	$end = $recent['added'];
	mysql_free_result($result);

	$end_url = "&end=$end";
        $end_query = " and added <= \"$end\"";
}

if(isset($HTTP_GET_VARS['normalize'])){
        if(strcmp($HTTP_GET_VARS['normalize'],"true")==0){
                $NORMALIZE=1;
        	$normalize_url = "&normalize=true";
		$def_normalize="CHECKED";
		$def_unnormalize="";
	}
        else{
		$normalize_url="";
                $NORMALIZE=0;
		$def_normalize="";
		$def_unnormalize="CHECKED";
        }
}
else{
        $NORMALIZE=0;
	$normalize_url="";
	$def_normalize="";
	$def_unnormalize="CHECKED";
}
if(isset($HTTP_GET_VARS['showdata'])){
        if(strcmp($HTTP_GET_VARS['showdata'],"true")==0){
		$SHOWDATA=1;
                $showdata="&showdata=true";
        }
        else{
                $SHOWDATA=0;
                $showdata="";
        }
}
else{
	$SHOWDATA=0;
        $showdata="";
}

if(isset($HTTP_GET_VARS['showpoints'])){
        if(strcmp($HTTP_GET_VARS['showpoints'],"true")==0){
                $SHOWPOINTS=1;
		$showpoints="&showpoints=true";
        }
        else{
		$SHOWPOINTS=0;
                $showpoints="";
        }
}
else{
	$SHOWPOINTS=0;
        $showpoints="";
}

/********************************
 *
 * printing error image if necessary
 *
 ********************************/
if($URL_ERROR==1){
        printErrorMsg("URL Error: $error_msg. Could not draw graph.");
}

/********************************
 *
 * creating the page
 *
 ********************************/

?>

<html>
<head>
<title>LLVM Nightly Test Results Machine Graph</title>
<script language="javascript">
function toggleLayer(whichLayer)
{
if (document.getElementById)
{
// this is the way the standards work
var style2 = document.getElementById(whichLayer).style;
style2.display = style2.display? "":"none";
var link  = document.getElementById(whichLayer+"_").innerHTML;
if(link.indexOf("(+)") >= 0){
      document.getElementById(whichLayer+"_").innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.getElementById(whichLayer+"_").innerHTML="(+)"+link.substring(3,link.length);
}

}//end if
else if (document.all)
{
// this is the way old msie versions work
var style2 = document.all[whichLayer].style;
style2.display = style2.display? "":"none";
var link  = document.all[wwhichLayer+"_"].innerHTML;
if(link.indexOf("(+)") >= 0){
      document.all[whichLayer+"_"].innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.all[whichLayer+"_"].innerHTML="(+)"+link.substring(3,link.length);
}

}
else if (document.layers)
{
// this is the way nn4 works
var style2 = document.layers[whichLayer].style;
style2.display = style2.display? "":"none";
var link  = document.layers[whichLayer+"_"].innerHTML;
if(link.indexOf("(+)") >= 0){
      document.layers[whichLayer+"_"].innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.layers[whichLayer+"_"].innerHTML="(+)"+link.substring(3,link.length);
}

}

}//end function
</script>

</head>
<body>

<center><font size=+3 face=Verdana><b>LLVM Nightly Test Results Custom Graphs For <?php print "{$meaningfull_names["$measure"]}"; ?></b></font></cen\
ter><br>

<table cellspacing=4 cellpadding=4 border=0>
        <tr align=left>
                <td valign=top>
                        <?
			$machine = $HTTP_GET_VARS['machine'];
                        $night=-1;
			include 'sidebar.php';
                        ?>
                </td>
                <td>
<?php


print "\t<img src=\"drawmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine_id$measure_url&end=$end$normalize_url$start_url$showdata$showpoints\" alt=\"$measure\" height=$ysize width=$xsize><br>\n";

/********************************
 *
 * creating resize links
 *
 ********************************/
$new_x = 1.25 * $xsize;
$new_y = 1.25 * $ysize;
print "Resize: <a href=\"individualmachinegraph.php?name=$measure&xsize=$new_x&ysize=$new_y&machine=$machine$measure_url&end=$end$normalize_url&start=$start\">Bigger</a> |\n";

$new_x = .8 * $xsize;
$new_y = .8 * $ysize;
print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$new_x&ysize=$new_y&machine=$machine$measure_url&end=$end$normalize_url&start=$start\">Smaller</a><br>\n";



/********************************
 *
 * creating links to other time frames
 *
 ********************************/

$result = mysql_query("SELECT * FROM night WHERE machine=$machine ORDER BY added ASC") or die (mysql_error());
$recent=mysql_fetch_array($result);
$all_tests = $recent['added'];
mysql_free_result($result);


$length_statement="\"$end\" - INTERVAL 1 YEAR";
$length_query=mysql_query("SELECT $length_statement") or die(mysql_error());
$row = mysql_fetch_array($length_query);
mysql_free_result($length_query);
$one_year = $row[0];

$length_statement="\"$end\" - INTERVAL 6 MONTH";
$length_query=mysql_query("SELECT $length_statement") or die(mysql_error());
$row = mysql_fetch_array($length_query);
mysql_free_result($length_query);
$six_month = $row[0];

$length_statement="\"$end\" - INTERVAL 3 MONTH";
$length_query=mysql_query("SELECT $length_statement") or die(mysql_error());
$row = mysql_fetch_array($length_query);
mysql_free_result($length_query);
$three_month = $row[0];

$length_statement="\"$end\" - INTERVAL 1 MONTH";
$length_query=mysql_query("SELECT $length_statement") or die(mysql_error());
$row = mysql_fetch_array($length_query);
mysql_free_result($length_query);
$one_month = $row[0];

$length_statement="\"$end\" - INTERVAL 7 DAY";
$length_query=mysql_query("SELECT $length_statement") or die(mysql_error());
$row = mysql_fetch_array($length_query);
mysql_free_result($length_query);
$one_week = $row[0];

if(strcmp($start, $all_tests)!=0){
	print "Time: <a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end&$normalize_url$showpoints$showdata&start=$all_tests\">All</a> |\n";
}
else{ print "Time: <b>All</b> |";}

if(strcmp($start, $one_year)!=0){
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url$showpoints$showdata&start=$one_year\">1 year</a> | \n";
}
else { print " <b>1 year</b> |";}

if(strcmp($start, $six_month)!=0){
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url$showpoints$showdata&start=$six_month\">6 months</a> | \n";
} 
else { print " <b>6 months</b> |";}

if(strcmp($start, $three_month)!=0){
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url$showpoints$showdata&start=$three_month\">3 months</a> | \n";
}
else { print " <b>3 months</b> |";}

if(strcmp($start, $one_month)!=0){
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url$showpoints$showdata&start=$one_month\">1 month</a> | \n";
}
else { print " <b>1 month</b> |";}

if(strcmp($start, $one_week)!=0){
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url$showpoints$showdata&start=$one_week&showdata=true&showpoints=true\">1 week</a><br> \n";
}
else { print " <b>1 week</b><br>";}

if($NORMALIZE==1){
	print "Data normalization: <b>On</b> |\n";
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end&normalize=false&start=$start$showdata$showpoints\">Off</a><br>\n";
}
else{
	print "Data normalization: <a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end&normalize=true&start=$start\">On</a> |\n";
	print "<b>Off</b><br>\n";
}

if($SHOWDATA==1){
	print "Show data on graph: <b>On</b> |\n";
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url&start=$start$showpoints\">Off</a><br>\n";
}
else{
	print "Show data on graph: <a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url&start=$start&$showpoints&showdata=true\">On</a> |\n";
	print "<b>Off</b><br>\n";
}

if($SHOWPOINTS==1){
	print "Show points on graph: <b>On</b> |\n";
	print "<a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url&start=$start$showdata\">Off</a><br>\n";
}
else{
	print "Show points on graph: <a href=\"individualmachinegraph.php?name=$measure&xsize=$xsize&ysize=$ysize&machine=$machine&night=$night$measure_url&end=$end$normalize_url&start=$start&$showdata&showpoints=true\">On</a> |\n";
	print "<b>Off</b><br>\n";
}


/*
 * Printing out data table
 *
 */
print "<br><font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('dataTable');\", id=\"dataTable_\">(-) Data table</a></font>\n";
print "<div id=\"dataTable\" style=\"display: none;\">\n";

$night_table_statement = "SELECT * FROM night WHERE machine=$machine_id $start_query ORDER BY added DESC";
$night_table_query = mysql_query($night_table_statement) or die(mysql_error());


print "<table border=1 cellspacing=0 cellpadding=6>\n";
print "\t<tr>\n";
print "\t\t<td>Date</td>\n";
foreach ($measure_arr as $m){	
	print "\t\t<td>{$meaningfull_names["$m"]}</td>\n";
}
print "\t</tr>\n";
while($row=mysql_fetch_array($night_table_query)){
	print "\t<tr>\n";
	print "\t\t<td>{$row['added']}</td>\n";
	foreach ($measure_arr as $m){
		print "\t\t<td align=center>{$row["$m"]}</td>\n";
	}	

}
mysql_free_result($night_table_query);

print "</table></div>\n";

?>
</td></tr></table>
</body></html>





