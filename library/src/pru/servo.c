/**
 * @file rc_pru.c
 *
 * @author     James Strawson
 * @date       3/7/2018
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <rc/pru.h>
#include <rc/gpio.h>
#include <rc/servo.h>

#define GPIO_POWER_PIN
#define SERVO_PRU_CH	1
#define SERVO_PRU_FW	"am335x-pru1-rc-servo-fw"

// pru shared memory pointer
static unsigned int* shared_mem_32bit_ptr = NULL;

int init_flag=0;

int rc_servo_init()
{
	int i;
	// start pru
	if(rc_pru_start(SERVO_PRU_CH, SERVO_PRU_FW)){
		fprintf(stderr,"ERROR in rc_servo_init, failed to start PRU%d\n", SERVO_PRU_CH);
		return -1;
	}
	// map memory
	shared_mem_32bit_ptr = rc_pru_shared_mem_ptr();
	if(shared_mem_32bit_ptr==NULL){
		fprintf(stderr, "ERROR in rc_servo_init, failed to map shared memory pointer\n");
		init_flag=0;
		return -1;
	}
	// zero out the memory
	for(i=0;i<RC_SERVO_CH_MAX;i++) shared_mem_32bit_ptr[i]=0;
	// start gpio pin
	if(rc_gpio_init(GPIO_POWER_PIN, GPIOHANDLE_REQUEST_OUTPUT)==-1){
		fprintf(stderr, "ERROR in rc_servo_init, failed to set up power rail GPIO pin\n");
		init_flag=0;
		return -1;
	}
	init_flag=1;
	return 0;
}


void rc_servo_cleanup()
{
	// zero out shared memory
	if(shared_mem_32bit_ptr != NULL){
		for(i=0;i<RC_SERVO_CH_MAX;i++) shared_mem_32bit_ptr[i]=0;
	}
	if(init_flag!=0){
		rc_gpio_set_value(GPIO_POWER_PIN,0);
		rc_gpio_cleanup(GPIO_POWER_PIN);
	}
	rc_pru_stop(SERVO_PRU_CH);
	shared_mem_32bit_ptr = NULL;
	init_flag=0;
	return;
}


int rc_servo_power_rail_en(int en)
{
	if(init_flag==0){
		fprintf(stderr, "ERROR in rc_servo_power_rail_en, call rc_servo_init first\n");
		return -1;
	}
	if(rc_gpio_set_value(GPIO_POWER_PIN,en)==-1){
		fprintf(stderr, "ERROR in rc_servo_power_rail_en, failed to write to GPIO pin\n");
		return -1;
	}
	return 0;
}


int rc_servo_send_pulse_us(int ch, int us)
{
	// Sanity Checks
	if(ch<0 || ch>RC_SERVO_CH_MAX){
		fprintf(stderr,"ERROR: in rc_servo_send_pulse_us, channel argument must be between 0&%d\n", RC_SERVO_CH_MAX);
		return -1;
	}
	if(shared_mem_32bit_ptr==NULL){
		printf("ERROR: PRU servo Controller not initialized\n");
		return -2;
	}

	// first check to make sure no pulse is currently being sent
	if(shared_mem_32bit_ptr[ch-1] != 0){
		printf("WARNING: Tried to start a new pulse amidst another\n");
		return -1;
	}

	// PRU runs at 200Mhz. find #loops needed
	unsigned int num_loops = ((us*200.0)/PRU_SERVO_LOOP_INSTRUCTIONS);
	// write to PRU shared memory
	shared_mem_32bit_ptr[ch-1] = num_loops;
	return 0;
}


int rc_send_servo_pulse_us_all(int us){
	int i, ret_ch;
	int ret = 0;
	for(i=1;i<=RC_SERVO_CH_MAX; i++){
		ret_ch = rc_send_servo_pulse_us(i, us);
		if(ret_ch == -2) return -2;
		else if(ret_ch == -1) ret=-1;
	}
	return ret;
}

int rc_send_servo_pulse_normalized(int ch, float input){
	if(ch<1 || ch>RC_SERVO_CH_MAX){
		printf("ERROR: Servo Channel must be between 1&%d\n", RC_SERVO_CH_MAX);
		return -1;
	}
	if(input<-1.5 || input>1.5){
		printf("ERROR: normalized input must be between -1 & 1\n");
		return -1;
	}
	float micros = SERVO_MID_US + (input*(SERVO_NORMAL_RANGE/2));
	return rc_send_servo_pulse_us(ch, micros);
}


int rc_send_servo_pulse_normalized_all(float input){
	int i, ret_ch;
	int ret = 0;
	for(i=1;i<=RC_SERVO_CH_MAX; i++){
		ret_ch = rc_send_servo_pulse_normalized(i, input);
		if(ret_ch == -2) return -2;
		else if(ret_ch == -1) ret=-1;
	}
	return ret;
}


int rc_send_esc_pulse_normalized(int ch, float input){
	if(ch < 1 || ch > RC_SERVO_CH_MAX){
		printf("ERROR: Servo Channel must be between 1&%d\n", RC_SERVO_CH_MAX);
		return -1;
	}
	if(input < -0.1 || input > 1.0){
		printf("ERROR: normalized input must be between 0 & 1\n");
		return -1;
	}
	float micros = 1000.0 + (input*1000.0);
	return rc_send_servo_pulse_us(ch, micros);
}


int rc_send_esc_pulse_normalized_all(float input){
	int i, ret_ch;
	int ret = 0;
	for(i=1;i<=RC_SERVO_CH_MAX; i++){
		ret_ch = rc_send_esc_pulse_normalized(i, input);
		if(ret_ch == -2) return -2;
		else if(ret_ch == -1) ret=-1;
	}
	return ret;
}



int rc_send_oneshot_pulse_normalized(int ch, float input){
	if(ch < 1 || ch > RC_SERVO_CH_MAX){
		printf("ERROR: Servo Channel must be between 1&%d\n", RC_SERVO_CH_MAX);
		return -1;
	}
	if(input < -0.1 || input > 1.0){
		printf("ERROR: normalized input must be between 0 & 1\n");
		return -1;
	}
	float micros = 125.0 + (input*125.0);
	return rc_send_servo_pulse_us(ch, micros);
}


int rc_send_oneshot_pulse_normalized_all(float input){
	int i, ret_ch;
	int ret = 0;
	for(i=1;i<=RC_SERVO_CH_MAX; i++){
		ret_ch = rc_send_oneshot_pulse_normalized(i, input);
		if(ret_ch == -2) return -2;
		else if(ret_ch == -1) ret=-1;
	}
	return ret;
}

/*

int get_pru_encoder_pos(){
	if(shared_mem_32bit_ptr == NULL) return -1;
	else return (int) shared_mem_32bit_ptr[CNT_OFFSET/4];
}


int set_pru_encoder_pos(int val){
	if(shared_mem_32bit_ptr == NULL) return -1;
	else shared_mem_32bit_ptr[CNT_OFFSET/4] = val;
	return 0;
}
*/