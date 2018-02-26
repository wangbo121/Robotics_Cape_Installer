/******************************************************************************
* ANALOG VOLTAGE SIGNALS
*
*
* @ float rc_battery_voltage()
* @ float rc_dc_jack_voltage()
* The Robotics cape includes two voltage dividers for safe measurement of the
* 2-cell lithium battery voltage and the voltage of any power source connected
* to the 6-16V DC power jack. These can be read with rc_battery_voltage()
* and rc_dc_jack_voltage()
*
* @ int rc_adc_raw(int ch)
* @ float rc_adc_volt(int ch)
*
* There is also a 6-pin JST-SH socket on the Cape for connecting up to 4
* potentiometers or general use analog signals. The pinout of this socket is
* as follows:
*
* 1 - Ground
* 2 - VDD_ADC (1.8V)
* 3 - AIN0
* 4 - AIN1
* 5 - AIN2
* 6 - AIN3
*
* All 7 ADC channels on the Sitara including the 4 listed above can be read
* with rc_adc_raw(int ch) which returns the raw integer output of the
* 12-bit ADC. rc_adc_volt(int ch) additionally converts this raw value to
* a voltage. ch must be from 0 to 6.
*
* See the test_adc example for sample use case.
 *
 * @addtogroup ADC
 * @{
******************************************************************************/

#ifndef RC_ADC_H
#define RC_ADC_H


#ifdef  __cplusplus
extern "C" {
#endif

int rc_adc_init();

int rc_adc_cleanup();

int rc_adc_read_raw(int ch);

float rc_adc_read_volt(int ch);

float rc_adc_battery_volt();

float rc_adc_dc_jack_volt();



#ifdef  __cplusplus
}
#endif

#endif // RC_ADC_H

/** @}  end group time*/