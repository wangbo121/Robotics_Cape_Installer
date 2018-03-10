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


#ifdef __cplusplus
}
#endif

#endif // RC_BUTTON_H

/** @}  end group Button */