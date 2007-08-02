<!--#set var="title" value="API Documentation" -->
<?php
  virtual("/incl/header.incl");
  if (isset($_GET['class']))
    $url = "/apis/html/classhlvm_1_1" . $_GET['class'] . '.html';
  else
    $url = "/apis/html";
?>
<h1>HLVM API Documentation</h1>
<iframe src="<?php echo $url?>" width="100%" height="600"></iframe>
<?php
  virtual("/incl/footer.incl");
?>
