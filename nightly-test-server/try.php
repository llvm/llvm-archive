<html>
<body>
<?php
/*
 foreach ($_ENV as $key => $value) {
  print "_ENV $key => $value<br>\n";
 }
 
 foreach ($_SERVER as $key => $value) {
  print "_SERVER $key => $value<br>\n";
 }
 
 foreach ($_GET as $key => $value) {
  print "_GET $key => $value<br>\n";
 }
 
 foreach ($_POST as $key => $value) {
  print "_POST $key => $value<br>\n";
 }
 
 foreach ($_COOKIE as $key => $value) {
  print "_COOKIE $key => $value<br>\n";
 }
 
 foreach ($_FILES as $key => $value) {
  print "_FILES $key => $value<br>\n";
 }
 
 foreach ($_REQUEST as $key => $value) {
  print "_REQUEST $key => $value<br>\n";
 }
*/

$info = "PASS: /Users/sabre/buildtest/llvm/test/Feature/alignment.ll\n";
$subpatterns = array();
preg_match("/^(XPASS|PASS|XFAIL|FAIL):\s(.+):?/", $info, $subpatterns);

foreach ($subpatterns as $key => $value) {
  print "$key => $value<br>\n";
}

?>
</body>
</html>
