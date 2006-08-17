<?php

if(!isset($HTTP_GET_VARS['measure'])){
        die("ERROR: Please select a checkbox at the top of a column to create a graph.\n");
}

$measure=$HTTP_GET_VARS['measure'];
if(sizeof($measure)>1){
	include("multiplemachinegraph.php");
}
else{
	include("individualmachinegraph.php");
}

?>