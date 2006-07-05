<?php
/******************************
 *
 * Checking input variables
 *
 ******************************/
if(!isset($HTTP_GET_VARS['machine']) || !is_numeric($HTTP_GET_VARS['machine'])){
        print "Error: Incorrect URL!\n";
        die();
}
$machine_id = $HTTP_GET_VARS['machine'];
$machine = $HTTP_GET_VARS['machine'];

if(!isset($HTTP_GET_VARS['night']) || !is_numeric($HTTP_GET_VARS['night'])){
        print "Error: Incorrect URL!\n";
        die();
}
$night_id = $HTTP_GET_VARS['night'];
$night = $HTTP_GET_VARS['night'];

$end = "";
if(isset($HTTP_GET_VARS['end'])){
        if(preg_match("/\d\d\d\d\-\d\d\-\d\d \d\d:\d\d:\d\d/", "{$HTTP_GET_VARS['end']}")>0){
                $end = "&end={$HTTP_GET_VARS['end']}";
        }
        else{
                print "Error: Incorrect URL!\n";
                die();
        }
}
//the following variables are added for sidebar.php to work
$end_url="";
$start_url="";

if(!isset($HTTP_GET_VARS['measure'])){
        die("ERROR: Incorrect URL\n");
}
$measure_arr = $HTTP_GET_VARS['measure'];

if(!isset($HTTP_GET_VARS['program'])){
        die("ERROR: Incorrect URL\n");
}
$program_arr = $HTTP_GET_VARS['program'];

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

$row = getMachineInfo($machine_id,$mysql_link);
$today_row = getNightInfo($night_id,$mysql_link);
$cur_date=$today_row['added'];

?>

<html>
<head><title>LLVM Nightly Test Graphs For <?php print $cur_date; ?></title></head>
<body>

<center><font size=+3 face=Verdana><b>LLVM Nightly Test Graphs For <?php print $cur_date; ?></b></font></cen\
ter><br>

<table cellspacing=4 cellpadding=4 border=0>
        <tr align=left>
                <td valign=top>
                        <?
                        $machine = $HTTP_GET_VARS['machine'];
                        $night = $HTTP_GET_VARS['night'];
                        include 'sidebar.php';
                        ?>
                </td>
                <td>
<?php
foreach ($measure_arr as $measure){

	$list_of_programs="";
	foreach($program_arr as $program){
		$list_of_programs.="&program[]=$program";
	}

	print "<a href=\"individualgraph.php?name=$measure&xsize=800&ysize=500&machine=$machine_id&night=$night_id&measure[]=$measure$list_of_programs$end\">\n";
	print "\t<img src=\"drawresultsgraph.php?name=$measure&xsize=800&ysize=400&machine=$machine_id&measure=$measure$list_of_programs$end\" alt=\"$measure\" height=400 width=800>\n";
	print "</a><br>\n";

}
	
	

?>
		
			

</td></tr></table>
</body></html>