<?php

$DEBUG=0;

if($DEBUG){
  $mysql_link=mysql_connect("127.0.0.1","llvm","ll2002vm") or die("Could not connect to server $!\n");
  mysql_select_db("nightlytestresults");
}

/*
 * This variable is used to determine file size cutoffs for displaying
 */
$byte_threshold=1000;

/*
 * These variables are used in determining
 * how to color table cells;
 */
$medium_number=0;
$medium_change=5;
$large_number=0;
$large_change=15;
$crazy_number=0;
$crazy_change=30;
$medium_increase="#FFD0D0";
$large_increase="#FF8080";
$crazy_increase="#FF2020";
$medium_decrease="#CCFFCC";
$large_decrease="#90FF90";
$crazy_decrease="#20FF20";

$category_match=array("GCCAS","Bytecode","LLC\<br\>compile",
          "LLC-BETA\<br\>compile","JIT\<br\>codegen","GCC",
          "CBE","LLC","LLC-BETA","JIT","GCC\/\<br\>CBE",
          "GCC\/\<br\>LLC","GCC\/\<br\>LLC-BETA");
$category_array=array("GCCAS","Bytecode","LLC compile","LLC\-BETA compile",
                 "JIT codegen","GCC","CBE","LLC",
                 "LLC\-BETA","JIT","GCC\/\s*CBE", "GCC\/\s*LLC",
                 "GCC\/\s*LLC\-BETA","LLC\/ LLC\-BETA");
$category_print_array=array("GCCAS","Bytecode","LLC compile","LLC-BETA compile",
                 "JIT codegen","GCC","CBE","LLC",
                 "LLC-BETA","JIT","GCC/ CBE", "GCC/ LLC",
                 "GCC/ LLC-BETA","LLC/ LLC-BETA");
                 
$category_print_array_description=array("GCCAS - Time to run LLVM optimizers on the<br>".
                    "program.",
                        
                    "Bytecode - The size of the bytecode for the<br>".
                    "program.",
                        
                    "LLC compile - The time taken to compile with LLC<br>".
                    "(the static backend).",
                    
                    "LLC-BETA compile - The time taken compile with LLC<br>".
                    "using experimental features.",
                        
                    "JIT codegen - The amount of time spent in the JIT<br>".
                    "itself, as opposed to executing the program.",
                    
                    "GCC - The time taken to execute the program when<br>".
                    "compiled with GCC -O2.",
                    
                    "CBE - The time taken to execute the program after<br>".
                    "compilation through the C backend, compiled with<br>".
                    "-O2.",
                        
                    "LLC - The length of time the program generated by<br>".
                    "the static backend LLC takes to execute.",
                    
                    "LLC-BETA - How long the program generated by the<br>".
                    "experimental static back end takes to execute.",
                        
                    "JIT codegen - The amount of time spent in the JIT<br>".
                    "itself, as opposed to executing the program.",
                        
                    "GCC/CBE - The speed-up of the CBE output vs the<br>".
                    "native GCC output: greater than 1 is a speedup,<br>".
                    "less than 1 is a slowdown.",
                    
                    "GCC/LLC - The speed-up of the LLC output vs the<br>".
                    "native GCC output: greater than 1 is a speedup,<br>".
                    "less than 1 is a slowdown.",
                    
                    "GCC/ LLC-BETA - The speed-up of the LLC output<br>".
                    "vs the native GCC output: greater than 1 is a<br>".
                    "speedup, less than 1 is a slowdown.",
                    
                    "LLC/ LLC-BETA - The speed-up of the LLC output<br>".
                    "vs the LLV-BETA output: greater than 1 is a <br>".
                    "speedup, less than 1 is a slowdown.");
                 
$category_print_array_ordered=array("CBE","LLC","JIT","GCCAS",
    "Bytecode","LLC compile","LLC-BETA compile",
                "JIT codegen", "LLC-BETA");
                
$category_print_array_ordered_description=array("CBE - The time taken to execute the program after<br>".
                        "compilation through the C backend, compiled with<br>".
                        "-O2.",
                        
                        "LLC - The length of time the program generated by<br>".
                        "the static backend LLC takes to execute.",
                        
                        "JIT - The amount of time spent running the program<br>".
                        "with the JIT; this includes the code generation<br>".
                        "phase and actually running the program.",
                        
                        "GCCAS - Time to run LLVM optimizers on the<br>".
                        "program.",
                        
                        "Bytecode - The size of the bytecode for the<br>".
                        "program.",
                        
                        "LLC compile - The time taken to compile with LLC<br>".
                        "(the static backend).",
                        
                        "LLC-BETA compile - The time taken compile with LLC<br>".
                        "using experimental features.",
                        
                        "JIT codegen - The amount of time spent in the JIT<br>".
                        "itself, as opposed to executing the program.",
                        
                        "LLC-BETA - How long the program generated by the<br>".
                        "experimental static back end takes to execute.");    


/*
 * Returns an array that contains the name of the program as the 
 * index and an array as the element of the array. The first element
 * in the second array will be either "multisource", "singlesource", or
 * "extenal." 
 *
 */
function GetDayResults($night_id, $array_of_measures ){
  $result=array();
  $query = "SELECT program, type, result FROM program WHERE night=$night_id ORDER BY program ASC";
  $program_query = mysql_query($query) or die (mysql_error());
  while($row = mysql_fetch_assoc($program_query)){
    $program = rtrim($row['program'], ": ");
    $result[$program] = array();
    array_push($result[$program], "{$row['type']}");
    $index=0;
    $data = $row['result'];
    $data = str_replace("<br>", " ", $data);
    foreach ($array_of_measures as $x){
      $value=array();
      $reg_exp="/$x:\s*([[0-9\.]+|\*|\-|n\/a|\?],)/";
      #print "{$program} => running preg_match($reg_exp, $data, $value)<br>\n";
      preg_match($reg_exp, $data, $value);
      if(isset($value[1])){
        array_push($result[$program], $value[1]);
      }
      else{
        array_push($result[$program], "-");  
      }
      $index++;
    }//end foreach
  }//end while
  mysql_free_result($program_query);
  return $result;
}//end function GetDayResults

/*
 * returns an array with the differences of each measurement
 * for each program, It will ignore differences of $min_diff
 *
 */
function CalculateChangeBetweenDays($yesterday_results, $today_results, $min_diff=2){
  $result=array();
  $medium_change=0;
  $large_change=0;
  $crazy_change=0;
  foreach ( array_keys($today_results) as $x){
    $result["$x"]=array();
    array_push($result["$x"], "{$today_results["$x"][0]}");
    for($y=1; $y<sizeof($today_results["$x"])-3; $y++){
      if(isset($yesterday_results["$x"][$y]) && $yesterday_results["$x"][$y]!=0){
        $delta=0;
        #$delta = round($today_results["$x"][$y] - $yesterday_results["$x"][$y],2);
        $delta = $today_results["$x"][$y] - $yesterday_results["$x"][$y];
        if(($delta > $min_diff || $delta < (-1*$min_diff)) &&
            $today_results["$x"][$y]!=0 &&
            $yesterday_results["$x"][$y]!=0){
          $result["$x"][$y-1]=(($today_results["$x"][$y] - $yesterday_results["$x"][$y])/$yesterday_results["$x"][$y])*100;
        }
        else{
          $result["$x"][$y-1]="n/a";
        }
      }
      else{
        $result["$x"][$y-1]="n/a";
      }
      if($result["$x"][$y-1]>=$GLOBALS['medium_change']){
        $GLOBALS['medium_number']++;        
      }
      if($result["$x"][$y-1]>=$GLOBALS['large_change']){
        $GLOBALS['large_number']++;
      }
      if($result["$x"][$y-1]>=$GLOBALS['crazy_change']){
        $GLOBALS['crazy_number']++;
      }
    }//end for        
  }//end foreach
  return $result;
}//end function


function CountSignifigantDifferences($percent_difference_arr, $measure_index, $delta){
  $result=0;
  foreach( array_keys($percent_difference_arr) as $x ){
    if($percent_difference_arr["$x"][$measure_index]>=$delta ||
       $percent_difference_arr["$x"][$measure_index]<=(-1*$delta)){
      $result++;
    }  
  }
  return $result;
}

function DetermineColor($number, $def_color="white"){  
  $result=$def_color;
  if($number>=$GLOBALS['crazy_change']){
    $result=$GLOBALS['crazy_increase'];
  }
  else if($number>=$GLOBALS['large_change']){
    $result=$GLOBALS['large_increase'];
  }
  else if($number>=$GLOBALS['medium_change']){
    $result=$GLOBALS['medium_increase'];
  }
  else if($number<=($GLOBALS['crazy_change']*-1)){
    $result=$GLOBALS['crazy_decrease'];
  }
  else if($number<=($GLOBALS['large_change']*-1)){
    $result=$GLOBALS['large_decrease'];
  }
  else if($number<=($GLOBALS['medium_change']*-1)){
    $result=$GLOBALS['medium_decrease'];
  }
  return $result;
  
}

/*
 * This will return an array which contains:
 * program name, measure, test type, % change, old value, new value 
 * The keys of the array will be the numbers 0 - size of array
 *
 */
function getSignifigantChanges($day, $prev_day, $diff, $measure){
  $result=array();
  foreach(array_keys($diff) as $program){
    for($x=0; $x<sizeof($diff["$program"]); $x++){
      if($diff["$program"][$x]>$GLOBALS['medium_change']){
        array_push($result, array($program, $measure[$x], $day["$program"][0], round($diff["$program"][$x],2), $prev_day["$program"][$x+1], $day["$program"][$x+1]));   
      }
    }//end for
  }//end foreach
  return $result;
}//end function

/*
 * This will return an array which contains:
 * program name, measure, test type, % change from yesterday,
 * old value, new value 
 * The keys of the array will be the numbers 0 - size of array
 *
 */
function getTwoDaySignifigantChanges($day, $prev_day, $diff, $measure){
  $result=array();
  foreach(array_keys($diff) as $program){
    for($x=0; $x<sizeof($diff["$program"]); $x++){
      if(strcmp($diff["$program"][$x],"-")!=0 && 
         ($diff["$program"][$x]>$GLOBALS['medium_change'] ||
          $diff["$program"][$x]<(-1 * $GLOBALS['medium_change']))){
        array_push($result, 
             array($program, 
               $measure[$x], 
             $day["$program"][0], 
             round($diff["$program"][$x],2), 
             "n/a", 
             $prev_day["$program"][$x+1], 
                         $day["$program"][$x+1]));   
      }
    }//end for
  }//end foreach
  return $result;
}//end function

/*
 * This will return an array which contains:
 * program name, measure, test type, % change from yesterday,
 * % change from two days ago , old value, new value 
 * The keys of the array will be the numbers 0 - size of array
 *
 */
function getThreeDaySignifigantChanges($day, $prev_day, $old_day, $diff, $twoday_diff, $measure){
  $result=array();
  foreach(array_keys($diff) as $program){
    for($x=0; $x<sizeof($diff["$program"]); $x++){
      if(strcmp($diff["$program"][$x],"-")!=0 && 
         ($diff["$program"][$x]>$GLOBALS['medium_change'] ||
          $diff["$program"][$x]<(-1 * $GLOBALS['medium_change']))){
        array_push($result, 
             array($program, 
               $measure[$x], 
             $day["$program"][0], 
             round($diff["$program"][$x],2), 
             round($twoday_diff["$program"][$x],2), 
             $prev_day["$program"][$x+1], 
                         $day["$program"][$x+1]));   
      }
    }//end for
  }//end foreach
  return $result;
}//end function

/*
 * Reorders the signifigant changes array by the $index'd element
 * in the 2nd array
 *
 */
function sortSignifigantChangeArray($changes, $index){
  $temp_arr=array();
  foreach (array_keys($changes) as $prog){
    array_push($temp_arr, $changes["$prog"][$index]);
  }
  array_multisort($temp_arr, SORT_DESC, SORT_REGULAR, $changes, SORT_DESC, SORT_REGULAR);
}


/*
 * This function takes in a mysql link, start date, end date, 
 * machine id, an array of programs, and a measurement
 * and will return and array with the dates as 
 * keys and the data for each key
 * being an array containing (date in seconds since epoch, program[0], program[1], ... , 
 * program[n]) for all the data between the two dates
 */
function buildResultsHistory($machine_id, $programs, $measure , $start="2000-01-01 01:01:01", $end="2020-01-01 01:01:01"){
  $preg_measure = str_replace("/","\/", $measure);
  $results_arr=array();
  $query = "SELECT id, added FROM night WHERE machine=$machine_id ". 
           "AND added >= \"$start\" AND added <= \"$end\" ORDER BY added DESC";
  $night_table_query = mysql_query($query ) or die(mysql_error());
  $night_arr=array();
  $night_query="(";
  while($row = mysql_fetch_assoc($night_table_query)){
          $night_arr["{$row['id']}"]=$row['added'];
    $results_arr["{$row['added']}"]=array();
    preg_match("/(\d\d\d\d)\-(\d\d)\-(\d\d)\s(\d\d)\:(\d\d)\:(\d\d)/", "{$row['added']}", $pjs);
                $seconds = mktime($pjs[4], $pjs[5], $pjs[6], $pjs[2], $pjs[3],$pjs[1]);
    array_push($results_arr["{$row['added']}"], $seconds);
    $night_query.=" night={$row['id']} or";
  }
  $night_query.=" night=0 )";
  mysql_free_result($night_table_query);

  $RELEVANT_DATA=0; //will be 0 if all data is null, else will be 1
  $prog_index=1;
  foreach ($programs as $prog){
    $prog=str_replace(" ", "+", $prog);
    $query="SELECT night, result FROM program WHERE program=\"$prog\" ".
           "AND $night_query ORDER BY night ASC";
    $night_table_query=mysql_query($query) or die(mysql_error());
    while($row=mysql_fetch_assoc($night_table_query)){
      $row['result'] = str_replace("<br>", " ", "{$row['result']}");
      $night_id=$row['night'];
      $data="-";
      $regexp = "/$preg_measure:\s*([0-9\.]+|\?)/";
      preg_match($regexp, "{$row['result']}", $ans);
      if(isset($ans[1])){
        $data=$ans[1];
        $RELEVANT_DATA++;
      }//end if isset
      $results_arr["{$night_arr["$night_id"]}"]["$prog_index"]=$data;
    }//end while
    mysql_free_result($night_table_query);
    $prog_index++;
  }//end foreach $programs

  if($RELEVANT_DATA>0){
    return $results_arr;
  }
  else{
    return array();
  }
}


/*
 * Return reason why a llvm test failed.
 */
function getFailReasons($test_result) {
  $result = "";
  $phases = split(", ", $test_result);
  
  for ($i = 0; $i < count($phases); $i++) {
    $phase = $phases[$i];
    if (strpos($phase, "*") !== false) {
      list($tool, $tool_result) = split(": ", $phase);
      if (strcmp($result, "") != 0) {
        $result .= ", ";
      }
      $result .= $tool;
    }
  }
  
  if (strcmp($result, "") != 0) {
    $result = " [" . $result . "]";
  }
  
  return $result;
}


/*
 * Trim test path to exclude redundant info.
 */
function trimTestPath($program) {
  list($head, $tail) = split("/llvm/test/", $program);
  if (isset($tail)) {
    $program = "test/" . $tail;
  }
  return rtrim($program, ": ");;
}
 
 
/*
 * Get failing tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test 
 * in their own table as opposed in the night table.
 */
function getFailures($night_id) {
  $result="";
  if ($night_id >= 684) {
    $query = "SELECT program FROM tests WHERE night=$night_id AND result=\"FAIL\" ORDER BY program ASC";
    $program_query = mysql_query($query) or die (mysql_error());
    while($row = mysql_fetch_assoc($program_query)) {
      $program = trimTestPath($row['program']);
      $result .= $program . "\n";
    }
    mysql_free_result($program_query);

    $query = "SELECT program, result FROM program WHERE night=$night_id ORDER BY program ASC";
    $program_query = mysql_query($query) or die (mysql_error());
    while($row = mysql_fetch_assoc($program_query)) {
      $test_result = $row['result'];
      if (!isTestPass($test_result)) {
        $program = trimTestPath($row['program']);
        $reasons = getFailReasons($test_result);        
        $result .= $program . $reasons . "\n";
      }
    }
    mysql_free_result($program_query);
  }
  return $result;
}

/*
 * Get Unexpected failing tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test 
 * in their own table as opposed in the night table.
 */
function getUnexpectedFailures($night_id){
  $result="";
  if($night_id<684){
    $query = "SELECT unexpfail_tests FROM night WHERE id = $night_id";
    $program_query = mysql_query($query) or die (mysql_error());
    $row = mysql_fetch_assoc($program_query);
    $result= $row['unexpfail_tests'];
    mysql_free_result($program_query);
  }
  else{
    $query = "SELECT program FROM tests WHERE night=$night_id AND result=\"FAIL\"";
    $program_query = mysql_query($query) or die (mysql_error());
    while($row = mysql_fetch_assoc($program_query)){
      $program = trimTestPath($row['program']);
      $result .= $program . "\n";
    }
    mysql_free_result($program_query);
  }
  return $result;
}

/*
 * HTMLify test results
 *
 */
function htmlifyTestResults($result) {
  $result = preg_replace("/\n/", "<br>\n", $result);
  $result = preg_replace("/\[/", "<font color=\"grey\">[", $result);
  $result = preg_replace("/\]/", "]</font>", $result);
  return $result;
 }

/*
 * Get set of tests
 *
 * Returns a hash of tests for a given night.
 */
function getTestSet($id, $table){
  $test_hash = array();
  $query = "SELECT program, result FROM $table WHERE night=$id";
  $program_query = mysql_query($query) or die (mysql_error());
  while ($row = mysql_fetch_assoc($program_query)) {
    $program = trimTestPath($row['program']);
    $test_hash[$program] = $row['result'];
  }
  mysql_free_result($program_query);
  return $test_hash;
}

/*
 * Get list of excluded tests
 *
 * Returns a list of tests for a given night that were excluded from the
 * hash.
 */
function getExcludedTests($id, $table, $test_hash){
  $result = "";
  $query = "SELECT program FROM $table WHERE night=$id ORDER BY program ASC";
  $program_query = mysql_query($query) or die (mysql_error());
  while ($row = mysql_fetch_assoc($program_query)) {
    $program = trimTestPath($row['program']);
    if (!isset($test_hash[$program])) {
      $result .= $program . "\n";
    }
  }
  mysql_free_result($program_query);
  return $result;
}

/*
 * Get New Tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test 
 * in their own table as opposed in the night table.
 */
function getNewTests($cur_id, $prev_id){
  if (strlen($prev_id) === 0 || strlen($cur_id) === 0) {
    return "";
  }
  
  $result="";
  if($cur_id<684){
    $query = "SELECT new_tests FROM night WHERE id = $cur_id";
    $program_query = mysql_query($query) or die (mysql_error());
    $row = mysql_fetch_assoc($program_query);
    $result = $row['new_tests'];
    mysql_free_result($program_query);
  } else {
    $test_hash = getTestSet($prev_id, "tests");
    $result .= getExcludedTests($cur_id, "tests", $test_hash);
    
    $test_hash = getTestSet($prev_id, "program");
    $result .= getExcludedTests($cur_id, "program", $test_hash);
  }
  return $result;
}

/*
 * Get Removed Tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test 
 * in their own table as opposed in the night table.
 */
function getRemovedTests($cur_id, $prev_id){
  if (strlen($prev_id) === 0 || strlen($cur_id) === 0) {
    return "";
  }
  
  $result="";
  if($cur_id<684){
    $query = "SELECT removed_tests FROM night WHERE id = $cur_id";
    $program_query = mysql_query($query) or die (mysql_error());
    $row = mysql_fetch_assoc($program_query);
    $result = $row['removed_tests'];
    mysql_free_result($program_query);
  } else {
    $test_hash = getTestSet($cur_id, "tests");
    $result .= getExcludedTests($prev_id, "tests", $test_hash);
    
    $test_hash = getTestSet($cur_id, "program");
    $result .= getExcludedTests($prev_id, "program", $test_hash);
  }
  return $result;
}

/*
 * Does the test pass
 *
 * Return true if the test result indicates a pass.  For "tests" the possible
 * conditions are "PASS", "FAIL" and "XFAIL" (expected to fail.)
 */
function isTestPass($test_result) {
  return !(strcmp($test_result, "FAIL") == 0 || strpos($test_result, "*") !== false);
}

/*
 * Merge program name and measure
 */
function MergeNameAndMeasureFromRow($row) {
  $program = trimTestPath($row['program']);
  $measure = $row['measure'];
  if (strcmp($measure, "dejagnu") != 0) {
    $program .= " [$measure]";
  }
  return $program;
}

/*
 * Get set of tests that fail
 *
 * Returns a hash of tests that fail for a given night.
 */
function getTestFailSet($id){
  $test_hash = array();
  $query = "SELECT program, result, measure FROM tests WHERE night=$id ORDER BY program ASC, measure ASC";
  $program_query = mysql_query($query) or die (mysql_error());
  while ($row = mysql_fetch_assoc($program_query)) {
    $result = $row['result'];
    if (!isTestPass($result)) {
      $program = MergeNameAndMeasureFromRow($row);
      $test_hash[$program] = $result;
    }
  }
  mysql_free_result($program_query);
  return $test_hash;
}

/*
 * Get list of newly passing tests
 *
 * Returns a list of tests for a given night that were included in the
 * hash and now pass.
 */
function getPassingTests($id, $table, $test_hash){
  $result = "";
  $query = "SELECT program, result, measure FROM tests WHERE night=$id ORDER BY program ASC, measure ASC";
  $program_query = mysql_query($query) or die (mysql_error());
  while ($row = mysql_fetch_assoc($program_query)) {
    $program = MergeNameAndMeasureFromRow($row);
    $result = $row['result'];
    $wasfailing = isset($test_hash[$program]);
    $ispassing = isTestPass($result);
    if ($wasfailing && $ispassing) {
      $result .= $program . "\n";
    }
  }
  mysql_free_result($program_query);
  return $result;
}

/*
 * Get Fixed Tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test 
 * in their own table as opposed in the night table.
 */
function getFixedTests($cur_id, $prev_id){
  if (strlen($prev_id) === 0 || strlen($cur_id) === 0) {
    return "";
  }
  
  $result="";
  if($cur_id<684){
    $query = "SELECT newly_passing_tests FROM night WHERE id = $cur_id";
    $program_query = mysql_query($query) or die (mysql_error());
    $row = mysql_fetch_assoc($program_query);
    $result = $row['newly_passing_tests'];
    mysql_free_result($program_query);
  } else {
    $test_hash = getTestFailSet($prev_id);
    $result .= getPassingTests($cur_id, $test_hash);
  }
  return $result;
}

/*
 * Get Broken Tests
 *
 * This is somewhat of a hack because from night 684 forward we now store the test
 * in their own table as opposed in the night table.
 */
function getBrokenTests($cur_id, $prev_id){
  if (strlen($prev_id) === 0 || strlen($cur_id) === 0) {
    return "";
  }

  $result="";
  if($cur_id<684){
    $query = "SELECT newly_failing_tests FROM night WHERE id = $cur_id";
    $program_query = mysql_query($query) or die (mysql_error());
    $row = mysql_fetch_assoc($program_query);
    $result = $row['newly_failing_tests'];
    mysql_free_result($program_query);
  } else {
    $test_hash = getTestFailSet($cur_id);
    $result .= getPassingTests($prev_id, $test_hash);
  }
  return $result;
}

/*
 * Get previous working night
 *
 * Returns the night id for the machine of the night passed in
 * where build status = OK
 */
function getPreviousWorkingNight($night_id ){
  $query = "SELECT machine FROM night WHERE id=$night_id";
  $program_query = mysql_query($query) or die (mysql_error());
  $row = mysql_fetch_assoc($program_query);
  $this_machine_id=$row['machine'];
  mysql_free_result($program_query);
  
  $query = "SELECT id FROM night WHERE machine=$this_machine_id ".
           "and id<$night_id and buildstatus=\"OK\" order by added desc";
  $program_query = mysql_query($query) or die (mysql_error());
  $row = mysql_fetch_assoc($program_query);
  $prev_id=$row['id'];
  mysql_free_result($program_query);

  return $prev_id;
}

/*
 * Email report.
 *
 */
function getEmailReport($cur_id, $prev_id) {
  $added = getNewTests($cur_id, $prev_id);
  $removed = getRemovedTests($cur_id, $prev_id);
  $passing = getFixedTests($cur_id, $prev_id);
  $failing = getBrokenTests($cur_id, $prev_id);
  
  $email = "";
  if (strcmp($passing, "") == 0) {
    $passing = "None";
  } 
  $email .= "\nNew Test Passes:\n$passing\n";
  if (strcmp($failing, "") == 0) {
    $failing = "None";
  } 
  $email .= "\nNew Test Failures:\n$failing\n";
  if (strcmp($added, "") == 0) {
    $added = "None";
  } 
  $email .= "\nAdded Tests:\n$added\n";
  if (strcmp($removed, "") == 0) {
    $removed = "None";
  } 
  $email .= "\nRemoved Tests:\n$removed\n";
  
  return $email;
}


/*$programs=array("Benchmarks/CoyoteBench/huffbench","Benchmarks/CoyoteBench/lpbench");
$history = buildResultsHistory(18, $programs,"GCCAS" );
foreach (array_keys($history) as $date){
  print "$date => ";
  foreach($history["$date"] as $data){
    print "$data, ";
  }
  print "<br>\n";
}*/

if($DEBUG){
  $today_results = GetDayResults(565, $category_array );
  $yesterday_results = GetDayResults(564, $category_array );
  $oldday_results = GetDayResults(563, $category_array );
  $percent_difference = CalculateChangeBetweenDays($yesterday_results, $today_results, .2);
  $twoday_difference = CalculateChangeBetweenDays($oldday_results, $today_results, .01);
  $count = CountSignifigantDifferences($percent_difference, 1, 25);
  $big_changes = getThreeDaySignifigantChanges($today_results, $yesterday_results, $oldday_results, $percent_difference, $twoday_difference, $category_print_array);
}

/*foreach ( array_keys($big_changes) as $x){
  print "$x => ";
  foreach ($big_changes["$x"] as $y){
    print "$y, ";
  }
  print "<br>\n";
}*/

/*foreach ( array_keys($percent_difference) as $x){
  print "$x => ";
  foreach ($percent_difference["$x"] as $y){
    print "$y, ";
  }
  print "<br>\n";
}*/




if($DEBUG){
  print "<script type=\"text/javascript\" src=\"sorttable.js\"></script>\n";
  print "<table class=\"sortable\" id=\"multisource_tests\" border='1' cellspacing='0' cellpadding='0'>\n";
  print "\t<tr bgcolor=#FFCC99>\n";
  print "\t\t<th>index</th>\n";
  print "\t\t<th>Program</th>\n";
  print "\t\t<th>Measurement</th>\n";
  print "\t\t<th>type</th>\n";
  print "\t\t<th>% Change from yesterday</th>\n";
  print "\t\t<th>% Change from two days ago</th>\n";
  print "\t\t<th>Previous day's test value</th>\n";
  print "\t\t<th>Current day's test value</th>\n";
  print "\t</tr>\n";
  print "\t<tr> <td></td> <td></td> <td></td> <td></td> <td></td> <td></td> <td></td> <td></td> </tr>\n";
  foreach ( array_keys($big_changes) as $x){
    print "\t<tr>";
    print "<td>$x</td>";
    foreach ($big_changes["$x"] as $y){
      print "<td>$y</td>";
    }
    print "</tr>\n";
  }
  print "</table>";
}


?>
