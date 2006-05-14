#!/usr/bin/perl

sub get_hlvm_dir 
{
  chomp(my $cwd=`pwd`);
  my $hlvmdir = $cwd;
  $hlvmdir =~ s|(.*hlvm).*|$1|;
  if (-d"$hlvmdir/AST") {
    $hlvmdir =~ s|(.*)/hlvm|$1|;
  }
  return $hlvmdir;
}

sub process_file
{
  my $preamble = shift(@_);
  my $input  = shift(@_);
  my $output = shift(@_);
  my $line   = "";
  my $hlvmdir = get_hlvm_dir();

  chomp($MODULE_PATH = `pwd`);
  $MODULE_PATH =~ s|$hlvmdir\/hlvm\/(.*)|$1|;
  $MODULE = $MODULE_PATH;
  $MODULE =~ s|\/|_|g;

  ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);

  local $YEAR = $year + 1900;
  local $DATE = sprintf("%4d/%02d/%02d", $YEAR, $mon + 1, $mday );
  local $TIME = sprintf("%02d:%02d:%02d", $hour, $min, $sec);

  local $AUTHOR = $ENV{XPS_AUTHOR};
  if ( length($AUTHOR) < 5 )
  {
      $AUTHOR = $ENV{AUTHOR};
      if (length($AUTHOR) < 5)
      {
          $AUTHOR = getpwuid($<);
          if (length($AUTHOR) < 1 )
          {
              $AUTHOR = $ENV{USER};
              if (length($AUTHOR) < 1)
              {
                  $AUTHOR = $ENV{LOGNAME};
                  if (length($AUTHOR) < 1)
                  {
                      $AUTHOR = "Author Unknown";
                  }
              }
          }
      }
  }

  local $NAMESPACE 		= "HLVM_$MODULE";
  local $module_header 	= ucfirst($MODULE);
  local $MODULE_INCLUDE = "<hlvm/$MODULE/${module_header}.h>";
  local $CLASS_INCLUDE	= "<hlvm/$MODULE/${CLASS}.h>";
  local $NAMESPACE_UC 	= uc( $NAMESPACE );
  local $CLASS_UC 	= uc( $CLASS );
  local $HEADER_UC   	= uc( $HEADER );

  open ( OUT,"> $output" ) || die ("Couldn't open $output for writing\n");

  for $infile ( $preamble , $input )
  {
      open ( IN, "< $infile" ) || die ("Couldn't open $infile for reading\n");

      while ( defined($line = <IN>) )
      {
          $line =~ s/\%ID\%/\$Id\$/g;
          $line =~ s/\%LOG\%/\$Log\$/g;
          $line =~ s/\%AUTHOR%/$AUTHOR/g;
          $line =~ s/\%USER%/$USER/g;
          $line =~ s/\%DATE%/$DATE/g;
          $line =~ s/\%TIME%/$TIME/g;
          $line =~ s/\%YEAR%/$YEAR/g;
          $line =~ s/\%MODULE%/$MODULE/g;
          $line =~ s/\%NAMESPACE\%/$NAMESPACE/g;
          $line =~ s/\%NAMESPACE_UC\%/$NAMESPACE_UC/g;
          $line =~ s/\%CLASS\%/$CLASS/g;
          $line =~ s/\%CLASS_UC\%/$CLASS_UC/g;
          $line =~ s/\%MODULE_PATH\%/$MODULE_PATH/g;
          $line =~ s/\%MODULE\%/$MODULE/g;
          $line =~ s/\%HEADER\%/$HEADER/g;
          $line =~ s/\%HEADER_UC\%/$HEADER_UC/g;
          $line =~ s/\%STYLE\%/$STYLE/g;
          $line =~ s/\%CLASS_INCLUDE\%/$CLASS_INCLUDE/g;
          $line =~ s/\%MODULE_INCLUDE\%/$MODULE_INCLUDE/g;
          $line =~ s/\%TOKEN_LIST\%/$TOKEN_LIST/g;
          $line =~ s/\%SCHEMA_NAME\%/$SCHEMA_NAME/g;

          print OUT $line || die ("Couldn't write to OUT file\n");
      }
      close IN;
  };

  close OUT;
}

1;
