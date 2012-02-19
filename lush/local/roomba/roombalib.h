/*
 * roombalib -- Roomba C API
 *
 * http://hackingroomba.com/
 *
 * Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
 * 
 * Updates:
 * 14 Dec 2006 - added more functions to roombalib
 */


#include <stdint.h>   /* Standard types */

#define DEFAULT_VELOCITY 200
#define COMMANDPAUSE_MILLIS 100

// holds all the per-roomba info
// consider it an opaque blob, please
typedef struct Roomba_struct {
    int fd;
    char portpath[80];
    uint8_t sensor_bytes[52];
    int velocity;
} Roomba;

// set to non-zero to see debugging output
extern int roombadebug;

// given a serial port name, create a Roomba object and return it
// or return NULL on error
Roomba* roomba_init( const char* portname );

// frees the memory of the Roomba object created with roomba_init
// will close the serial port if it's open
void roomba_free( Roomba* roomba );

// close the serial port connected to the Roomba
void roomba_close( Roomba* roomba );

// is this Roomba pointer valid (but not necc connected)
int roomba_valid( Roomba* roomba );

// return the serial port path for the given roomba
const char* roomba_get_portpath( Roomba* roomba );

// send an arbitrary length roomba command
int roomba_send( Roomba* roomba, const uint8_t* cmd, int len );

// Move Roomba with direct wheel speeds
void roomba_drive_direct( Roomba* roomba, int vleft, int vright );

// Move Roomba with low-level DRIVE command
void roomba_drive( Roomba* roomba, int velocity, int radius );

// stop the Roomba 
void roomba_stop( Roomba* roomba );

// Move Roomba forward at current velocity
void roomba_forward( Roomba* roomba );
void roomba_forward_at( Roomba* roomba, int velocity );

// Move Roomba backward at current velocity
void roomba_backward( Roomba* roomba );
void roomba_backward_at( Roomba* roomba, int velocity );

// Spin Roomba left at current velocity
void roomba_spinleft( Roomba* roomba );
void roomba_spinleft_at( Roomba* roomba, int velocity );

// Spin Roomba right at current velocity
void roomba_spinright( Roomba* roomba );
void roomba_spinright_at( Roomba* roomba, int velocity );

// Set current velocity for higher-level movement commands
void roomba_set_velocity( Roomba* roomba, int velocity );

// Get current velocity for higher-level movement commands
int roomba_get_velocity( Roomba* roomba );

// play a musical note
void roomba_play_note( Roomba* roomba, uint8_t note, uint8_t duration );

// set digital output bits on expansion conector 
void roomba_digital_out( Roomba* roomba, uint8_t bits);

// Turns on/off the non-drive motors (main brush, vacuum, sidebrush).
void roomba_set_motors( Roomba* roomba, uint8_t mainbrush, uint8_t vacuum, uint8_t sidebrush);

// Turns on/off the various LEDs.
void roomba_set_leds( Roomba* roomba, uint8_t status_green, uint8_t status_red,
                      uint8_t spot, uint8_t clean, uint8_t max, uint8_t dirt, 
                      uint8_t power_color, uint8_t power_intensity );

// Turn all vacuum motors on or off according to state
void roomba_vacuum( Roomba* roomba, uint8_t state );

// Get the sensor data from the Roomba
// group is a sensor group id between 0 and 6 
// (see page 147 of roomba create manual).
// returns 0 on success, -2 if invalid argument,
// -1 on read failure.
int roomba_read_sensors( Roomba* roomba, int group);

// print existing sensor data nicely
void roomba_print_sensors( Roomba* roomba );

// print existing sensor data as string of hex chars
void roomba_print_raw_sensors( Roomba* roomba );

// utility function
void roomba_delay( int millisecs );
#define roomba_wait roomba_delay

// some simple macros of bit manipulations
#define bump_right(b)           ((b & 0x01)!=0)
#define bump_left(b)            ((b & 0x02)!=0)
#define wheeldrop_right(b)      ((b & 0x04)!=0)
#define wheeldrop_left(b)       ((b & 0x08)!=0)
#define wheeldrop_caster(b)     ((b & 0x10)!=0)

#define motorover_sidebrush(b)  ((b & 0x01)!=0) 
#define motorover_vacuum(b)     ((b & 0x02)!=0) 
#define motorover_mainbrush(b)  ((b & 0x04)!=0) 
#define motorover_driveright(b) ((b & 0x08)!=0) 
#define motorover_driveleft(b)  ((b & 0x10)!=0) 

