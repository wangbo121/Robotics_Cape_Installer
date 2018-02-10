/*******************************************************************************
* rc_blink.c
*
* This is an example program to demonstrate use of LEDs and button handlers
* in the robotics cape API. Once started, blink will flash the green and red
* LEDs. Pressing the mode button will cycle through 3 blinking speeds, slow
* medium, and fast. Momentarily pressing the pause button will stop and start
* the blinking by toggling the global state between PAUSED and RUNNING. If the
* user holds the pause button for more than 1.5 seconds then the blink program
* will flash the red LED and exit cleanly.
*
* This should be used as a reference for how to handle buttons and how to
* control program flow cleanly utilizing rc_get_state() and rc_set_state().
*******************************************************************************/

#include <stdio.h>
#include "rc/button.h"
#include "rc/led.h"
#include "rc/flow.h"
#include "rc/time.h"

#define QUIT_TIMEOUT_US 1500000 // quit after 1.5 seconds holding pause button
#define QUIT_CHECK_US	100000	// check every 1/10 second

// mode=0,1,2 corresponds to us_delay index for slow,medium,fast
const int us_delay[] = {400000, 170000, 100000};
int mode;
int toggle; // toggles between 0&1 for led blink


/*******************************************************************************
* void on_pause_release()
*
* Make the Pause button toggle between paused and running states.
*******************************************************************************/
void on_pause_release(){
	// toggle betewen paused and running modes
	if(rc_get_state()==RUNNING){
		rc_set_state(PAUSED);
		printf("PAUSED\n");
	}
	else if(rc_get_state()==PAUSED){
		rc_set_state(RUNNING);
		printf("RUNNING\n");
	}
	fflush(stdout);
	return;
}

/*******************************************************************************
* void on_pause_press()
*
* If the user holds the pause button for 2 seconds, set state to exiting which
* triggers the rest of the program to exit cleanly.
*******************************************************************************/
void on_pause_press(){
	int i=0;
	const int samples = QUIT_TIMEOUT_US/QUIT_CHECK_US;

	// now keep checking to see if the button is still held down
	for(i=0;i<samples;i++){
		rc_usleep(QUIT_CHECK_US);
		//if(rc_get_pause_button() == RELEASED){
		if(rc_button_get_state(RC_PAUSE_BTN) == RELEASED){
			printf("shutdown check sees released\n");
			return;
		}
	}
	printf("long press detected, shutting down\n");
	rc_set_state(EXITING);
	return;
}

/*******************************************************************************
* void on_mode_release()
*
* If the user holds the pause button for 2 seconds, set state to exiting which
* triggers the rest of the program to exit cleanly.
*******************************************************************************/
void on_mode_release(){
	if(mode<2)	mode++;
	else		mode=0;
	printf("setting mode: %d\n", mode);
	return;
}

/*******************************************************************************
* int main()
*
* main function sits in one while loop blinking LEDs while button handlers
* control the blink speed and program state
*******************************************************************************/
int main(){
	rc_button_state_t ret;

	// start signal handler so we can exit cleanly
	if(rc_enable_signal_handler()<0){
		fprintf(stderr,"ERROR: failed to complete rc_enable_signal_handler\n");
		return -1;
	}

	ret = rc_button_get_state(RC_MODE_BTN);
	if((int)ret==-1) return -1;
	else if(ret == RELEASED) printf("released BEGINNING\n");
	else printf("pressed BEGINNING\n");

	// start button handler threads
	if(rc_button_init()){
		fprintf(stderr,"ERROR: failed to run rc_init_buttons(), are you root?\n");
		return -1;
	}

	printf("\nPress mode to change blink rate\n");
	printf("hold pause button to exit\n");

	//Assign your own functions to be called when events occur
	if(rc_button_set_callback(RC_PAUSE_BTN,PRESSED,&on_pause_press)) return -1;
	if(rc_button_set_callback(RC_PAUSE_BTN,RELEASED,&on_pause_release)) return -1;
	if(rc_button_set_callback(RC_MODE_BTN,RELEASED,&on_mode_release)) return -1;

	// start in slow mode with both LEDs off
	mode = 0;
	rc_led_set(GREEN,OFF);
	rc_led_set(RED,OFF);

	// Run the main loop untill state is EXITING which is set by hitting ctrl-c
	// or holding down the pause button for more than the quit timeout period
	rc_set_state(RUNNING);
	while(rc_get_state()!=EXITING){
		// if the state is RUNNING (instead of PAUSED) then blink!
		if(rc_get_state()==RUNNING){
			if(toggle){
				rc_led_set(GREEN,OFF);
				rc_led_set(RED,ON);
				toggle = 0;
			}
			else{
				rc_led_set(GREEN,ON);
				rc_led_set(RED,OFF);
				toggle=1;
			}
		}
		if(rc_button_get_state(RC_MODE_BTN) == RELEASED){
			printf("released\n");
		}
		else printf("pressed\n");
		// sleep the right delay based on current mode.
		rc_usleep(us_delay[mode]);
	}

	// now that the while loop has exited, clean up neatly and exit compeltely.
	rc_led_set(GREEN,OFF);
	rc_led_set(RED,OFF);
	rc_button_cleanup();
	return 0;
}
