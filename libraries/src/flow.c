/*******************************************************************************
* flow.c
*
* program flow state variable and signal handlers for coordinating the safe
* running and shutdown of single or multi-threaded processes.
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for system()
#include <unistd.h> // for access()
#include "rc/flow.h"
#include "rc/time.h"
#include "rc/preprocessor_macros.h"

#define PID_FILE "/var/run/robotics_cape.pid"

// global roboticscape state
static enum rc_state_t rc_state = UNINITIALIZED;

// local function declarations
static void shutdown_signal_handler(int signum);
static void segfault_handler(int signum, siginfo_t *info, void *context);


/*******************************************************************************
* rc_state_t rc_get_state()
*
* returns the high-level robot state variable
* use this for managing how your threads start and stop
*******************************************************************************/
rc_state_t rc_get_state()
{
	return rc_state;
}

/*******************************************************************************
* int rc_set_state(rc_state_t new_state)
*
* sets the high-level robot state variable
* use this for managing how your threads start and stop
*******************************************************************************/
int rc_set_state(rc_state_t new_state)
{
	rc_state = new_state;
	return 0;
}

/*******************************************************************************
* int rc_print_state()
*
* Prints the textual name of the state to the current state to the screen.
*******************************************************************************/
int rc_print_state()
{
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
		fprintf(stderr,"ERROR: invalid state\n");
		return -1;
	}
	return 0;
}




/*******************************************************************************
* int rc_make_pid_file()
*
* writes a PID file /var/run/robotics_cape.pid containing the current PID of
* your process. Returns 0 if successful. If that file already exists then it is
* not touched and this function returns 1 at which point we suggest you run
* rc_kill() to kill that process. Returns -1 if there is some other problem
* writing to the file.
*******************************************************************************/
int rc_make_pid_file()
{
	FILE *fd;
	pid_t current_pid;
	// start by checking if a pid file exists
	if(access(PID_FILE, F_OK ) == 0){
		fprintf(stderr,"ERROR: PID file already exists, a new one was not written\n");
		return 1;
	}
	// open new file for writing
	fd = fopen(PID_FILE, "ab+");
	if (fd == NULL) {
		fprintf(stderr,"error opening PID_FILE for writing\n");
		return -1;
	}
	current_pid = getpid();
	fprintf(fd,"%d",(int)current_pid);
	fflush(fd);
	fclose(fd);
	return 0;
}

/*******************************************************************************
* int rc_kill()
*
* This function is used to make sure any existing program using the PID file
* is stopped. The user doesn't need to integrate
* this in their own program  However, the user may
* call the rc_kill example program from the command line to close whatever
* program is running in the background.
*
* return values:
* -2 : unreadable or invalid contents in PID_FILE
* -1 : existing project failed to close cleanly and had to be killed
*  0 : No existing program is running
*  1 : An existing program was running but it shut down cleanly.
*******************************************************************************/
int rc_kill()
{
	FILE* fd;
	int old_pid, i;
	// start by checking if a pid file exists
	if(access(PID_FILE, F_OK ) != 0){
		// PID file missing, nothing is running
		return 0;
	}
	// attempt to open PID file if it fails something very wrong with it
	fd = fopen(PID_FILE, "r");
	if(fd==NULL){
		remove(PID_FILE);
		return -2;
	}
	// try to read the current process ID
	fscanf(fd,"%d", &old_pid);
	fclose(fd);

	// if the file didn't contain a PID number, remove it and
	// return -2 indicating weird behavior
	if(old_pid == 0){
		remove(PID_FILE);
		return -2;
	}

	// check if it's our own pid, if so return 0
	if(old_pid == (int)getpid()) return 0;

	// now see if the process for the read pid is still running
	if(getpgid(old_pid) < 0){
		// process not running, remove the pid file
		remove(PID_FILE);
		return 0;
	}

	// process must be running, attempt a clean shutdown
	kill((pid_t)old_pid, SIGINT);

	// check every 0.1 seconds to see if it closed
	for(i=0; i<30; i++){
		if(getpgid(old_pid) >= 0) rc_usleep(100000);
		else{ // succcess, it shut down properly
			remove(PID_FILE);
			return 1;
		}
	}
	// otherwise force kill the program if the PID file never got cleaned up
	kill((pid_t)old_pid, SIGKILL);
	rc_usleep(500000);

	// delete the old PID file if it was left over
	remove(PID_FILE);
	// return -1 indicating the program had to be killed
	return -1;
}


/*******************************************************************************
* int rc_remove_pid_file()
*
* removes PID file created by rc_make_pid_file(). This should be called before
* your program closes to make sure it's not left behind. Returns 0 whether or
* not the file was actually there. Returns -1 if there was a filesystem error.
*******************************************************************************/
int rc_remove_pid_file()
{
	// if PID file exists, remove it
	if(access(PID_FILE, F_OK ) == 0) return remove(PID_FILE);
	return 0;
}




/*******************************************************************************
* @ int rc_enable_signal_handler
*
* Enables a signal handler that catches SIGINT & SIGTERM to set rc_state to
* EXITING indicating to your program to shut down cleanly instead of stopping
* immediately as is the defualt value. SIGHUP is ignored to prevent your program
* from stopping due to loose USB network connection. Also segfaults will be
* caught and print some debugging info to the screen before setting rc_state to
* EXITING. Returns 0 on success or -1 if there was an error setting the signal
* handlers
*******************************************************************************/
int rc_enable_signal_handler()
{
	// make the sigaction struct for shutdown signals
	struct sigaction action;
	action.sa_sigaction = NULL;
	action.sa_handler = shutdown_signal_handler;
	// set actions
	if(sigaction(SIGINT, &action, NULL) < 0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	if(sigaction(SIGTERM, &action, NULL) < 0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	if(sigaction(SIGHUP, &action, NULL) < 0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	// different handler for segfaults
	// here we want SIGINFO too so we use sigaction intead of handler
	// also use RESETHAND to stop infinite loops
	action.sa_handler = NULL;
	action.sa_sigaction = segfault_handler;
	action.sa_flags = SA_SIGINFO | SA_RESETHAND;
	if(sigaction(SIGSEGV, &action, NULL) < 0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	return 0;
}

/*******************************************************************************
* int rc_disable_signal_handler(
*
* Disables the built-in signal handler. Use only if you want to implement your
* own signal handler. Make sure your handler sets rc_state to EXITING or calls
* cleanup_cape on shutdown to ensure roboticscape library threads close
* cleanly.
*******************************************************************************/
int rc_disable_signal_handler()
{
	// reset all to defaults
	struct sigaction action;
	action.sa_handler = SIG_DFL;

	if(sigaction(SIGINT, &action, NULL)<0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	if(sigaction(SIGTERM, &action, NULL)<0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	if(sigaction(SIGHUP, &action, NULL)<0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	if(sigaction(SIGSEGV, &action, NULL)<0){
		fprintf(stderr, "ERROR: failed to set sigaction\n");
		return -1;
	}
	return 0;
}


/*******************************************************************************
* static void segfault_handler(int signum, siginfo_t *info, void *context)
*
* custom segfault catcher to show useful info
*******************************************************************************/
static void segfault_handler(__unused int signum, siginfo_t *info, __unused void *context)
{
	fprintf(stderr, "ERROR: Segmentation Fault\n");
	fprintf(stderr, "Fault address: %p\n", info->si_addr);
	switch (info->si_code){
	case SEGV_MAPERR:
		fprintf(stderr, "Address not mapped.\n");
		break;
	case SEGV_ACCERR:
		fprintf(stderr, "Access to this address is not allowed.\n");
		break;
	default:
		fprintf(stderr, "Unknown reason.\n");
		break;
	}
	rc_set_state(EXITING);
	// here we would normally reset the signal handler to prevent infinite
	// loop, but this happens automatically with the SA_RESETHAND flag
	return;
}

/*******************************************************************************
* static void shutdown_signal_handler(int signo)
*
* catch Ctrl-C signal and change system state to EXITING
* all threads should watch for rc_get_state()==EXITING and shut down cleanly
*******************************************************************************/
static void shutdown_signal_handler(int signo)
{
	switch(signo){
	case SIGINT: // normal ctrl-c shutdown interrupt
		rc_set_state(EXITING);
		printf("\nreceived SIGINT Ctrl-C\n");
		break;
	case SIGTERM: // catchable terminate signal
		rc_set_state(EXITING);
		printf("\nreceived SIGTERM\n");
		break;
	case SIGHUP: // terminal closed or disconnected, carry on anyway
		break;
	default:
		break;
	}
	return;
}