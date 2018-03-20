/**
 * \example rc_benchmark_algebra.c
 * \example rc_mpu_calibrate_gyro.c
 * \example rc_mpu_calibrate_mag.c
 * \example rc_test_dmp.c
 * \example rc_test_dmp_tap.c
 * \example rc_test_gpio.c
 * \example rc_test_mpu.c
 * \example rc_test_pthread.c
 * \example rc_version.c
 */


/**
 * @defgroup   Math
 *
 * @brief      C routines for linear algebra quaternion manipulation, and
 *             discrete time filters
 */

/**
 * @defgroup   IO
 *
 * @brief      C interface for Linux userspace IO
 */

/**
 * @defgroup   Quadrature_Encoder
 *
 * @brief      Functions for reading quadrature encoders.
 *
 *             Channels 1-3 on the Robotics Cape and BeagleBone Blue are counted
 *             in hardware by the eQEP modules and are extremely resistant to
 *             skipping steps and use negligible power. They can also be used
 *             without root permissions. If necessary, encoder channel 4 on the
 *             Robotics Cape and BeagleBone Blue can be used which uses PRU0 to
 *             count steps so the ARM core doesn't have to do anything. It is
 *             less robust than the dedicated hardware counters and requires
 *             root permissions to use, please use channels 1-3 first.
 */

