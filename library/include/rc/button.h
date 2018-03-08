/**
 * <rc/button.h>
 *
 * Functions for assigning button callback functions. This is based on the GPIO
 * character device driver instead of the gpio-keys driver which means it can
 * be used with any GPIO pin.
 *
 * @author     James Strawson
 * @date       3/7/2018
 *
 * @addtogroup Motor
 * @{
 */


#ifndef RC_BUTTON_H
#define RC_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif


#define RC_BTN_PIN_PAUSE		69	//gpio2.5 P8.9
#define RC_BTN_PIN_MODE			68	//gpio2.4 P8.10

#define RC_BTN_STATE_PRESSED		1
#define RC_BTN_STATE_RELEASED		0

#define RC_BTN_POLARITY_NORM_HIGH	1
#define RC_BTN_POLARITY_NORM_LOW	0

#define RC_BTN_DEBOUNCE_DEFAULT_US	2000


int rc_button_init(int pin, char polarity, int debounce_us);
void rc_button_cleanup();
int rc_button_set_callbacks(int pin, void (*press_func)(void), void (*release_func)(void));
int rc_button_get_state(int pin);
//int rc_button_wait(int_pin);


#ifdef __cplusplus
}
#endif

#endif // RC_BUTTON_H

/** @}  end group Button */

/*******************************************************************************
* BUTTONS
*
* The Robotics Cape includes two buttons labeled PAUSE and MODE. Like the LEDs,
* they are not used by any background library functions and the user can assign
* them to any function they wish. However, the user is encouraged to use the
* pause button to toggle the program flow state between PAUSED and RUNNING
* using the previously described rc_set_state() function.
*
* @ typedef enum rc_button_state_t
*
* A button state can be either RELEASED or PRESSED as defined by this enum.
*
* @ int rc_initialize_buttons()
*
* This initializes the pins and gpio button handler for the pause and mode
* buttons. This is done already in rc_initialize() but exists here if you wish
* to use the button handlers independently from other functionality. Returns 0
* on success or -1 on failure.
*
* @ void rc_stop_buttons()
*
* This tells the button handler threads to stop. It will return immediately.
*
* @ int rc_cleanup_buttons()
*
* After stopping the button handlers, you may wish to wait for the threads to
* stop cleanly before continuing your program flow. This wait function will
* return after all of the user-defined press & release functions have returned
* and the button handlers have shut down cleanly. Will return 0 if everything
* shut down cleanly or the handlers were not running in the first place. It will
* return -1 if the threads timed out and had to be force closed.
*
* @ int rc_set_pause_pressed_func(int (*func)(void))
* @ int rc_set_pause_released_func(int (*func)(void))
* @ int rc_set_mode_pressed_func(int (*func)(void))
* @ int rc_set_mode_released_func(int (*func)(void))
*
* rc_initialize() sets up interrupt handlers that run in the background to
* poll changes in button state in a way that uses minimal resources. The
* user can assign which function should be called when either button is pressed
* or released. Functions can also be assigned under both conditions.
* for example, a timer could be started when a button is pressed and stopped
* when the button is released. Pass
*
* For simple tasks like pausing the robot, the user is encouraged to assign
* their function to be called when the button is released as this provides
* a more natural user experience aligning with consumer product functionality.
*
* The user can also just do a basic call to rc_get_pause_button_state() or
* rc_get_mode_buttom_state() which returns the enumerated type RELEASED or
* PRESSED.
*
* See the rc_blink and rc_test_buttons example programs for sample use cases.
******************************************************************************/
