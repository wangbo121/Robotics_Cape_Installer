/*******************************************************************************
* rc_buttons.c
*
* 4 threads for managing the RC_PAUSE_BTN and press buttons
*******************************************************************************/
#define _GNU_SOURCE
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include "rc/button.h"
#include "rc/io/gpio.h"
#include "rc/preprocessor_macros.h"
#include "rc/time.h"
#include "rc/rc_defs.h"

#define POLL_TIMEOUT		100	// 0.1 seconds
#define NUM_THREADS		4	// 2 for each button
#define DEBOUNCE_INTERVAL_US	2000	// 2ms
// local function declarations
static int get_index(rc_button_t button, rc_button_state_t state);
static void* button_handler(void* ptr);
static void rc_null_func();

// local variables, arrays follow indexing of RC_MODE_BTN[] below
static void (*callbacks[4])(void);
static pthread_t threads[4];
static pthread_mutex_t mutex[4];
static pthread_cond_t condition[4];
static int pin_init_flag = 0;
static int initialized_flag = 0;	// set to 1 but initialize_buttons
static int shutdown_flag = 0;		// set to 1 by rc_stop_buttons

// RC_MODE_BTNs for the watch threads to run in
typedef struct thread_mode_t {
	rc_button_t button;
	rc_button_state_t state;
	int gpio;
	int index;
} thread_mode_t;

static const thread_mode_t mode[] = {
	{RC_PAUSE_BTN, PRESSED,  PAUSE_BTN_PIN, 0},
	{RC_PAUSE_BTN, RELEASED, PAUSE_BTN_PIN, 1},
	{RC_MODE_BTN,  PRESSED,  MODE_BTN_PIN,  2},
	{RC_MODE_BTN,  RELEASED, MODE_BTN_PIN,  3}
};

// local states set by the button threads
static rc_button_state_t current_pause_state;
static rc_button_state_t current_mode_state;

// local function
static int init_pins();
static void rc_null_func();



/*******************************************************************************
* static int init_pins();
*
* Basic export and direction setup of pins, just enough to read the state of
* a button without setting edge detection yet.
*******************************************************************************/
static int init_pins(){
	if(pin_init_flag) return 0;
	// basic gpio setup
	if(rc_gpio_export(PAUSE_BTN_PIN)){
		fprintf(stderr,"ERROR: Failed to export gpio pin %d\n", PAUSE_BTN_PIN);
		return -1;
	}
	if(rc_gpio_set_dir(PAUSE_BTN_PIN, INPUT_PIN)){
		fprintf(stderr,"ERROR: Failed to set gpio pin %d as output\n", PAUSE_BTN_PIN);
		return -1;
	}
	if(rc_gpio_export(MODE_BTN_PIN)){
		fprintf(stderr,"ERROR: Failed to export gpio pin %d\n", MODE_BTN_PIN);
		return -1;
	}
	if(rc_gpio_set_dir(MODE_BTN_PIN, INPUT_PIN)){
		fprintf(stderr,"ERROR: Failed to set gpio pin %d as output\n", MODE_BTN_PIN);
		return -1;
	}
	pin_init_flag=1;
	return 0;
}

/*******************************************************************************
* static void rc_null_func()
*
* A simple function that just returns. This exists so function pointers can be
* set to do nothing such as button and imu interrupt handlers.
*******************************************************************************/
static void rc_null_func(){
	return;
}

/*******************************************************************************
* int rc_button_init()
*
* start 4 threads to handle 4 interrupt routines for pressing and
* releasing the two buttons.
*******************************************************************************/
int rc_button_init()
{
	int i;
	// sanity checks
	if(initialized_flag!=0 && shutdown_flag==0){
		fprintf(stderr,"ERROR: in rc_buttons_init, buttons already initialized\n");
		return -1;
	}
	if(initialized_flag!=0 && shutdown_flag!=0){
		fprintf(stderr,"ERROR: in rc_button_init, button threads are still shutting down\n");
		return -1;
	}

	// basic gpio setup
	if(init_pins()){
		fprintf(stderr,"ERROR: in rc_button_init, failed to setup pins\n");
		return -1;
	}

	// read in current values first
	if(rc_gpio_get_value(PAUSE_BTN_PIN)) current_pause_state=RELEASED;
	else current_pause_state=PRESSED;
	if(rc_gpio_get_value(MODE_BTN_PIN)) current_mode_state=RELEASED;
	else current_mode_state=PRESSED;

	// now set up edge detection
	rc_gpio_set_edge(MODE_BTN_PIN, EDGE_BOTH);
	rc_gpio_set_edge(PAUSE_BTN_PIN, EDGE_BOTH);

	// wipe the callback functions
	#ifdef DEBUG
	printf("setting default callback function\n");
	#endif
	for(i=0;i<4;i++){
		callbacks[i]=&rc_null_func;
	}

	// reset mutex
	#ifdef DEBUG
	printf("resetting button mutex\n");
	#endif
	for(i=0;i<4;i++){
		pthread_mutex_init(&mutex[i],NULL);
		pthread_cond_init(&condition[i],NULL);
	}

	// make sure shutdown flag is turned off
	shutdown_flag=0;

	// spawn off threads
	#ifdef DEBUG
	printf("starting button threads\n");
	#endif
	for(i=0;i<4;i++){
		pthread_create(&threads[i],NULL,button_handler,(void*)&mode[i]);
	}

	// apply priority to all threads
	#ifdef DEBUG
	printf("setting button thread priorities\n");
	#endif
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO)-5;
	for(i=0;i<4;i++){
		pthread_setschedparam(threads[i], SCHED_FIFO, &params);
	}
	initialized_flag=1;
	return 0;
}



/*******************************************************************************
* void* button_handler(void* ptr)
*
* poll a pgio edge with debounce check. When the button changes state broadcast
* a mutex condition so blocking wait functions return and execute a user defined
* callback if set.
*******************************************************************************/
void* button_handler(void* ptr)
{
	thread_mode_t* mode = (thread_mode_t*)ptr;
	struct pollfd fdset;
	char ch;
	int gpio_fd = rc_gpio_fd_open(mode->gpio);
	rc_button_state_t new_state;
	if(gpio_fd == -1){
		fprintf(stderr,"ERROR initializing buttons, failed to get GPIO fd for pin %d\n",mode->gpio);
		return NULL;
	}
	fdset.fd = gpio_fd;
	fdset.events = POLLPRI; // high-priority interrupt
	// keep running until the program closes
	while(shutdown_flag==0) {
		// purge any interrupts that may have stacked up
		lseek(fdset.fd, 0, SEEK_SET);
		read(fdset.fd, &ch, 1);
		// debounce wait
		rc_usleep(DEBOUNCE_INTERVAL_US);
		// purge any interrupts that may have stacked up
		lseek(fdset.fd, 0, SEEK_SET);
		read(fdset.fd, &ch, 1);
		// system waits here until FIFO interrupt
		poll(&fdset, 1, POLL_TIMEOUT);
		// if poll returned for something other than POLLPRI, ignore
		if(!(fdset.revents&POLLPRI)) continue;
		// read in buffer from gpio, only 1 character
		lseek(fdset.fd, 0, SEEK_SET);
		read(fdset.fd, &ch, 1);

		#ifdef DEBUG
		printf("button thread: %d  gpiofd reads: %c\n",mode->index,ch);
		#endif

		// see what the new gpio state is
		if(ch=='0')	new_state=PRESSED;
		else		new_state=RELEASED;

		// if the new state isn't what this thread is looking
		// for go back to poll
		if(new_state!=mode->state) continue;

		// set new button state
		if(mode->button==RC_PAUSE_BTN) current_pause_state=mode->state;
		else current_mode_state=mode->state;

		// broadcast condition for blocking wait to return
		pthread_mutex_lock(&mutex[mode->index]);
		pthread_cond_broadcast(&condition[mode->index]);
		pthread_mutex_unlock(&mutex[mode->index]);
		// execute the callback
		callbacks[mode->index]();
	}
	rc_gpio_fd_close(gpio_fd);
	return 0;
}

/*******************************************************************************
* int rc_button_set_callback(rc_button_t button, rc_button_state_t state,void (*func)(void))
*
*
*******************************************************************************/
int rc_button_set_callback(rc_button_t button, rc_button_state_t state,void (*func)(void))
{
	if(!initialized_flag){
		fprintf(stderr,"ERROR: call to rc_set_button_callback when buttons have not been initialized.\n");
		return -2;
	}
	int index = get_index(button,state);
	if(index<0) return -1;
	if(func==NULL){
		printf("ERROR: trying to assign NULL pointer to button callback\n");
		return -1;
	}
	callbacks[index] = func;
	return 0;
}

/*******************************************************************************
* rc_button_state_t rc_button_get_state(rc_button_t button)
*******************************************************************************/
rc_button_state_t rc_button_get_state(rc_button_t button)
{
	int pin,ret;
	if(init_pins()){
		fprintf(stderr,"ERROR: call to rc_button_get_state() when pins have not been initialized.\n");
		return -1;
	}

	switch(button){
	case RC_PAUSE_BTN:
		pin=PAUSE_BTN_PIN;
		break;
	case RC_MODE_BTN:
		pin=MODE_BTN_PIN;
		break;
	default:
		fprintf(stderr,"ERROR: in rc_button_get_state, invalid rc_button_t enum\n");
		return -1;
	}

	ret=rc_gpio_get_value(pin);
	switch(ret){
	case 0:
		return PRESSED;
	case 1:
		return RELEASED;
	default:
		fprintf(stderr,"ERROR: in rc_button_get_state, pin read failure\n");
		return -1;
	}
}

/*******************************************************************************
* waiting functions
*******************************************************************************/
int rc_button_wait(rc_button_t button, rc_button_state_t state)
{
	if(shutdown_flag!=0){
		fprintf(stderr,"ERROR: call to rc_wait_for_button() after buttons have been powered off.\n");
		return -1;
	}
	if(!initialized_flag){
		fprintf(stderr,"ERROR: call to rc_wait_for_button() when buttons have not been initialized.\n");
		return -1;
	}
	// get index of the mutex arrays from the arguments
	int index = get_index(button,state);
	if(index<0){
		fprintf(stderr,"ERROR in rc_wait_for_button, invalid button/state combo\n");
		return -1;
	}
	// wait for condition signal which unlocks mutex
	pthread_mutex_lock(&mutex[index]);
	pthread_cond_wait(&condition[index], &mutex[index]);
	pthread_mutex_unlock(&mutex[index]);
	// check if condition was broadcast due to shutdown
	if(shutdown_flag) return 1;
	// otherwise return 0 on actual button press
	return 0;
}


/*******************************************************************************
* int get_index(rc_button_t button, rc_button_state_t state)
*******************************************************************************/
int get_index(rc_button_t button, rc_button_state_t state){
	if	(button==RC_PAUSE_BTN && state==PRESSED)	return 0;
	else if	(button==RC_PAUSE_BTN && state==RELEASED)	return 1;
	else if	(button==RC_MODE_BTN  && state==PRESSED)	return 2;
	else if	(button==RC_MODE_BTN  && state==RELEASED)	return 3;
	else{
		fprintf(stderr,"ERROR in rc_buttons.c, invalid button/state combo\n");
		return -1;
	}
}



/*******************************************************************************
* int rc_button_cleanup()
*******************************************************************************/
int rc_button_cleanup()
{
	// if buttons not initialized, just return, nothing to do
	if(!initialized_flag) return 0;
	shutdown_flag = 1;
	int ret = 0;
	//allow up to 3 seconds for thread cleanup
	struct timespec thread_timeout;
	clock_gettime(CLOCK_REALTIME, &thread_timeout);
	thread_timeout.tv_sec += 3;
	int thread_err = 0;
	int i;
	for(i=0;i<NUM_THREADS;i++){
		thread_err = pthread_timedjoin_np(threads[i], NULL,&thread_timeout);
		if(thread_err == ETIMEDOUT){
			printf("WARNING: button thread exit timeout\n");
			ret = -1;
		}
	}
	initialized_flag=0;
	return ret;
}


/*******************************************************************************
* DEPRECATED button function assignments
*******************************************************************************/
int rc_set_pause_pressed_func(void (*func)(void))
{
	if(func==NULL){
		printf("ERROR: trying to assign NULL pointer to pause_press_func\n");
		return -1;
	}
	callbacks[0] = func;
	return 0;
}
int rc_set_pause_released_func(void (*func)(void)){
	if(func==NULL){
		printf("ERROR: trying to assign NULL pointer to pause_release_func\n");
		return -1;
	}
	callbacks[1] = func;
	return 0;
}
int rc_set_mode_pressed_func(void (*func)(void)){
	if(func==NULL){
		printf("ERROR: trying to assign NULL pointer to mode_press_func\n");
		return -1;
	}
	callbacks[2] = func;
	return 0;
}
int rc_set_mode_released_func(void (*func)(void)){
	if(func==NULL){
		printf("ERROR: trying to assign NULL pointer to mode_release_func\n");
		return -1;
	}
	callbacks[3] = func;
	return 0;
}

/*******************************************************************************
* DEPRECATED
* rc_button_state_t rc_get_pause_button()
*******************************************************************************/
rc_button_state_t rc_get_pause_button()
{
	if(!initialized_flag){
		fprintf(stderr,"ERROR: call to rc_get_pause_button() when buttons have not been initialized.\n");
		return -2;
	}
	if(rc_gpio_get_value(PAUSE_BTN_PIN)==0){
		return PRESSED;
	}
	return RELEASED;
}

/*******************************************************************************
* DEPRECATED
* rc_button_state_t rc_get_mode_button()
*******************************************************************************/
rc_button_state_t rc_get_mode_button()
{
	if(!initialized_flag){
		fprintf(stderr,"ERROR: call to rc_get_mode_button() when buttons have not been initialized.\n");
		return -2;
	}
	if(rc_gpio_get_value(MODE_BTN_PIN)==HIGH){
		return RELEASED;
	}
	return PRESSED;
}
