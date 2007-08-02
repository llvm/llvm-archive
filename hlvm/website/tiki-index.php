<?php
if(!(headers_sent())){
header ("location: wiki");
}
die("header already sent");
?>
