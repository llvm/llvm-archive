<?php

/*******************************************************************************
*
* Debug info
*
*******************************************************************************/
$print_debug = 0;

/*******************************************************************************
* mysql> describe machineInfo;
* +--------------+----------+------+-----+---------+----------------+
* | Field        | Type     | Null | Key | Default | Extra          |
* +--------------+----------+------+-----+---------+----------------+
* | id           | int(11)  |      | PRI | NULL    | auto_increment |
* | targetTriple | tinytext |      |     |         |                |
* | hostname     | tinytext |      |     |         |                |
* | nickname     | tinytext |      |     |         |                |
* +--------------+----------+------+-----+---------+----------------+
* id = unqiue id assigned to machine
* targetTriple = llvm TARGET_TRIPLE value
* hostname = full hostname (ie. zion.llvm.org)
* nickname = short name (ie. zion)
*
* Returns id of machine and adds machine if it is not already in database.
*******************************************************************************/
function getMachineID($targetTriple, $hostname, $nickname) {
  $sqlQuery = "SELECT id from machineInfo WHERE targetTriple=\"$targetTriple\" " 
               . "AND hostname=\"$hostname\" AND nickname=\"$nickname\"";
  $result = mysql_query($sqlQuery) or die(mysql_error());

  $row = mysql_fetch_assoc($result);
  
  if($row) {
    return $row['id']; 
  }
  else {
    $sqlQuery = "INSERT into machineInfo (targetTriple, hostname, nickname) VALUES "
                . "(\"$targetTriple\", \"$hostname\", \"$nickname\")";
    mysql_query($sqlQuery) or die(mysql_error());
    $id =  mysql_insert_id() or die(mysql_error());
    return $id;
  }
}

/*******************************************************************************
* mysql> describe testRunInfo;
* +-------------------+-----------------------------------+------+-----+---------------------+----------------+
* | Field             | Type                              | Null | Key | Default             | Extra          |
* +-------------------+-----------------------------------+------+-----+---------------------+----------------+
* | id                | int(11)                           |      | PRI | NULL                | auto_increment |
* | runDateTime       | datetime                          |      |     | 0000-00-00 00:00:00 |                |
* | machineId         | int(11)                           |      |     | 0                   |                |
* | machineUname      | text                              | YES  |     | NULL                |                |
* | gccVersion        | text                              | YES  |     | NULL                |                |
* | cvsCpuTime        | double                            | YES  |     | NULL                |                |
* | cvsWallTime       | double                            | YES  |     | NULL                |                |
* | configureCpuTime  | double                            | YES  |     | NULL                |                |
* | configureWallTime | double                            | YES  |     | NULL                |                |
* | buildCpuTime      | double                            | YES  |     | NULL                |                |
* | buildWallTime     | double                            | YES  |     | NULL                |                |
* | dejagnuCpuTime    | double                            | YES  |     | NULL                |                |
* | dejagnuWallTime   | double                            | YES  |     | NULL                |                |
* | warnings          | mediumtext                        | YES  |     | NULL                |                |
* | warningsAdded     | text                              | YES  |     | NULL                |                |
* | warningsRemoved   | text                              | YES  |     | NULL                |                |
* | cvsUsersAdd       | text                              | YES  |     | NULL                |                |
* | cvsUsersCO        | text                              | YES  |     | NULL                |                |
* | cvsFilesAdded     | text                              | YES  |     | NULL                |                |
* | cvsFilesRemoved   | text                              | YES  |     | NULL                |                |
* | cvsFilesModified  | text                              | YES  |     | NULL                |                |
* | buildStatus       | tinyint(4)                        | YES  |     | NULL                |                |
* | type              | enum('release','debug','unknown') | YES  |     | unknown             |                |
* +-------------------+-----------------------------------+------+-----+---------------------+----------------+
* Add a test run configuration to the database for logging test results.
*
*******************************************************************************/
function addTestRunConfig($runDateTime, $machineId, $machineUname, $gccVersion,
                         $cvsCpuTime, $cvsWallTime, $configureCpuTime,
                         $configureWallTime,
                         $buildCpuTime, $buildWallTime, $dejagnuCpuTime,
                         $dejagnuWallTime, $warnings, $warningsAdded, 
                         $warningsRemoved, $cvsUsersAdd, $cvsUsersCO, 
                         $cvsFilesAdded, $cvsFilesRemoved, $cvsFilesModified,
                         $buildStatus) {
  
  $sqlQuery = "INSERT into testRunInfo (runDateTime, machineId, machineUname," .
              " gccVersion, cvsCpuTime, cvsWallTime, configureCpuTime," .
              " configureWallTime, buildCpuTime, buildWallTime," .
              " dejagnuCpuTime, dejagnuWallTime, warnings, warningsAdded," .
              " warningsRemoved, cvsUsersAdd, cvsUsersCO, cvsFilesAdded," .
              " cvsFilesRemoved, cvsFilesModified, buildStatus) VALUES (".
              " \"$runDateTime\", \"$machineId\", \"$machineUname\"," .
              " \"$gccVersion\", \"$cvsCpuTime\", \"$cvsWallTime\"," . 
              " \"$configureCpuTime\", \"$configureWallTime\", \"$buildCpuTime\"," .
              " \"$buildWallTime\", \"$dejagnuCpuTime\", \"$dejagnuWallTime\"," .
              " \"$warnings\", \"$warningsAdded\", \"$warningsRemoved\"," .
              " \"$cvsUsersAdd\", \"$cvsUsersCO\", \"$cvsFilesAdded\"," .
              " \"$cvsFilesRemoved\", \"$cvsFilesModified\", \"$buildStatus\")";
  mysql_query($sqlQuery) or die(mysql_error());
  $id = mysql_insert_id() or die(mysql_error());  
  return $id;
}

/*******************************************************************************
*
* mysql> describe dejagnuTests;
* +-------+----------+------+-----+---------+----------------+
* | Field | Type     | Null | Key | Default | Extra          |
* +-------+----------+------+-----+---------+----------------+
* | id    | int(11)  |      | PRI | 0       | auto_increment |
* | name  | tinytext | YES  |     | NULL    |                |
* +-------+----------+------+-----+---------+----------------+
*
* mysql> describe dejagnuTestResults;
* +----------+-------------------------------------+------+-----+---------+----------------+
* | Field    | Type                                | Null | Key | Default | Extra          |
* +----------+-------------------------------------+------+-----+---------+----------------+
* | id       | int(11)                             |      | PRI | NULL    | auto_increment |
* | testId   | int(11)                             | YES  |     | NULL    |                |
* | configId | int(11)                             | YES  |     | NULL    |                |
* | result   | enum('PASS','FAIL','XFAIL','XPASS') | YES  |     | NULL    |                |
* +----------+-------------------------------------+------+-----+---------+----------------+
*
* Add dejagnu test and its status (PASS, XPASS, XFAIL, FAIL).
* Dejagnu tests each have their own unique id to represent them.
*******************************************************************************/
function addDejagnuTestResult($testName, $status, $testRunConfigId) {
  // Strip off path from testName $path/test/($testName)
  
  // Get testID for testName
  $sqlQuery = "SELECT id FROM dejagnuTests WHERE name=\"$testName\"";
  $result = mysql_query($sqlQuery) or die(mysql_error());
  $row = mysql_fetch_assoc($result);
  
  if($row) {
    $id = $row['id']; 
  }
  else {
    $sqlQuery = "INSERT into dejagnuTests (name) VALUES (\"$testName\")";
    mysql_query($sqlQuery) or die(mysql_error());
    $id =  mysql_insert_id() or die(mysql_error());
  }

  // Insert test result
  $query = "INSERT into dejagnuTestResults (testId, configId, result) " .
    "VALUES (\"$id\", \"$testRunConfigId\", \"$status\")";
  mysql_query($query) or die(mysql_error());
  $testResultId = mysql_insert_id() or die(mysql_error());
  return $testResultId;
  
}
/*******************************************************************************
* mysql> describe programs;
* +-------+------------+------+-----+---------+----------------+
* | Field | Type       | Null | Key | Default | Extra          |
* +-------+------------+------+-----+---------+----------------+
* | id    | int(11)    |      | PRI | NULL    | auto_increment |
* | name  | mediumtext | YES  |     | NULL    |                |
* +-------+------------+------+-----+---------+----------------+
*
* mysql> describe programResults;
* +--------------------+---------+------+-----+---------+----------------+
* | Field              | Type    | Null | Key | Default | Extra          |
* +--------------------+---------+------+-----+---------+----------------+
* | id                 | int(11) |      | PRI | NULL    | auto_increment |
* | testId             | int(11) | YES  |     | NULL    |                |
* | nightId            | int(11) | YES  |     | NULL    |                |
* | gccasTime          | double  | YES  |     | NULL    |                |
* | bytecodeSize       | int(11) | YES  |     | NULL    |                |
* | llcTime            | double  | YES  |     | NULL    |                |
* | llcBetaTime        | double  | YES  |     | NULL    |                |
* | jitTime            | double  | YES  |     | NULL    |                |
* | gccExecuteTime     | double  | YES  |     | NULL    |                |
* | cbeExecuteTime     | double  | YES  |     | NULL    |                |
* | llcExecuteTime     | double  | YES  |     | NULL    |                |
* | llcBetaExecuteTime | double  | YES  |     | NULL    |                |
* | jitExecuteTime     | double  | YES  |     | NULL    |                |
* +--------------------+---------+------+-----+---------+----------------+
*
*
*
*******************************************************************************/
function addProgramResult($programLine, $configId, $prefix) {
  if (!$programLine) {
    return;
  }
  
  // Skip headers
  $subpatterns = array();
  if (preg_match("/^Program,(.+)/", $programLine, $subpatterns)) {
    return;
  }

  $results = split(",", $programLine);
  
  // If the output order changes, so must this
  if(count($results) != 15) {
    return;
  }
  
  $program = $results[0];
  $program = $prefix . $program;
  $gccasTime = $results[1];
  $byteCodeSize = $results[2];
  $llcTime = $results[3];
  $llcBetaTime = $results[4];
  $jitTime = $results[5];
  $gccRunTime = $results[6];
  $cbeRunTime = $results[7];
  $llcRunTime = $results[8];
  $llcBetaRunTime = $results[9];
  $jitRunTime = $results[10];
    
  // Get id for program name 
  $sqlQuery = "SELECT id FROM programs WHERE name=\"$program\"";
  $result = mysql_query($sqlQuery) or die(mysql_error());
  $row = mysql_fetch_assoc($result);
  
  if($row) {
    $id = $row['id']; 
  }
  else {
    $sqlQuery = "INSERT into programs (name) VALUES (\"$program\")";
    mysql_query($sqlQuery) or die(mysql_error());
    $id =  mysql_insert_id() or die(mysql_error());
  }
    
  $query = "INSERT INTO programResults (testId, nightId, gccasTime, 
                                        bytecodeSize, llcTime, llcBetaTime, 
                                        jitTime, gccExecuteTime, 
                                        cbeExecuteTime, llcExecuteTime,
                                        llcBetaExecuteTime, jitExecuteTime)" .
    " VALUES (\"$id\", \"$configId\", \"$gccasTime\", \"$byteCodeSize\",
              \"$llcTime\", \"$llcBetaTime\", \"$jitTime\", \"$gccRunTime\",
              \"$cbeRunTime\", \"$llcRunTime\", \"$llcBetaRunTime\",
              \"$jitRunTime\")";
  
  mysql_query($query) or die(mysql_error());
}

/*******************************************************************************
* mysql> describe files;
* +----------+----------+------+-----+---------+----------------+
* | Field    | Type     | Null | Key | Default | Extra          |
* +----------+----------+------+-----+---------+----------------+
* | id       | int(11)  |      | PRI | NULL    | auto_increment |
* | filename | tinytext | YES  |     | NULL    |                |
* +----------+----------+------+-----+---------+----------------+
*
*
* mysql> describe fileSizes;
* +---------+---------+------+-----+---------+----------------+
* | Field   | Type    | Null | Key | Default | Extra          |
* +---------+---------+------+-----+---------+----------------+
* | id      | int(11) |      | PRI | NULL    | auto_increment |
* | fileId  | int(11) | YES  |     | NULL    |                |
* | nightId | int(11) | YES  |     | NULL    |                |
* | size    | int(11) | YES  |     | NULL    |                |
* +---------+---------+------+-----+---------+----------------+
*
* Adds a file and its size to the database.
*******************************************************************************/
function addFileSize($file, $size, $configId) {
  
  // Get file id
  $query = "SELECT id FROM files WHERE filename=\"$file\"";
  $result = mysql_query($query) or die(mysql_error());
  $row = mysql_fetch_assoc($result);
  
  if($row) {
    $id = $row['id']; 
  }
  else {
    $sqlQuery = "INSERT into files (filename) VALUES (\"$file\")";
    mysql_query($sqlQuery) or die(mysql_error());
    $id =  mysql_insert_id() or die(mysql_error());
  }
  
  $insertQuery = "INSERT INTO fileSizes (fileId, nightId, size) VALUES".
  " (\"$id\", \"$configId\", \"$size\")";
  mysql_query($insertQuery) or die(mysql_error());
}

/*******************************************************************************
*
* mysql> describe llvmStats;
* +-------+----------+------+-----+---------+----------------+
* | Field | Type     | Null | Key | Default | Extra          |
* +-------+----------+------+-----+---------+----------------+
* | id    | int(11)  |      | PRI | NULL    | auto_increment |
* | date  | datetime | YES  |     | NULL    |                |
* | loc   | int(11)  | YES  |     | NULL    |                |
* | files | int(11)  | YES  |     | NULL    |                |
* | dirs  | int(11)  | YES  |     | NULL    |                |
* +-------+----------+------+-----+---------+----------------+
* 5 rows in set (0.00 sec)
*
* Updates the loc, num files, and num dirs for the date. 
*
*******************************************************************************/
function updateLLVMStats($date, $loc, $files, $dirs, $configId) {
    $query = "INSERT INTO llvmStats (date, loc, files, dirs, nightId) VALUES (\"$date\", \"$loc\", \"$files\", \"$dirs\", \"$configId\")";
    mysql_query($query) or die(mysql_error());
}

/*******************************************************************************
*
* Match one substring and return string result.
*
*******************************************************************************/
function matchOne($pattern, $string, $default) {
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
function match($pattern, $string) {
  $subpatterns = array();
  if (isset($string) && preg_match($pattern, $string, $subpatterns)) {
    return $subpatterns;
  }
  
  return array();
}


function shutdown($mysql_link) {
 mysql_close($mysql_link); 
}



/*******************************************************************************
*
* Begin processing data
*
*******************************************************************************/

function acceptTestResults() {
  //print "content-type: text/text\n\n";
  
  // Database connection info
  $database = "llvmTestResults";
  $loginname = "llvm";
  $password = "ll2002vm";
  
  // Connect to database
  $mysql_link = mysql_connect("127.0.0.1", $loginname, $password) or die("Error: could not connect to database!");
  
  mysql_select_db($database) or die("Error: could not find \"$database\" database!");
  
  if ($print_debug) {
    print "Database connected\n";
  }
  
  
// Extract the machine information (FIXME: create regex)
$machine_data = $_POST['machine_data'];

if (!isset($_POST['machine_data'])) {
  shutdown();
}

$MACHINE_DATA = split("\n", $machine_data);
$uname    = matchOne("/uname\:\s*(.+)/",    $MACHINE_DATA[0], "");
$hardware = matchOne("/hardware\:\s*(.+)/", $MACHINE_DATA[1], "");
$os       = matchOne("/os\:\s*(.+)/",       $MACHINE_DATA[2], "");
$hostname = matchOne("/name\:\s*(.+)/",     $MACHINE_DATA[3], "");
$date     = matchOne("/date\:\s*(.+)/",     $MACHINE_DATA[4], "");
$time     = matchOne("/time\:\s*(.+)/",     $MACHINE_DATA[5], "");
$nickname = $_POST['nickname'];

//Get machine id or add new machine
$machineId = getMachineID($targetTriple, $hostname, $nickname);

if(!$machineId) {
  shutdown($mysql_link); 
}

//Extract addition test run information, bail out if missing starttime
if( !isset($_POST['starttime']) ) {
  shutdown($mysql_link);
}
           
$runDateTime = $_POST['starttime'];
$gcc_version = $_POST['gcc_version']; 
$cvsCpuTime = $_POST['cvscheckouttime_cpu'];
$cvsWallTime = $_POST['cvscheckouttime_wall'];
$configureWallTime = $_POST['configtime_wall'];
$configureCpuTime = $_POST['configtime_cpu'];
$buildCpuTime = $_POST['buildtime_cpu'];
$buildWallTime = $_POST['buildtime_wall'];
$dejagnuCpuTime = $_POST['dejagnutime_cpu'];
$dejagnuWallTime = $_POST['dejagnutime_wall'];
$cvsFilesAdded = $_POST['cvsaddedfiles'];
$cvsFilesRemoved = $_POST['cvsremovedfiles'];
$cvsFilesModified = $_POST['cvsmodifiedfiles'];
$cvsUsersAdd = $_POST['cvsusercommitlist'];
$cvsUsersCO = $_POST['cvsuserupdatelist'];
$warnings = $_POST['warnings'];
$warningsAdded = $_POST['warnings_removed'];
$warningsRemoved = $_POST['warnings_added'];
$buildstatus = $_POST['buildstatus'];


// Add new test run config
$testRunConfigId = addTestRunConfig($runDateTime, $machineId, $machineUname, 
                                    $gccVersion, $cvsCpuTime, $cvsWallTime, 
                                    $configureCpuTime, $configureWallTime,
                                    $buildCpuTime, 
                                    $buildWallTime, $dejagnuCpuTime,
                                    $dejagnuWallTime, $warnings, $warningsAdded, 
                                    $warningsRemoved, $cvsUsersAdd, $cvsUsersCO, 
                                    $cvsFilesAdded, $cvsFilesRemoved, $cvsFilesModified,
                                    $buildStatus);


//Print data obtained so far
if ($print_debug) {

  print "Machine ID: $machineId\n";
  print "TargetTriple: $targetTriple\n";
  print "Hostname: $hostname\n";
  print "Nickname: $nickname\n";
 
  print "\n";
  print "Test Run Config ID: $testRunConfigID\n";
  print "Start Time: $runDateTime\n";
  print "Uname: $uname\n";
  print "GCC Version: $gccVersion\n";
  print "Cvs Cpu Time: $cvsCpuTime\n";
  print "Cvs Wall Time: $cvsWallTime\n";
  print "Configure Cpu Time: $configureCpuTime\n";
  print "Configure Wall Time: $configureWallTime\n";
  print "Build Cpu Time: $buildCpuTime\n";
  print "Build Wall Time: $buildWallTime\n";
  print "Dejagnu Cpu Time: $dejagnuCupTime\n";
  print "Dejagnu Wall Time: $dejagnuWallTime\n";
  print "Warnings: $warnings\n";
  print "Warnings Added: $warningsAdded\n";
  print "Warnings Removed: $warningsRemoved\n";
  print "Cvs Files Added: $cvsFilesAdded\n";
  print "Cvs Files Removed: $cvsFilesRemoved\n";
  print "Cvs Files Modified: $cvsFilesModified\n";
  print "Cvs Users Commit: $cvsUsersAdd\n";
  print "Cvs Users Checkout: $cvsUsersCO\n";
  print "Build Status: $buildStatus\n";

}

// Extract dejagnu tests and their results
$dejagnutests_results = $_POST['dejagnutests_results'];
if (!isset($dejagnutests_results)) {
  $dejagnutests_results = "";
}
$DEJAGNUTESTS_RESULTS = split("\n", $dejagnutests_results);

foreach ($DEJAGNUTESTS_RESULTS as $info) {
  $subpatterns = array();
  if (preg_match("/^(XPASS|PASS|XFAIL|FAIL):\s(.[^:]+):?/", $info, $subpatterns)) {
    list($ignore, $result, $program) = $subpatterns;
    if($program) {
      addDejagnuTestResult($program, $result, $testRunConfigId);
      if($print_debug) {
        print "Program: $program\n";
        print "Result: $result\n";
      }
    }
  }
}

// Extract single source, multisource, and external tests into the database
$singlesource_tests = $_POST['singlesource_programstable'];
if (!isset($singlesource_tests)) {
  $singlesource_tests = "";
}
$ss_programs = split("\n", $singlesource_tests);

foreach ($ss_programs as $programLine) {
  addProgramResult($programLine, $testRunConfigId, "SingleSource/"); 
}

$multisource_tests = $_POST['multisource_programstable'];
if (!isset($multisource_tests)) {
  $multisource_tests = "";
}
$ms_programs = split("\n", $multisource_tests);

foreach ($ms_programs as $programLine) {
  addProgramResult($programLine, $testRunConfigId, "MultiSource/"); 
}

$external_tests = $_POST['externalsource_programstable'];
if (!isset($external_tests)) {
  $external_tests = "";
}
$ext_programs = split("\n", $external_tests);

foreach ($ext_programs as $programLine) {
  addProgramResult($programLine, $testRunConfigId, "External/"); 
}

// Extract ofile sizes
$o_file_size = $_POST['o_file_sizes']; 
if (!isset($o_file_size)) {
  $o_file_size = "";
}
$o_file_size = rtrim($o_file_size);
$O_FILE_SIZE = split("\n", $o_file_size);

foreach ($O_FILE_SIZE as $info) {
  list($ignore, $size, $file, $type) = match("/(.+)\s+(.+)\s+(.+)/", $info);
  addFileSize($file, $size, $testRunConfigId, $type);
}

// Extract afile sizes
$a_file_size = $_POST['a_file_sizes']; 
if (!isset($a_file_size)) {
  $a_file_size = "";
}
$a_file_size = rtrim($a_file_size);
$A_FILE_SIZE = split("\n", $a_file_size);

foreach ($A_FILE_SIZE as $info) {
  list($ignore, $size, $file, $type) = match("/(.+)\s+(.+)\s+(.+)/", $info);
  addFileSize($file, $size, $testRunConfigId, $type);
}

// Extract code size
$loc = $_POST['lines_of_code'];
$filesincvs = $_POST['cvs_file_count'];
$dirsincvs = $_POST['cvs_dir_count'];
if(strcmp($buildstatus, "OK") ==  0) {
  
  // only update loc if successful build
  updateLLVMStats($runDateTime, $loc, $filesincvs, $dirsincvs, $testRunConfigId);
}

shutdown($mysql_link);
}

acceptTestResults();
?>