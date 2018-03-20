/**
 * @file pru.c
 *
 * @brief      C interface for starting and stopping the PRU from userspace.
 *
 *             This is primarily for the PRU-dependent servo and encoder
 *             functions to use, however the user may elect to use their own PRU
 *             routines separately from those.
 *
 * @author James Strawson
 * @date 3/15/2018
 */


#include <stdio.h>
#include <fcntl.h>	// for open
#include <unistd.h>	// for close
#include <sys/mman.h>	// mmap
#include <string.h>
#include <rc/pru.h>

// remoteproc driver
#define PRU0_STATE	"/sys/class/remoteproc/remoteproc1/state"
#define PRU1_STATE	"/sys/class/remoteproc/remoteproc2/state"
#define PRU0_FW		"/sys/class/remoteproc/remoteproc1/firmware"
#define PRU1_FW		"/sys/class/remoteproc/remoteproc2/firmware"

// share memory pointer location
#define PRU_ADDR	0x4A300000		// Start of PRU memory Page 184 am335x TRM
#define PRU_LEN		0x80000			// Length of PRU memory
#define PRU_SHAREDMEM	0x10000			// Offset to shared memory

static unsigned int* shared_mem_32bit_ptr = NULL;

int rc_pru_start(int ch, const char* fw_name)
{
	int fd, fw_fd, ret;
	char buf[64];

	// sanity checks
	if(ch!=0 && ch!=1){
		fprintf(stderr, "ERROR in rc_pru_start, PRU channel must be 0 or 1\n");
		return -1;
	}
	if(fw_name==NULL){
		fprintf(stderr, "ERROR in rc_pru_start, received NULL pointer\n");
		return -1;
	}

	// check state
	if(ch=0) fd=open(PRU0_STATE, O_RDWR);
	else fd=open(PRU1_STATE, O_RDWR);
	if(fd==-1){
		perror("ERROR in rc_pru_start opening remoteproc driver");
		fprintf(stderr,"PRU probably not enabled in device tree\n");
		return -1;
	}
	ret=read(fd, buf, sizeof(buf));
	if(ret==-1){
		perror("ERROR in rc_pru_start reading state");
		close(fd);
		return -1;
	}
	// if already running, warn and stop it
	if(strcmp(buf,"running")==0){
		fprintf(stderr,"WARNING: pru%d is already running, restarting with requested firmware\n",ch);
		ret=write(fd,"stop",5);
		if(ret==-1){
			perror("ERROR in rc_pru_start while writing to remoteproc state");
			close(fd);
			return -1;
		}
	}
	// if we read anything except "offline" there is something weird
	else if(strcmp(buf,"offline")){
		fprintf(stderr, "ERROR: remoteproc state should be 'offline' or 'running', read:%s\n", buf);
		close(fd);
		return -1;
	}

	// now write firmware title
	if(ch=0) fw_fd=open(PRU0_FW, O_WRONLY);
	else fw_fd=open(PRU1_FW, O_WRONLY);
	if(fw_fd==-1){
		perror("ERROR in rc_pru_start opening remoteproc driver");
		fprintf(stderr,"kernel probably too old\n");
		return -1;
	}
	if(write(fw_fd, fw_name, strlen(fw_name))==-1){
		perror("ERROR in rc_pru_start setting firmware name");
		close(fw_fd);
		close(fd);
		return -1;
	}
	close(fw_fd);

	// finally start the pru
	ret=write(fd, "start", 6);
	if(ret==-1){
		perror("ERROR in rc_pru_start starting remoteproc");
		close(fd);
		return -1;
	}

	// make sure it's running
	ret=read(fd, buf, sizeof(buf));
	if(ret==-1){
		perror("ERROR in rc_pru_start reading state");
		close(fd);
		return -1;
	}
	if(strcmp(buf,"running")){
		fprintf(stderr,"ERROR: pru%d failed to start\n", ch);
		fprintf(stderr,"expected state to become 'running', instead is: %s\n",buf);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}


uint32_t* rc_pru_shared_mem_ptr()
{
	int fd;
	uint32_t* map;

	// if already set, just return the pointer
	if(shared_mem_32bit_ptr!=NULL){
		return shared_mem_32bit_ptr;
	}

	// map shared memory
	fd=open("/dev/mem", O_RDWR | O_SYNC);
	if(fd==-1){
		perror("ERROR: in rc_pru_shared_mem_ptr could not open /dev/mem.\n\n");
		return NULL;
	}

	map = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
	if(map==MAP_FAILED) {
		perror("ERROR in rc_pru_shared_mem_ptr failed to map memory");
		close(fd);
		return NULL;
	}
	close(fd);

	// set global shared memory pointer
	// Points to start of shared memory
	shared_mem_32bit_ptr = map + PRU_SHAREDMEM/4;

	return shared_mem_32bit_ptr;
}


int rc_pru_stop(int ch)
{
	int fd, ret;
	char buf[64];

	// sanity checks
	if(ch!=0 && ch!=1){
		fprintf(stderr, "ERROR in rc_pru_stop, PRU channel must be 0 or 1\n");
		return -1;
	}

	// check state
	if(ch=0) fd=open(PRU0_STATE, O_RDWR);
	else fd=open(PRU1_STATE, O_RDWR);
	if(fd==-1){
		perror("ERROR in rc_pru_stop opening remoteproc driver");
		fprintf(stderr,"PRU probably not enabled in device tree\n");
		return -1;
	}
	ret=read(fd, buf, sizeof(buf));
	if(ret==-1){
		perror("ERROR in rc_pru_stop reading state");
		close(fd);
		return -1;
	}
	// running, stop it
	if(strcmp(buf,"running")==0){
		ret=write(fd,"stop",5);
		if(ret==-1){
			perror("ERROR in rc_pru_stop while writing to remoteproc state");
			close(fd);
			return -1;
		}
	}
	// if we read anything except "offline" there is something weird
	if(strcmp(buf,"offline")){
		fprintf(stderr, "ERROR: remoteproc state should be 'offline', read:%s\n", buf);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

