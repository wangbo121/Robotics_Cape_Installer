/*******************************************************************************
* Other Functions
*
* This is a collection of miscellaneous useful functions that are part of the
* robotics cape library. These do not necessarily interact with hardware.
*
* @ void rc_null_func()
*
* A simple function that just returns. This exists so function pointers can be
* set to do nothing such as button and imu interrupt handlers.
*
* @ float rc_get_random_float()
* @ double rc_get_random_double()
*
* Returns a random single or double prevision point value between -1 and 1.
* These are here because the rand() function from stdlib.h only returns and
* integer. These are highly optimized routines that use bitwise operation
* instead of floating point division.
*
* @ int rc_saturate_float(float* val, float min, float max)
* @ int rc_saturate_double(double* val, double min, double max)
*
* Modifies val to be bounded between between min and max. Returns 1 if
* saturation occurred, 0 if val was already in bound, and -1 if min was falsely
* larger than max.
*
* @ char *rc_byte_to_binary(char x)
*
* This returns a string (char*) of '1' and '0' representing a character.
* For example, print "00101010" with printf(rc_byte_to_binary(42));
*
* @ int rc_suppress_stdout(int (*func)(void))
*
* Executes a functiton func with all outputs to stdout suppressed. func must
* take no arguments and must return an integer. Adapt this source to your
* liking if you need to pass arguments. For example, if you have a function
* int foo(), call it with supressed output to stdout as follows:
* int ret = rc_suppress_stdout(&foo);
*
* @ int rc_suppress_stderr(int (*func)(void))
*
* executes a functiton func with all outputs to stderr suppressed. func must
* take no arguments and must return an integer. Adapt this source to your
* liking if you need to pass arguments. For example, if you have a function
* int foo(), call it with supressed output to stderr as follows:
* int ret = rc_suppress_stderr(&foo);
*
* @ rc_continue_or_quit()
*
* This is a blocking function which returns 1 if the user presses ENTER.
* it returns 0 on any other keypress. If ctrl-C is pressed it will
* additionally set the global state to EXITITING and return -1.
* This is a useful function for checking if the user wishes to continue with a
* process or quit.
*
* @ float rc_version_float()
*
* Returns a floating-point representation of the roboticscape library version
* for easy comparison.
*
* @ const char* rc_version_string()
*
* Returns a string of the roboticscape package version for printing.
*******************************************************************************/

#ifndef RC_OTHER_H
#define RC_OTHER_H

void rc_null_func();
char* rc_byte_to_binary(unsigned char x);
int rc_suppress_stdout(int (*func)(void));
int rc_suppress_stderr(int (*func)(void));
int rc_continue_or_quit();

#endif // RC_OTHER_H
