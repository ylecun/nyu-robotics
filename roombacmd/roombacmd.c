/*
 * roombacmd -- Roomba command-line interface
 *
 * http://hackingroomba.com/
 *
 * Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
 *
 * Updates:
 * 14 Dec 2006 - added more functions to roombalib
 */

#include <stdio.h>  
#include <getopt.h>
#include <stdlib.h>   /* calloc, strtol */
#include <string.h>   /* String function definitions */

#include "roombalib.h"

void usage()
{
    printf("Usage: roombacmd -p <serialport> [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -h, --help                   Print this help message\n"
    "  -p, --port=serialport        Serial port roomba is on\n"
    "  -v, --verbose=NUM            Be verbosive (use more for more verbose)\n"
    "  -f, --forward                Go forward at current speed\n"
    "  -b, --backward               Go backward at current speed\n"
    "  -l, --spin-left              Spin left at current speed\n"
    "  -r, --spin-right             Spin right at current speed\n"
    "  -s, --stop                   Stop a moving Roomba\n"
    "  -w, --wait=millis            Wait some milliseconds\n"
    "  -V, --velocity=val           Set current speed to val (1 to 500)\n"
    "  -S, --sensors                Read Roomba sensors,display nicely\n"
    "  -R, --sensors-raw            Read Roomba sensors,display in hex\n"
    "      --debug                  Print out boring details\n"
    "\n"
    "Examples:\n"
    " roombacmd -p /dev/ttyS0 -V 250 --forward \n"
    " roombacmd -p /dev/ttyS0 --spin-left \n"
    " roombacmd -p /dev/ttyS0 --sensors \n"
    " roombacmd -p /dev/ttyS0 --stop \n"
    "\n"
    "Notes:\n"
    "- The '-p' port option must be first option and is required.\n"
    "- All options/commands can be cascaded & are executed in order, like:\n"
    "    roombacmd -p /dev/ttyS0 -f -w 1000 -b -w 1000 -s \n"
    "    to go forward for 1 sec, go back for 1 sec, then stop. \n"
    "\n");
}

int main(int argc, char *argv[]) 
{
    char serialport[256];
    int velocity = 0;
    int waitmillis = 1000;

    Roomba* roomba = NULL; 
    
    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",       no_argument,       0, 'h'},
        {"port",       required_argument, 0, 'p'},
        {"reset",      no_argument,       0, 0},
        {"drive",      required_argument, 0, 'd'},
        {"forward",    optional_argument, 0, 'f'},
        {"backward",   optional_argument, 0, 'b'},
        {"spin-left",  optional_argument, 0, 'l'},
        {"spin-right", optional_argument, 0, 'r'},
        {"stop",       no_argument,       0, 's'},
        {"wait",       optional_argument, 0, 'w'},
        {"velocity",   required_argument, 0, 'V'},
        {"sensors",    no_argument,       0, 'S'},
        {"sensors-raw",no_argument,       0, 'R'},
        {"spy",        no_argument,       0, 'Y'},
        {"debug",      no_argument,       0, 'D'},
        {0,0,0,0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hp:d:fblrsw:V:SRD", 
                           loptions, &option_index);
        if (opt==-1) break;
        
        switch (opt) {
        case 0:
            //if (!(strcmp("dir",loptions[option_index].name)))
            //		strcpy(dir, optarg);
            //else if(!(strcmp("period",loptions[option_index].name)))
            //    period = strtol(optarg,NULL,10);
            //else if(!(strcmp("imgsize",loptions[option_index].name)))
            break;
        case 'h':
            usage();
        case 'p':
            strcpy(serialport,optarg);
            roomba = roomba_init( serialport );
            if( roomba == NULL ) {
                fprintf(stderr,"error: couldn't open roomba\n");
                return -1;
            }
            break;
        case 'f':
            if(roomba) roomba_forward( roomba );
            break;
        case 'b':
            if(roomba) roomba_backward( roomba );
            break;
        case 'l':
            if(roomba) roomba_spinleft( roomba );
            break;
        case 'r':
            if(roomba) roomba_spinright( roomba );
            break;
        case 's':
            if(roomba) roomba_stop( roomba );
            break;
        case 'w':
            waitmillis = strtol(optarg,NULL,10);
            roomba_wait( waitmillis );
            break;
        case 'V':
            velocity = strtol(optarg,NULL,10);
            if(roomba) roomba_set_velocity( roomba, velocity );
            break;
        case 'S':
            if(roomba) {
                roomba_read_sensors(roomba);
                roomba_print_sensors(roomba);
            }
            break;
        case 'R':
            if(roomba) {
                roomba_read_sensors(roomba);
                roomba_print_raw_sensors(roomba);
            }
        case 'D':
            roombadebug++;
        case '?':
            break;
        }
    }
    if (argc==1) {
        usage();
        return 0;
    }
    
    //printf("argv0:%s\n", argv[0]);
    if( roomba ) roomba_close( roomba );

    return 0;
}


