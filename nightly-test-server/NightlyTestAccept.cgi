#!/usr/bin/perl -w
#
# Background: Written on June 1st 2006 by Patrick Jenkins <pjenkins@apple.com>
# to accept test results  from the llvm NightlyTest.pl script. It is just a CGI
# script that takes input by the POST method. After this it parses the 
# information and places it into the database.
#
################################################################################
use CGI qw(:standard);
use DBI;

################################################################################
#
# Important variables
#
################################################################################
my $DATABASE="nightlytestresults";
my $LOGINNAME="llvm";
my $PASSWORD="ll2002vm";

################################################################################
#
# Connecting to the database
#
################################################################################
my $dbh = DBI->connect("DBI:mysql:$DATABASE",$LOGINNAME,$PASSWORD);

################################################################################
#
# Some methods to help the process
#
################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Changes directory to specified directory, the second 
# paramater used to be printed out if verbose was turned on. PJ
# found that annoying so he deleted it.
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub ChangeDir { # directory, logical name
    my ($dir,$name) = @_;
    chomp($dir);
    chdir($dir) || die "Cannot change directory to: $name ($dir) ";
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Opens a file handle to the specified filename, then writes the contents out,
# and finally closes the filehandle.
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub WriteFile {  # (filename, contents)
    open (FILE, ">$_[0]") or die "Could not open file '$_[0]' for writing because $!!";
    print FILE $_[1];
    close FILE;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Queries database to see if there is a machine with the same uname as
# the value passed in
#
# DoesMachineExist $uname,$hardware,$os,$name,$nickname, $gcc_version
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub DoesMachineExist{ #(uname,hardware,os,name,nickname,gcc)
  my($uname,$hardware,$os,$name,$nickname,$gcc_version)=($_[0],$_[1],$_[2],$_[3],$_[4],$_[5]);
  my $d = $dbh->prepare("select * from machine where uname = \"$_[0]\" and nickname=\"$nickname\" and gcc=\"$gcc_version\"");
  $d->execute;

  $row=$d->fetchrow_hashref;
  if(%$row && $row->{'uname'} eq $uname &&
     $row->{'hardware'} eq $hardware &&
     $row->{'os'} eq $os &&
     $row->{'nickname'} eq $nickname &&
     $row->{'gcc'} eq $gcc_version){
    return 1;
  }
  else{
    return 0;
  }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Difference: returns a list of lines that are in the first value but not
# in the second
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub Difference{ 
	$one = $_[0];
	$two = $_[1];
	
	@ONE = split "\n", $one;
	@TWO = split "\n", $two;
	
	$value=1;
	
	my %hash_of_diff=();
	foreach $x (@TWO){
		$hash_of_diff{$x}=$value;
	}
	
	$result="";
	foreach $x (@ONE){
		if($hash_of_diff{$x}!=$value){
			$result.="$x\n";
		}
	}
	
	chomp $result;
	return $result;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# mysql> describe machine;
# +-----------+----------+------+-----+---------+----------------+
# | Field     | Type     | Null | Key | Default | Extra          |
# +-----------+----------+------+-----+---------+----------------+
# | id        | int(11)  |      | PRI | NULL    | auto_increment |
# | uname     | text     |      |     |         |                |
# | hardware  | text     |      |     |         |                |
# | os        | text     |      |     |         |                |
# | name      | text     |      |     |         |                |
# | nickname  | tinytext | YES  |     | NULL    |                |
# | gcc       | text     | YES  |     | NULL    |                |
# | directory | text     | YES  |     | NULL    |                |
# +-----------+----------+------+-----+---------+----------------+
# 8 rows in set (0.00 sec)
#
#
# Creates an entry in the machine table in the database
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub AddMachine{ #(uname, hardware, os, name, nickname, gcc_version,directory)
    #print "insert into machine (uname, hardware, os, name, nickname, directory) values (\"$_[0]\",\"$_[1]\",\"$_[2]\",\"$_[3]\",\"$_[4]\",\"$_[5]\")\n";
    my $d = $dbh->prepare("insert into machine (uname, hardware, os, name, nickname,gcc, directory) values (\"$_[0]\",\"$_[1]\",\"$_[2]\",\"$_[3]\",\"$_[4]\",\"$_[5]\",\"$_[6]\")");
    $d->execute;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Returns the id number of the machine with the passed in uname
#
# $machine_id = GetMachineId $uname. $hardware, $os, $name, $nickname, $gcc_version;
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub GetMachineId{ #uname, hardware, os, name, nickname, gcc
    $_[0] = "" unless $_[0];
    my $d = $dbh->prepare("select * from machine where uname = \"$_[0]\" and hardware=\"$_[1]\" ".
                          "and os=\"$_[2]\" and name=\"$_[3]\" and gcc=\"$_[5]\"");
    
    $d->execute;
    @row = $d->fetchrow_array;
    if(@row){
        return $row[0];
    }
    else{
        return -1;
    }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#Since this is ugly, iv included an example call and a view of the table.
#
# +---------------------+------------+------+-----+---------+----------------+
# | Field               | Type       | Null | Key | Default | Extra          |
# +---------------------+------------+------+-----+---------+----------------+
# | id                  | int(11)    |      | PRI | NULL    | auto_increment |
# | machine             | text       |      |     |         |                |
# | added               | datetime   | YES  |     | NULL    |                |
# | buildstatus         | tinytext   | YES  |     | NULL    |                |
# | getcvstime_cpu      | double     | YES  |     | NULL    |                |
# | getcvstime_wall     | double     | YES  |     | NULL    |                |
# | configuretime_cpu   | double     | YES  |     | NULL    |                |
# | configuretime_wall  | double     | YES  |     | NULL    |                |
# | buildtime_cpu       | double     | YES  |     | NULL    |                |
# | buildtime_wall      | double     | YES  |     | NULL    |                |
# | dejagnutime_cpu     | double     | YES  |     | NULL    |                |
# | dejagnutime_wall    | double     | YES  |     | NULL    |                |
# | warnings            | mediumtext | YES  |     | NULL    |                |
# | warnings_added      | text       | YES  |     | NULL    |                |
# | warnings_removed    | text       | YES  |     | NULL    |                |
# | teststats_exppass   | int(11)    | YES  |     | NULL    |                |
# | teststats_unexpfail | int(11)    | YES  |     | NULL    |                |
# | teststats_expfail   | int(11)    | YES  |     | NULL    |                |
# | all_tests           | text       | YES  |     | NULL    |                |
# | passing_tests       | text       | YES  |     | NULL    |                |
# | unexpfail_tests     | text       | YES  |     | NULL    |                |
# | expfail_tests       | text       | YES  |     | NULL    |                |
# | newly_passing_tests | text       | YES  |     | NULL    |                |
# | newly_failing_tests | text       | YES  |     | NULL    |                |
# | new_tests           | text       | YES  |     | NULL    |                |
# | removed_tests       | text       | YES  |     | NULL    |                |
# | cvs_added           | text       | YES  |     | NULL    |                |
# | cvs_removed         | text       | YES  |     | NULL    |                |
# | cvs_modified        | text       | YES  |     | NULL    |                |
# | cvs_usersadd        | text       | YES  |     | NULL    |                |
# | cvs_usersco         | text       | YES  |     | NULL    |                |
# | a_file_size         | text       | YES  |     | NULL    |                |
# | o_file_size         | text       | YES  |     | NULL    |                |
# +---------------------+------------+------+-----+---------+----------------+
#
#$night_id= CreateNight $machine_id, $db_date, $buildstatus, 
#            $configtime_cpu, $configtime_wall, $cvscheckouttime_cpu,
#            $cvscheckouttime_wall, $buildtime_cpu, $buildtime_wall,
#            $dejagnutime_cpu, $dejagnutime_wall, $warnings, 
#            $warnings_added, $warnings_removed,
#            $dejagnu_exp_passes, $dejagnu_unexp_failures, $dejagnu_exp_failures, #expected pass, unexp fails, exp fails
#            $all_tests, $passing_tests, $unexpfail_tests, 
#            $expfail_tests, $newly_passing_tests, $newly_failing_tests,
#            $new_tests, $removed_tests,
#            $cvsaddedfiles, $cvsremovedfiles, $cvsmodifiedfiles,
#            $cvsusercommitlist, $cvsuserupdatelist, 
#			 $a_file_size, $o_file_size;
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub CreateNight{

    for($x=0; $x<@_; $x++){
		$_[$x]="" unless $_[$x];
    }

	$y=0;
	$machine_id=$_[$y];
	$y++;
	$db_date=$_[$y];
	$y++;
	$buildstatus=$_[$y];
	$y++;
	$configtime_cpu=$_[$y];
	$y++;
	$configtime_wall=$_[$y];
	$y++;
	$cvscheckouttime_cpu=$_[$y];
	$y++;
	$cvscheckouttime_wall=$_[$y];
	$y++;
	$buildtime_cpu=$_[$y];
	$y++;
	$buildtime_wall=$_[$y];
	$y++;
	$dejagnutime_cpu=$_[$y];
	$y++;
	$dejagnutime_wall=$_[$y];
	$y++;
	$warnings=$_[$y];
	$y++;
	$warnings_added=$_[$y];
	$y++;
	$warnings_removed=$_[$y];
	$y++;
	$dejagnu_exp_passes=$_[$y];
	$y++;
	$dejagnu_unexp_failures=$_[$y];
	$y++;
	$dejagnu_exp_failures=$_[$y];
	$y++;
	$all_tests=$_[$y];
	$y++;
	$passing_tests=$_[$y];
	$y++;
	$unexpfail_tests=$_[$y];
	$y++;
	$expfail_tests=$_[$y];
	$y++;
	$newly_passing_tests=$_[$y];
	$y++;
	$newly_failing_tests=$_[$y];
	$y++;
	$new_tests=$_[$y];
	$y++;
	$removed_tests=$_[$y];
	$y++;
	$cvsaddedfiles=$_[$y];
	$y++;
	$cvsremovedfiles=$_[$y];
	$y++;
	$cvsmodifiedfiles=$_[$y];
	$y++;
	$cvsusercommitlist=$_[$y];
	$y++;
	$cvsuserupdatelist=$_[$y];
	$y++;

	

	
    my $d = $dbh->prepare("insert into night (machine, added, buildstatus, configuretime_cpu, configuretime_wall,".
			  " getcvstime_cpu, getcvstime_wall, buildtime_cpu,".
			  " buildtime_wall, dejagnutime_cpu, dejagnutime_wall, warnings,".
			  " warnings_added, warnings_removed, teststats_exppass,".
			  " teststats_unexpfail, teststats_expfail, all_tests,".
              " passing_tests, unexpfail_tests, expfail_tests,".
			  " newly_passing_tests, newly_failing_tests, new_tests,".
			  " removed_tests, cvs_added, cvs_removed, cvs_modified,".
			  " cvs_usersadd, cvs_usersco) values (".
			  "\"$machine_id\", \"$db_date\", \"$buildstatus\",".
			  "\"$configtime_cpu\", \"$configtime_wall\", \"$cvscheckouttime_cpu\",".
			  "\"$cvscheckouttime_wall\", \"$buildtime_cpu\", \"$buildtime_wall\",".
			  "\"$dejagnutime_cpu\", \"$dejagnutime_wall\", \"$warnings\",".
			  "\"$warnings_added\", \"$warnings_removed\",".
			  "\"$dejagnu_exp_passes\", \"$dejagnu_unexp_failures\", \"$dejagnu_exp_failures\",".
			  "\"$all_tests\", \"$passing_tests\", \"$unexpfail_tests\",". 
              "\"$expfail_tests\", \"$newly_passing_tests\", \"$newly_failing_tests\",".
			  "\"$new_tests\", \"$removed_tests\",".
			  "\"$cvsaddedfiles\", \"$cvsremovedfiles\", \"$cvsmodifiedfiles\",".
			  "\"$cvsusercommitlist\", \"$cvsuserupdatelist\")");

    $d->execute;

    my $e = $dbh->prepare("SELECT id FROM night where machine=$_[0] and added=\"$_[1]\"");
    $e->execute;
    @row=$e->fetchrow_array;
    if(@row){
	return $row[0];
    }
    else{
	return -1;
    }
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub GetMachineNights{ #machine_id
    my @result;
    my $d = $dbh->prepare("select * from night where machine = \"$_[0]\"");
    $d->execute;
    while (@row = $d->fetchrow_array){
	push(@result, $row[0]);
    }
    $result[0]="" unless $result[0];
    return @result;

}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# mysql> describe program;
# +---------+---------+------+-----+---------+-------+
# | Field   | Type    | Null | Key | Default | Extra |
# +---------+---------+------+-----+---------+-------+
# | program | text    |      |     |         |       |
# | result  | text    | YES  |     | NULL    |       |
# | type    | text    | YES  |     | NULL    |       |
# | night   | int(11) |      |     | 0       |       |
# +---------+---------+------+-----+---------+-------+
# 4 rows in set (0.00 sec)
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub AddProgram{ #$program, $result, $type, $night
		$query = "INSERT INTO program (program, result, type, night) VALUES".
						 " (\"$_[0]\", \"$_[1]\", \"$_[2]\", $_[3])";
    my $d = $dbh->prepare($query);
    $d->execute;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# mysql> describe file;
# +-------+---------+------+-----+---------+-------+
# | Field | Type    | Null | Key | Default | Extra |
# +-------+---------+------+-----+---------+-------+
# | file  | text    |      |     |         |       |
# | size  | int(11) |      |     | 0       |       |
# | night | int(11) |      |     | 0       |       |
# | type  | text    | YES  |     | NULL    |       |
# +-------+---------+------+-----+---------+-------+
# 4 rows in set (0.00 sec)
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub AddFile{ #$file, $size, $night, $type
		$query = "INSERT INTO file (file, size, night, type) VALUES (\"$_[0]\", ".
						 "\"$_[1]\", \"$_[2]\", \"$_[3]\")";
    my $d = $dbh->prepare($query);
    $d->execute;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# mysql> describe code;
# +-------+----------+------+-----+---------------------+-------+
# | Field | Type     | Null | Key | Default             | Extra |
# +-------+----------+------+-----+---------------------+-------+
# | added | datetime |      |     | 0000-00-00 00:00:00 |       |
# | loc   | int(11)  |      |     | 0                   |       |
# | files | int(11)  |      |     | 0                   |       |
# | dirs  | int(11)  |      |     | 0                   |       |
# +-------+----------+------+-----+---------------------+-------+
# 4 rows in set (0.00 sec)
#
# This function checks to see if the last entered values in the database
# about code information are the same as our current information. If they
# differ we will put our information into the database.
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub UpdateCodeInfo{ #date, loc, files, dirs
    my $d = $dbh->prepare("SELECT * FROM code ORDER BY added DESC");
    $d->execute;
    $row=$d->fetchrow_hashref;
    if(%$row && ($row->{'loc'} != $_[1] ||
		 $row->{'files'} != $_[2] ||
		 $row->{'dirs'} != $_[3])){
	my $e = $dbh->prepare("insert into code (added, loc, files, dirs) values (\"$_[0]\", \"$_[1]\", \"$_[2]\", \"$_[3]\")");
    $e->execute;
    }
}

################################################################################
#
# Setting up variables
#
################################################################################
my $spliton ="\n";

my $machine_data=param('machine_data');
   $machine_data="" unless $machine_data;
my @MACHINE_DATA = split $spliton, $machine_data;

my $cvs_log=param('cvs_data');
   $cvs_log="" unless $cvs_log;
my @CVS_LOG = split $spliton, $cvs_log;

my $build_log=param('build_data');
   $build_log="" unless $build_log;
my @BUILD_LOG = split $spliton, $build_log;

my $dejagnutests_log=param('dejagnutests_log');
   $dejagnutests_log="" unless $dejagnutests_log;
my @DEJAGNUTESTS_LOG = split $spliton, $dejagnutests_log;


my $dejagnutests_sum=param('dejagnutests_sum');
   $dejagnutests_sum="" unless $dejagnutests_sum;
my @DEJAGNUTESTS_SUM = split $spliton, $dejagnutests_sum;

my $singlesource_tests = param('singlesource_programstable');
$singlesource_tests="" unless $singlesource_tests;
my @SINGLESOURCE_TESTS =split $spliton, $singlesource_tests;

my $multisource_tests = param('multisource_programstable');
$multisource_tests = "" unless $multisource_tests;
my @MULTISOURCE_TESTS = split $spliton, $multisource_tests;

my $external_tests = param('externalsource_programstable');
$external_tests = "" unless $external_tests;
my @EXTERNAL_TESTS = split $spliton, $external_tests;

my $o_file_size = param('o_file_sizes'); 
	 $o_file_size="" unless $o_file_size;
	 chomp($o_file_size);
my @O_FILE_SIZE = split $spliton, $o_file_size;
my $a_file_size = param('a_file_sizes'); 
   $a_file_size="" unless $a_file_size;
   chomp($a_file_size);
my @A_FILE_SIZE = split $spliton, $a_file_size;

my $filesincvs = param('cvs_file_count');
my $dirsincvs = param('cvs_dir_count');
my $loc = param('lines_of_code');
my $nickname = param('nickname');
my $cvscheckouttime_cpu=param('cvscheckouttime_cpu');
my $cvscheckouttime_wall=param('cvscheckouttime_wall');
my $configtime_wall=param('configtime_wall');
my $configtime_cpu=param('configtime_cpu');
my $buildtime_cpu=param('buildtime_cpu');
my $buildtime_wall=param('buildtime_wall');
my $dejagnutime_cpu=param('dejagnutime_cpu');
my $dejagnutime_wall=param('dejagnutime_wall');
my $buildwarnings=param('warnings');
my $cvsaddedfiles=param('cvsaddedfiles');
my $cvsremovedfiles=param('cvsremovedfiles');
my $cvsmodifiedfiles=param('cvsmodifiedfiles');
my $cvsusercommitlist=param('cvsusercommitlist');
my $cvsuserupdatelist=param('cvsuserupdatelist');
my $buildstatus=param('buildstatus');
my $warnings_added=param('warnings_removed');
my $warnings_removed=param('warnings_added');
my $all_tests=param('all_tests');
my $unexpfail_tests=param('unexpfail_tests');
my $passing_tests = param('passing_tests');
my $expfail_tests = param('expfail_tests');
my $newly_passing_tests=param('newly_passing_tests');
my $newly_failing_tests=param('newly_failing_tests');
my $new_tests=param('new_tests');
my $removed_tests=param('removed_tests');
my $gcc_version = param('gcc_version');            
my $warnings = param('warnings');            
my $lines_of_code = param('lines_of_code');

################################################################################
#
# Extracting the machine information
#
################################################################################
if($MACHINE_DATA[0]){ $MACHINE_DATA[0] =~ m/uname\:\s*(.+)/; $uname = $1; chomp($uname)}
if($MACHINE_DATA[1]){ $MACHINE_DATA[1] =~ m/hardware\:\s*(.+)/; $hardware=$1; chomp($hardware)}
if($MACHINE_DATA[2]){ $MACHINE_DATA[2] =~ m/os\:\s*(.+)/; $os=$1; chomp($os)}
if($MACHINE_DATA[3]){ $MACHINE_DATA[3] =~ m/name\:\s*(.+)/; $name=$1; chomp($name)}
if($MACHINE_DATA[4]){ $MACHINE_DATA[4] =~ m/date\:\s*(.+)/; $date=$1; chomp($date)}
if($MACHINE_DATA[5]){ $MACHINE_DATA[5] =~ m/time\:\s*(.+)/; $time=$1; chomp($time)}

################################################################################
#
# Extracting the dejagnu test numbers
#
################################################################################
print "content-type: text/text\r\n\r\n";

my $dejagnu_exp_passes=0;
if( ($dejagnutests_log =~ m/\# of expected passes\s*([0-9]+)/) ){
  $dejagnu_exp_passes=$1;
}

my $dejagnu_unexp_failures=0;
if( ($dejagnutests_log =~ m/unexpected failures\s*([0-9]+)/) ){
  $dejagnu_unexp_failures=$1;
}

my $dejagnu_exp_failures=0;
if( ($dejagnutests_log =~ m/\# of expected failures\s*([0-9]+)/) ){
  $dejagnu_exp_failures=$1;
}

################################################################################
#
# Processing Program Test Table Logs
#
################################################################################
$linebreak="OA<br>";
$seperator=",";

$SINGLESOURCE_TESTS[0] =~ s/$linebreak/ /g;
@singlesource_headings = split $seperator,$SINGLESOURCE_TESTS[0];
my %singlesource_processed;
for($x=1; $x<@SINGLESOURCE_TESTS; $x++){
    my @temp_outcome=split $seperator,$SINGLESOURCE_TESTS[$x];
    my $outcome = "";
    for($y=1; $y<@singlesource_headings; $y++){
	$outcome.="$singlesource_headings[$y]: $temp_outcome[$y], ";
    }
    $singlesource_processed{$temp_outcome[0]}=$outcome;   
}

$MULTISOURCE_TESTS[0] =~ s/$linebreak/ /g; 
@multisource_headings = split $seperator,$MULTISOURCE_TESTS[0];
my %multisource_processed;
for($x=1; $x<@MULTISOURCE_TESTS; $x++){
    my @temp_outcome=split $seperator,$MULTISOURCE_TESTS[$x];
    my $outcome = "";
    for($y=1; $y<@multisource_headings; $y++){
	$outcome.="$multisource_headings[$y]: $temp_outcome[$y], ";
    }
    $multisource_processed{$temp_outcome[0]}=$outcome;
}

$EXTERNAL_TESTS[0] =~ s/$linebreak/ /g; 
@external_headings = split $seperator,$EXTERNAL_TESTS[0];
my %external_processed;
for($x=1; $x<@EXTERNAL_TESTS; $x++){
    my @temp_outcome=split $seperator,$EXTERNAL_TESTS[$x];
    my $outcome = "";
    for($y=1; $y<@external_headings; $y++){
	$outcome.="$external_headings[$y]: $temp_outcome[$y], ";
    }
    $external_processed{$temp_outcome[0]}=$outcome;
}

################################################################################
#
# creating the response
#
################################################################################
print "content-type: text/text\r\n\r\n";

if (!DoesMachineExist $uname,$hardware,$os,$name,$nickname,$gcc_version){
  AddMachine $uname,$hardware,$os,$name,$nickname,$gcc_version,"test";
}
$machine_id = GetMachineId $uname, $hardware, $os, $name, $nickname, $gcc_version;

################################################################################
#
# Creating test lists
# 		All of these if-else statements are to ensure that if the previous
#			night's test had a build failure and reported all tests as not passing,
#			all failing, etc, etc then we dont report all the tests as newly passing
#     or their equivalent.
#
################################################################################
my $d = $dbh->prepare("select * from night where machine = $machine_id ORDER BY added DESC");
$d->execute;
my $row=$d->fetchrow_hashref;
$yesterdays_tests = $row->{'all_tests'};
$yesterdays_passes = $row->{'passing_tests'};
$yesterdays_fails = $row->{'unexpfail_tests'};
$yesterdays_xfails = $row->{'expfail_tests'};
if($yesterdays_passes ne ""){
	$newly_passing_tests = Difference $passing_tests, $yesterdays_passes;
}
else{ $newly_passing_tests=""; }
if($yesterdays_xfails ne "" and $yesterdays_fails ne ""){
	$newly_failing_tests = Difference $expfail_tests."\n".$unexpfail_tests,
								  $yesterdays_xfails."\n".$yesterdays_fails;
}
else{ $newly_failing_tests=""; }
if($yesterdays_tests ne ""){
	$new_tests = Difference $all_tests, $yesterdays_tests;
}
else{ $new_tests=""; }
if($all_tests ne ""){
	$removed_tests = Difference $yesterdays_tests, $all_tests;
}
else{ $removed_tests=""; }

################################################################################
#
# Submitting information to database
#
################################################################################
#$db_date = $date." ".$time;
$db_date = `date "+20%y-%m-%d %H:%M:%S"`;
chomp($db_date);
$night_id= CreateNight $machine_id, $db_date, $buildstatus, 
            $configtime_cpu, $configtime_wall, $cvscheckouttime_cpu,
            $cvscheckouttime_wall, $buildtime_cpu, $buildtime_wall,
            $dejagnutime_cpu, $dejagnutime_wall, $warnings, 
            $warnings_added, $warnings_removed,
            $dejagnu_exp_passes, $dejagnu_unexp_failures, $dejagnu_exp_failures, #expected pass, unexp fails, exp fails
            $all_tests, $passing_tests, $unexpfail_tests, 
            $expfail_tests, $newly_passing_tests, $newly_failing_tests,
            $new_tests, $removed_tests,
            $cvsaddedfiles, $cvsremovedfiles, $cvsmodifiedfiles,
            $cvsusercommitlist, $cvsuserupdatelist;

foreach $x(keys %singlesource_processed){
    AddProgram $x, $singlesource_processed{$x}, "singlesource", $night_id; 
}

foreach $x(keys %multisource_processed){
    AddProgram $x, $multisource_processed{$x}, "multisource", $night_id; 
}

foreach $x(keys %external_processed){
    AddProgram $x, $external_processed{$x}, "external", $night_id; 
}

$len=@O_FILE_SIZE;
if($len>1){
	foreach $x (@O_FILE_SIZE){
		$x =~ m/(.+)\s+(.+)\s+(.+)/;
		AddFile $2, $1, $night_id, $3;
	}
}

$len=@A_FILE_SIZE;
if($len>1){
	foreach $x (@A_FILE_SIZE){
		$x =~ m/(.+)\s+(.+)\s+(.+)/;
		AddFile $2, $1, $night_id, $3;
	}
}

################################################################################
#
# Adding lines of code to the database
#
################################################################################
UpdateCodeInfo $db_date, $loc, $filesincvs, $dirsincvs;

print "received $ENV{CONTENT_LENGTH} bytes\n";
         
@nights = GetMachineNights $machine_id;
$length = @nights;
print "DB date : $db_date\n";
print "Machine $machine_id now has ids [@nights]{$length} associated with it in the database\n";

################################################################################
#
# building hash of arrays of signifigant changes to place into the nightly
# test results email. This is ugly code and is somewhat of a hack. However, it
# adds very useful information to the nightly test email.
#
################################################################################
$query = "select id from night where id<$night_id and machine=$machine_id order by id desc";
my $g = $dbh->prepare($query);
$g->execute;
$row = $g->fetchrow_hashref;
$prev_night=$row->{'id'};

$query = "select * from program where night=$night_id";
my $e = $dbh->prepare($query);
$e->execute;
%prog_hash_new=();
while ($row=$e->fetchrow_hashref){
    $prog_hash_new{"$row->{'program'}"}=$row->{'result'};
}

$query = "select * from program where night=$prev_night";
my $f = $dbh->prepare($query);
$f->execute;
%prog_hash_old=();
while ($row=$f->fetchrow_hashref){
    $prog_hash_old{"$row->{'program'}"}=$row->{'result'};
}

%output_big_changes=();
foreach my $prog(keys(%prog_hash_new)){
    #get data from server
    my @vals_new = split(",", $prog_hash_new{"$prog"});
    my @vals_old = split(",", $prog_hash_old{'$prog'});

    #get list of values measured from newest test
    my @measures={};
    foreach my $x(@vals_new){
        $x =~ s/\,//g;
        $x =~ s/\.//g;
        $x =~ s/\://g;
        $x =~ s/\d//g;
        $x =~ s/\-//g;
        $x =~ s/ //g;
        if($x !~ m/\//){
            push @measures, $x;
        }
    }

    #put measures into hash of arrays
    foreach my $x(@measures){
        $prog_hash_new{"$prog"} =~ m/$x:.*?(\d*\.*\d+)/;
        my $value_new = $1;
        $value_old=0;
	if(exists $prog_hash_old{"$prog"}){
            $prog_hash_old{"$prog"} =~ m/$x:.*?(\d*\.*\d+)/;
            $value_old = $1;
        }
        my $diff = ($value_old - $value_new);
        my $perc=0;
        if( $value_old!=0 && ($diff > .2 || $diff < -.2) ){
            $perc=($diff/$value_old) * 100;
        }
        if($perc > 5 || $perc < -5){
            if( ! exists $output_big_changes{"$x"} ){
                my $rounded_perc = sprintf("%1.2f", $perc);
                $output_big_changes{"$x"}[0]="$prog ($x) changed \%$rounded_perc ($value_old => $value_new)\n";
            }
            else{
                my $rounded_perc = sprintf("%1.2f", $perc);
                push(@{ $output_big_changes{"$x"} },"$prog ($x) changed \%$rounded_perc ($value_old => $value_new)\n");
            } #end else
        }# end if $perc is acceptable
    }# end foreach measure taken
} #end for each program we have measurements for

################################################################################
#
# Sending email to nightly test email archive
#
################################################################################
$link_to_page="http://llvm.org/nightlytest/test.php?machine=$machine_id&night=$night_id";
$email  = "$link_to_page\n";
$email .= "Name: $name\n";
$email .= "Nickname: $nickname\n";
$email .= "Buildstatus: $buildstatus\n";

if($buildstatus eq "OK") {
	if ($newly_passing_tests ne "") {
		$newly_passing_tests = "\n$newly_passing_tests\n";
	} else {
		$newly_passing_tests = "None";
	}
	$email .= "\nNew Test Passes: $newly_passing_tests";
	if ($newly_failing_tests ne "") {
		$newly_failing_tests = "\n$newly_failing_tests\n";
	} else {
		$newly_failing_tests = "None";
	}
	$email .= "\nNew Test Failures: $newly_failing_tests";
	if ($new_tests ne "") {
		$new_tests = "\n$new_tests\n";
	} else {
		$new_tests = "None";
	}
	$email .= "\nAdded Tests: $new_tests";
	if ($removed_tests ne "") {
		$removed_tests = "\n$removed_tests\n";
	} else {
		$removed_tests = "None";
	}
	$email .= "\nRemoved Tests: $removed_tests\n";

	$email .= "\nSignificant changes in test results:\n";
	foreach my $meas(keys(%output_big_changes)){
	    $email.= "$meas:\n";
	    foreach my $x(@{ $output_big_changes{$meas} } ){
		$email.= "--- $x";
	    }
	}
}
else{
	$temp_date = $db_date;
	$temp_date =~s/ /\_/g;
	$email .= "\nBuildlog available at http://llvm.org/nightlytest/".
	          "machines/$machine_id/$temp_date-Build-Log.txt";
}

$email_addr = "llvm-testresults\@cs.uiuc.edu";
`echo "$email" | mail -s '$nickname $hardware nightly tester results' $email_addr`;

################################################################################
#
# writing logs to directory
#
################################################################################
$curr=`pwd`;
chomp($curr);

if(! -d "machines"){
    `mkdir machines -m 777`;
    #mkdir "$curr/machines", 0777 or print("couldnt create directory $base");
}
ChangeDir("$curr/machines", "Moving into machines directory");
if(! -d "$machine_id"){
    `mkdir $machine_id -m 777`;
    #mkdir "$curr/machines/$machine_id", 777 or print("couldnt create directory $machine_id because $!");
}
ChangeDir("$curr/machines/$machine_id", "Moving into machine $machine_id 's directory");

$db_date =~ s/ /\_/g;
my $build_file = "$db_date-Build-Log.txt";
my $o_file= "$db_date-O-files.txt";
my $a_file= "$db_date-A-files.txt";
#my $dejagnu_testrun_log_file = "Dejagnu-testrun.log";
#my $dejagnu_testrun_sum_file = "Dejagnu-testrun.sum";
#my $dejagnu_tests_file = "DejagnuTests-Log.txt";
#my $warnings_fie = "Warnings.txt";


WriteFile "$build_file", $build_log;
WriteFile "$o_file", $o_file_size;
WriteFile "$a_file", $a_file_size;
#WriteFile "$this_days_logs/$dejagnu_testrun_log_file",$dejagnutests_log;
#WriteFile "$this_days_logs/$dejagnu_testrun_sum_file",$dejagnutests_sum;
#WriteFile "$this_days_logs/$warnings_file",$buildwarnings;
