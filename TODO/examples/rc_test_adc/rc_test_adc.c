/*******************************************************************************
* rc_test_adc.c
*
* James Strawson 2016
* prints voltages read by all adc channels
*******************************************************************************/

#include <stdio.h>
#include "rc/io/adc.h"
#include "rc/time.h"
#include "rc/flow.h"


int main(){
	int i;
	// start signal handler so we can exit cleanly
	if(rc_enable_signal_handler()<0){
		fprintf(stderr,"ERROR: failed to complete rc_enable_signal_handler\n");
		return -1;
	}
	// initialize hardware first
	if(rc_adc_init()){
		fprintf(stderr,"ERROR: failed to run rc_init_adc(), are you root?\n");
		return -1;
	}
	// finished initializing so set state to RUNNING
	rc_set_state(RUNNING);

	printf(" adc_0 |");
	printf(" adc_1 |");
	printf(" adc_2 |");
	printf(" adc_3 |");
	printf("DC_Jack|");
	printf("Battery|");
	printf("\n");

	// run untill the signal handler sets state to EXITING
	while(rc_get_state()!=EXITING){
		printf("\r");
		//print all channels
		for(i=0;i<4;i++){
			printf("%6.2f |", rc_adc_volt(i));
		}
		printf("%6.2f |", rc_dc_jack_voltage());
		printf("%6.2f |", rc_battery_voltage());
		fflush(stdout);
		//rc_usleep(100000);
	}


	return 0;
}
