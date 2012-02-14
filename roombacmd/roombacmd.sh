#!/bin/sh
#
# roombacmd.sh -- Simple shell-based Roomba command-line tool
#
# http://roombahacking.com/
#
# Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
#
#

# in case we have stty in the current directory
PATH=${PATH}:.

usage() {
        echo "Usage: $0 {serialport} {init|forward|backward|spinleft|spinright|stop}" >&2
        exit 1
}

roomba_init() {
    os=`uname -s`
    if [ "$os" == "Linux" ]; then
        stty -F $PORT 57600 raw -parenb -parodd cs8 -hupcl -cstopb clocal
    elif [ "$os" == "Darwin" ]; then
        stty -f $PORT 57600 raw -parenb -parodd cs8 -hupcl -cstopb clocal
    fi
    printf "\x80" > $PORT;   sleep 1
    printf "\x82" > $PORT;   sleep 1
}
roomba_forward() {
    vel="\x00\xc8"
    rad="\x80\x00"
    printf "\x89$vel$rad" > $PORT
}
roomba_backward() {
    vel="\xff\x38"
    rad="\x80\x00"
    printf "\x89$vel$rad" > $PORT
}
roomba_spinleft() {
    vel="\x00\xc8"
    rad="\x00\x01"
    printf "\x89$vel$rad" > $PORT
}
roomba_spinright() {
    vel="\x00\xc8"
    rad="\xff\xff"
    printf "\x89$vel$rad" > $PORT
}
roomba_stop() {
    vel="\x00\x00"
    rad="\x00\x00"
    printf "\x89$vel$rad" > $PORT
}

# If not enough arguments were passed, return
[ -z "$2" ] && usage


PORT=$1

case $2 in
    init)
        roomba_init
        ;;
    forward)
        roomba_forward
        ;;
    backward)
        roomba_backward
        ;;
    spinleft)
        roomba_spinleft
        ;;
    spinright)
        roomba_spinright
        ;;
    stop)
        roomba_stop
        ;;
    *)
        usage
        ;;
esac
#exit 0