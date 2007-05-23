#!/usr/local/bin/perl

open( MUDFILE, "system/mud.pid" );
$pid = <MUDFILE>;

print( "Kill del processo $pid\n" );
system( "kill -9 $pid" );
