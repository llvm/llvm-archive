<?php
include("jpgraph/jpgraph.php");
include("jpgraph/jpgraph_line.php");
include("jpgraph/jpgraph_utils.inc");
include("jpgraph/jpgraph_date.php");

$locgraphlink = mysql_connect("localhost", "llvm", "ll2002vm");
mysql_select_db("nightlytestresults");

$lines = array();
$files = array();
$dirs = array();
$xdata = array();

function GetSqlQuery() {
  if (isset($HTTP_GET_VARS['start'])) {
    $start = $HTTP_GET_VARS['start'];
  }
  if (isset($HTTP_GET_VARS['end'])) {
    $end = $HTTP_GET_VARS['end'];
  }
  $query_tpl = "SELECT * FROM code %s ORDER BY added DESC";
  if (isset($start) && isset($end)) {
    return sprintf($query_tpl, "WHERE added <= \"$end\" AND added >= \"$start\"");
  } else if (!isset($start) && isset($end)) {
    return sprintf($query_tpl, "WHERE added <=\"$end\"");
  } else if (isset($start) && !isset($end)) {
    return sprintf($query_tpl, "WHERE added >= \"$start\"");
  } else {
    return sprintf($query_tpl, "");
  }
}

$sql_data = mysql_query(GetSqlQuery()) or die(mysql_error());
while ($row = mysql_fetch_array($sql_data)) {
  array_push($lines, $row['loc']);
  #array_push($dirs, $row['dirs']);
  #array_push($files, $row['files']);
  preg_match("/(\d\d\d\d)\-(\d\d)\-(\d\d)\s(\d\d)\:(\d\d)\:(\d\d)/", $row['added'], $values);
  $seconds = mktime($values[4], $values[5], $values[6], $values[2], $values[3], $values[1]);
  array_push($xdata, $seconds);
}

function TimeCallback($aVal) {
  return Date('m-d-y', $aVal);
}

$xsize = isset($HTTP_GET_VARS['xsize']) ? $HTTP_GET_VARS['xsize'] : 400;
$ysize = isset($HTTP_GET_VARS['ysize']) ? $HTTP_GET_VARS['ysize'] : 250;
$graph = new Graph($xsize, $ysize);
$graph->SetScale("datelin");
$graph->tabtitle->Set(" Lines of Code in SVN Repository ");
#$graph->tabtitle->SetFont(FF_ARIAL,FS_BOLD,13);
$graph->xaxis->SetLabelAngle(90);
$graph->SetMargin(70,40,30,80);
$graph->xaxis->title->SetMargin(70);
$graph->yaxis->title->SetMargin(30);
$graph->SetFrame(false);
$graph->SetMarginColor("White");
$graph->legend->SetShadow('gray@0.4', 5);
$graph->legend->SetPos(0.2, 0.13, 'left', 'top');
$graph->legend->hide(true);
$graph->ygrid->SetFill(true, '#EFEFEF@0.5', '#BBCCFF@0.5');
$graph->xgrid->Show();
$graph->xaxis->scale->SetTimeAlign(MONTHADJ_1);
$graph->xaxis->SetLabelFormatCallback('TimeCallback');

$line = new LinePlot($lines, $xdata);
$line->SetLegend("Lines of code");
$line->SetColor("red");

$graph->Add($line);
$graph->Stroke();

mysql_close($locgraphlink);
?>
