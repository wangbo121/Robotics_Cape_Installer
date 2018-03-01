/**
 * @example    rc_test_gpio.c
 *  this is an example of how to use the gpio routines in <rc/gpio.h>
 *
 * Optionally provide the pin number as an argument.
 */




#include <stdio.h>
#include <stdlib.h> // for atoi and 'system'
#include <unistd.h> // for sleep

#include <rc/gpio.h>

#define PIN 68

int main(int argc, char *argv[])
{
	int pin,val;
	if(argc==1) pin=PIN;
	else if(argc==2) pin=atoi(argv[1]);
	else{
		fprintf(stderr, "too many arguments\n");
		return -1;
	}

	//rc_gpio_init_output(pin);

	rc_gpio_init_input(pin);

	while(1){
		val=rc_gpio_get_value(pin);
		printf("line is %d\n", val);
	}

	return 0;

}
