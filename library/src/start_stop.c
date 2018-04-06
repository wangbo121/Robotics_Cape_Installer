/**
 * @file start_stop.c
 *
 * @author     James Strawson
 * @date       2/1/2018
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for system()
#include <unistd.h> // for access()
#include <sys/stat.h> // for mkdir and chmod
#include <sys/types.h> // for mkdir and chmod

#include <rc/start_stop.h>
#include <rc/time.h>

// global process state
static enum rc_state_t rc_state = UNINITIALIZED;

// local function declarations
static void shutdown_signal_handler(int signum);
static void segfault_handler(int signum, siginfo_t *info, void *context);


rc_state_t rc_get_state()
{
	return rc_state;
}


void rc_set_state(rc_state_t new_state)
{
	rc_state = new_state;
	return;
}


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


int rc_make_pid_file()
{
	FILE *fd;
	pid_t current_pid;
	// start by checking if a pid file exists
	if(access(RC_PID_FILE, F_OK ) == 0){
		fprintf(stderr,"ERROR: in rc_make_pid_file, file already exists, a new one was not written\n");
		return 1;
	}
	// open new file for writing
	fd = fopen(RC_PID_FILE, "w");
	if (fd == NULL) {
		perror("ERROR in rc_make_pid_file");
		return -1;
	}
	current_pid = getpid();
	fprintf(fd,"%d",(int)current_pid);
	fflush(fd);
	fclose(fd);
	// now set the correct permissions
	if(chmod(RC_PID_FILE, 0777)==-1){
		perror("ERROR setting permissions of PID directory");
		return -1;
	}
	return 0;
}


int rc_kill_existing_process(float timeout_s)
{
	FILE* fd;
	int old_pid, i, ret, num_checks;

	// sanity checks
	if(timeout_s<0.1f){
		fprintf(stderr, "ERROR in rc_kill_existing_process, timeout_s must be >= 0.1f\n");
		return -2;
	}

	// start by checking if a pid file exists
	if(access(RC_PID_FILE, F_OK)){
		// PID file missing, nothing is running
		return 0;
	}
	// attempt to open PID file if it fails something very wrong with it
	fd = fopen(RC_PID_FILE, "r");
	if(fd==NULL){
		remove(RC_PID_FILE);
		return -2;
	}
	// try to read the current process ID
	ret=fscanf(fd,"%d", &old_pid);
	if(ret!=1){
		fprintf(stderr,"WARNING in rc_kill_existing_process, read invalid contents in PID file\n");
	}
	fclose(fd);

	// if the file didn't contain a PID number, remove it and
	// return -2 indicating weird behavior
	if(old_pid == 0){
		remove(RC_PID_FILE);
		return -2;
	}

	// check if it's our own pid, if so return 0
	if(old_pid == (int)getpid()) return 0;

	// now see if the process for the read pid is still running
	if(getpgid(old_pid) < 0){
		// process not running, remove the pid file
		remove(RC_PID_FILE);
		return 0;
	}

	// process must be running, attempt a clean shutdown
	kill((pid_t)old_pid, SIGINT);

	// check every 0.1 seconds to see if it closed
	num_checks=timeout_s/0.1f;
	for(i=0; i<=num_checks; i++){
		// check if PID has stopped
		if(getpgid(old_pid)==-1){
			// succcess, it shut down properly
			remove(RC_PID_FILE);
			return 1;
		}
		else rc_usleep(100000);
	}

	// otherwise force kill the program if the PID file never got cleaned up
	kill((pid_t)old_pid, SIGKILL);
	for(i=0; i<=num_checks; i++){
		// check if PID has stopped
		if(getpgid(old_pid)==-1){
			// succcess, it shut down properly
			remove(RC_PID_FILE);
			return 1;
		}
		else rc_usleep(100000);
	}

	// delete the old PID file if it was left over
	remove(RC_PID_FILE);
	// return -1 indicating the program had to be killed
	return -1;
}


int rc_remove_pid_file()
{
	// if PID file exists, remove it
	if(access(RC_PID_FILE, F_OK ) == 0) return remove(RC_PID_FILE);
	return 0;
}


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


static void segfault_handler(__attribute__ ((unused)) int signum, siginfo_t *info, __attribute__ ((unused)) void *context)
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