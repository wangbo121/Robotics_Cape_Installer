/******************************************************************************
* DC MOTOR CONTROL
*
* The robotics cape can drive 4 DC motors bidirectionally powered only from a
* 2-cell lithium battery pack connected to the cape. The motors will not draw
* power from USB or the 9-18v DC Jack. Each channel can support 1.2A continuous
* and the user must be careful to choose motors which will not exceed this
* rating when stalled. Each channel is broken out on an independent 2-pin
* JST ZH connector.
*
* @ int rc_enable_motors()
* @ int rc_disable_motors()
*
* The motor drivers are initially in a low-power standby state and must be
* woken up with rc_enable_motors() before use. The user can optionally put the
* motor drivers back into low power state with rc_disable_motors().
*
* @ int rc_set_motor(int motor, float duty)
* @ int rc_set_motor_all(float duty)
*
* These will take in a motor index from 1 to 4 and a duty between -1 & 1
* corresponding to full power reverse to full power forward.
* rc_set_motor_all() applies the same duty cycle to all 4 motor channels.
*
* @ int rc_set_motor_free_spin(int motor)
* @ int set motor_free_spin_all()
*
* This puts one or all motor outputs in high-impedance state which lets the
* motor spin freely as if it wasn't connected to anything.
*
* @ int rc_set_motor_brake(int motor)
* @ int rc_set_motor_brake_all()
*
* These will connect one or all motor terminal pairs together which
* makes the motor fight against its own back EMF turning it into a brake.
*
* See the test_motors example for sample use case.
******************************************************************************/
#ifndef RC_MOTOR_H
#define RC_MOTOR_H

int rc_initialize_motors();
int rc_cleanup_motors();
int rc_enable_motor_standby();
int rc_disable_motor_standby();
int rc_set_motor(int motor, float duty);
int rc_set_motor_all(float duty);
int rc_set_motor_free_spin(int motor);
int rc_set_motor_free_spin_all();
int rc_set_motor_brake(int motor);
int rc_set_motor_brake_all();

#endif // RC_MOTOR_H