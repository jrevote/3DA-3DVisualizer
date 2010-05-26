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
   }
}

sub pre_process {
   my $help_string = "
   This script performs pre-processing (determining x, y, z resolutions, columns switching) on 3d cloud datasets.
 
   To run this script:

   ./prepare-cloud-data.pl -input <cloud ascii file> -order <1st>:<2nd>:<3rd>

   Where:

   <cloud ascii file> : is the 3d cloud ascii input file. This is in column-based order and usually contains
                        the columns - longitude, latitude, height and attirubutes. Currently, the only attribute
                        currently supplied in the input files is the 'cloudiness' or cloud fraction for that location.

   <1st>:<2nd>:<3rd>  : are colon ':' seperated column arrangements that will be applied to the preprocessed input file.
                        For example, if the 3rd column is the fastest changing value and the 1st column the slowest-changing
                        value, then these columns needs to be switched in order for it to be propoerly loaded and visualized.
                        
   Example: ./prepare-cloud-data.pl -input cloud_3d.asc -order 3:2:1
   \n";     
   my @counts = (0, 0, 0);
   my @min = (0.0, 0.0, 0.0);
   my @max = (0.0, 0.0, 0.0);
   my @elements = ((), (), (), ());
   my @seen = ((), (), ());
   my @order = (0, 0, 0);
   my $input_file = "";
   my $flag = 0;
   my $i;

   &read_arguments( \@ARGV, $help_string, $input_file, \@order );

   for( $i = 0; $i < scalar(@order); $i++ ) {
      $order[$i]--;
   }

   if( $input_file eq "" ) {
      die( "\n\nNo specified input file provided" ); 
   }

   open( CLOUD_INPUT, $input_file ) or die;
   while( <CLOUD_INPUT> ) {
      my $count;
      my ($line) = $_;
      my @container = {0.0,0.0,0.0,0.0};
      
      chomp($line);
      ($container[0], $container[1], $container[2], $container[3]) = split( ' ', $line );
      push @{$elements[0]}, $container[0];
      push @{$elements[1]}, $container[1];
      push @{$elements[2]}, $container[2];
      push @{$elements[3]}, $container[3];
      $counts[0] += 1 unless $seen[0]{$container[0]}++;
      $counts[1] += 1 unless $seen[1]{$container[1]}++;
      $counts[2] += 1 unless $seen[2]{$container[2]}++;

      if( $flag == 1 ) {
         for( $count = 0; $count < 3; $count++ ) {
            if( $min[$count] > $container[$count] ) { 
               $min[$count] = $container[$count];
            }
            elsif( $max[$count] < $container[$count] ) {
               $max[$count] = $container[$count];
            }
         }
      } 
      else {
         ($min[0], $min[1], $min[2]) = ($container[0], $container[1], $container[2]);
         ($max[0], $max[1], $max[2]) = ($container[0], $container[1], $container[2]);
         $flag = 1;
      }
   }
   print "X (".$min[$order[0]]." to ".$max[$order[0]].") Y (".$min[$order[1]]." to ".$max[$order[1]].") Z (".$min[$order[2]]." to ".$max[$order[2]].")\n";
    
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

      open( SCALAR_OUTPUT, ">$input_file" . ".preprocessed.fraction" );
      print( SCALAR_OUTPUT "gridsize ". $counts[$order[0]]." ".$counts[$order[1]]." ".scalar($counts[$order[2]])."\n" );
      print( SCALAR_OUTPUT "scalar fraction\n" );
      print( SCALAR_OUTPUT join("\n", @{$elements[3]}) ); 
      close( SCALAR_OUTPUT );
   } 
}
