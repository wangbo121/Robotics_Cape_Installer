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

#define MOTOR_CHANNELS		4
#define PWM_FREQ		25000	// 25kHz

static int mdir1a, mdir2b; // variable gpio pin assignments
static int init_flag = 0;
static int stby_state = 0;


static int set_pins(int motor, float duty, char a, char b)
{
	int ret;
	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			ret|=rc_gpio_set_value(mdir1a, a);
			ret|=rc_gpio_set_value(MDIR1B, b);
			ret|=rc_pwm_set_duty(1, 'A', duty);
			break;
		case 2:
			ret|=rc_gpio_set_value(MDIR2A, b);
			ret|=rc_gpio_set_value(mdir2b, a);
			ret|=rc_pwm_set_duty(1, 'B', duty);
			break;
		case 3:
			ret|=rc_gpio_set_value(MDIR3A, b);
			ret|=rc_gpio_set_value(MDIR3B, a);
			ret|=rc_pwm_set_duty(2, 'A', duty);
			break;
		case 4:
			ret|=rc_gpio_set_value(MDIR4A, a);
			ret|=rc_gpio_set_value(MDIR4B, b);
			ret|=rc_pwm_set_duty(2, 'B', duty);
			break;
		default:
			fprintf(stderr,"ERROR in rc_motor, motor must be between 1 and 4\n");
			return -1;
	}
	return ret;
}



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

	// set up pwm channels
	if(unlikely(rc_pwm_init(1,PWM_FREQ))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to initialize pwm subsystem 1\n");
		return -1;
	}
	if(unlikely(rc_pwm_init(2,PWM_FREQ))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to initialize pwm subsystem 2\n");
		return -1;
	}

	// set up gpio
	if(unlikely(rc_gpio_init(MOT_STBY, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MOT_STBY);
		return -1;
	}
	if(unlikely(rc_gpio_init(mdir1a, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", mdir1a);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR1B, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR1B);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR2A, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR2A);
		return -1;
	}
	if(unlikely(rc_gpio_init(mdir2b, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", mdir2b);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR3A, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR3A);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR3B, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR3B);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR4A, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR4A);
		return -1;
	}
	if(unlikely(rc_gpio_init(MDIR4B, GPIO_HANDLE_REQUEST_OUTPUT))){
		fprintf(stderr,"ERROR in rc_motor_init, failed to set up gpio %d\n", MDIR4B);
		return -1;
	}

	// now set all the gpio pins and pwm to something predictable
	if(unlikely(rc_set_motor_free_spin_all())){
		fprintf(stderr,"ERROR in rc_motor_init\n");
		return -1;
	}

	// make sure standby is off since most users won't use it
	if(unlikely(rc_gpio_set_value(MOT_STBY,1))){
		fprintf(stderr,"ERROR in rc_motor_init, can't write to gpio %d\n",MOT_STBY);
		return -1;
	}
	stby_state = 0;
	init_flag = 1;
	return 0;
}



int rc_motor_cleanup()
{
	if(!init_flag) return 0;
	if(rc_set_motor_free_spin_all()){
		fprintf(stderr,"ERROR in rc_motor_cleanup\n");
		init_flag;
		return -1;
	}
	rc_pwm_cleanup(1);
	rc_pwm_cleanup(2);
	rc_gpio_cleanup(MOT_STBY);
	rc_gpio_cleanup(mdir1a);
	rc_gpio_cleanup(MDIR1B);
	rc_gpio_cleanup(mdir2b);
	rc_gpio_cleanup(MDIR2B);
	rc_gpio_cleanup(MDIR3A);
	rc_gpio_cleanup(MDIR3B);
	rc_gpio_cleanup(MDIR4A);
	rc_gpio_cleanup(MDIR4B);
	return 0;
}


int rc_motor_standby(int standby_en)
{
	int val;
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in rc_motor_standby, must call rc_motor_init first\n");
		return -1;
	}
	// if use is requesting standby
	if(standby_en){
		// return if already in standby
		if(stby_state) return 0;
		val=0;
		rc_set_motor_free_spin_all();
	}
	else{
		if(!stby_state) return 0;
		val=1;
	}
	if(unlikely(rc_gpio_set_value(MOT_STBY,val))){
		fprintf(stderr,"ERROR in rc_motor_standby, unable to write to gpio %d\n", MOT_STBY);
		return -1;
	}
	stby_state = standby_en;
	return 0;
}


int rc_set_motor(int motor, float duty){
	int a,b;
	int ret=0;
	if(unlikely(init_flag==0)){
		fprintf(stderr, "ERROR in rc_set_motor, call rc_motor_init first\n");
		return -1;
	}
	if(unlikely(stby_state)){
		fpirntf(stderr,"ERROR in rc_set_motor, motors are currently in standby mode\n");
		return -1;
	}
	//check that the duty cycle is within +-1
	if (duty>1.0f)	duty = 1.0f;
	else if(duty<-1.0f) duty=-1.0f;

	//switch the direction pins to H-bridge
	if (duty>=0){
		a=1;
		b=0;
	}
	else{
		a=0;
		b=1;
		duty=-duty;
	}


	return 0;
}



int rc_motor_set_all(float duty){
	int i;
	for(i=1;i<=MOTOR_CHANNELS; i++){
		if(rc_set_motor(i, duty)) return -1;
	}
	return 0;
}



int rc_motor_free_spin(int motor){
	int ret=0;
	if(unlikely(init_flag==0)){
		fprintf(stderr, "ERROR in rc_motor_free_spin, call rc_motor_init first\n");
		return -1;
	}
	if(unlikely(stby_state)){
		fpirntf(stderr,"ERROR in rc_motor_free_spin, motors are currently in standby mode\n");
		return -1;
	}
	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			ret|=rc_gpio_set_value(mdir1a, 0);
			ret|=rc_gpio_set_value(MDIR1B, 0);
			ret|=rc_pwm_set_duty(1, 'A', 0.0);
			break;
		case 2:
			ret|=rc_gpio_set_value(MDIR2A, 0);
			ret|=rc_gpio_set_value(mdir2b, 0);
			ret|=rc_pwm_set_duty(1, 'B', 0.0);
			break;
		case 3:
			ret|=rc_gpio_set_value(MDIR3A, 0);
			ret|=rc_gpio_set_value(MDIR3B, 0);
			ret|=rc_pwm_set_duty(2, 'A', 0.0);
			break;
		case 4:
			ret|=rc_gpio_set_value(MDIR4A, 0);
			ret|=rc_gpio_set_value(MDIR4B, 0);
			ret|=rc_pwm_set_duty(2, 'B', 0.0);
			break;
		default:
			fprintf(stderr,"ERROR in rc_motor_free_spin, motor must be between 1 and 4\n");
			return -1;
	}
	if(ret){
		fprintf(stderr,"ERROR in rc_motor_free_spin\n");
		return -1;
	}
	return 0;
}


int rc_set_motor_free_spin_all(){
	int i;
	for(i=1;i<=MOTOR_CHANNELS; i++){
		if(rc_motor_free_spin(i)) return -1;
	}
	return 0;
}


int rc_motor_brake(int motor){
	int ret=0;
	if(unlikely(init_flag==0)){
		fprintf(stderr, "ERROR in rc_motor_brake, call rc_motor_init first\n");
		return -1;
	}
	if(unlikely(stby_state)){
		fpirntf(stderr,"ERROR in rc_motor_brake, motors are currently in standby mode\n");
		return -1;
	}
	// set gpio direction outputs & duty
	switch(motor){
		case 1:
			ret!=rc_gpio_set_value(mdir1a, 1);
			rc_gpio_set_value(MDIR1B, 1);
			rc_pwm_set_duty(1, 'A', 0.0);
			break;
		case 2:
			rc_gpio_set_value(MDIR2A, 1);
			rc_gpio_set_value(mdir2b, 1);
			rc_pwm_set_duty(1, 'B', 0.0);
			break;
		case 3:
			rc_gpio_set_value(MDIR3A, 1);
			rc_gpio_set_value(MDIR3B, 1);
			rc_pwm_set_duty(2, 'A', 0.0);
			break;
		case 4:
			rc_gpio_set_value(MDIR4A, 1);
			rc_gpio_set_value(MDIR4B, 1);
			rc_pwm_set_duty(2, 'B', 0.0);
			break;
		default:
			fprintf(stderr,"ERROR in rc_motor_brake, motor must be between 1 and 4\n");
			return -1;
	}
	return 0;
}


int rc_motor_brake_all()
{
	int i;
	for(i=1;i<=MOTOR_CHANNELS; i++){
		if(rc_motor_brake(i)) return -1;
	}
	return 0;
}