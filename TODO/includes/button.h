#ifndef RC_BUTTONS_H
#define RC_BUTTONS_H

/*******************************************************************************
* roboticscape_buttons.h
*
* function declarations for starting and stopping button handlers
* the rest of the button functions are in roboticscape.h for the user to access
*******************************************************************************/




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
#include "rc/preprocessor_macros.h"

// state enumerated type
typedef enum rc_button_state_t {
	PRESSED,
	RELEASED
} rc_button_state_t;

// button enumerated type
typedef enum rc_button_t {
	RC_PAUSE_BTN,
	RC_MODE_BTN
} rc_button_t;

// initialize and cleanup
int rc_button_init();
int rc_button_cleanup();

// general functions
int rc_button_set_callback(rc_button_t button, rc_button_state_t state,void (*func)(void));
rc_button_state_t rc_button_get_state(rc_button_t button);
int rc_button_wait(rc_button_t button, rc_button_state_t state);



// deprecated callback function initializers
__deprecated int rc_set_pause_pressed_func(void (*func)(void));
__deprecated int rc_set_pause_released_func(void (*func)(void));
__deprecated int rc_set_mode_pressed_func(void (*func)(void));
__deprecated int rc_set_mode_released_func(void (*func)(void));

// deprecated state functions
__deprecated rc_button_state_t rc_get_pause_button();
__deprecated rc_button_state_t rc_get_mode_button();



#endif // RC_BUTTONS_H
