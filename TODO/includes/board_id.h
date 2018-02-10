/*******************************************************************************
* Board identification
*
* Because we wish to support different beagleboard products with this same
* library, we must internally determine which board we are running on to decide
* which pins to use. We make these functions available to the user in case they
* wish to do the same.
* See the check_board example for a demonstration.
*******************************************************************************/
#ifndef RC_BOARD_ID_H
#define RC_BOARD_ID_H

typedef enum rc_bb_model_t{
	UNKNOWN_MODEL,
	BB_BLACK,
	BB_BLACK_RC,
	BB_BLACK_W,
	BB_BLACK_W_RC,
	BB_GREEN,
	BB_GREEN_W,
	BB_BLUE
} rc_bb_model_t;

rc_bb_model_t rc_get_bb_model();
void rc_print_bb_model();

#endif // RC_BOARD_ID_H