/**
 * @file gpio_mmap_sitara.h
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include "rc/gpio_mmap_sitara.h"
#include "sitara_gpio_registers.h"

// preposessor macros
#define unlikely(x)	__builtin_expect (!!(x), 0)
#define likely(x)	__builtin_expect (!!(x), 1)

#define MAX_BUF 64
#define NUM_PINS 128


// mmap variables
static volatile uint32_t *map; // pointer to /dev/mem
static int mmap_initialized_flag = 0; // boolean to check if mem mapped


/*******************************************************************************
* by default gpio subsystems 0,1,&2 have clock signals on boot
* this function exports a pin from the last subsystem (113) so the
* gpio driver enables the clock signal so the rest of the gpio
* functions here work.
*******************************************************************************/
int rc_gpio_mmap_init()
{
	// return immediately if gpio already initialized
	if(mmap_initialized_flag) return 0;

	// open dev/mem
	errno=0;
	int fd = open("/dev/mem", O_RDWR);
	if(unlikely(fd==-1)){
		perror("ERROR: in rc_gpio_mmap_init, Unable to open /dev/mem");
		return -1;
	}
	map = (uint32_t*)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,\
								fd, MMAP_OFFSET);
	if(map == MAP_FAILED){
		perror("ERROR: in rc_gpio_mmap_init, Unable to map /dev/mem\n");
		close(fd);
		return -1;
	}

	// now we must enable clock signal to the gpio subsystems
	// first open the clock register to see if the PWMSS has clock enabled
	int clock_fd;
	clock_fd = open("/dev/mem", O_RDWR);
	if(clock_fd == -1) {
		perror("ERROR: in rc_gpio_mmap_init,Unable to open /dev/mem");
		return -1;
	}
	volatile char *cm_per_base;
	cm_per_base=mmap(0,CM_PER_PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,\
								clock_fd,CM_PER);
	if(cm_per_base == (void *) -1){
		perror("ERROR: in rc_gpio_mmap_init, Unable to map /dev/mem");
		close(fd);
		close(clock_fd);
		return -1;
	}

	#ifdef DEBUG
	fprintf(stderr,"turning on cm-per module_enable bits for gpio 1,2,3\n");
	#endif
	*(uint16_t*)(cm_per_base + CM_PER_GPIO1_CLKCTRL) |= MODULEMODE_ENABLE;
	*(uint16_t*)(cm_per_base + CM_PER_GPIO2_CLKCTRL) |= MODULEMODE_ENABLE;
	*(uint16_t*)(cm_per_base + CM_PER_GPIO3_CLKCTRL) |= MODULEMODE_ENABLE;

	#ifdef DEBUG
	fprintf(stderr,"new cm_per_gpio3_clkctrl: %d\n", *(uint16_t*)(cm_per_base+CM_PER_GPIO3_CLKCTRL));
	#endif
	// done setting clocks, close map
	close(clock_fd);
	mmap_initialized_flag=1;
	return 0;
}


/*******************************************************************************
* write HIGH or LOW to a pin
* pinmux must already be configured for output
*******************************************************************************/
int rc_gpio_mmap_set_value(int pin, int state)
{
	int bank = pin/32;
	int id = pin - bank*32;
	int bank_offset;

	// sanity checks
	if(unlikely(!mmap_initialized_flag)){
		fprintf(stderr,"ERROR in rc_gpio_mmap_set_value, must initialize first\n");
		return -1;
	}
	if(unlikely(pin<0 || pin>128)){
		fprintf(stderr,"ERROR: in rc_gpio_mmap_set_value, invalid gpio pin\n");
		return -1;
	}
	// pick correct bank to write to
	switch(bank){
		case 0:
			bank_offset=GPIO0;
			break;
		case 1:
			bank_offset=GPIO1;
			break;
		case 2:
			bank_offset=GPIO2;
			break;
		case 3:
			bank_offset=GPIO3;
			break;
		default:
			return -1;
	}
	// write the correct bit
	if(state) map[(bank_offset-MMAP_OFFSET+GPIO_DATAOUT)/4] |= (1<<id);
	else map[(bank_offset-MMAP_OFFSET+GPIO_DATAOUT)/4] &= ~(1<<id);
	return 0;
}

/*******************************************************************************
* returns 1 or 0 for HIGH or LOW
* pinMUX must already be configured for input
*******************************************************************************/
int rc_gpio_mmap_get_value(int pin)
{
	int bank = pin/32;
	int id = pin - bank*32;
	int bank_offset;

	// sanity checks
	if(unlikely(!mmap_initialized_flag)){
		fprintf(stderr,"ERROR in rc_gpio_mmap_get_value, must initialize first\n");
		return -1;
	}
	if(pin<0 || pin>128){
		fprintf(stderr,"ERROR: in rc_gpio_get_value_mmap(),invalid gpio pin\n");
		return -1;
	}
	// pick correct bank to write to
	switch(bank){
		case 0:
			bank_offset=GPIO0;
			break;
		case 1:
			bank_offset=GPIO1;
			break;
		case 2:
			bank_offset=GPIO2;
			break;
		case 3:
			bank_offset=GPIO3;
			break;
		default:
			return -1;
	}
	//pick right bit and return
	return (map[(bank_offset-MMAP_OFFSET+GPIO_DATAIN)/4] & (1<<id))>>id;
}