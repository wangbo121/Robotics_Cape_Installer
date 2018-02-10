/*******************************************************************************
* LEDs
*
* Since the 4 blue USR LEDs on the Beaglebone are normally used by the OS,
* the Robotics Cape provides two LEDs for sole use by the user. One is red
* and one is green. The included examples use the red LED to indicate a paused
* or stopped state, and the green LED to indicate a running state. However
* they are not tied to any other robotics cape library functions and can be
* used for whatever the user desires.
*
* @ typedef enum rc_led_t
*
* Two LEDs are available and defined by an enumerated type led_t which can be
* RED or GREEN. Just like most boolean states in the C language, a 0 indicates
* 'false' or 'off' and anything else indicates 'on' or 'true'. To make code
* easier to read, #defines are provided for 'ON' and 'OFF'.
*
* @ int rc_set_led(rc_led_t led, int state)
*
* If state is 0 the LED will be turned off. If int state is non-zero then the
* LED will be turned on. Returns 0 on success, -1 on failure.
*
* @ int rc_get_led_state(rc_led_t led)
*
* returns 1 if the LED is on or 0 if it is off.
* This function is typically used in multithreded applications where multiple
* threads may wish to use the same LED.
*
* @ int rc_blink_led(rc_led_t led, float hz, float period)
*
* Flash an LED at a set frequency for a finite period of time.
* This is a blocking call and only returns after flashing.
*
* See the blink example for sample use case of all of these functions.
*******************************************************************************/
#ifndef RC_LED_H
#define RC_LED_H

#ifndef ON
#define ON 1
#endif

#ifndef OFF
#define OFF 0
#endif

typedef enum rc_led_t {
	GREEN,
	RED,
	USR0,
	USR1,
	USR2,
	USR3,
	BAT0,
	BAT1,
	BAT2,
	BAT3,
	WIFI
} rc_led_t;


int rc_led_set(rc_led_t led, int value);
int rc_led_get(rc_led_t led);

int rc_led_blink(rc_led_t led, float hz, float duration);
int rc_led_stop_blink(rc_led_t led);
int rc_led_stop_blink_all();

#endif // RC_LED_H