/******************************************************************************
* SERVO AND ESC
*
* The Robotics Cape has 8 3-pin headers for connecting hobby servos and ESCs.
* The connectors are not polarized so pay close attention to the symbols
* printed in white silkscreen on the cape before plugging anything in.
* The standard pinnout for these 3 pin connectors is as follows.
*
* 1 - Ground
* 2 - 6V Power
* 3 - Pulse width signal
*
* @ int rc_enable_servo_power_rail()
* @ int rc_disable_servo_power_rail()
*
* The user must call rc_enable_servo_power_rail() to enable the 6V voltage
* regulator and send power to servos. This can be ignored if using ESCs or
* servos that are driven by an external power source.
* rc_disable_servo_power_rail() can be called to turn off power to the servos for
* example when the robot is in a paused state to save power or prevent noisy
* servos from buzzing.
*
* @ int rc_send_servo_pulse_normalized(int ch, float input)
* @ int rc_send_servo_pulse_normalized_all(float input)
*
* The normal operating range of hobby servos is usually +- 60 degrees of
* rotation from the neutral position but they often work up to +- 90 degrees.
* rc_send_servo_pulse_normalized(int ch, float input) will send a single pulse to
* the selected channel. the normalized input should be between -1.5 and 1.5
* corresponding to the following pulse width range and angle.
*
* input     width   angle
* -1.5      600us  90 deg anticlockwise
* -1.0      900us  60 deg anticlockwise
*  0.0     1500us   0 deg neutral
*  1.0     2100us  60 deg clockwise
*  1.5     2400us  90 deg clockwise
*
* Note that all servos are different and do not necessarily allow the full
* range of motion past +-1.0. DO NOT STALL SERVOS.
*
* @ int rc_send_esc_pulse_normalized(int ch, float input)
* @ int rc_send_esc_pulse_normalized_all(float input)
*
* Brushless motor controllers (ESCs) for planes and multirotors are
* unidirectional and lend themselves better to a normalized range from 0 to 1.
* rc_send_esc_pulse_normalized(int ch, float input) also sends a single pulse
* but the range is set for common ESCs
*
* input    width       power
* -0.1      900      armed but idle
* 0.0      1000us       0%   off
* 0.5      1500us      50%  half-throttle
* 1.0      2000us      100% full-throttle
*
* This assumes the ESCs have been calibrated for the 1000-2000us range. Use the
* calibrate_escs example program to be sure.
*
* @ int rc_send_servo_pulse_us(int ch, int us)
* @ int rc_send_servo_pulse_us_all(int us)
*
* The user may also elect to manually specify the exact pulse width in
* in microseconds with rc_send_servo_pulse_us(int ch, int us). When using any of
* these functions, be aware that they only send a single pulse to the servo
* or ESC. Servos and ESCs typically require an update frequency of at least
* 10hz to prevent timing out. The timing accuracy of this loop is not critical
* and the user can choose to update at whatever frequency they wish.
*
* See the test_servos, sweep_servos, and calibrate_escs examples.
******************************************************************************/
#ifndef RC_SERVER_H
#define RC_SERVO_H

int rc_initialize_servo();
int rc_cleanup_servo();
int rc_enable_servo_power_rail();
int rc_disable_servo_power_rail();
int rc_send_servo_pulse_us(int ch, int us);
int rc_send_servo_pulse_us_all(int us);
int rc_send_servo_pulse_normalized(int ch, float input);
int rc_send_servo_pulse_normalized_all(float input);
int rc_send_esc_pulse_normalized(int ch, float input);
int rc_send_esc_pulse_normalized_all(float input);
int rc_send_oneshot_pulse_normalized(int ch, float input);
int rc_send_oneshot_pulse_normalized_all(float input);

#endif // RC_SERVO_H