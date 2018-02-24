/**
 * <rc/encoder_eqep.h>
 *
 * @brief
 *
 *
 * @author     James Strawson
 * @date       1/31/2018
 *
 * @addtogroup encoder
 * @{
 */

#ifndef RC_ENCODER_EQEP_H
#define RC_ENCODER_EQEP_H

#ifdef  __cplusplus
extern "C" {
#endif


int rc_encoder_eqep_init();

int rc_encoder_eqep_cleanup();

int rc_encoder_eqep_read(int ch);

int rc_encoder_eqep_write(int ch, int pos);






#ifdef  __cplusplus
}
#endif

#endif // RC_ENCODER_EQEP_H

/** @}  end group encoder*/