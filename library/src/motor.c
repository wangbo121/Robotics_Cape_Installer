/**
 * @file motor.c
 */
#include <stdio.h>
#include <rc/motor.h>
#include <rc/model.h>
#include <rc/gpio.h>
#include <rc/pwm.h>

// motor pin definitions
#define MDIR1A			60	//gpio1.28	P9.12
#define MDIR1A_BLUE		64	//gpio2.0	pin T13
#define MDIR1B			31	//gpio0.31	P9.13
#define MDIR2A			48	//gpio1.16	P9.15
#define MDIR2B			81	//gpio2.17	P8.34
#define MDIR2B_BLUE		10	//gpio0.10	P8_31
#define MDIR4A			70	//gpio2.6	P8.45
#define MDIR4B			71	//gpio2.7	P8.46
#define MDIR3B			72	//gpio2.8	P8.43
#define MDIR3A			73	//gpio2.9	P8.44
#define MOT_STBY		20	//gpio0.20	P9.41

#define PWM_FREQ 25000 // 25kHz

// global variables
int mdir1a, mdir2b; // variable gpio pin assignments
int init_flag = 0;



int rc_motor_init()
{
	// assign gpio pins for blue/black
	if(rc_model_get()==BB_BLUE){
		mdir1a = MDIR1A_BLUE;
		mdir2b = MDIR2B_BLUE;
	}
	else{
		mdir1a = MDIR1A;
		mdir2b = MDIR2B;
	}

	#ifdef DEBUG
	printf("Initializing: PWM\n");
	#endif
	if(rc_pwm_init(1,PWM_FREQ)){
		fprintf(stderr,"ERROR in rc_motor_init, failed to initialize pwm subsystem 1\n");
		return -1;
	}
	if(rc_pwm_init(2,PWM_FREQ)){
		fprintf(stderr,"ERROR in rc_motor_init, failed to initialize pwm subsystem 2\n");
		return -1;
	}

	init_flag = 1;
	// make sure motors are off
	rc_disable_motors();
	return 0;
}



int rc_motor_cleanup(){

}



int rc_enable_motors(){
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to enable motors before they have been initialized\n");
		return -1;
	}
	rc_set_motor_free_spin_all();
	return rc_gpio_set_value_mmap(MOT_STBY, HIGH);
}

/*******************************************************************************
* int rc_disable_motors()
*
* turns off the standby pin to disable the h-bridge ICs
* and disables PWM output signals, returns 0 on success
*******************************************************************************/
int rc_disable_motors(){
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to disable motors before they have been initialized\n");
		return -1;
	}
	rc_gpio_set_value_mmap(MOT_STBY, LOW);
	rc_set_motor_free_spin_all();
	return 0;
}

/*******************************************************************************
* int rc_set_motor(int motor, float duty)
*
* set a motor direction and power
* motor is from 1 to 4, duty is from -1.0 to +1.0
*******************************************************************************/
int rc_set_motor(int motor, float duty){
	uint8_t a,b;
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor before they have been initialized\n");
		return -1;
	}
	//check that the duty cycle is within +-1
	if (duty>1.0){
		duty = 1.0;
	}
	else if(duty<-1.0){
		duty=-1.0;
	}
	//switch the direction pins to H-bridge
	if (duty>=0){
	 	a=HIGH;
		b=LOW;
	}
	else{
		a=LOW;
		b=HIGH;
		duty=-duty;
	}

	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			rc_gpio_set_value_mmap(mdir1a, a);
			rc_gpio_set_value_mmap(MDIR1B, b);
			rc_pwm_set_duty_mmap(1, 'A', duty);
			break;
		case 2:
			rc_gpio_set_value_mmap(MDIR2A, b);
			rc_gpio_set_value_mmap(mdir2b, a);
			rc_pwm_set_duty_mmap(1, 'B', duty);
			break;
		case 3:
			rc_gpio_set_value_mmap(MDIR3A, b);
			rc_gpio_set_value_mmap(MDIR3B, a);
			rc_pwm_set_duty_mmap(2, 'A', duty);
			break;
		case 4:
			rc_gpio_set_value_mmap(MDIR4A, a);
			rc_gpio_set_value_mmap(MDIR4B, b);
			rc_pwm_set_duty_mmap(2, 'B', duty);
			break;
		default:
			printf("enter a motor value between 1 and 4\n");
			return -1;
	}
	return 0;
}

/*******************************************************************************
* int rc_set_motor_all(float duty)
*
* applies the same duty cycle argument to all 4 motors
*******************************************************************************/
int rc_set_motor_all(float duty){
	int i;
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor_all before they have been initialized\n");
		return -1;
	}
	for(i=1;i<=MOTOR_CHANNELS; i++){
		rc_set_motor(i, duty);
	}
	return 0;
}

/*******************************************************************************
* int rc_set_motor_free_spin(int motor)
*
* This puts one or all motor outputs in high-impedance state which lets the
* motor spin freely as if it wasn't connected to anything.
*******************************************************************************/
int rc_set_motor_free_spin(int motor){
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor_free_spin before they have been initialized\n");
		return -1;
	}
	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			rc_gpio_set_value_mmap(mdir1a, 0);
			rc_gpio_set_value_mmap(MDIR1B, 0);
			rc_pwm_set_duty_mmap(1, 'A', 0.0);
			break;
		case 2:
			rc_gpio_set_value_mmap(MDIR2A, 0);
			rc_gpio_set_value_mmap(mdir2b, 0);
			rc_pwm_set_duty_mmap(1, 'B', 0.0);
			break;
		case 3:
			rc_gpio_set_value_mmap(MDIR3A, 0);
			rc_gpio_set_value_mmap(MDIR3B, 0);
			rc_pwm_set_duty_mmap(2, 'A', 0.0);
			break;
		case 4:
			rc_gpio_set_value_mmap(MDIR4A, 0);
			rc_gpio_set_value_mmap(MDIR4B, 0);
			rc_pwm_set_duty_mmap(2, 'B', 0.0);
			break;
		default:
			printf("enter a motor value between 1 and 4\n");
			return -1;
	}
	return 0;
}

/*******************************************************************************
* @ int rc_set_motor_free_spin_all()
*******************************************************************************/
int rc_set_motor_free_spin_all(){
	int i;
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor_free_spin_all before they have been initialized\n");
		return -1;
	}
	for(i=1;i<=MOTOR_CHANNELS; i++){
		rc_set_motor_free_spin(i);
	}
	return 0;
}

/*******************************************************************************
* int rc_set_motor_brake(int motor)
*
* These will connect one or all motor terminal pairs together which
* makes the motor fight against its own back EMF turning it into a brake.
*******************************************************************************/
int rc_set_motor_brake(int motor){
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor_brake before they have been initialized\n");
		return -1;
	}
	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			rc_gpio_set_value_mmap(mdir1a, 1);
			rc_gpio_set_value_mmap(MDIR1B, 1);
			rc_pwm_set_duty_mmap(1, 'A', 0.0);
			break;
		case 2:
			rc_gpio_set_value_mmap(MDIR2A, 1);
			rc_gpio_set_value_mmap(mdir2b, 1);
			rc_pwm_set_duty_mmap(1, 'B', 0.0);
			break;
		case 3:
			rc_gpio_set_value_mmap(MDIR3A, 1);
			rc_gpio_set_value_mmap(MDIR3B, 1);
			rc_pwm_set_duty_mmap(2, 'A', 0.0);
			break;
		case 4:
			rc_gpio_set_value_mmap(MDIR4A, 1);
			rc_gpio_set_value_mmap(MDIR4B, 1);
			rc_pwm_set_duty_mmap(2, 'B', 0.0);
			break;
		default:
			printf("enter a motor value between 1 and 4\n");
			return -1;
	}
	return 0;
}

/*******************************************************************************
* @ int rc_set_motor_brake_all()
*******************************************************************************/
int rc_set_motor_brake_all(){
	int i;
	if(init_flag==0){
		fprintf(stderr, "ERROR trying to rc_set_motor_brake_all before they have been initialized\n");
		return -1;
	}
	for(i=1;i<=MOTOR_CHANNELS; i++){
		rc_set_motor_brake(i);
	}
	return 0;
}