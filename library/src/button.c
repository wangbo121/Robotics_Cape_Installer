/**
 * @file button.c
 */

#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <rc/gpio.h>
#include <rc/time.h>
#include <rc/pthread.h>
#include <rc/button.h>

#define MAX_PINS	128
#define POLL_TIMEOUT_MS	100	// 0.1 seconds
#define THREAD_TIMEOUT	3.0	// 3 seconds

static void (*press_cb[MAX_PINS])(void);
static void (*release_cb[MAX_PINS])(void);
static pthread_t press_thread[MAX_PINS];
static pthread_t release_thread[MAX_PINS];
static char init_flag[MAX_PINS];
static char started[MAX_PINS];
static char pol[MAX_PINS];
static int shutdown_flag = 0;

#define MODE_PRESS	1<<0
#define MODE_RELEASE	1<<1

typedef struct thread_cfg_t{
	int pin;
	int direction;
	int debounce;
	int mode;
} thread_cfg_t;

//static pthread_mutex_t mutex[4];
//static pthread_cond_t condition[4];

/**
 * poll a pgio edge with debounce check. When the button changes state broadcast
 * a mutex condition so blocking wait functions return and execute a user
 * defined callback if set.
 *
 * @param      ptr   The pointer
 *
 * @return     { description_of_the_return_value }
 */
void* button_handler(void* arg)
{
	int ret;
	thread_cfg_t cfg = *(thread_cfg_t*)arg;
	// once config data has been saved locally, flag that the thread has started
	started[cfg.pin] |= cfg.mode;

	// keep running until the program closes
	while(!shutdown_flag){
		ret=rc_gpio_poll(cfg.pin,POLL_TIMEOUT_MS,NULL);
		if(ret==RC_GPIOEVENT_ERROR){
			fprintf(stderr,"ERROR in rc_button handler thread\n");
			return NULL;
		}
		if(ret!=RC_GPIOEVENT_TIMEOUT){
			printf("event in thread: %d\n",cfg.direction);
		}
		if(ret==cfg.direction){
			// debounce
			if(cfg.debounce){
				rc_usleep(cfg.debounce);
				ret=rc_gpio_get_value(cfg.pin);
				if(ret==-1){
					fprintf(stderr,"ERROR in rc_button handler thread\n");
					return NULL;
				}
				if(cfg.direction==RC_GPIOEVENT_FALLING_EDGE
					&& ret!=0){
					continue;
				}
				if(cfg.direction==RC_GPIOEVENT_RISING_EDGE
					&& ret!=1){
					continue;
				}
			}
			// call appropriate callback
			if(cfg.mode==MODE_PRESS){
				if(press_cb[cfg.pin]!=NULL){
					press_cb[cfg.pin]();
				}
			}
			else{
				if(release_cb[cfg.pin]!=NULL){
					release_cb[cfg.pin]();
				}
			}
		}
		/*
		// broadcast condition for blocking wait to return
		pthread_mutex_lock(&mutex[mode->index]);
		pthread_cond_broadcast(&condition[mode->index]);
		pthread_mutex_unlock(&mutex[mode->index]);
		*/
	}
	printf("thread returning\n");
	return NULL;
}

int rc_button_init(int pin, char polarity, int debounce_us)
{
	int i;
	thread_cfg_t press_config;
	thread_cfg_t release_config;

	// sanity checks
	if(pin<0 || pin>MAX_PINS){
		fprintf(stderr,"ERROR in rc_button_init, pin must be between 0 and %d\n",MAX_PINS);
		return -1;
	}
	if(polarity!=RC_BTN_POLARITY_NORM_LOW && polarity!=RC_BTN_POLARITY_NORM_HIGH){
		fprintf(stderr,"ERROR in rc_button_init\n");
		fprintf(stderr,"polarity must be RC_BTN_POLARITY_NORM_LOW or RC_BTN_POLARITY_NORM_HIGH\n");
		return -1;
	}
	if(debounce_us<0){
		fprintf(stderr, "ERROR in rc_button_init, debounce_us must be >=0\n");
		return -1;
	}

	// basic gpio setup
	if(rc_gpio_init_event(pin,GPIOHANDLE_REQUEST_INPUT,GPIOEVENT_REQUEST_BOTH_EDGES)){
		fprintf(stderr,"ERROR: in rc_button_init, failed to setup GPIO pin\n");
		return -1;
	}

	/*
	// reset mutex
	for(i=0;i<4;i++){
		pthread_mutex_init(&mutex[i],NULL);
		pthread_cond_init(&condition[i],NULL);
	}
	*/

	// set up thread config structs
	press_config.pin = pin;
	press_config.debounce = debounce_us;
	press_config.mode = MODE_PRESS;
	release_config.pin = pin;
	release_config.debounce = debounce_us;
	release_config.mode = MODE_RELEASE;
	pol[pin] = polarity;
	if(polarity==RC_BTN_POLARITY_NORM_HIGH){
		press_config.direction   = RC_GPIOEVENT_FALLING_EDGE;
		release_config.direction = RC_GPIOEVENT_RISING_EDGE;
	}
	else{
		press_config.direction   = RC_GPIOEVENT_RISING_EDGE;
		release_config.direction = RC_GPIOEVENT_FALLING_EDGE;
	}

	// start threads
	shutdown_flag=0;
	started[pin]=0;
	if(rc_pthread_create(&press_thread[pin], button_handler,
				(void*)&press_config, SCHED_OTHER, 0)){
		fprintf(stderr,"ERROR in rc_button_init, failed to start press handler thread\n");
		return -1;
	}
	if(rc_pthread_create(&release_thread[pin], button_handler,
				(void*)&release_config, SCHED_OTHER, 0)){
		fprintf(stderr,"ERROR in rc_button_init, failed to start release handler thread\n");
		return -1;
	}
	rc_usleep(100000);
	// wait for threads to start
	//while(started[pin] != MODE_PRESS|MODE_RELEASE) rc_usleep(1000);

	// set flags
	init_flag[pin]=1;
	shutdown_flag=0;
	return 0;
}


void rc_button_cleanup()
{
	int i, ret;
	// signal threads to close
	shutdown_flag=1;
	// loop through all pins
	for(i=0;i<MAX_PINS;i++){
		// skip uninitialized pins
		if(!init_flag[i]) continue;
		// join pressed thread
		ret=rc_pthread_timed_join(press_thread[i],NULL,THREAD_TIMEOUT);
		if(ret==-1){
			fprintf(stderr,"WARNING in rc_button_cleanup, problem joining button PRESS handler thread for pin %d\n",i);
		}
		else if(ret==1){
			fprintf(stderr,"WARNING in rc_button_cleanup, PRESSED thread exit timeout for pin %d\n",i);
			fprintf(stderr,"most likely cause is your button press callback function is stuck and didn't return\n");
		}
		// join released thread
		ret=rc_pthread_timed_join(release_thread[i],NULL,THREAD_TIMEOUT);
		if(ret==-1){
			fprintf(stderr,"WARNING in rc_button_cleanup, problem joining button RELEASE handler thread for pin %d\n",i);
		}
		else if(ret==1){
			fprintf(stderr,"WARNING in rc_button_cleanup, RELEASE thread exit timeout for pin %d\n",i);
			fprintf(stderr,"most likely cause is your button release callback function is stuck and didn't return\n");
		}
		rc_gpio_cleanup(i);
		init_flag[i]=0;
	}
	return;
}




int rc_button_set_callbacks(int pin, void (*press_func)(void), void (*release_func)(void))
{
	// sanity checks
	if(pin<0 || pin>MAX_PINS){
		fprintf(stderr,"ERROR in rc_button_set_callabcks, pin must be between 0 and %d\n",MAX_PINS);
		return -1;
	}
	press_cb[pin] = press_func;
	release_cb[pin] = release_func;
	return 0;
}


int rc_button_get_state(int pin)
{
	int val,ret;
	if(!init_flag[pin]){
		fprintf(stderr,"ERROR: call to rc_button_get_state without calling rc_button_init first\n");
		return -1;
	}
	val=rc_gpio_get_value(pin);
	if(val==-1){
		fprintf(stderr,"ERROR in rc_button_get_state\n");
		return -1;
	}
	// determine return based on polarity
	if(pol[pin]==RC_BTN_POLARITY_NORM_HIGH){
		if(val) ret=RC_BTN_STATE_RELEASED;
		else ret=RC_BTN_STATE_PRESSED;
	}
	else{
		if(val) ret=RC_BTN_STATE_PRESSED;
		else ret=RC_BTN_STATE_RELEASED;
	}
	return ret;
}

/*
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
*/




