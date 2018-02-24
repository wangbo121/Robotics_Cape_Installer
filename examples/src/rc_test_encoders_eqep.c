/*******************************************************************************
* rc_test_encoders.c
*
* Prints out current encoder ticks for all 4 channels
* channels 1-3 are counted using eQEP 0-2. Channel 4 is counted by PRU0
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <rc/encoder_eqep.h>
#include <rc/time.h>

int running;

// interrupt handler to catch ctrl-c
void signal_handler(__attribute__ ((unused)) int dummy)
{
	running=0;
	return;
}

int main()
{
	int i;

	// initialize hardware first
	if(rc_encoder_eqep_init()){
		fprintf(stderr,"ERROR: failed to run rc_encoder_eqep_init\n");
		return -1;
	}

	// set signal handler so the loop can exit cleanly
	signal(SIGINT, signal_handler);
	running=1;

	printf("\nRaw encoder positions\n");
	printf("      E1   |");
	printf("      E2   |");
	printf("      E3   |");
	printf(" \n");

	while(running){
		printf("\r");
		for(i=1;i<=3;i++){
			printf("%10d |", rc_encoder_eqep_read(i));
		}
		fflush(stdout);
		rc_usleep(50000);
	}
	printf("\n");

	rc_encoder_eqep_cleanup();
	return 0;
}

