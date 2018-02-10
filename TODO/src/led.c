/*******************************************************************************
* leds.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "rc/led.h"
#include "rc/time.h"


#define SYSFS_LED_DIR "/sys/devices/platform/leds/leds/"
#define BRIGHTNESS_FILE	"/brightness"
#define MAX_BUF 128
#define NUM_LEDS 11

static const char* paths[] = {
	"green",
	"red",
	"beaglebone:green:usr0",
	"beaglebone:green:usr1",
	"beaglebone:green:usr2",
	"beaglebone:green:usr3",
	"bat0",
	"bat1",
	"bat2",
	"bat3",
	"wifi"
};

static int fd[NUM_LEDS];
static int blinking[NUM_LEDS];
static int stop_blinking_flag[NUM_LEDS];

// local function declaration
static int init_led(rc_led_t);

/*******************************************************************************
* static int init_led(rc_led_t led)
*******************************************************************************/
static int init_led(rc_led_t led)
{
	if(fd[(int)led] != 0){
		#ifdef DEBUG
		fprintf(stderr,"WARNING, trying to initialize an LED which already has file descriptor\n");
		#endif
		return 0;
	}
	// open file descriptor for read and write
	char buf[MAX_BUF];
	int temp_fd;
	strcpy(buf, SYSFS_LED_DIR);
	strcat(buf,paths[(int)led]);
	strcat(buf,BRIGHTNESS_FILE);
	temp_fd = open(buf, O_RDWR);
	if(temp_fd<0){
		fprintf(stderr,"ERROR: in rc_init_led, failed to open LED file handle\n");
		return -1;
	}
	fd[(int)led]=temp_fd;
	return 0;
}


/*******************************************************************************
* int rc_led_set(rc_led_t led, int value)
*******************************************************************************/
int rc_led_set(rc_led_t led, int value)
{
	if(fd[(int)led] == 0){
		#ifdef DEBUG
		fprintf(stderr,"initializing led\n");
		#endif
		if(init_led(led)<0) return -1;
	}

	if(value)	write(fd[(int)led], "1", 2);
	else		write(fd[(int)led], "0", 2);
	// yup, that's it
	return 0;
}

/*******************************************************************************
* int rc_led_get(rc_led_t led)
*******************************************************************************/
int rc_led_get(rc_led_t led)
{
	int ret;
	char ch;
	if(fd[(int)led] == 0){
		#ifdef DEBUG
		fprintf(stderr,"initializing led\n");
		#endif
		if(init_led(led)<0) return -1;
	}

	read(fd[(int)led], &ch, 1);
	if (ch != '0') ret = 1;
	else ret = 0;
	// yup that's it
	return ret;
}

/*******************************************************************************
* int rc_led_blink(rc_led_t led, float hz, float duration)
*
* Flash an LED at a set frequency for a finite duration of time.
* This is a blocking call and only returns after blinking.
* Blinking can be interrupted with rc_led_stop_blink() from another thread
* to let this return before completion.
*******************************************************************************/
int rc_led_blink(rc_led_t led, float hz, float duration)
{
	int i;
	int toggle = 0;

	if(blinking[(int)led]){
		fprintf(stderr, "ERROR: in rc_led_blink(), starting blink while led already blinking\n");
		return -1;
	}
	stop_blinking_flag[(int)led]=0;
	blinking[(int)led]=1;
	const int delay_us = 1000000.0/(2.0*hz);
	const int blinks = duration*2.0*hz;

	stop_blinking_flag[(int)led]=0;
	// loop though blinks, exiting if necessary
	for(i=0;i<blinks;i++){
		toggle = !toggle;
		if(stop_blinking_flag[(int)led]) break;
		if(rc_led_set(led,toggle)<0) return -1;
		// wait for next blink
		rc_usleep(delay_us);
	}
	rc_led_set(led, 0); // make sure it is left off
	blinking[(int)led]=0;
	return 0;
}



/*******************************************************************************
* int rc_led_stop_blink(rc_led_t led)
*
* Stops a particular LED from blinking and causes the associated rc_led_blink
* function to return. Use this to prematurely top a blink or to make sure
* threads exit in a timely manner during program cleanup and shutdown.
*******************************************************************************/
int rc_led_stop_blink(rc_led_t led)
{
	stop_blinking_flag[(int)led]=0;
	return 0;
}

/*******************************************************************************
* int rc_led_stop_blink_all()
*
* Same as rc_led_stop_blink except it applies to all LEDs
*******************************************************************************/
int rc_led_stop_blink_all()
{
	int i;
	for(i=0;i<NUM_LEDS;i++) stop_blinking_flag[i]=0;
	return 0;
}