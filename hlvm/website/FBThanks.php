<?php
  session_start();
?>
<!--#set var="title" value="Thanks" -->
<?php
  virtual("/incl/header.incl");
  if (!isset($_POST['secode'])) {
?>
<h1>No Security Code</h1>
<p>You did not supply a security code.</p>
<?php
  } else if (!isset($_SESSION['secode'])) {
?>
<h1>No Security Data</h1>
<p>You have not entered this page via the Feedback page.</p>
<?php
  } else if ($_POST['secode'] != $_SESSION['secode']) {
?>
<h1>Feedback Security Failure</h1>
<p>You have entered the wrong code. Please
<a href="javascript:history.back()">try again</a>.
<?php
  } else if (!isset($_POST['Comment']) || !isset($_POST['Topic']) ||
      $_POST['Comment'] == "" || $_POST['Topic'] == "") {
?>
<h1>Insufficient Feedback Content</h1>
Thanks for trying to send us feedback. Unfortunately, your comment or topic 
was empty. We won't have anything to respond to so your feedback was not 
submitted. Please <a href="javascript:history.back()">try again</a></p>
<?php
  } else {
    $first = $HTTP_POST_VARS['FirstName'];
    $last  = $HTTP_POST_VARS['LastName'];
    $topic = "HLVM Feedback: " . $HTTP_POST_VARS['Topic'];
    $email = $HTTP_POST_VARS['Email']; 
    $comment = $HTTP_POST_VARS['Comment'];
    $message = sprintf("\nFeedback from %s %s (%s) at %s regarding %s:\n\n%s\n",
      $first, $last, $email, date( "Y/m/d H:i:s", time() ), $topic, $comment);
    echo "<p>$first, ";
    if ( mail("rspencer@x10sys.com",$topic,$message) )
    {
?>
<h1>Feedback Received</h1>
Thankyou for your feedback. We have been successfully notified.</p>
<p>We appreciate you taking the time to write to us. If a response to 
your subject (<?php echo "$topic "; ?>) is due, we will send it promptly to 
the email address you provided (<?php echo "$email"; ?>).</p>
<?php
    }
    else
    {
?>
Thankyou for your feedback. Unfortunately, we are unable to process your 
feedback at this time. Please try again later. We sincerely regret the 
inconvenience.</p>
<?php
    }
  }
  virtual("/incl/footer.incl");
?>
