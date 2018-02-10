#include "rc_state.h"

// global roboticscape state
enum rc_state_t rc_state = UNINITIALIZED;

/*******************************************************************************
* @ rc_state_t rc_get_state()
*
* returns the high-level robot state variable
* use this for managing how your threads start and stop
*******************************************************************************/
rc_state_t rc_get_state(){
	return rc_state;
}

/*******************************************************************************
* @ int rc_set_state(rc_state_t new_state)
*
* sets the high-level robot state variable
* use this for managing how your threads start and stop
*******************************************************************************/
int rc_set_state(rc_state_t new_state){
	rc_state = new_state;
	return 0;
}

/*******************************************************************************
* @ int rc_print_state()
* 
* Prints the textual name of the state to the current state to the screen.
*******************************************************************************/
int rc_print_state(){
	switch(rc_state){
	case UNINITIALIZED:
		printf("UNINITIALIZED");
		break;
	case PAUSED:
		printf("PAUSED");
		break;
	case RUNNING:
		printf("RUNNING");
		break;
	case EXITING:
		printf("EXITING");
		break;
	default:
		printf("ERROR: invalid state\n");
		return -1;
	}
	return 0;
}
