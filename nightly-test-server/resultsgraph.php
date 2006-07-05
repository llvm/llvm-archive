<?php

$measure=$HTTP_GET_VARS['measure'];
if(sizeof($measure)>1){
        include("multipleresultsgraph.php");
}
else{
        include("individualgraph.php");
}

?>

