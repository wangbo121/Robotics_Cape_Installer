/*******************************************************************************
* CPU Frequency Control
*
* @ int rc_set_cpu_freq(rc_cpu_freq_t freq)
*
* Sets the CPU frequency to either a fixed value or to on-demand automatic
* scaling mode. Returns 0 on success, -1 on failure.
*
* @ rc_cpu_freq_t rc_get_cpu_freq()
*
* Returns the current clock speed of the Beaglebone's Sitara processor in the
* form of the provided enumerated type. It will never return the FREQ_ONDEMAND
* value as the intention of this function is to see the clock speed as set by
* either the user or the ondemand governor itself.
*
* @ int rc_print_cpu_freq()
*
* Prints the current frequency to the screen. For example "300MHZ".
* Returns 0 on success or -1 on failure.
*******************************************************************************/
#ifndef RC_CPU_FREQ_H
#define RC_CPU_FREQ_H

typedef enum rc_cpu_freq_t{
	FREQ_ONDEMAND,
	FREQ_300MHZ,
	FREQ_600MHZ,
	FREQ_800MHZ,
	FREQ_1000MHZ
} rc_cpu_freq_t;

int rc_set_cpu_freq(rc_cpu_freq_t freq);
rc_cpu_freq_t rc_get_cpu_freq();
int rc_print_cpu_freq();

#endif // RC_CPU_FREQ_H