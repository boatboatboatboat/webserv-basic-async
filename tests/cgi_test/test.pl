#!/usr/bin/perl

print "Content-type: text/html\n\n";
print "<h1>Environment</h1><hr>";

foreach (sort keys %ENV) {
   print "<b>$_</b>: $ENV{$_}<br>\n";
}

1;