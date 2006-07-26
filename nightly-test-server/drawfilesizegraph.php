<?php
include("jpgraph/jpgraph.php");
include("jpgraph/jpgraph_line.php");
include("jpgraph/jpgraph_utils.inc");
include("jpgraph/jpgraph_date.php");
include("ProgramResults.php");

$DEBUG=0;

/********************************
 *
 * printing the appropriate error
 * image if necessary
 *
 ********************************/
function printErrorMsg( $error_string , $DEBUG ){
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
 * connecting to DB
 *
 ********************************/

$resultsgraphlink=mysql_connect("localhost","llvm","ll2002vm");
mysql_select_db("nightlytestresults");

/********************************
 *
 * gathering url information and creating query
 * If url is invalid an error image will be
 * returned.
 *
 ********************************/
$URL_ERROR=0;
$error_msg="";

if(isset($HTTP_GET_VARS['files'])){
	$files=$HTTP_GET_VARS['files'];
}
else{$URL_ERROR=1;$error_msg="no value for program";}

if(isset($HTTP_GET_VARS['xsize'])){
	$xsize=$HTTP_GET_VARS['xsize'];
}
else{$xsize=800;}

if(isset($HTTP_GET_VARS['ysize'])){
	$ysize = $HTTP_GET_VARS['ysize'];
}else{$ysize=300;}

if(isset($HTTP_GET_VARS['name'])){
	$name = $HTTP_GET_VARS['name'];
}
else{$name = "Results Graph";}

if(isset($HTTP_GET_VARS['machine'])){
	$machine_id=$HTTP_GET_VARS['machine'];
}
else{$URL_ERROR=1;$error_msg="no value for machine";}

if(isset($HTTP_GET_VARS['end'])){
	$end = $HTTP_GET_VARS['end'];
	$end_query = "and added <= \"$end\"";
	$start_query = "and (\"$end\" - INTERVAL 6 MONTH) <= added";
}
else{
	$end = "";
	$end_query = " ";
	$start_query= "";
}

if(isset($HTTP_GET_VARS['start'])){
	$start = $HTTP_GET_VARS['start'];
	$start_query = "and added >= \"$start\"";
}


if(isset($HTTP_GET_VARS['normalize'])){
	if(strcmp($HTTP_GET_VARS['normalize'],"true")==0){
		$NORMALIZE=1;
	}
	else{
		$NORMALIZE=0;
	}	
}
else{
	$NORMALIZE=0;
}

if(isset($HTTP_GET_VARS['showdata'])){
	if(strcmp($HTTP_GET_VARS['showdata'],"true")==0){
		$SHOWDATA=1;
	}
	else{
		$SHOWDATA=0;
	}	
}
else{
	$SHOWDATA=0;
}
if(isset($HTTP_GET_VARS['showpoints'])){
	if(strcmp($HTTP_GET_VARS['showpoints'],"true")==0){
		$SHOWPOINTS=1;
	}
	else{
		$SHOWPOINTS=0;
	}	
}
else{
	$SHOWPOINTS=0;
}

/********************************
 *
 * printing error image if necessary
 *
 ********************************/
if($URL_ERROR==1){
	printErrorMsg("URL Error: $error_msg. Could not draw graph.", $DEBUG);
}

/********************************
 *
 * creating the graph
 *
 ********************************/
// Callback formatting function for the X-scale to convert timestamps
// to hour and minutes.
function  TimeCallback( $aVal) {
    return Date ('m-d-y', $aVal);
}

$graph = new Graph($xsize,$ysize);
$graph->SetScale("datelin");
$graph->tabtitle->Set(" $name ");
$graph->xaxis->SetLabelAngle(90);
$graph->SetMargin(50,10,30,110+sizeof($files)*18);
$graph->xaxis->title->Set("");
$graph->xaxis->title->SetMargin(80);
$graph->yaxis->title->Set("");
$graph->yaxis->title->SetMargin(30);
$graph->SetFrame(false);
$graph->SetMarginColor("White");
$graph->legend->SetShadow('gray@0.4',5);
$graph->legend->SetAbsPos($xsize*.2,$ysize-(40+(sizeof($files)*18)),'top','left');
$graph->legend->hide(false);
$graph->ygrid->SetFill(true,'#EFEFEF@0.5','#BBCCFF@0.5');
$graph->xgrid->Show();
$graph->xaxis->scale->SetTimeAlign(MONTHADJ_1);
$graph->xaxis-> SetLabelFormatCallback( 'TimeCallback');

/********************************
 *
 * executing the query by...
 * 1.) finding all the nights associated with the provided machine
 * 2.) getting all the correct programs for that night
 *
 ********************************/

$line_arr=array();
$color_arr = array("red","blue","green","orange","black", "brown","gray");

$RELEVANT_DATA=0;

$index=0;
foreach ($files as $file){

	$data = get_file_history($mysql_link, $machine_id, $file)

	$xdata=array();
	$values=array();
	$data_max=-1;

	foreach ($data as $d){
		push_array($xdata, $d[0]);
		if($d[2] > $data_max){ $data_max=$d[2]; }
		push_array($values, $d[2]);
	}

	if($NORMALIZE){
		for($i=0; $i<sizeof($values); $i++){
			if(is_numeric($values[$i])){
				$values[$i]=$values[$i]/$data_max;
			}#end if
		}#end for
	}#end if
	

	$line_arr[$index] = new LinePlot($values, $xdata);
	$line_arr[$index]->SetLegend("$file");
	if($SHOWDATA==1){
		$line_arr[$index]->value->Show();
	}
	if($SHOWPOINTS==1){
		$line_arr[$index]->mark->SetType(MARK_UTRIANGLE);
	}
	$color_index=$index % sizeof($color_arr);
	$line_arr[$index]->SetColor($color_arr[$color_index]);
	$graph->Add($line_arr[$index]);
	$index++;
}#end foreach

if($RELEVANT_DATA==0){
	printErrorMsg("Error: Graph of $measure contains no data", $DEBUG);
}

if($DEBUG){
	print "Finished!<br>$RELEVANT_DATA<br>\n";
}
else{	
$graph->Stroke();		
}

?>

