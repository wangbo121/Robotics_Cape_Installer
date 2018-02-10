/******************************************************************************
* DSM2/DSMX RC radio functions
*
* The Robotics Cape features a 3-pin JST ZH socket for connecting a DSM2/DSMX
* compatible satellite receiver. See the online manual for more details.
*
* @ int rc_initialize_dsm()
* Starts the background service.
*
* @ rc_is_new_dsm_data()
*
* Returns 1 when new data is available.
*
* @ int rc_is_dsm_active()
*
* Returns 1 if packets are arriving in good health without timeouts.
* Returns 0 otherwise.
*
* @ int rc_set_dsm_data_func(int (*func)(void));
*
* Much like the button handlers, this assigns a user function to be called when
* new data arrives. Be careful as you should still check for radio disconnects
*
* @ int rc_get_dsm_ch_raw(int channel)
*
* Returns the pulse width in microseconds commanded by the transmitter for a
* particular channel. The user can specify channels 1 through 9 but non-zero
* values will only be returned for channels the transmitter is actually using.
* The raw values in microseconds typically range from 900-2100us for a standard
* radio with default settings.
*
* @ rc_get_dsm_ch_normalized(int channel)
*
* Returns a scaled value from -1 to 1 corresponding to the min and max values
* recorded during calibration. The user
* MUST run the clalibrate_dsm example to ensure the normalized values returned
* by this function are correct.
*
* @ uint64_t rc_nanos_since_last_dsm_packet();
*
* returns the number of nanoseconds since the last dsm packet was received.
* if no packet has ever been received, returns UINT64_MAX;
*
* @ int rc_stop_dsm_service()
*
* stops the background thread. Not necessary to be called by the user as
* cleanup_cape() calls this anyway.
*
* @ int rc_bind_dsm()
*
* Puts a satellite receiver in bind mode. Use the rc_bind_dsm example program
* instead of calling this in your own program.
*
* int rc_calibrate_dsm_routine()
*
* Starts a calibration routine.
*
* see rc_test_dsm, rc_calibrate_dsm, and rc_dsm_passthroguh examples
******************************************************************************/
#ifndef RC_DSM_H
#define RC_DSM_H

int   rc_initialize_dsm();
int   rc_stop_dsm_service();
int   rc_get_dsm_ch_raw(int channel);
float rc_get_dsm_ch_normalized(int channel);
int   rc_is_new_dsm_data();
int   rc_set_dsm_data_func(void (*func)(void));
int   rc_is_dsm_active();
uint64_t rc_nanos_since_last_dsm_packet();
int   rc_get_dsm_resolution();
int   rc_num_dsm_channels();
int   rc_bind_dsm();
int   rc_calibrate_dsm_routine();

#endif // RC_DSM_H