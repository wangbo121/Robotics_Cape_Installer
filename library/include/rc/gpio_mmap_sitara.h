/**
 * @headerfile gpio_mmap_sitara.h <rc/gpio_mmap_sitara.h>
 *
 * @brief      userspace C interface for controlling the sitara's gpio with
 *             direct memory access as fast as possible
 *
 *             This works only with the sitara AM335X found in the BeagleBone
 *             and requires root access to work. It is extremely fast but use at
 *             your own risk.
 *
 * @author     James Strawson
 *
 * @date       2016
 *
 *
 *
 * @addtogroup gpio_mmap_sitara
 * @ingroup    IO
 * @{
 */

#ifndef RC_GPIO_MMAP_SITARA_H
#define RC_GPIO_MMAP_SITARA_H

#ifdef  __cplusplus
extern "C" {
#endif


/**
 * @brief      initializes mmap functions for sitara gpio
 *
 *             Ideally the user should export and configure all the pins they
 *             wish to use before calling this. If the pins were already
 *             configured in the device tree this is not strictly necessary but
 *             encouraged practice anyway.
 *
 *             This function opens a pointer to /dev/mem with mmap and as such
 *             requires root access like the other functions in this header.
 *
 * @return     0 on success, -1 on failure
 */
int rc_gpio_mmap_init();


/**
 * @brief      sets the value of a gpio output pin
 *
 *             The pin must already be configured as an output pin either
 *             through the device tree, pinmux, or with the /sys/class/gpio
 *             driver such as the gpio.h functions in the robotics cape library.
 *
 * @param[in]  pin    The pin ID
 * @param[in]  state  The state (0 or 1)
 *
 * @return     { description_of_the_return_value }
 */
int rc_gpio_mmap_set_value(int pin, int state);


/**
 * @brief      fetches the value of a GPIO input pin
 *
 *             The pin must already be set up as an input pin either through the
 *             device tree, pinmux, or with the /sys/class/gpio driver such as
 *             the gpio.h functions in the robotics cape library.
 *
 * @param[in]  pin   The pin ID
 *
 * @return     0 if low, 1 if high, -1 on error
 */
int rc_gpio_mmap_get_value(int pin);


#ifdef  __cplusplus
}
#endif

#endif // RC_GPIO_MMAP_SITARA__H

///@} end group IO