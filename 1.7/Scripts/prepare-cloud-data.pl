#!/usr/bin/perl -w 

use strict;

sub read_arguments;
sub pre_process;

&pre_process();

sub read_arguments {
   my @arguments_list = @{$_[0]};
   my $help_string = $_[1];
   my $argument;
   my $i;

   for( $i = 0; $i < scalar(@arguments_list); $i++ ) {
      $argument = $arguments_list[$i];
      if( $argument =~ m/^\-h/ ) { print $help_string; exit }
      elsif( $argument =~ m/^\-\-help/ ) { print $help_string; exit }
      elsif( $argument =~ m/^\-input/ ) { $_[2] = $arguments_list[$i+1]; $i++; }
      elsif( $argument =~ m/^\-order/ ) { @{$_[3]} = split(":", $arguments_list[$i+1]); $i++; }
      elsif( $argument =~ m/^\-scalar/ ) {
         if( scalar(@arguments_list) >= 6 ) {
            my @elements = split(",", $arguments_list[$i+1]);
            my $key;
            $i++;
            foreach $key (@elements) {
               my @container = split(":", $key);
               if( scalar(@container) == 2 ) {
                  push @{$_[4]}, $container[0] if $container[0] ne "";
                  push @{$_[5]}, $container[1] if $container[1] ne "";
               }
               else {
                  die( "\n\nWrong number of scalar:columns provided" );
               }
            }
         }
         else {
            die( "\n\nWrong number of scalar:columns provided" );
         }
      }
   }
}

sub pre_process {
   my $help_string = "
   This script performs pre-processing (determining x, y, z resolutions, columns switching) on 3d cloud datasets.
 
   To run this script:

   ./prepare-cloud-data.pl -input <cloud ascii file> -order <1st>:<2nd>:<3rd> -scalar <name>:<column no.>,..

   Where:

   <cloud ascii file>  : is the 3d cloud ascii input file. This is in column-based order and usually contains
                         the columns - longitude, latitude, height and attirubutes. Currently, the only attribute
                         currently supplied in the input files is the 'cloudiness' or cloud fraction for that location.

   <1st>:<2nd>:<3rd>   : are colon ':' seperated column arrangements that will be applied to the preprocessed input file.
                         For example, if the 3rd column is the fastest changing value and the 1st column the slowest-changing
                         value, then these columns needs to be switched in order for it to be propoerly loaded and visualized.

   <name>:<column no.> : specifies the scalar values associated to each location. <name> specifies the name of the scalar
                         value and <column no.> specifies on which column this scalar can be found. Multiple scalar variables
                         can be specified via a series of ',' comma separated <name>:<column no.>.
                        
   Example: ./prepare-cloud-data.pl -input cloud_3d.asc -order 3:2:1
   \n";     
   my @counts = (0, 0, 0);
   my @min = (0.0, 0.0, 0.0);
   my @max = (0.0, 0.0, 0.0);
   my @elements = ((), (), ());
   my @seen = ((), (), ());
   my @order = (0, 0, 0);
   my @scalars;
   my @columns;
   my $input_file = "";
   my $flag = 0;
   my $i;

   &read_arguments( \@ARGV, $help_string, $input_file, \@order, \@scalars, \@columns );

   for( $i = 0; $i < scalar(@order); $i++ ) {
      $order[$i]--;
   }

   if( $input_file eq "" ) {
      die( "\n\nNo specified input file provided" ); 
   }
   if( scalar(@scalars) != scalar(@columns) or scalar(@scalars) == 0 or scalar(@columns) == 0 ) {
      die( "\n\nWrong number of scalar:columns provided" );
   }

   for( $i = 0; $i < scalar(@scalars); $i++ ) {
      push @elements, ();
      push @min, 0.0;
      push @max, 0.0;
      push @order, $columns[$i] - 1;
   }
   open( CLOUD_INPUT, $input_file ) or die;
   while( <CLOUD_INPUT> ) {
      my $count;
      my ($line) = $_;
      my @container = (0.0,0.0,0.0);
      
      chomp($line);

      for( $i = 0; $i < scalar(@scalars); $i++ ) {
         push @container, 0.0;
      }
      (@container) = split( ' ', $line );
      for( $i = 0; $i < scalar(@container); $i++ ) {
         push @{$elements[$i]}, $container[$i];
      }
      for( $i = 0; $i < 3 + scalar(@scalars); $i++ ) {
         $counts[$i] += 1 unless $seen[$i]{$container[$i]}++;
      }

      if( $flag == 1 ) {
         for( $count = 0; $count < 3 + scalar(@scalars); $count++ ) {
            if( $min[$count] > $container[$count] ) { 
               $min[$count] = $container[$count];
            }
            elsif( $max[$count] < $container[$count] ) {
               $max[$count] = $container[$count];
            }
         }
      } 
      else {
         for( $i = 0; $i < 3 + scalar(@scalars); $i++ ) {
             $min[$i] = $container[$i];
             $max[$i] = $container[$i];
         }
         $flag = 1;
      }
   }
   print "X (".$min[$order[0]]." to ".$max[$order[0]].") Y (".$min[$order[1]]." to ".$max[$order[1]].") Z (".$min[$order[2]]." to ".$max[$order[2]].")\n";
   for( $i = 0; $i < scalar(@scalars); $i++ ) {
       print( $scalars[$i]." (".$min[$order[$i+3]]," to ".$max[$order[$i+3]].")\n" );
   }
    
   if( scalar(@{$elements[0]}) == scalar(@{$elements[1]}) and scalar(@{$elements[1]}) == scalar(@{$elements[2]}) ) {
      open( CLOUD_OUTPUT, ">$input_file" . ".preprocessed" );
      print( "gridsize ". $counts[$order[0]] ." ". $counts[$order[1]] ." ". scalar($counts[$order[2]] )."\n" );
      print( CLOUD_OUTPUT "gridsize ". $counts[$order[0]] ." ". $counts[$order[1]] ." ". scalar($counts[$order[2]] )."\n" );
      print( CLOUD_OUTPUT "coordinate cartesian\n" ); 

      my $index;
      for( $index = 0; $index < scalar(@{$elements[0]}); $index++ ) {
         print( CLOUD_OUTPUT @{$elements[$order[0]]}[$index]." ".@{$elements[$order[1]]}[$index]." ".@{$elements[$order[2]]}[$index]."\n" );
      }
      close( CLOUD_OUTPUT );

      if( scalar(@scalars) >= 1 ) {
         for( $i = 0; $i < scalar(@scalars); $i++ ) {
            open( SCALAR_OUTPUT, ">$input_file" . ".preprocessed.$scalars[$i]" );
            print( SCALAR_OUTPUT "gridsize ". $counts[$order[0]]." ".$counts[$order[1]]." ".scalar($counts[$order[2]])."\n" );
            print( SCALAR_OUTPUT "scalar $scalars[$i]\n" );
            print( SCALAR_OUTPUT join("\n", @{$elements[$i+3]}) ); 
            close( SCALAR_OUTPUT );
         }
      }
   } 
}
