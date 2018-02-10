/*******************************************************************************
* ADC
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#include "rc/io/adc.h"
#include "rc/time.h"
#include "rc/preprocessor_macros.h"
#include "adc_registers.h"

#define DC_JACK_OFFSET -0.15
#define LIPO_OFFSET -0.10
#define LIPO_ADC_CH 6
#define DC_JACK_ADC_CH  5
#define V_DIV_RATIO 11.0

static volatile uint32_t *map; // pointer to /dev/mem
static int adc_initialized_flag = 0; // boolean to check if mem mapped


/*******************************************************************************
* int rc_adc_init()
*
* Initialize the Analog-Digital Converter
* each channel is set up in software one-shot mode for general purpose reading
* internal averaging set to 8 samples to reduce noise
*******************************************************************************/
int rc_adc_init()
{
	if(adc_initialized_flag) return 0;
	int fd = open("/dev/mem", O_RDWR);
	errno=0;
	if(unlikely(fd==-1)){
		fprintf(stderr,"ERROR: in rc_init_adc,Unable to open /dev/mem\n");
		if(unlikely(errno==EPERM)) fprintf(stderr,"Insufficient privileges\n");
		return -1;
	}
	map = (uint32_t*)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,\
								fd, MMAP_OFFSET);
	if(map==MAP_FAILED){
		close(fd);
		fprintf(stderr,"ERROR: in rc_init_adc, Unable to map /dev/mem\n");
		return -1;
	}

	// enable the CM_WKUP_ADC_TSC_CLKCTRL with CM_WKUP_MODUELEMODE_ENABLE
	#ifdef DEBUG
	printf("Enabling Clock to ADC_TSC\n");
	#endif
	map[(CM_WKUP+CM_WKUP_ADC_TSC_CLKCTRL-MMAP_OFFSET)/4] |= MODULEMODE_ENABLE;

	// waiting for adc clock module to initialize
	#ifdef DEBUG
	printf("Checking clock signal has started\n");
	#endif
	while(!(map[(CM_WKUP+CM_WKUP_ADC_TSC_CLKCTRL-MMAP_OFFSET)/4] & MODULEMODE_ENABLE)){
		rc_usleep(10);
		#ifdef DEBUG
		printf("Waiting for CM_WKUP_ADC_TSC_CLKCTRL to enable with MODULEMODE_ENABLE\n");
		#endif

	}
	// temporarily disable adc to configure registers
	#ifdef DEBUG
	printf("Temporarily Disabling ADC\n");
	#endif
	map[(ADC_CTRL-MMAP_OFFSET)/4] &= !0x01;

	// make sure STEPCONFIG write protect is off
	map[(ADC_CTRL-MMAP_OFFSET)/4] |= ADC_STEPCONFIG_WRITE_PROTECT_OFF;

	// set up each ADCSTEPCONFIG for each ain pin
	map[(ADCSTEPCONFIG1-MMAP_OFFSET)/4] = 0x00<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY1-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG2-MMAP_OFFSET)/4] = 0x01<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY2-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG3-MMAP_OFFSET)/4] = 0x02<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY3-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG4-MMAP_OFFSET)/4] = 0x03<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY4-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG5-MMAP_OFFSET)/4] = 0x04<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY5-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG6-MMAP_OFFSET)/4] = 0x05<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY6-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG7-MMAP_OFFSET)/4] = 0x06<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY7-MMAP_OFFSET)/4]  = 0<<24;
	map[(ADCSTEPCONFIG8-MMAP_OFFSET)/4] = 0x07<<19 | ADC_AVG8 | ADC_SW_ONESHOT;
	map[(ADCSTEPDELAY8-MMAP_OFFSET)/4]  = 0<<24;

	// enable the ADC
	map[(ADC_CTRL-MMAP_OFFSET)/4] |= 0x01;

	// clear the FIFO buffer
	int output;
	while(map[(FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK){
		output =  map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
	}
	// just suppress the warning about output not being used which will
	// happen in the FIFO buffer is empty already causing the above
	// while loop to exit immediately
	if(output){}

	adc_initialized_flag = 1;
	return 0;
}

/*******************************************************************************
* int mmap_adc_read_raw(int ch)
*
* Read in from an analog pin with oneshot mode
*******************************************************************************/
int mmap_adc_read_raw(int ch)
{
	int output;
	// clear the FIFO buffer just in case it's not empty
	while(map[(FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK){
		output =  map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
	}
	// enable step for the right pin
	map[(ADC_STEPENABLE-MMAP_OFFSET)/4] |= (0x01<<(ch+1));
	// wait for sample to appear in the FIFO buffer
	while(!(map[(FIFO0COUNT-MMAP_OFFSET)/4] & FIFO_COUNT_MASK)){}
	// return the the FIFO0 data register
	output =  map[(ADC_FIFO0DATA-MMAP_OFFSET)/4] & ADC_FIFO_MASK;
	return output;
}

/*******************************************************************************
* float rc_battery_voltage()
*
* returns the LiPo battery voltage on the robotics cape
* this accounts for the voltage divider ont he cape
*******************************************************************************/
float rc_battery_voltage()
{
	float v = (rc_adc_volt(LIPO_ADC_CH)*V_DIV_RATIO)+LIPO_OFFSET;
	if(v<0.3) v = 0.0;
	return v;
}

/*******************************************************************************
* float rc_dc_jack_voltage()
*
* returns the DC power jack voltage on the robotics cape
* this accounts for the voltage divider ont he cape
*******************************************************************************/
float rc_dc_jack_voltage()
{
	float v = (rc_adc_volt(DC_JACK_ADC_CH)*V_DIV_RATIO)+DC_JACK_OFFSET;
	if(v<0.3) v = 0.0;
	return v;
}

/*******************************************************************************
* int rc_adc_raw(int ch)
*
* returns the raw adc reading
*******************************************************************************/
int rc_adc_raw(int ch)
{
	if (ch < 0 || ch > 6) {
		fprintf(stderr, "ERROR: analog pin must be in 0-6\n");
		return -1;
	}
	return mmap_adc_read_raw((uint8_t)ch);
}

/*******************************************************************************
* float rc_adc_volt(int ch)
*
* returns an actual voltage for an adc channel
*******************************************************************************/
float rc_adc_volt(int ch)
{
	if (ch < 0 || ch > 6) {
		fprintf(stderr, "ERROR: analog pin must be in 0-6\n");
		return -1;
	}
	int raw_adc = mmap_adc_read_raw((uint8_t)ch);
	return raw_adc * 1.8 / 4095.0;
}