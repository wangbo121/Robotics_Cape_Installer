/******************************************************************************
* QUADRATURE ENCODER
*
* @ int rc_get_encoder_pos(int ch)
* @ int rc_set_encoder_pos(int ch, int value)
*
* The Robotics Cape includes 4 JST-SH sockets {E1 E2 E3 E4} for connecting
* quadrature encoders. The pins for each are assigned as follows:
*
* 1 - Ground
* 2 - 3.3V
* 3 - Signal A
* 4 - Signal B
*
* The first 3 channels are counted by the Sitara's eQEP hardware encoder
* counters. The 4th channel is counted by the PRU. As a result, no CPU cycles
* are wasted counting encoders and the user only needs to read the channel
* at any point in their code to get the current position. All channels are
* reset to 0 when initialize_cape() is called. However, the user can reset
* the counter to zero or any other signed 32 bit value with rc_set_encoder_pos()
*
*
* @ int rc_initialize_encoders()
*
* initializes both the eQEP (#1-3) channels and the PRU (#4) channel.
*
* @ int rc_initialize_eqep_encoders()
*
* initializes only the eQEP (#1-3) channels and leaves the PRU core #0 alone
*
* @ int rc_initialize_pru_encoder()
*
* enables encoder channel #4 by loading the appropriate firmware into PRU core 0
*
* See the test_encoders example for sample use case.
******************************************************************************/
#ifndef RC_ENCODER_H
#define RC_ENCODER_H

int rc_initialize_encoders();
int rc_initialize_eqep_encoders();
int rc_initialize_pru_encoder();
int rc_get_encoder_pos(int ch);
int rc_set_encoder_pos(int ch, int value);

#endif // RC_ENCODER_H