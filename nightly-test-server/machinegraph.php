<?php

if(!isset($HTTP_GET_VARS['measure'])){
        die("ERROR: Incorrect URL\n");
}

$measure=$HTTP_GET_VARS['measure'];
if(sizeof($measure)>1){
	include("multiplemachinegraph.php");
}
else{
	include("individualmachinegraph.php");
}

?>