/*******************************************************************************
* PWM
*
* These functions provide a general interface to all 3 PWM subsystems, each of
* which have two available channels A and B. PWM subsystems 1 and 2 are used for
* controlling the 4 motors on the Robotics Cape, however they may be controlled
* by the user directly instead of using the motor API. PWM subsystem 0 channels
* A and B can be accessed on the UART1 header if set up with the Pinmux API to
* do so. The user may have exclusive use of that subsystem.
*
* @ int rc_pwm_init(int ss, int frequency)
*
* Configures subsystem 0, 1, or 2 to operate at a particular frequency. This may
* be called at runtime to change the pwm frequency without stopping the motors
* or pwm signal. Returns 0 on success or -1 on failure.
*
* @ int rc_pwm_close(int ss){
*
* Unexports a subsystem to put it into low-power state. Not necessary for the
* the user to call during normal program operation. This is mostly for internal
* use and cleanup.
*
* @ int rc_pwm_set_duty(int ss, char ch, float duty)
*
* Updates the duty cycle through the file system userspace driver. subsystem ss
* must be 0,1,or 2 and channel 'ch' must be A or B. Duty cycle must be bounded
* between 0.0f (off) and 1.0f(full on). Returns 0 on success or -1 on failure.
*
* @ int rc_pwm_set_duty_ns(int ss, char ch, int duty_ns)
*
* like rc_pwm_set_duty() but takes a pulse width in nanoseconds which must range
* from 0 (off) to the number of nanoseconds in a single cycle as determined
* by the freqency specified when calling rc_pwm_init(). The default PWM
* frequency of the motors is 25kz corresponding to a maximum pulse width of
* 40,000ns. However, this function will likely only be used by the user if they
* have set a custom PWM frequency for a more specific purpose. Returns 0 on
* success or -1 on failure.
*
* @ int rc_pwm_set_duty_mmap(int ss, char ch, float duty)
*
* This is the fastest way to set the pwm duty cycle and is used internally by
* the rc_set_motor() function but is also available to the user. This is done
* with direct memory access from userspace to the pwm subsystem. It's use is
* identical to rc_pwm_set_duty where subsystem ss must be 0,1, or 2 where
* 1 and 2 are used by the motor H bridges. Channel 'ch' must be 'A' or 'B' and
* duty must be from 0.0f to 1.0f. The subsystem must be intialized with
* rc_pwm_init() before use. Returns 0 on success or -1 on failure.
*******************************************************************************/
#ifndef RC_PWM_H
#define RC_PWM_H

int rc_pwm_init(int ss, int frequency);
int rc_pwm_close(int ss);
int rc_pwm_set_duty(int ss, char ch, float duty);
int rc_pwm_set_duty_ns(int ss, char ch, int duty_ns);
int rc_pwm_set_duty_mmap(int ss, char ch, float duty);

#endif //RC_PWM_H