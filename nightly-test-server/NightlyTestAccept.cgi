#!/usr/bin/perl -w
#
# Background: Written on June 1st 2006 by Patrick Jenkins <pjenkins@apple.com> to accept test results 
#             from the llvm NightlyTest.pl script. It is just a CGI script that takes input by the
#             POST method. After this it parses the information and places it into the database.
#
######################################################################################################
use CGI qw(:standard);
use DBI;

######################################################################################################
#
# Important variables
#
######################################################################################################
my $DATABASE="nightlytestresults";
my $LOGINNAME="llvm";
my $PASSWORD="ll2002vm";

######################################################################################################
#
# Connecting to the database
#
######################################################################################################
my $dbh = DBI->connect("DBI:mysql:$DATABASE",$LOGINNAME,$PASSWORD);

######################################################################################################
#
# Some methods to help the process
#
######################################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
# Checks to see if our information about the llvm code base is different
# from the last update and if so, adds the information to the database
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub CheckCodeBase{ #(added, lines of code, files, directories)
    my $d = $dbh->prepare("SELECT * FROM code ORDER BY added DESC");
    $d->execute;
    $row=$d->fetchrow_hashref;
    if(%$row && ($row->{'loc'} != $_[1] ||
		 $row->{'files'} != $_[2] ||
		 $row->{'dirs'} != $_[3])){
	my $e = $dbh->prepare("insert into code (added, loc, files, dirs) ".
			      "values (\"$_[0]\", $_[1], $_[2], $_[3])");
    }
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
#+---------------------+------------+------+-----+---------+----------------+
#| Field               | Type       | Null | Key | Default | Extra          |
#+---------------------+------------+------+-----+---------+----------------+
#| id                  | int(11)    | NO   | PRI | NULL    | auto_increment |
#| machine             | int(11)    | NO   |     |         |                |
#| added               | datetime   | YES  |     | NULL    |                |
#| buildstatus         | tinytext   | YES  |     | NULL    |                |
#| configuretime_cpu   | double     | YES  |     | NULL    |                |
#| configuretime_wall  | double     | YES  |     | NULL    |                |
#| getcvstime_cpu      | double     | YES  |     | NULL    |                |
#| getcvstime_wall     | double     | YES  |     | NULL    |                |
#| buildtime_cpu       | double     | YES  |     | NULL    |                |
#| buildtime_wall      | double     | YES  |     | NULL    |                |
#| dejagnutime_cpu     | double     | YES  |     | NULL    |                |
#| dejagnutime_wall    | double     | YES  |     | NULL    |                |
#| warnings            | mediumtext | YES  |     | NULL    |                |
#| warnings_added      | text       | YES  |     | NULL    |                |
#| warnings_removed    | text       | YES  |     | NULL    |                |
#| teststats_exppass   | int(11)    | YES  |     | NULL    |                |
#| teststats_unexpfail | int(11)    | YES  |     | NULL    |                |
#| teststats_expfail   | int(11)    | YES  |     | NULL    |                |
#| unexpfail_tests     | text       | YES  |     | NULL    |                |
#| newly_passing_tests | text       | YES  |     | NULL    |                |
#| newly_failing_tests | text       | YES  |     | NULL    |                |
#| new_tests           | text       | YES  |     | NULL    |                |
#| removed_tests       | text       | YES  |     | NULL    |                |
#| cvs_added           | text       | YES  |     | NULL    |                |
#| cvs_removed         | text       | YES  |     | NULL    |                |
#| cvs_modified        | text       | YES  |     | NULL    |                |
#| cvs_usersadd        | text       | YES  |     | NULL    |                |
#| cvs_usersco         | text       | YES  |     | NULL    |                |
#+---------------------+------------+------+-----+---------+----------------+
#
#
#CreateNight $machine_id, $db_date, $buildstatus, 
#            $configtime_cpu, $configtime_wall, $cvscheckouttime_cpu,
#            $cvscheckouttime_wall, $buildtime_cpu, $buildtime_wall,
#            $dejagnutime_cpu, $dejagnutime_wall, $warnings, 
#            $warnings_added, $warnings_removed,
#            $dejagnu_exp_passes, $dejagnu_unexp_failures, $dejagnu_exp_failures, #expected pass, unexp fails, exp fails
#            $unexpfail_tests, $newly_passing_tests, $newly_failing_tests,
#            $new_tests, $removed_tests,
#            $cvsaddedfiles, $cvsremovedfiles, $cvsmodifiedfiles,
#            $cvsusercommitlist, $cvsuserupdatelist;
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub CreateNight{

    for($x=0; $x<20; $x++){
	$_[$x]="" unless $_[$x];
    }
		
    my $d = $dbh->prepare("insert into night (machine, added, buildstatus, configuretime_cpu, configuretime_wall,".
			  " getcvstime_cpu, getcvstime_wall, buildtime_cpu,".
			  " buildtime_wall, dejagnutime_cpu, dejagnutime_wall, warnings,".
			  " warnings_added, warnings_removed, teststats_exppass,".
			  " teststats_unexpfail, teststats_expfail, unexpfail_tests,".
			  " newly_passing_tests, newly_failing_tests, new_tests,".
			  " removed_tests, cvs_added, cvs_removed, cvs_modified,".
			  " cvs_usersadd, cvs_usersco) values (\"$_[0]\",\"$_[1]\",\"$_[2]\",\"$_[3]\",\"$_[4]\"".
			  ",\"$_[5]\",\"$_[6]\",\"$_[7]\",\"$_[8]\",\"$_[9]\",\"$_[10]\",\"$_[11]\",\"$_[12]\",\"$_[13]\"".
			  ",\"$_[14]\",\"$_[15]\",\"$_[16]\",\"$_[17]\",\"$_[18]\",\"$_[19]\",\"$_[20]\",\"$_[21]\",\"$_[22]\"".
			  ",\"$_[23]\",\"$_[24]\",\"$_[25]\",\"$_[26]\")");
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
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub AddProgram{ #$program, $result, $type, $night
    my $d = $dbh->prepare("INSERT INTO program (program, result, type, night) VALUES (\"$_[0]\", \"$_[1]\", \"$_[2]\", $_[3])");
    $d->execute;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# This function checks to see if the last entered values in the database
# about code information are the same as our current information. If they
# differ we will put our information into the database.
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub UpdateCodeInfo{ #date, loc, files, dirs

    my $d = $dbh->prepare("select * from code ORDER BY added DESC");
    $d->execute;
    @row = $d->fetchrow_array;
    if(!@row or ($row[1] != $_[1] && $row[2] != $_[2] && $row[3] != $_[3])){
	my $e = $dbh->prepare("insert into code (added, loc, files, dirs) values (\"$_[0]\", $_[1], $_[2], $_[3])");
	$e->execute;
    }

}

######################################################################################################
#
# Setting up variables
#
######################################################################################################
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

my $olden_tests=param('olden_tests');
   $olden_tests="" unless $olden_tests;
my @OLDEN_TESTS = split $spliton, $singlesource_tests;

my $filesincvs = param('filesincvs');
my $dirsincvs = param('dirsindvs');
my $loc = param('loc');
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
my $unexpfail_tests=param('unexpfail_tests');
my $newly_passing_tests=param('newly_passing_tests');
my $newly_failing_tests=param('newly_failing_tests');
my $new_tests=param('new_tests');
my $removed_tests=param('removed_tests');
my $gcc_version = param('gcc_version');            
my $warnings = param('warnings');            
my $lines_of_code = param('lines_of_code');            
my $cvs_dir_count = param('cvs_file_count');            
my $cvs_file_count = param('cvs_dir_count');            


######################################################################################################
#
# Extracting the machine information
#
######################################################################################################
if($MACHINE_DATA[0]){ $MACHINE_DATA[0] =~ m/uname\:\s*(.+)/; $uname = $1; chomp($uname)}
if($MACHINE_DATA[1]){ $MACHINE_DATA[1] =~ m/hardware\:\s*(.+)/; $hardware=$1; chomp($hardware)}
if($MACHINE_DATA[2]){ $MACHINE_DATA[2] =~ m/os\:\s*(.+)/; $os=$1; chomp($os)}
if($MACHINE_DATA[3]){ $MACHINE_DATA[3] =~ m/name\:\s*(.+)/; $name=$1; chomp($name)}
if($MACHINE_DATA[4]){ $MACHINE_DATA[4] =~ m/date\:\s*(.+)/; $date=$1; chomp($date)}
if($MACHINE_DATA[5]){ $MACHINE_DATA[5] =~ m/time\:\s*(.+)/; $time=$1; chomp($time)}

######################################################################################################
#
# Adding lines of code
#
######################################################################################################


######################################################################################################
#
# Extracting the dejagnu test numbers
#
######################################################################################################
print "content-type: text/text\r\n\r\n";

$dejagnutests_log =~ m/\# of expected passes\s*([0-9]+)/;
$dejagnu_exp_passes=$1;
$dejagnutests_log =~ m/\# of unexpected failures\s*([0-9]+)/;
$dejagnu_unexp_failures=$1;
$dejagnutests_log =~ m/\# of expected failures\s*([0-9]+)/;
$dejagnu_exp_failures=$1;

######################################################################################################
#
# Processing Program Test Table Logs
#
######################################################################################################
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

######################################################################################################
#
# creating the response
#
######################################################################################################
print "content-type: text/text\r\n\r\n";

if (!DoesMachineExist $uname,$hardware,$os,$name,$nickname,$gcc_version){
  AddMachine $uname,$hardware,$os,$name,$nickname,$gcc_version,"test";
}
$machine_id = GetMachineId $uname, $hardware, $os, $name, $nickname, $gcc_version;

$db_date = $date." ".$time;
$night_id= CreateNight $machine_id, $db_date, $buildstatus, 
            $configtime_cpu, $configtime_wall, $cvscheckouttime_cpu,
            $cvscheckouttime_wall, $buildtime_cpu, $buildtime_wall,
            $dejagnutime_cpu, $dejagnutime_wall, $warnings, 
            $warnings_added, $warnings_removed,
            $dejagnu_exp_passes, $dejagnu_unexp_failures, $dejagnu_exp_failures, #expected pass, unexp fails, exp fails
            $unexpfail_tests, $newly_passing_tests, $newly_failing_tests,
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

UpdateCodeInfo $db_date, $loc, $filesincvs, $dirsincvs;

print "received $ENV{CONTENT_LENGTH} bytes\n";
         
@nights = GetMachineNights $machine_id;
$length = @nights;
print "DB date : $db_date\n";
print "Machine $machine_id now has ids [@nights]{$length} associated with it in the database\n";


######################################################################################################
#
# writing logs to directory
#
######################################################################################################
$curr=`pwd`;
chomp($curr);

if(! -d "machines"){
    mkdir "$curr/machines", 0777 or print("couldnt create directory $base");
  }
ChangeDir("$curr/machines", "Moving into machines directory");
if(! -d "$machine_id"){
    mkdir "$curr/machines/$machine_id", 0777 or print("couldnt create directory $machine_id because $!");
  }
ChangeDir("$curr/machines/$machine_id", "Moving into machine $machine_id 's directory");

$db_date =~ s/ /\_/g;
my $build_file = "$db_date-Build-Log.txt";
#my $dejagnu_testrun_log_file = "Dejagnu-testrun.log";
#my $dejagnu_testrun_sum_file = "Dejagnu-testrun.sum";
#my $dejagnu_tests_file = "DejagnuTests-Log.txt";
#my $warnings_fie = "Warnings.txt";


WriteFile "$build_file", $build_log;
#WriteFile "$this_days_logs/$dejagnu_testrun_log_file",$dejagnutests_log;
#WriteFile "$this_days_logs/$dejagnu_testrun_sum_file",$dejagnutests_sum;
#WriteFile "$this_days_logs/$warnings_file",$buildwarnings;

######################################################################################################
#
# Sending email to nightly test email archive
#
######################################################################################################

$email = "$machine_data\n\n$dejagnutests_log\n\ncvs user commit list:\n$cvsusercommitlist\n\ncvs user ".
          "update list:\n$cvsuserupdatelist\n\ncvs changed files:\n$cvsmodifiedfiles\n";
$email_addr = "llvm-testresults\@cs.uiuc.edu";
$themail = `mail -s 'X86 nightly tester results' $email_addr < $email`;
