<?php
$mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Error: could not connect to database!\n");
mysql_select_db("nightlytestresults");

$params = $_SERVER['QUERY_STRING'];

if(is_numeric($params)){
	$query="SELECT url FROM tinyurl WHERE id=$params";
	$resource = mysql_query($query);

	if($row = mysql_fetch_array($resource)){
		include("http://{$row['url']}");
	}
	else{
		print "No such page!\n";
	}
	mysql_free_result($resource);
}
else if (strcmp($params, "")!=0){
	$this_url = $params;	
	$query="SELECT * FROM tinyurl WHERE url=\"$this_url\"";
        $result = mysql_query($query);
	if($row = mysql_fetch_array($result)){
               header( "Location: http://{$_SERVER['SERVER_NAME']}{$_SERVER['SCRIPT_NAME']}?{$row['id']}" );
               include("http://{$row['url']}");
        }
        else{
		$query="INSERT INTO tinyurl (url) values (\"$this_url\")";
		$add_url = mysql_query($query);
		$query="SELECT * FROM tinyurl WHERE url=\"$this_url\"";
	        $resource = mysql_query($query);
	
	        if($row = mysql_fetch_array($resource)){
			header( "Location: http://{$_SERVER['SERVER_NAME']}{$_SERVER['SCRIPT_NAME']}?{$row['id']}" );
			include("http://{$row['url']}");	
	        }
	        else{
	                print "Error, could not create page!!\n";
	        }
	        mysql_free_result($add_url);
		mysql_free_result($resource);
	}
	mysql_free_result($result);
}
else{
	print "No such page!\n";
}





?>