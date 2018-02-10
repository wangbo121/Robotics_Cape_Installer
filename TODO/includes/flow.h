/*******************************************************************************
* INITIALIZATION AND CLEANUP
*
* Because the Robotics Cape library is tightly integrated with hardware and
* utilizes background threads, it is critical that the user calls
* rc_initialize() and rc_cleanup() at the beginning and end of their programs to
* ensure predictable operation. These methods, also enable the following two
* features.
*
* Firstly, to ensure only one program interfaces with the Robotics Cape core
* functions at once, it  makes a PID file in /var/run/ called robotics.pid that
* contains the process ID of the currently running robotics cape project. If a
* second process is started that uses the Robotics Cape library, the call to
* rc_initialize() in the new process cleanly shuts down the existing process
* before returning. This PID file also helps systemd monitor robotics cape
* projects that are started on boot as a service. See the services section of
* the manual for more details on systemd services.
*
* Secondly, the call to rc_initialize() registers a system signal handler to
* intercept shutdown and halt signals. It is typical to stop a Linux program by
* pressing Ctrl-C but this can halt the program in a problematic state if not
* handled correctly. When the user presses Ctrl-C, the Linux kernel sends the
* SIGINT signal to the program which is intercepted by the Robotics Cape signal
* handler to set the program flow state to EXITING. The user's program should
* then monitor for this condition in each thread and exit in a safe manner. This
* is particularly important when driving motors and other actuators which should
* be turned off or put in a safe position before the program closes. See the
* flow state section of this manual for more details. The SIGHUP signal is also
* caught and ignored so that robotics programs started from the command line
* over a USB connection will continue to run if the USB cable is disconnected.

* @ int rc_initialize()
*
* To ensure full library functionality and to take advantage of the above
* features, the user must call rc_initialize() at beginning of their program.
* rc_initialize() will make sure any existing Robotics Cape project is shut down
* cleanly before continuing to avoid conflicts. It then proceeds to open file
* descriptors and start background threads that are necessary for the Robotics
* Cape library to run. If you are running a kernel with missing drivers or the
* Robotics Cape device tree is not loaded then this function will return -1 and
* print an error message to indicate what is wrong. Otherwise it will return 0
* to indicate success.
*
* @ int rc_cleanup()
*
* rc_cleanup() undoes everything done by initialize cape. It closes file
* pointers, waits for background threads to stop cleanly, and finally removes
* the PID file from /var/run/. Additionally it sets LEDs and slave select pins
* in an 'OFF' state and puts the h-bridges into a low power standby state. This
* should be called at the very end of the user's program. It returns 0 on
* success or -1 on error.
*
* * @ int rc_kill()
*
* This function is used by initialize_cape to make sure any existing program
* using the robotics cape lib is stopped. The user doesn't need to integrate
* this in their own program as initialize_cape calls it. However, the user may
* call the kill_robot example program from the command line to close whatever
* program is running in the background.
* return values:
* -2 : invalid contents in PID_FILE
* -1 : existing project failed to close cleanly and had to be killed
*  0 : No existing program is running
*  1 : An existing program was running but it shut down cleanly.
*
* All example programs use these functions. See the bare_minimum example
* for a skeleton outline.
*
* @ void rc_disable_signal_handler()
*
* Disables the built-in signal handler. Use only if you want to implement your
* own signal handler. Make sure your handler sets rc_state to EXITING or calls
* cleanup_cape on shutdown to ensure Robotics Cape library threads close
* cleanly.
*
* @ void rc_enable_signal_handler()
*
* Re-enables the built-in signal handler if it was disabled before. The built-in
* signal handler is enabled by default in rc_initialize().
*******************************************************************************/

/*******************************************************************************
* FLOW STATE FOR HIGH LEVEL PROGRAM CONTROL
*
* It can be tricky to manage the starting and stopping of mutiple threads and
* loops. Since the robotics cape library has several background threads in
* addition to any user-created threads, we encourage the use of the consolidated
* high-level program flow control method described here.
*
* The rc_state_t struct tries to cover the majority of use cases in the context
* of a robotics application. After the user has called rc_initialize(), the
* program flow state will be set to UNINITIALIZED. When the user's own
* initialization sequence is complete they should set the flow state to PAUSED
* or RUNNING to indicate to their own threads that the program should now behave
* in normal ongoing operational mode.
*
* During normal operation, the user may elect to implement a PAUSED state where
* the user's own threads may keep running to read sensors but do not actuate
* motors, leaving their robot in a safe state. For example, pressing the pause
* button could be assigned to change this state back and forth between RUNNING
* and PAUSED. This is entirely optional.
*
* The most important state here is EXITING. The signal handler described in the
* Init & Cleanup section intercepts the SIGINT signal when the user pressed
* Ctrl-C and sets the flow state to EXITING. It is then up to the user's threads
* to watch for this condition and exit quickly and cleanly. The user may also
* set the flow state to EXITING at any time to trigger the closing of their own
* threads and Robotics Cape library's own background threads.
*
* The flow state variable is kept safely in the robotics cape library's memory
* space and should be read and modified by the rc_get_state() and rc_set_state()
* functions above. The user may optionally use the rc_print_state() function to
* print a human readable version of the state enum to the screen.

* All example programs use these functions. See the rc_bare_minimum example
* for a skeleton outline.
*******************************************************************************/
#ifndef RC_STATE_H
#define RC_STATE_H

#include "preprocessor_macros.h"

typedef enum rc_state_t {
	UNINITIALIZED,
	RUNNING,
	PAUSED,
	EXITING
} rc_state_t;



rc_state_t rc_get_state();
int rc_set_state(rc_state_t new_state);
int rc_print_state();

int rc_make_pid_file();
int rc_kill();
int rc_remove_pid_file();


int rc_enable_signal_handler();
int rc_disable_signal_handler();

// deprecated functions
__deprecated int rc_initialize();	// call at the beginning of main()
__deprecated int rc_cleanup();	// call at the end of main()



#endif // RC_STATE_H
