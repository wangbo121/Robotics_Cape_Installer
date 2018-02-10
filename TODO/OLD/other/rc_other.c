/*******************************************************************************
* rc_other.c
*
* This is a collection of miscellaneous useful functions that are part of the
* robotics cape library. These do not necessarily interact with hardware.
*******************************************************************************/

#include "../roboticscape.h"
#include <stdio.h>
#include <stdlib.h> // for system()
#include <unistd.h>
#include <string.h> // for strcat()

/*******************************************************************************
* @ void rc_null_func()
*
* A simple function that just returns. This exists so function pointers can be
* set to do nothing such as button and imu interrupt handlers.
*******************************************************************************/
void rc_null_func(){
	return;
}



/*******************************************************************************
* @ char *rc_byte_to_binary(char x)
*
* This returns a string (char*) of '1' and '0' representing a character.
* For example, print "00101010" with printf(rc_byte_to_binary(42));
*******************************************************************************/
char *rc_byte_to_binary(unsigned char c){
	static char b[9];
	unsigned char x = (unsigned char)c; //cast to unsigned
	b[0] = '\0';
	unsigned char z;
	for (z = 128; z > 0; z >>= 1){
		if(x&z) strcat(b, "1");
		else strcat(b, "0");
	}
	return b;
}

/*******************************************************************************
* @ int rc_suppress_stdout(int (*func)(void))
*
* Executes a functiton func with all outputs to stdout suppressed. func must
* take no arguments and must return an integer. Adapt this source to your
* liking if you need to pass arguments. For example, if you have a function
* int foo(), call it with supressed output to stdout as follows:
* int ret = rc_suppress_stdout(&foo);
*******************************************************************************/
int rc_suppress_stdout(int (*func)(void)){
	int ret=0;
	int old_stdout;
	FILE  *null_out;

	// change stdout to null for this operation as the prussdrv.so
	// functions are noisy
	old_stdout = dup(STDOUT_FILENO);
	fflush(stdout);
	null_out = fopen("/dev/null", "w");
	dup2(fileno(null_out), STDOUT_FILENO);

	// execute the function
	ret=func();

	// put back stdout
	fflush(stdout);
	fclose(null_out);
	dup2(old_stdout,STDOUT_FILENO);
	close(old_stdout);

	return ret;
}

/*******************************************************************************
* @ int rc_suppress_stderr(int (*func)(void))
*
* executes a functiton func with all outputs to stderr suppressed. func must
* take no arguments and must return an integer. Adapt this source to your
* liking if you need to pass arguments. For example, if you have a function
* int foo(), call it with supressed output to stderr as follows:
* int ret = rc_suppress_stderr(&foo);
*******************************************************************************/
int rc_suppress_stderr(int (*func)(void)){
	int ret=0;
	int old_stderr;
	FILE  *null_out;

	// change stdout to null for this operation as the prussdrv.so
	// functions are noisy
	old_stderr = dup(STDERR_FILENO);
	fflush(stderr);
	null_out = fopen("/dev/null", "w");
	dup2(fileno(null_out), STDERR_FILENO);

	// execute the function
	ret=func();

	// put back stdout
	fflush(stderr);
	fclose(null_out);
	dup2(old_stderr,STDERR_FILENO);
	close(old_stderr);

	return ret;
}

/*******************************************************************************
* rc_continue_or_quit()
*
* This is a blocking function which returns 1 if the user presses ENTER.
* it returns 0 on any other keypress. If ctrl-C is pressed it will
* additionally set the global state to EXITITING and return -1.
* This is a useful function for checking if the user wishes to continue with a
* process or quit.
*******************************************************************************/
int rc_continue_or_quit(){
	// set stdin to non-canonical raw mode to capture all button presses
	fflush(stdin);
	system("stty raw");
	int c=getchar();
	int ret;
	if(c==3){
		printf("recieved Ctrl-C\n");
		rc_set_state(EXITING);
		ret = -1;
	}
	else if(c=='\r' || c=='\n'){
		ret = 1;
	}
	else{
		ret = 0;
	}
	// put stdin back to normal canonical mode
	system ("stty cooked");
	printf("\n");
	return ret;
}

/*******************************************************************************
* @ float rc_version_float()
*
* Returns a floating-point representation of the roboticscape library version
* for easy comparison.
*******************************************************************************/
float rc_version_float(){
	return RC_LIB_VERSION_FLOAT;
}

/*******************************************************************************
* @ const char* rc_version_string()
*
* Returns a string of the roboticscape package version for printing.
*******************************************************************************/
const char* rc_version_string(){
	// annoying macro hack to convert to string
	#define Q(x) #x
	#define QUOTE(x) Q(x)
	return QUOTE(RC_LIB_VERSION_STRING);
	#undef Q
	#undef QUOTE
}



