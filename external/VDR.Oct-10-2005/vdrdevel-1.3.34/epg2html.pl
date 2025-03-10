#!/usr/bin/perl

# A simple EPG to HTML converter
#
# Converts the EPG data written by 'vdr' into the file /video/epg.data
# into a simple HTML programme listing, consisting of one file per channel
# plus an 'index.htm' file. All output files are written into the current
# directory.
#
# Usage: epg2html.pl < /video/epg.data
#
# See the main source file 'vdr.c' for copyright information and
# how to reach the author.
#
# $Id: epg2html.pl 1.6 2004/03/28 11:15:01 kls Exp $

@Index = ();

sub GetDay
{
  return substr(localtime(shift), 0, 10);
}

sub GetTime
{
  return substr(localtime(shift), 11, 5);
}

sub Tags
{
  my $s = shift;
  $s =~ s/\&/&amp;/g;
  $s =~ s/</&lt;/g;
  $s =~ s/>/&gt;/g;
  return $s;
}

while (<>) {
      chomp;
      if (/^C ([^ ]+) *(.*)/) {
         my $Channel = $2;
         (my $Page = $Channel) =~ y/\/ /-_/;
         $Page .= ".htm";
         $Channel = Tags($Channel);
         push(@Index, qq{<a href="$Page">$Channel</a><br>\n});
         my %Events = ();
         while (<>) {
               if (/^E (.*?) (.*?) ([^ ]*)/) {
                  (my $Time, $Duration) = ($2, $3);
                  my $Title = "", $Subtitle = "", $Description = "", $Vps = 0;
                  while (<>) {
                        if    (/^T (.*)/) { $Title       = Tags($1); }
                        elsif (/^S (.*)/) { $Subtitle    = Tags($1); }
                        elsif (/^D (.*)/) { $Description = Tags($1); $Description =~ s/\|/<br>/g; }
                        elsif (/^V (.*)/) { $Vps         = $1; }
                        elsif (/^e/) {
                           $Events{$Time} = [($Duration, $Title, $Subtitle, $Description, $Vps)];
                           last;
                           }
                        }
                  }
               elsif (/^c/) {
                  my @Schedule = ();
                  my $Day = "";
                  for $t (sort keys %Events) {
                      (my $Duration, $Title, $Subtitle, $Description, $Vps) = @{$Events{$t}};
                      my $d = GetDay($t);
                      if ($d ne $Day) {
                         push(@Schedule, "</table>\n") if ($Day && @Schedule);
                         push(@Schedule, "<h2>$d</h2>\n");
                         push(@Schedule, "<table cellspacing=2>\n");
                         $Day = $d;
                         }
                      my $Entry = $Title;
                      $Entry .= "<br><i>$Subtitle</i>" if $Subtitle;
                      $Entry .= "<br>$Description" if $Description;
                      $Entry .= "<br>(VPS = " . scalar localtime($Vps) . ")" if $Vps && $Vps != $t;
                      push(@Schedule, "<tr><td valign=top>" . GetTime($t) . "</td><td>$Entry</td></tr>\n");
                      }
                  push(@Schedule, "</table>\n") if (@Schedule);
                  open(PAGE, ">$Page") or die "$Page: $!\n";
                  print PAGE "<html>\n<head><title>$Channel</title><head>\n<body>\n";
                  print PAGE "<h1>$Channel</h1>\n";
                  print PAGE @Schedule;
                  print PAGE "</body>\n</html>\n";
                  close(PAGE);
                  last;
                  }
               }
         }
      }

open(INDEX, ">index.htm") or die "index.htm: $!\n";
print INDEX "<html>\n<head><title>EPG Index</title><head>\n<body>\n";
print INDEX sort { lc($a) cmp lc($b) } @Index;
print INDEX "</body>\n</html>\n";
close(INDEX);
   
