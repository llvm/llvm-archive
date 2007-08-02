#!/usr/bin/perl -w
#
# This script randomly selects a file from the Content directory to display 
# on the web page.

# Read the contenst of a file.
sub ReadFile {
  undef $/;
  if (open (FILE, $_[0])) {
    my $Ret = <FILE>;
    close FILE;
    return $Ret;
  } else {
    print "Could not open file '$_[0]' for reading!";
    return "";
  }
}

# Print an HTTP header to ensure anything we print is interpeted as HTML
print "Content-type: text/html\n\n";

# Open the directory and get the file names that
# contain at least one digit
$Dir = "/home/vadve/shared/llvm-wwwroot/hlvm/random";
opendir DH, $Dir  or die "Where did the random content go?";
@Files = grep /^[0-9]/, readdir DH;
closedir DH;

# Seed the random number generator
srand(time ^ $$);

# Select a file randomly and print it.
if (defined(@Files)) {
  $File = $Files[rand(@Files)];
  $File =~ /^[0-9]/; 
  print ReadFile "$Dir/$File" ;
  print "\n";
} else {
  $pwd = `pwd`;
  print "On a clear disk you can seek forever. $pwd";
}

# All done.
exit(0);
