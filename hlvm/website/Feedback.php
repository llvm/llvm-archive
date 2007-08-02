<?php
  session_start();
?>
<!--#set var="title" value="Feedback" -->
<?php
  virtual("/incl/header.incl");
?>
<h1>Send Us Some Feedback</h1>
<p>Use the fields below to submit your feedback.  If you require a response, 
please include your name and email address.</p>
<form action="/FBThanks.php" method="post" name="Feedback" 
id="Feedback">
<table>
  <tr>
    <td>
      <p><em>First Name:</em> 
      <input name="FirstName" type="text" id="FirstName" size="30"/> </p>
      <p><em>Last Name:</em>
      <input name="LastName" type="text" id="LastName" size="30"/> </p>
      <p><em>Email: </em><input name="Email" type="text" id="Email" size="70"/>
      </p>
      <p><em>Topic:</em>
      <select name="Topic" id="Topic">
	  <option value="Ask A Question" selected="selected">Ask A Question</option>
	  <option value="Submit FAQ Question">Submit FAQ Question</option>
	  <option value="Comment On HLVM">Comment On HLVM</option>
	  <option value="HLVM Architecture">HLVM Architecture</option>
	  <option value="Using HLVM">Using HLVM</option>
	  <option value="Documentation">Documentation</option>
	  <option value="Web Site">Web Site</option>
	  <option value="Other Issue">Other Issue</option>
        </select>
      </p>
      <p><em>Question or Comment:</em><br/>
        <textarea name="Comment" cols="60" rows="10" id="Comment"></textarea>
        <br/><br/>
        <img src="/security-image.php?width=144" width="144" height="30"
        alt="Security Image" /><br/>
        <br/><label for="secode"><em>Enter Security Code:</em></label>
        <input type="text" name="secode" id="secode" size="40" value="Enter Code In Image Here" />
        <input type="submit" name="Submit" value="Submit"/>
      </p>
    </td>
  </tr>
</table>
</form>
<?php
  virtual("/incl/footer.incl");
?>
