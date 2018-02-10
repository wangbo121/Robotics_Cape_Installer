// Collection of register definitions for the TI AM335X
// credit to Ethan Hayon for writing the majority of this


#ifndef RC_GPIO_SITARA_REGISTERS_H
#define RC_GPIO_SITARA_REGISTERS_H

#include <stdint.h>

#define MMAP_OFFSET		(0x44C00000)
#define MMAP_SIZE		(0x481AEFFF-MMAP_OFFSET)


#define GPIO_REGISTER_SIZE	(4)

#define GPIO0			(0x44E07000)
#define GPIO1			(0x4804C000)
#define GPIO2			(0x481AC000)
#define GPIO3			(0x481AE000)

#define GPIO_CLEARDATAOUT	(0x190)
#define GPIO_SETDATAOUT		(0x194)
#define GPIO_OE			(0x134)
#define GPIO_DATAOUT		(0x13C)
#define GPIO_DATAIN		(0x138)

#define INPUT			((unsigned char)(1))
#define OUTPUT			((unsigned char)(0))
#define PULLUP			((unsigned char)(1))
#define PULLDOWN		((unsigned char)(0))
#define PULL_DISABLED		((unsigned char)(2))

/*********************************
* clock control registers
*********************************/
//base of Clock Module Peripheral control
#define CM_PER			0x44E00000
#define CM_PER_PAGE_SIZE	1024 //1kb
#define CM_PER_GPIO1_CLKCTRL	0xAC
#define CM_PER_GPIO2_CLKCTRL	0xB0
#define CM_PER_GPIO3_CLKCTRL	0xB4
#define CM_WKUP			0x44E00400
#define MODULEMODE_DISABLED	0x0
#define MODULEMODE_ENABLE	0x2
#define CM_WKUP_ADC_TSC_CLKCTRL	0xBC

#endif // RC_GPIO_SITARA_REGISTERS_H
