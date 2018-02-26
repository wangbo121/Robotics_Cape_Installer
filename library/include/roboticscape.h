/*******************************************************************************
* roboticscape.h
*
* This contains the complete Robotics Cape API. All functions declared here can
* be executed by linking to /usr/lib/robotics_cape.so
*
* All functions return 0 on success or -1 on failure unless otherwise stated.
*
* James Strawson - 2016
*******************************************************************************/

#ifndef ROBOTICS_CAPE
#define ROBOTICS_CAPE

#include "rc/bbb/board_id.h"
#include "rc/bbb/cpu_freq.h"
#include "rc/bbb/pinmux.h"
#include "rc/bbb/rc_defs.h"

#include <rc/general/adc.h>
#include <rc/general/buttons.h>
#include <rc/general/dsm.h>
#include <rc/general/encoder.h>
#include <rc/general/led.h>
#include <rc/general/motor.h>
#include <rc/general/servo.h>
#include <rc/general/state.h>

#include "rc/io/i2c.h"
#include "rc/io/spi.h"
#include "rc/io/uart.h"
#include "rc/io/gpio.h"
#include "rc/io/pwm.h"

#include "rc/math/filter.h"
#include "rc/math/linear_algebra.h"
#include "rc/math/matrix.h"
#include "rc/math/polynomial.h"
#include "rc/math/quaternion.h"
#include "rc/math/ring_buffer.h"
#include "rc/math/vector.h"

#include "rc/other/other.h"
#include "rc/other/time.h"
#include "rc/other/version.h"

#include <rc/sensor/mpu9250.h>
#include "rc/sensor/bmp280.h"



#endif //ROBOTICS_CAPE


