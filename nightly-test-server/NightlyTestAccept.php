<?php
/*******************************************************************************
 *
 * Background: Written on June 1st 2006 by Patrick Jenkins <pjenkins@apple.com>
 * to accept test results  from the llvm NightlyTest.pl script.
 * Rewritten by Jim Laskey to use php, so that we can reuse code in
 * ProgramResults.php
 *
 *******************************************************************************/

if(!(include "NightlyTester.php")){
  print "Error: could not load necessary files!\n";
  die();
}

if(!(include"ProgramResults.php")){
  print "Error: could not load necessary files!\n";
  die();
}

/*******************************************************************************
 *
 * Important variables
 *
 *******************************************************************************/
$database = "nightlytestresults";
$loginname = "llvm";
$password = "ll2002vm";

/*******************************************************************************
 *
 * Connecting to the database
 *
 *******************************************************************************/
$mysql_link = mysql_connect("127.0.0.1", $loginname, $password) or die("Error: could not connect to database!");
mysql_select_db($database);

/*******************************************************************************
 *
 * Some methods to help the process
 *
 *******************************************************************************/

/*******************************************************************************
 *
 * Opens a file handle to the specified filename, then writes the contents out,
 * and finally closes the filehandle.
 *
 *******************************************************************************/
function WriteFile($filename, $contents) {
  $file = fopen($filename, "w") or die("Could not open file $filename for writing");
  fwrite($file, $contents) or die("Could not write to file $filename");
  fclose($file);
}

/*******************************************************************************
 *
 * Queries database to see if there is a machine with the same uname as
 * the value passed in
 *
 *******************************************************************************/
function DoesMachineExist($uname, $hardware, $os, $name, $nickname, $gcc_version) {
  $query = "SELECT * FROM machine WHERE uname=\"$uname\" AND nickname=\"$nickname\" AND gcc=\"$gcc_version\"";
  $machine_query = mysql_query($query) or die(mysql_error());
  $row = mysql_fetch_array($machine_query) or die(mysql_error());
  mysql_free_result($machine_query);
  
  if($row &&
     strcmp($row['uname'], $uname) == 0 &&
     strcmp($row['hardware'], $hardware) == 0 &&
     strcmp($row['os'], $os) == 0 &&
     strcmp($row['nickname'], $nickname) == 0 &&
     strcmp($row['gcc'], $gcc_version) == 0) {
    return 1;
  }

  return 0;
}

/*******************************************************************************
 *
 * mysql> describe machine;
 * +-----------+----------+------+-----+---------+----------------+
 * | Field     | Type     | Null | Key | Default | Extra          |
 * +-----------+----------+------+-----+---------+----------------+
 * | id        | int(11)  |      | PRI | NULL    | auto_increment |
 * | uname     | text     |      |     |         |                |
 * | hardware  | text     |      |     |         |                |
 * | os        | text     |      |     |         |                |
 * | name      | text     |      |     |         |                |
 * | nickname  | tinytext | YES  |     | NULL    |                |
 * | gcc       | text     | YES  |     | NULL    |                |
 * | directory | text     | YES  |     | NULL    |                |
 * +-----------+----------+------+-----+---------+----------------+
 * 8 rows in set (0.00 sec)
 *
 *
 * Creates an entry in the machine table in the database
 *
 *******************************************************************************/
function AddMachine($uname, $hardware, $os, $name, $nickname, $gcc_version, $directory) {
  $query = "INSERT INTO machine (uname, hardware, os, name, nickname, gcc, directory) " .
           "VALUES (\"$uname\",\"$hardware\",\"$os\",\"$name\",\"$nickname\",\"$gcc_version\",\"$directory\")";
  $insert_query = mysql_query($query) or die(mysql_error());
  mysql_free_result($insert_query);
}

/*******************************************************************************
 *
 * Returns the id number of the machine with the passed in uname
 *
 *******************************************************************************/
function GetMachineId($uname, $hardware, $os, $name, $nickname, $gcc_version) {
  $query = "SELECT * FROM machine WHERE uname=\"$uname\" AND hardware=\"$hardware\" ".
           "AND os=\"$os\" AND name=\"$name\" AND gcc=\"$gcc_version\"";
  $machine_query = mysql_query($query) or die(mysql_error());
  $row = mysql_fetch_array($machine_query) or die(mysql_error());
  mysql_free_result($machine_query);

  if($row) {
    return $row['id'];
  } else {
    return -1;
  }
}

/*******************************************************************************
 *Since this is ugly, iv included an example call and a view of the table.
 *
 * +---------------------+------------+------+-----+---------+----------------+
 * | Field               | Type       | Null | Key | Default | Extra          |
 * +---------------------+------------+------+-----+---------+----------------+
 * | id                  | int(11)    |      | PRI | NULL    | auto_increment |
 * | machine             | text       |      |     |         |                |
 * | added               | datetime   | YES  |     | NULL    |                |
 * | buildstatus         | tinytext   | YES  |     | NULL    |                |
 * | getcvstime_cpu      | double     | YES  |     | NULL    |                |
 * | getcvstime_wall     | double     | YES  |     | NULL    |                |
 * | configuretime_cpu   | double     | YES  |     | NULL    |                |
 * | configuretime_wall  | double     | YES  |     | NULL    |                |
 * | buildtime_cpu       | double     | YES  |     | NULL    |                |
 * | buildtime_wall      | double     | YES  |     | NULL    |                |
 * | dejagnutime_cpu     | double     | YES  |     | NULL    |                |
 * | dejagnutime_wall    | double     | YES  |     | NULL    |                |
 * | warnings            | mediumtext | YES  |     | NULL    |                |
 * | warnings_added      | text       | YES  |     | NULL    |                |
 * | warnings_removed    | text       | YES  |     | NULL    |                |
 * | teststats_exppass   | int(11)    | YES  |     | NULL    |                |
 * | teststats_unexpfail | int(11)    | YES  |     | NULL    |                |
 * | teststats_expfail   | int(11)    | YES  |     | NULL    |                |
 * | all_tests           | text       | YES  |     | NULL    |                |
 * | passing_tests       | text       | YES  |     | NULL    |                |
 * | unexpfail_tests     | text       | YES  |     | NULL    |                |
 * | expfail_tests       | text       | YES  |     | NULL    |                |
 * | newly_passing_tests | text       | YES  |     | NULL    |                |
 * | newly_failing_tests | text       | YES  |     | NULL    |                |
 * | new_tests           | text       | YES  |     | NULL    |                |
 * | removed_tests       | text       | YES  |     | NULL    |                |
 * | cvs_added           | text       | YES  |     | NULL    |                |
 * | cvs_removed         | text       | YES  |     | NULL    |                |
 * | cvs_modified        | text       | YES  |     | NULL    |                |
 * | cvs_usersadd        | text       | YES  |     | NULL    |                |
 * | cvs_usersco         | text       | YES  |     | NULL    |                |
 * | a_file_size         | text       | YES  |     | NULL    |                |
 * | o_file_size         | text       | YES  |     | NULL    |                |
 * +---------------------+------------+------+-----+---------+----------------+
 *
 *
 *******************************************************************************/

function CreateNight($machine_id,
                $added,
                $buildstatus,
                $getcvstime_cpu,
                $getcvstime_wall,
                $configuretime_cpu,
                $configuretime_wall,
                $buildtime_cpu,
                $buildtime_wall,
                $dejagnutime_cpu,
                $dejagnutime_wall,
                $warnings,
                $warnings_added,
                $warnings_removed,
                $teststats_exppass,
                $teststats_unexpfail,
                $teststats_expfail,
                $all_tests,
                $passing_tests,
                $unexpfail_tests,
                $expfail_tests,
                $newly_passing_tests,
                $newly_failing_tests,
                $new_tests,
                $removed_tests,
                $cvs_added,
                $cvs_removed,
                $cvs_modified,
                $cvs_usersadd,
                $cvs_usersco,
                $a_file_size,
                $o_file_size) {
  $query = "INSERT INTO night (".
      "machine, added, buildstatus, configuretime_cpu, configuretime_wall," .
      "getcvstime_cpu, getcvstime_wall, buildtime_cpu, buildtime_wall," .
      "dejagnutime_cpu, dejagnutime_wall, warnings, warnings_added, warnings_removed," .
      "teststats_exppass, teststats_unexpfail, teststats_expfail," .
      "all_tests, passing_tests, unexpfail_tests, expfail_tests," .
      "newly_passing_tests, newly_failing_tests, new_tests, removed_tests," .
      "cvs_added, cvs_removed, cvs_modified, cvs_usersadd, cvs_usersco" .
      ") VALUES (".
      "\"$machine_id\", \"$added\", \"$buildstatus\", \"$configuretime_cpu\", \"$configuretime_wall\"," .
      "\"$getcvstime_cpu\", \"$getcvstime_wall\", \"$buildtime_cpu\", \"$buildtime_wall\"," .
      "\"$dejagnutime_cpu\", \"$dejagnutime_wall\", \"$warnings\", \"$warnings_added\", \"$warnings_removed\"," .
      "\"$teststats_exppass\", \"$teststats_unexpfail\", \"$teststats_expfail\"," .
      "\"$all_tests\", \"$passing_tests\", \"$unexpfail_tests\", \"$expfail_tests\", \"$newly_passing_tests\"," .
      "\"$newly_failing_tests\", \"$new_tests\", \"$removed_tests\", \"$cvs_added\"," .
      "\"$cvs_removed\", \"$cvs_modified\", \"$cvs_usersadd\", \"$cvs_usersco\"" .
      ")";
  $insert_query = mysql_query($query) or die(mysql_error());
  mysql_free_result($insert_query) or die(mysql_error());
  
  $query = "SELECT id FROM night WHERE machine=$machine_id AND added=\"$added\"";
  $machine_query = mysql_query($query) or die(mysql_error());
  $row = mysql_fetch_array($machine_query) or die(mysql_error());
  mysql_free_result($machine_query);
  
  if($row) {
    return $row['id'];
  } else {
    return -1;
  }
}

/*******************************************************************************
 *******************************************************************************/
function GetMachineNights($machine_id) {
  $result = array();
  $query = "SELECT * FROM night WHERE machine = \"$machine_id\"";
  $night_query = mysql_query($query) or die(mysql_error());
  while ($row = mysql_fetch_array($night_query) or die(mysql_error())) {
    array_push($result, $row['id']);
  }
  mysql_free_result($night_query);
  return $result;
}

/*******************************************************************************
 *
 * mysql> describe program;
 * +---------+---------+------+-----+---------+-------+
 * | Field   | Type    | Null | Key | Default | Extra |
 * +---------+---------+------+-----+---------+-------+
 * | program | text    |      |     |         |       |
 * | result  | text    | YES  |     | NULL    |       |
 * | type    | text    | YES  |     | NULL    |       |
 * | night   | int(11) |      |     | 0       |       |
 * +---------+---------+------+-----+---------+-------+
 * 4 rows in set (0.00 sec)
 *
 *******************************************************************************/
function AddProgram($program, $result, $type, $night) {
  $query = "INSERT INTO program (program, result, type, night) VALUES".
           " (\"$program\", \"$result\", \"$type\", $night)";
  $program_query = mysql_query($query) or die(mysql_error());
  mysql_free_result($program_query);
}

/*******************************************************************************
 *
 * mysql> describe tests;
 * +---------+-----------------------------+------+-----+---------+-------+
 * | Field   | Type                        | Null | Key | Default | Extra |
 * +---------+-----------------------------+------+-----+---------+-------+
 * | program | tinytext                    |      |     |         |       |
 * | result  | enum('PASS','FAIL','XFAIL') |      |     | PASS    |       |
 * | measure | tinytext                    | YES  |     | NULL    |       |
 * | night   | int(11)                     | YES  |     | NULL    |       |
 * +---------+-----------------------------+------+-----+---------+-------+
 * 4 rows in set (0.00 sec)
 *
 *******************************************************************************/
function AddTests($program, $result, $measure, $night) {
  $query = "INSERT INTO program (program, result, measure, night) VALUES".
           " (\"$$program\", \"$result\", \"$measure\", $night)";
  $program_query = mysql_query($query) or die(mysql_error());
  mysql_free_result($program_query);
}


/*******************************************************************************
 *
 * mysql> describe file;
 * +-------+---------+------+-----+---------+-------+
 * | Field | Type    | Null | Key | Default | Extra |
 * +-------+---------+------+-----+---------+-------+
 * | file  | text    |      |     |         |       |
 * | size  | int(11) |      |     | 0       |       |
 * | night | int(11) |      |     | 0       |       |
 * | type  | text    | YES  |     | NULL    |       |
 * +-------+---------+------+-----+---------+-------+
 * 4 rows in set (0.00 sec)
 *
 *******************************************************************************/
function AddFile($file, $size, $night, $type) {
  $query = "INSERT INTO file (file, size, night, type) VALUES".
           " (\"$file\", \"$size\", \"$night\", \"$type\")";
  $file_query = mysql_query($query) or die(mysql_error());
  mysql_free_result($file_query);
}

/*******************************************************************************
 *
 * mysql> describe code;
 * +-------+----------+------+-----+---------------------+-------+
 * | Field | Type     | Null | Key | Default             | Extra |
 * +-------+----------+------+-----+---------------------+-------+
 * | added | datetime |      |     | 0000-00-00 00:00:00 |       |
 * | loc   | int(11)  |      |     | 0                   |       |
 * | files | int(11)  |      |     | 0                   |       |
 * | dirs  | int(11)  |      |     | 0                   |       |
 * +-------+----------+------+-----+---------------------+-------+
 * 4 rows in set (0.00 sec)
 *
 * This function checks to see if the last entered values in the database
 * about code information are the same as our current information. If they
 * differ we will put our information into the database.
 *
 *******************************************************************************/
function UpdateCodeInfo($date, $loc, $files, $dirs) {
  $query = "SELECT * FROM code ORDER BY added DESC";
  $code_query = mysql_query($query) or die(mysql_error());
  $row = mysql_fetch_array($code_query) or die(mysql_error());
  mysql_free_result($code_query);
  if ($row &&
      ($row['loc'] != $loc ||
       $row['files'] != $files ||
       $row['dirs'] != $dirs)) {
    $query = "INSERT INTO code (added, loc, files, dirs) VALUES (\"$date\", \"$loc\", \"$files\", \"$dirs\")";
    $code_query = mysql_query($query) or die(mysql_error());
    mysql_free_result($code_query);
  }
}

/*******************************************************************************
 *
 * Match one substring and return string result.
 *
 *******************************************************************************/
function MatchOne($pattern, $string, $default) {
  $subpatterns = array();
  if (isset($string) && preg_match($pattern, $string, $subpatterns)) {
    return rtrim($subpatterns[1]);
  }
  
  return $default;
}

/*******************************************************************************
 *
 * Match all substrings and return array result.
 *
 *******************************************************************************/
function Match($pattern, $string) {
  $subpatterns = array();
  if (isset($string) && preg_match($pattern, $string, $subpatterns)) {
    return $subpatterns;
  }
  
  return array();
}

/*******************************************************************************
 *
 * ProcessProgram Test Table Logs
 *
 * The logs are start out like;
 *    Program,GCCAS,Bytecode,LLC compile,LLC-BETA compile,JIT codegen,GCC,CBE,LLC,LLC-BETA,JIT,GCC/CBE,GCC/LLC,GCC/LLC-BETA
 *    Benchmarks/CoyoteBench/almabench,0.0230,11702,0.0278,0.0211,0.0255,  15.26,  14.36,  14.40,   0.01,  16.26,1.06,1.06,-,-
 *    ...
 *
 *******************************************************************************/
function ProcessProgramLogs($tests) {
  $processed = array();
  $headings = $tests[0];
  $headings = preg_replace("/<br>/", " ", $headings);
  $headings = split(",", $headings);
  for ($i = 1; $i < count($tests); $i++) {
    $results = split(",", $tests[$i]);
    $program = $results[0];
    $outcome = "";
    for ($j = 1; $j < count($headings); $j++) {
      $heading = $headings[$j];
      $result = $results[$j];
      $outcome .= "$heading: $result, ";
    }
    $processed[$program] = $outcome;
  }
  return $processed;
}
 
/*******************************************************************************
 *
 * Setting up variables
 *
 *******************************************************************************/
$spliton ="\n";

$machine_data = $_POST['machine_data'];
if (!isset($machine_data)) {
  $machine_data = "";
}
$MACHINE_DATA = split($spliton, $machine_data);

$cvs_log = $_POST['cvs_data'];
if (!isset($cvs_log)) {
  $cvs_log = "";
}
$CVS_LOG = split($spliton, $cvs_log);

$build_log = $_POST['build_data'];
if (!isset($build_log)) {
  $build_log = "";
}
$BUILD_LOG = split($spliton, $build_log);

$dejagnutests_results = $_POST['dejagnutests_results'];
if (!isset($dejagnutests_results)) {
  $dejagnutests_results = "";
}
$DEJAGNUTESTS_RESULTS = split($spliton, $dejagnutests_results);

$dejagnutests_log = $_POST['dejagnutests_log'];
if (!isset($dejagnutests_log)) {
  $dejagnutests_log = "";
}
$DEJAGNUTESTS_LOG = split($spliton, $dejagnutests_log);

$singlesource_tests = $_POST['singlesource_programstable'];
if (!isset($singlesource_tests)) {
  $singlesource_tests = "";
}
$SINGLESOURCE_TESTS = split($spliton, $singlesource_tests);

$multisource_tests = $_POST['multisource_programstable'];
if (!isset($multisource_tests)) {
  $multisource_tests = "";
}
$MULTISOURCE_TESTS = split($spliton, $multisource_tests);

$external_tests = $_POST['externalsource_programstable'];
if (!isset($external_tests)) {
  $external_tests = "";
}
$EXTERNAL_TESTS = split($spliton, $external_tests);

$o_file_size = $_POST['o_file_sizes']; 
if (!isset($o_file_size)) {
  $o_file_size = "";
}
$o_file_size = rtrim($o_file_size);
$O_FILE_SIZE = split($spliton, $o_file_size);

$a_file_size = $_POST['a_file_sizes']; 
if (!isset($a_file_size)) {
  $a_file_size = "";
}
$a_file_size = rtrim($a_file_size);
$A_FILE_SIZE = split($spliton, $a_file_size);

$filesincvs = $_POST['cvs_file_count'];
$dirsincvs = $_POST['cvs_dir_count'];
$loc = $_POST['lines_of_code'];
$nickname = $_POST['nickname'];
$cvscheckouttime_cpu = $_POST['cvscheckouttime_cpu'];
$cvscheckouttime_wall = $_POST['cvscheckouttime_wall'];
$configtime_wall = $_POST['configtime_wall'];
$configtime_cpu = $_POST['configtime_cpu'];
$buildtime_cpu = $_POST['buildtime_cpu'];
$buildtime_wall = $_POST['buildtime_wall'];
$dejagnutime_cpu = $_POST['dejagnutime_cpu'];
$dejagnutime_wall = $_POST['dejagnutime_wall'];
$buildwarnings = $_POST['warnings'];
$cvsaddedfiles = $_POST['cvsaddedfiles'];
$cvsremovedfiles = $_POST['cvsremovedfiles'];
$cvsmodifiedfiles = $_POST['cvsmodifiedfiles'];
$cvsusercommitlist = $_POST['cvsusercommitlist'];
$cvsuserupdatelist = $_POST['cvsuserupdatelist'];
$buildstatus = $_POST['buildstatus'];
$warnings_added = $_POST['warnings_removed'];
$warnings_removed = $_POST['warnings_added'];
$all_tests = $_POST['all_tests'];
$unexpfail_tests = $_POST['unexpfail_tests'];
$passing_tests = $_POST['passing_tests'];
$expfail_tests = $_POST['expfail_tests'];
$newly_passing_tests = $_POST['newly_passing_tests'];
$newly_failing_tests = $_POST['newly_failing_tests'];
$new_tests = $_POST['new_tests'];
$removed_tests = $_POST['removed_tests'];
$gcc_version = $_POST['gcc_version'];            
$warnings = $_POST['warnings'];            
$lines_of_code = $_POST['lines_of_code'];

/*******************************************************************************
 *
 * Extracting the machine information
 *
 *******************************************************************************/
$uname    = MatchOne("/uname\:\s*(.+)/",    $MACHINE_DATA[0], "");
$hardware = MatchOne("/hardware\:\s*(.+)/", $MACHINE_DATA[1], "");
$os       = MatchOne("/os\:\s*(.+)/",       $MACHINE_DATA[2], "");
$name     = MatchOne("/name\:\s*(.+)/",     $MACHINE_DATA[3], "");
$date     = MatchOne("/date\:\s*(.+)/",     $MACHINE_DATA[4], "");
$time     = MatchOne("/time\:\s*(.+)/",     $MACHINE_DATA[5], "");

/*******************************************************************************
 *
 * Extracting the dejagnu test numbers
 *
 *******************************************************************************/

$dejagnu_exp_passes     = MatchOne("/\# of expected passes\s*([0-9]+)/",   $dejagnutests_log, 0);
$dejagnu_unexp_failures = MatchOne("/unexpected failures\s*([0-9]+)/",     $dejagnutests_log, 0);
$dejagnu_exp_failures   = MatchOne("/\# of expected failures\s*([0-9]+)/", $dejagnutests_log, 0);

/*******************************************************************************
 *
 * Processing Program Test Table Logs
 *
 *******************************************************************************/
 
$singlesource_processed = ProcessProgramLogs($SINGLESOURCE_TESTS);
$multisource_processed = ProcessProgramLogs($MULTISOURCE_TESTS);
$external_processed = ProcessProgramLogs($EXTERNAL_TESTS);

/*******************************************************************************
 *
 * creating the response
 *
 *******************************************************************************/
print "content-type: text/text\r\n\r\n";

if (!DoesMachineExist($uname, $hardware, $os, $name, $nickname, $gcc_version)) {
  AddMachine($uname, $hardware, $os, $name, $nickname, $gcc_version, "test");
}
$machine_id = GetMachineId($uname, $hardware, $os, $name, $nickname, $gcc_version);

/*******************************************************************************
 *
 * Submitting information to database
 *
 *******************************************************************************/
$db_date = date("Y-m-d H:i:s");
$blank="";
$night_id= CreateNight($machine_id, $db_date, $buildstatus, 
            $configtime_cpu, $configtime_wall, $cvscheckouttime_cpu,
            $cvscheckouttime_wall, $buildtime_cpu, $buildtime_wall,
            $dejagnutime_cpu, $dejagnutime_wall, $warnings, 
            $warnings_added, $warnings_removed,
            $dejagnu_exp_passes, $dejagnu_unexp_failures, $dejagnu_exp_failures,
            $blank, $blank, $blank,       // $all_tests, $passing_tests, $unexpfail_tests, 
            $blank, $blank, $blank,       // $expfail_tests, $TestsFixed, $TestsBroken,
            $blank, $blank,               // $TestsAdded, $TestsRemoved,
            $cvsaddedfiles, $cvsremovedfiles, $cvsmodifiedfiles,
            $cvsusercommitlist, $cvsuserupdatelist);


foreach ($singlesource_processed as $key => $value) {
   AddProgram($key, $value, "singlesource", $night_id); 
}

foreach ($multisource_processed as $key => $value) {
   AddProgram($key, $value, "multisource", $night_id); 
}

foreach ($external_processed as $key => $value) {
   AddProgram($key, $value, "external", $night_id); 
}

foreach ($O_FILE_SIZE as $info) {
  list($ignore, $size, $file, $type) = Match("/(.+)\s+(.+)\s+(.+)/", $info);
  AddFile($file, $size, $night_id, $type);
}

foreach ($A_FILE_SIZE as $info) {
  list($ignore, $size, $file, $type) = Match("/(.+)\s+(.+)\s+(.+)/", $info);
  AddFile($file, $size, $night_id, $type);
}

/*******************************************************************************
 *
 * Adding test results pass/fail/xfail status to database
 *
 *******************************************************************************/
$ALL_TESTS = split("\n", $all_tests);
foreach ($ALL_TESTS as $info) {
  $subpatterns = array();
  if (preg_match("/(TEST-)?(XPASS|PASS|XFAIL|FAIL):\s(.+?)\s(.+)/", $info, $subpatterns)) {
    list($ignore1, $ignore2, $result, $measure, $program) = $subpatterns;
    $query = "INSERT INTO tests (program, result, measure, night) ".
             "VALUES (\"$program\", \"$result\", \"$measure\", $night_id)";
    $test_query = mysql_query($query) or die(mysql_error());
    mysql_free_result($test_query);
  } else {
    print "Unrecognized test: $info\n";
  }
}

foreach ($DEJAGNUTESTS_RESULTS as $info) {
  $subpatterns = array();
  if (preg_match("/^(XPASS|PASS|XFAIL|FAIL):\s(.+):?/", $info, $subpatterns)) {
    list($ignore, $result, $program) = $subpatterns;
  	$query="INSERT INTO tests (program, result, measure, night) ".
  	       "VALUES(\"$result\", \'$program\', \"dejagnu\", $night_id)";
    $test_query = mysql_query($query) or die(mysql_error());
    mysql_free_result($test_query);
  }
}

/*******************************************************************************
 *
 * Adding lines of code to the database
 *
 *******************************************************************************/
UpdateCodeInfo($db_date, $loc, $filesincvs, $dirsincvs);

// print "received ${_POST['CONTENT_LENGTH']} bytes\n";
print "received 0 bytes\n";
         
$nights = GetMachineNights($machine_id);
$length = count($nights);
print "DB date : $db_date\n";
print "Machine $machine_id now has ids [$nights]{$length} ".
      "associated with it in the database\n";

/*******************************************************************************
 *
 * building hash of arrays of signifigant changes to place into the nightly
 * test results email. This is ugly code and is somewhat of a hack. However, it
 * adds very useful information to the nightly test email.
 *
 *******************************************************************************/
$query = "SELECT id FROM night WHERE id<$night_id AND machine=$machine_id AND ".
         "buildstatus=\"OK\" ORDER BY id DESC";
$night_query = mysql_query($query) or die(mysql_error());
$row = mysql_fetch_array($night_query) or die(mysql_error());
$prev_night = $row['id'];
mysql_free_result($night_query);

$query = "SELECT * FROM program WHERE night=$night_id";
$program_query = mysql_query($query) or die(mysql_error());

$prog_hash_new = array();
while ($row = mysql_fetch_array($program_query)) {
  $program = $row['program'];
  $result = $row['result'];
  $prog_hash_new[$program] = $result;
}
mysql_free_result($program_query);

$query = "SELECT * FROM program WHERE night=$prev_night";
$prog_hash_old = array();
while ($row = mysql_fetch_array($program_query)) {
  $program = $row['program'];
  $result = $row['result'];
  $prog_hash_old[$program] = $result;
}
mysql_free_result($program_query);

$output_big_changes = array();
foreach ($prog_hash_new as $prog) {
  $prog_new = $prog_hash_new[$prog];
  $prog_old = $prog_hash_old[$prog];
    
  // get data from server
  $vals_new = split(",", $prog_new);
  $vals_old = split(",", $prog_old);
  
  // get list of values measured from newest test
  $measures = array();
  foreach ($vals_new as $measure) {
    $measure = preg_replace("/[ \,\.\:\-0-9]*/", "", $measure);
    if(strpos($measure, "/") == 0) {
      array_push($measures, $measure);
    }
  }
  
  // put measures into hash of arrays
  foreach ($measures as $measure) {
    $value_new = MatchOne("/$measure:.*?(\d*\.*\d+)/", $prog_new, "0");
    $value_old = MatchOne("/$measure:.*?(\d*\.*\d+)/", $prog_old, "0");
    $diff = ($value_old - $value_new);
    $perc = 0;
    
    if ($value_old != 0 && ($diff > 0.2 || $diff < -0.2) ) {
      $perc = ($diff / $value_old) * 100;
    }
    
    if ($perc > 5 || $perc < -5) {
      $changes = $output_big_changes[$measure];
      
      if (!isset($changes)) {
        $changes = array();
      }
      
      $rounded_perc = sprintf("%1.2f", $perc);
      array_push($changes, "$prog: $rounded_perc\% ($value_old => $value_new)\n");
      $output_big_changes[$measure] = $changes;
    }
  }
}

/*******************************************************************************
 *
 * Determining changes in new tests and old tests
 *
 *******************************************************************************/
 
$removed = getRemovedTests($night_id, $prev_night);
$added = getNewTests($night_id, $prev_night);
$passing = getFixedTests($night_id, $prev_night);
$failing = getBrokenTests($night_id, $prev_night);


/*******************************************************************************
 *
 * Encode date for file name use
 *
 *******************************************************************************/
 
$db_date = preg_replace("/ /", "_", $db_date);

/*******************************************************************************
 *
 * Sending email to nightly test email archive
 *
 *******************************************************************************/
$link_to_page="http://llvm.org/nightlytest/test.php?machine=$machine_id&".
              "night=$night_id";
$email  = "$link_to_page\n";
$email .= "Name: $name\n";
$email .= "Nickname: $nickname\n";
$email .= "Buildstatus: $buildstatus\n";

if(strcmp($buildstatus, "OK") == 0) {
  if (strlen($passing) == 0) {
    $passing = "None";
  } 
  $email .= "\nNew Test Passes: $passing\n";
  if (strlen($failing) == 0) {
    $failing = "None";
  } 
  $email .= "\nNew Test Failures: $failing\n";
  if (strlen($added) == 0) {
    $added = "None";
  } 
  $email .= "\nAdded Tests: $added\n";
  if (strlen($removed) == 0) {
    $removed= "None";
  } 
  $email .= "\nRemoved Tests: $removed\n";

  $email .= "\nSignificant changes in test results:\n";
  foreach ($output_big_changes as $measure => $values) {
    $email.= "$measure:\n";
    foreach ($values as $value) {
      $email.= " $value";
    }
  }
}
else{
  $email .= "\nBuildlog available at http://llvm.org/nightlytest/".
            "machines/$machine_id/$db_date-Build-Log.txt\n";
}

$email_addr = "jlaskey\@apple.com";
//$email_addr = "llvm-testresults\@cs.uiuc.edu";
`echo "$email" | mail -s '$nickname $hardware nightly tester results' $email_addr`;

/*******************************************************************************
 *
 * writing logs to directory
 *
 *******************************************************************************/
$cwd = getcwd();

if (!file_exists('machines')) {
  mkdir('machines', 777);
}
chdir("$curr/machines");

if (!file_exists("$machine_id")) {
    mkdir("$machine_id", 777);
    
}
chdir("$curr/machines/$machine_id");

WriteFile("$db_date-Build-Log.txt", $build_log);
WriteFile("$db_date-O-files.txt", $o_file_size);
WriteFile("$db_date-A-files.txt", $a_file_size);


$sentdata="";
foreach ($_GET as $key => $value) {
	$sentdata .= "$key  =>  $value}\n";
}
foreach ($_POST as $key => $value) {
	$sentdata .= "$key  =>  $value}\n";
}
WriteFile("$db_date-senddata.txt", $sentdata);


mysql_close($mysql_link);
?>
