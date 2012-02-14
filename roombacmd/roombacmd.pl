#!/usr/bin/perl
#
# roombacmd.pl -- Roomba command-line tool,  part of RoombaComm
#
# http://roombahacking.com/
#
# Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
#
#

# in case we have stty in the current directory
$ENV{'PATH'}="$ENV{PATH}:.";


sub usage() {
    printf "Usage: $0 {serialport} {init|forward|backward|spinleft|spinright|stop}\n";
    exit(1);
}


sub roomba_init() {
    # this style stty is for linux
    system("stty -F $PORT 57600 raw -parenb -parodd cs8 -hupcl -cstopb clocal");
    printf "\x80" > $PORT;   sleep 1;
    printf "\x82" > $PORT;   sleep 1;
}
sub roomba_forward() {
    $vel="\x00\xc8";
    $rad="\x80\x00";
    printf "\x89$vel$rad" > $PORT;
}
sub roomba_backward() {
    $vel="\xff\x38";
    $rad="\x80\x00";
    printf "\x89$vel$rad" > $PORT;
}
sub roomba_spinleft() {
    $vel="\x00\xc8";
    $rad="\x00\x01";
    printf "\x89$vel$rad" > $PORT;
}
sub roomba_spinright() {
    $vel="\x00\xc8";
    $rad="\xff\xff";
    printf "\x89$vel$rad" > $PORT;
}
sub roomba_stop() {
    $vel="\x00\x00";
    $rad="\x00\x00";
    printf "\x89$vel$rad" > $PORT;
}

# If not enough arguments were passed, return
usage() if( @ARGV < 2 );

$PORT = $ARGV[0];
$CMD  = $ARGV[1];

if( $CMD eq 'init' ) {
    roomba_init();
}
elsif( $CMD eq 'forward' ) {
    roomba_forward();
}
elsif( $CMD eq 'backward' ) {
    roomba_backard();
}
elsif( $CMD eq 'spinleft' ) {
    roomba_spinleft();
}
elsif( $CMD eq 'spinright' ) {
    roomba_spinright();
}
elsif( $CMD eq 'stop' ) {
    roomba_stop();
}
else {
    usage();
}

