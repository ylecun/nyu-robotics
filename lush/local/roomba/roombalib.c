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


#include <stdio.h>    /* Standard input/output definitions */
#include <stdint.h>   /* Standard types */
#include <stdlib.h>   /* calloc, strtol */
#include <string.h>   /* strcpy */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>

#include "roombalib.h"

int roombadebug = 0;

// internal use only
int roomba_init_serialport( const char* serialport, speed_t baud );

Roomba* roomba_init( const char* portpath ) 
{
    int fd = roomba_init_serialport( portpath, B57600 );
    if( fd == -1 ) return NULL;
    uint8_t cmd[1];
    
    cmd[0] = 128;      // START
    int n = write(fd, cmd, 1);
    if( n!=1 ) {
        perror("open_port: Unable to write to port ");
        return NULL;
    }
    roomba_delay(COMMANDPAUSE_MILLIS);
    
    cmd[0] = 130;   // CONTROL
    n = write(fd, cmd, 1);
    if( n!=1 ) {
        perror("open_port: Unable to write to port ");
        return NULL;
    }
    roomba_delay(COMMANDPAUSE_MILLIS);

    Roomba* roomba = calloc( 1, sizeof(Roomba) );
    roomba->fd = fd;
    strcpy(roomba->portpath, portpath);
    roomba->velocity = DEFAULT_VELOCITY;

    return roomba;
}

void roomba_free( Roomba* roomba ) 
{
    if( roomba!= NULL ) {
        if( roomba->fd ) roomba_close( roomba );
        free( roomba );
    }
}

const char* roomba_get_portpath( Roomba* roomba )
{
    return roomba->portpath;
}

void roomba_close( Roomba* roomba )
{
    close( roomba->fd );
    roomba->fd = 0;
}

// is this Roomba pointer valid (but not necc connected)
int roomba_valid( Roomba* roomba )
{
    return (roomba!=NULL && roomba->fd != 0);
}

void roomba_set_velocity( Roomba* roomba, int vel )
{
    roomba->velocity = vel;
}

int roomba_get_velocity( Roomba* roomba )
{
    return roomba->velocity;
}

// send an arbitrary length roomba command
int roomba_send( Roomba* roomba, const uint8_t* cmd, int len )
{
    int n = write( roomba->fd, cmd, len);
    if( n!=len )
        perror("roomba_send: couldn't write to roomba");
    return (n!=len); // indicate error, can usually ignore
}

// Move Roomba with low-level DRIVE command
void roomba_drive( Roomba* roomba, int velocity, int radius )
{
    uint8_t vhi = velocity >> 8;
    uint8_t vlo = velocity & 0xff;
    uint8_t rhi = radius   >> 8;
    uint8_t rlo = radius   & 0xff;
    if(roombadebug) 
        fprintf(stderr,"roomba_drive: %.2hhx %.2hhx %.2hhx %.2hhx\n",
                vhi,vlo,rhi,rlo);
    uint8_t cmd[] = { 137, vhi,vlo, rhi,rlo };  // DRIVE
    int n = write(roomba->fd, cmd, 5);
    if( n!=5 )
        perror("roomba_drive: couldn't write to roomba");
}

void roomba_drive_direct( Roomba* roomba, int vleft, int vright ) {
    uint8_t vlhi = vleft >> 8;
    uint8_t vllo = vleft & 0xff;
    uint8_t vrhi = vright >> 8;
    uint8_t vrlo = vright & 0xff;
    uint8_t cmd[] = { 145, vrhi,vrlo, vlhi,vllo };  // DRIVE_DIRECT
    int n = write(roomba->fd, cmd, 5);
    if( n!=5 )
        perror("roomba_drive_direct: couldn't write to roomba");
}


void roomba_stop( Roomba* roomba ) {
    roomba_drive( roomba, 0, 0 );
}

void roomba_forward( Roomba* roomba )
{
    roomba_drive( roomba, roomba->velocity, 0x8000 );  // 0x8000 = straight
}
void roomba_forward_at( Roomba* roomba, int velocity )
{
    roomba_drive( roomba, velocity, 0x8000 );
}

void roomba_backward( Roomba* roomba )
{
    roomba_drive( roomba, -roomba->velocity, 0x8000 );
}
void roomba_backward_at( Roomba* roomba, int velocity )
{
    roomba_drive( roomba, -velocity, 0x8000 );
}

void roomba_spinleft( Roomba* roomba )
{
    roomba_drive( roomba, roomba->velocity, 1 );
}
void roomba_spinleft_at( Roomba* roomba, int velocity )
{
    roomba_drive( roomba, velocity, 1 );
}

void roomba_spinright( Roomba* roomba )
{
    roomba_drive( roomba, roomba->velocity,  -1 );
}
void roomba_spinright_at( Roomba* roomba, int velocity )
{
    roomba_drive( roomba, velocity,  -1 );
}

void roomba_play_note( Roomba* roomba, uint8_t note, uint8_t duration ) 
{
    uint8_t cmd[] = { 140, 15, 1, note, duration, // SONG, then
                      141, 15 };                  // PLAY
    int n = write( roomba->fd, cmd, 7);
    if( n!=7 )
        perror("roomba_play_note: couldn't write to roomba");
}

void roomba_digital_out( Roomba* roomba, uint8_t bits ) 
{
    uint8_t cmd[] = { 147, (bits & 7) };

    int n = write( roomba->fd, cmd, 2);
    if( n!=2 )
        perror("roomba_digital_out: couldn't write to roomba");
}

// Turns on/off the non-drive motors (main brush, vacuum, sidebrush).
void roomba_set_motors( Roomba* roomba, uint8_t mainbrush, uint8_t vacuum, uint8_t sidebrush)
{
    uint8_t cmd[] = { 138,                        // MOTORS
                      ((mainbrush?0x04:0)|(vacuum?0x02:0)|(sidebrush?0x01:0))};
    int n = write( roomba->fd, cmd, 2);
    if( n!=2 )
        perror("roomba_set_motors: couldn't write to roomba");
}

// Turns on/off the various LEDs.
void roomba_set_leds( Roomba* roomba, uint8_t status_green, uint8_t status_red,
                      uint8_t spot, uint8_t clean, uint8_t max, uint8_t dirt, 
                      uint8_t power_color, uint8_t power_intensity )
{
    uint8_t v = (status_green?0x20:0) | (status_red?0x10:0) |
                (spot?0x08:0) | (clean?0x04:0) | (max?0x02:0) | (dirt?0x01:0);
    uint8_t cmd[] = { 139, v, power_color, power_intensity }; // LEDS
    int n = write( roomba->fd, cmd, 4);
    if( n!=4 )
        perror("roomba_set_leds: couldn't write to roomba");
}

// Turn all vacuum motors on or off according to state
void roomba_vacuum( Roomba* roomba, uint8_t state ) {
    roomba_set_motors( roomba, state,state,state);
}

// sensor map: packet-id, offset, size
// 07 00 1; 08 01 1; 09 02 1; 10 03 1; 11 04 1;
// 12 05 1; 13 06 1; 14 07 1; 15 08 1; 16 09 1;
// 17 10 1; 18 11 1; 19 12 2; 20 14 2; 21 16 1;
// 22 17 2; 23 19 2; 24 21 1; 25 22 2; 26 24 2;
// 27 26 2; 28 28 2; 29 30 2; 30 32 2; 31 34 2;
// 32 36 1; 33 37 2; 34 39 1; 35 40 1; 36 41 1;
// 37 42 1; 38 43 1; 39 44 2; 40 46 2; 41 48 2;
// 42 50 2; xx 52 bytes total

// read sensor. There are various groups of sensors
// as documented in the create manual page 147.
// The total number of bytes of all sensors is 52.
// Each groups gets written in the appropriate segment
// of the sensor_bytes array.
int roomba_read_sensors( Roomba* roomba, int group )
{
  if ( (group<0) || (group>6)) return -2;
    uint8_t cmd[] = { 142, group }; 
    int n, m, p;
    switch (group) {
    case 0: m = 26; p=0; break;  // packets 7-26
    case 1: m = 10; p=0; break;  // packets 7-16
    case 2: m = 6; p=10; break;  // packets 17-20
    case 3: m = 10; p=16; break; // packets 21-26
    case 4: m = 14; p=26; break; // packets 27-34
    case 5: m = 12; p=40; break; // packets 35-42
    case 6: m = 52; p=0; break;  // packets 7-42
    default: m = 0; p=0; return -2;
    }
    n = write( roomba->fd, cmd, 2);
    roomba_delay(COMMANDPAUSE_MILLIS);  //hmm, why isn't VMIN & VTIME working?
    n = read( roomba->fd, (roomba->sensor_bytes)+p, m);
    if( n != m ) {
      if(roombadebug)
	fprintf(stderr,"roomba_read_sensors: not enough read (%d != %d)\n",m,n);
      return -1;
    }
    return 0;
}

void roomba_print_raw_sensors( Roomba* roomba )
{
    uint8_t* sb = roomba->sensor_bytes;
    int i;
    for(i=0;i<26;i++) {
        printf("%.2hhx ",sb[i]);
    }
    printf("\n");
}

void roomba_print_sensors( Roomba* roomba )
{
    uint8_t* sb = roomba->sensor_bytes;
    printf("bump: %x %x\n", bump_left(sb[0]), bump_right(sb[0]));
    printf("wheeldrop: %x %x %x\n", wheeldrop_left(sb[0]),
           wheeldrop_caster(sb[0]), wheeldrop_right(sb[0]));
    printf("wall: %x\n", sb[1]);
    printf("cliff: %x %x %x %x\n", sb[2],sb[3],sb[4],sb[5] );
    printf("virtual_wall: %x\n", sb[6]);
    printf("motor_overcurrents: %x %x %x %x %x\n", motorover_driveleft(sb[7]),
           motorover_driveright(sb[7]), motorover_mainbrush(sb[7]),
           motorover_sidebrush(sb[7]),  motorover_vacuum(sb[7]));
    printf("dirt: %x %x\n", sb[8],sb[9]);
    printf("remote_opcode: %.2hhx\n", sb[10]);
    printf("buttons: %.2hhx\n", sb[11]);
    printf("distance: %.4x\n",    (sb[12]<<8) | sb[13] );
    printf("angle: %.4x\n",       (sb[14]<<8) | sb[15] );
    printf("charging_state: %.2hhx\n", sb[16]);
    printf("voltage: %d\n",     (sb[17]<<8) | sb[18] );
    printf("current: %d\n",     ((int8_t)sb[19]<<8) | sb[20] );
    printf("temperature: %d\n",    sb[21]);
    printf("charge: %d\n",      (sb[22]<<8) | sb[23] );
    printf("capacity: %d\n",    (sb[24]<<8) | sb[25] );
}

// 100,000 us == 100 ms == 0.1s
void roomba_delay( int millisecs )
{
    usleep( millisecs * 1000 );
}



// private
// returns valid fd, or -1 on error
int roomba_init_serialport( const char* serialport, speed_t baud )
{
    struct termios toptions;
    int fd;

    if(roombadebug)
        fprintf(stderr,"roomba_init_serialport: opening port %s\n",serialport);

    fd = open( serialport, O_RDWR | O_NOCTTY | O_NDELAY );
    if (fd == -1)  {     // Could not open the port.
        perror("roomba_init_serialport: Unable to open port ");
        return -1;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        perror("roomba_init_serialport: Couldn't get term attributes");
        return -1;
    }
    
    cfsetispeed(&toptions, baud);
    cfsetospeed(&toptions, baud);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag    |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag    &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag    &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag    &= ~OPOST; // make raw

    toptions.c_cc[VMIN]  = 26;
    toptions.c_cc[VTIME] = 2;           // FIXME: not sure about this
    
    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        perror("roomba_init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}
