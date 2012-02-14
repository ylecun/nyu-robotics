/*
 * simpletest -- simple demo of roombalib
 *
 * http://hackingroomba.com/
 *
 * Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
 *
 * Updates:
 * 14 Dec 2006 - added more functions to roombalib
 */


#include <stdio.h>    /* Standard input/output definitions */
#include <string.h>   /* String function definitions */

#include "roombalib.h"

int main(int argc, char *argv[]) 
{
    char* serialport;
    if( argc>1 && strcmp(argv[1],"-p" )==0 ) {
        serialport = argv[2];
    } else {
        fprintf(stderr,"usage: simpletst -p serialport\n");
        return 0;
    }

    roombadebug = 1;

    Roomba* roomba = roomba_init( serialport );    
    
    roomba_forward( roomba );
    roomba_delay(1000); 

    roomba_backward( roomba );
    roomba_delay(1000); 

    roomba_spinleft( roomba );
    roomba_delay(1000); 

    roomba_spinright( roomba );
    roomba_delay(1000); 

    roomba_stop( roomba );
    
    roomba_close( roomba );
    roomba_free( roomba );

    return 0;
}


