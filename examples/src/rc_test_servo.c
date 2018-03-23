/**
 * @file rc_test_servo.c
 * @example    rc_test_servo
 *
 *
 * Demonstrates use of pru to control servos. This program
 * operates in 4 different modes. See the option list below for how to select an
 * operational mode from the command line.
 *
 * SERVO: uses rc_send_servo_pulse_normalized() to set one or all servo
 * positions to a value from -1.5 to 1.5 corresponding to their extended range.
 * -1 to 1 is considered the "safe" normal range as some servos will not go
 * beyond this. Test your servos incrementally to find their safe range.
 *
 * ESC: For unidirectional brushless motor speed controllers specify a range
 * from 0 to 1 as opposed to the bidirectional servo range. Be sure to run the
 * calibrate_esc example first to make sure the ESCs are calibrated to the right
 * pulse range. This mode uses the rc_send_esc_pulse_normalized() function.
 *
 * MICROSECONDS: You can also specify your own pulse width in microseconds (us).
 * This uses the rc_send_servo_pulse_us() function.
 *
 * SWEEP: This is intended to gently sweep a servo back and forth about the
 * center position. Specify a range limit as a command line argument as
 * described below. This also uses the rc_send_servo_pulse_normalized()
 * function.
 *
 * SERVO POWER RAIL: The robotics cape has a software-controlled 6V power
 * regulator allowing controlled steady power to drive servos. This can be
 * enabled at the command line with the -v option. It will not allow you to
 * enable the power rail when using the ESC mode as sending 6V into an ESC may
 * damage it. It is best to physically cut the center wire on ESC connections as
 * the BEC function is not needed when using the Robotics Cape.
 *
 *
 * @author     James Strawson
 * @date       3/20/2018
 */

#include <stdio.h>
#include <rc/time.h>
#include <rc/adc.h>
#include <rc/servo.h>

int running;

typedef enum test_mode_t{
	DISABLED,
	NORM,
	MICROSECONDS,
	SWEEP,
	RADIO
}test_mode_t;


void print_usage()
{
	printf("\n");
	printf(" Options\n");
	printf(" -c {channel}   Specify one channel from 1-8.\n");
	printf("                Otherwise all channels will be driven equally\n");
	printf(" -f {hz}        Specify pulse frequency, otherwise 50hz is used\n");
	printf(" -p {position}  Drive servo to a position between -1.5 & 1.5\n");
	printf(" -u {width_us}  Send pulse width in microseconds (us)\n");
	printf(" -s {limit}     Sweep servo back/forth between +- limit\n");
	printf("                Limit can be between 0 & 1.5\n");
	printf(" -r             Use DSM radio input to set position\n");
	printf(" -h             Print this help messege \n\n");
	printf("sample use to center servo channel 1:\n");
	printf("   rc_test_servo -c 1 -p 0.0\n\n");
}

// interrupt handler to catch ctrl-c
void signal_handler(__attribute__ ((unused)) int dummy)
{
	running=0;
	return;
}

int main(int argc, char *argv[])
{
	float servo_pos=0;
	float sweep_limit=0;
	int width_us=0;
	int ch=0;	// channel to test, 0 means all channels
	float direction = 1; // switches between 1 & -1 in sweep mode
	int c;
	test_mode_t mode = DISABLED; //start mode disabled
	int frequency_hz = 50; // default 50hz frequency to send pulses
	int toggle = 0;
	int i;

	// parse arguments
	opterr = 0;
	while ((c = getopt(argc, argv, "c:f:vrp:e:u:s:h")) != -1){
		switch (c){
		// channel option
		case 'c':
			ch = atoi(optarg);
			if(ch<RC_SERVO_CH_MIN || ch>RC_SERVO_CH_MAX){
				fprintf(stderr,"ERROR channel option must be between %d and %d\n", RC_SERVO_CH_MIN, RC_SERVO_CH_MAX);
				return -1;
			}
			break;

		// pulse frequency option
		case 'f':
			frequency_hz = atoi(optarg);
			if(frequency_hz<1){
				fprintf(stderr,"Frequency option must be >=1\n");
				return -1;
			}
			break;

		// power rail option
		case 'v':
			power_en=1;
			break;

		// position option
		case 'p':
			// make sure only one mode in requested
			if(mode!=DISABLED){
				print_usage();
				return -1;
			}
			servo_pos = atof(optarg);
			if(servo_pos>1.5 || servo_pos<-1.5){
				fprintf(stderr,"Servo position must be from -1.5 to 1.5\n");
				return -1;
			}
			mode = SERVO;
			break;

		// esc throttle option
		case 'e':
			// make sure only one mode in requested
			if(mode!=DISABLED){
				print_usage();
				return -1;
			}
			esc_throttle = atof(optarg);
			if(esc_throttle>1.0 || esc_throttle<0){
				fprintf(stderr,"ESC throttle must be from 0 to 1\n");
				return -1;
			}
			mode = ESC;
			break;

		// width in microsecons option
		case 'u':
			// make sure only one mode in requested
			if(mode!=DISABLED){
				print_usage();
				return -1;
			}
			width_us = atof(optarg);
			if(width_us<10){
				printf("ERROR: Width in microseconds must be >10\n");
				return -1;
			}
			mode = MICROSECONDS;
			break;

		// sweep mode option
		case 's':
			// make sure only one mode in requested
			if(mode!=DISABLED){
				print_usage();
				return -1;
			}
			sweep_limit = atof(optarg);
			if(sweep_limit>1.5 || sweep_limit<-1.5){
				fprintf(stderr,"ERROR: Sweep limit must be from -1.5 to 1.5\n");
				return -1;
			}
			mode = SWEEP;
			servo_pos = 0.0;
			break;

		// radio mode option
		case 'r':
			// make sure only one mode in requested
			if(mode!=DISABLED){
				print_usage();
				return -1;
			}
			if(rc_dsm_init_dsm()==-1) return -1;
			mode = RADIO;
			break;

		// help mode
		case 'h':
			print_usage();
			return 0;

		default:
			printf("\nInvalid Argument \n");
			print_usage();
			return -1;
		}
	}

	// if the user didn't give enough arguments, exit
	if(mode==DISABLED){
		fprintf(stderr,"\nNot enough input arguments\n");
		print_usage();
		return -1;
	}

	// set signal handler so the loop can exit cleanly
	signal(SIGINT, signal_handler);
	running=1;

	// read adc to make sure battery is connected
	if(rc_adc_init()){
		fprintf(stderr,"ERROR: failed to run rc_init_adc()\n");
		return -1;
	}
	if(rc_adc_battery_volt()<6.0){
		fprintf(stderr,"ERROR: battery disconnected or insufficently charged to drive servos\n");
		return -1;
	}

	// initialize PRU
	if(rc_servo_init()) return -1;

	// turn on power if option was given
	if(power_en){
		printf("Turning On 6V Servo Power Rail\n");
		rc_servo_power_rail_en(1);
	}

	// print out what the program is doing
	printf("\n");
	if(ch==0) printf("Sending on all channels.\n");
	else	  printf("Sending only to channel %d.\n", ch);
	switch(mode){
	case SERVO:
		printf("Using rc_servo_send_pulse_normalized\n");
		printf("Normalized Signal: %f  Pulse Frequency: %d\n", servo_pos, frequency_hz);
		break;
	case ESC:
		printf("Using rc_servo_send_esc_pulse_normalized\n");
		printf("Normalized Signal: %f  Pulse Frequency: %d\n", esc_throttle, frequency_hz);
		break;
	case MICROSECONDS:
		printf("Using rc_servo_send_pulse_microseconds\n");
		printf("Pulse_width: %d  Pulse Frequency: %d\n", width_us, frequency_hz);
		break;
	case SWEEP:
		printf("Sweeping servos back/forth between +-%f\n", sweep_limit);
		printf("Pulse Frequency: %d\n", frequency_hz);
		break;
	case RADIO:
		printf("Listening for DSM radio signal\n");
		printf("Pulse Frequency: %d\n", frequency_hz);
		break;
	default:
		fprintf(stderr,"ERROR invalid mode enum\n");
		return -1;
	}

	if(mode==RADIO){
		printf("Waiting for first DSM packet");
		fflush(stdout);
		while(rc_dsm_is_new_data()==0){
			if(rc_get_state()==EXITING) return 0;
			rc_usleep(50000);
		}
	}

	// if driving an ESC, send throttle of 0 first
	// otherwise it will go into calibration mode
	if(mode==ESC || mode==RADIO){
		printf("waking ESC up from idle\n");
		for(i=0;i<frequency*3;i++){
			rc_servo_send_esc_pulse_normalized(ch,0);
			rc_usleep(frequency_hz/1000000);
		}
	}

	// Main loop runs at frequency_hz
	while(rc_get_state()!=EXITING){
		switch(mode){

		case SERVO:
			if(all) rc_send_servo_pulse_normalized_all(servo_pos);
			else rc_send_servo_pulse_normalized(ch, servo_pos);
			break;

		case ESC:
			if(all) rc_send_esc_pulse_normalized_all(esc_throttle);
			else rc_send_esc_pulse_normalized(ch, esc_throttle);
			break;

		case RADIO:
			if(rc_is_new_dsm_data()) {
				printf("\r");// keep printing on same line
				int channels = rc_num_dsm_channels();
				// print framerate
				printf("%d/", rc_get_dsm_resolution());
				// print num channels in use
				printf("%d-ch ", channels);
				//print all channels
				for(i=0;i<channels;i++) {
					printf("%d:% 0.2f ", i+1, rc_get_dsm_ch_normalized(i+1));
				}
				fflush(stdout);
				esc_throttle = (rc_get_dsm_ch_normalized(1) + 1.0) / 2.0;
			} else {
				printf("\rSeconds since last DSM packet: ");
				printf("%lld ", rc_nanos_since_last_dsm_packet()/1000000000);
				printf("                             ");
				if(rc_nanos_since_last_dsm_packet() > 200000000) {
					esc_throttle = 0;
				}
			}
			fflush(stdout);
			if(all) rc_send_esc_pulse_normalized_all(esc_throttle);
			else rc_send_esc_pulse_normalized(ch, esc_throttle);
			break;


		case MICROSECONDS:
			if(all) rc_send_servo_pulse_us_all(width_us);
			else rc_send_servo_pulse_us(ch, width_us);
			break;

		case SWEEP:
			// increase or decrease position each loop
			// scale with frequency
			servo_pos += direction * sweep_limit / frequency_hz;

			// reset pulse width at end of sweep
			if(servo_pos>sweep_limit){
				servo_pos = sweep_limit;
				direction = -1;
			}
			else if(servo_pos < (-sweep_limit)){
				servo_pos = -sweep_limit;
				direction = 1;
			}
			// send result
			if(all) rc_send_servo_pulse_normalized_all(servo_pos);
			else rc_send_servo_pulse_normalized(ch, servo_pos);
			break;

		default:
			rc_set_state(EXITING); //should never actually get here
			break;
		}

		// blink green led
		rc_set_led(GREEN, toggle);
		toggle = !toggle;

		// sleep roughly enough to maintain frequency_hz
		rc_usleep(1000000/frequency_hz);
	}

	rc_cleanup();
	return 0;
}

