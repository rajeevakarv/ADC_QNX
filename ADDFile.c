#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>       /* for sleep() */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <sys/mman.h>     /* for mmap_device_io() */


#define BASE_ADDR 0x280
#define PORT_LENGTH 1

int main(int argc, char *argv[]) {

	int privity_err;
	uintptr_t ctrl_handle_chennalReg;
	uintptr_t ctrl_handle_Inputgain;
	uintptr_t ctrl_handle_commandRegMSB;
	uintptr_t ctrl_handle_commandRegLSB;
	uintptr_t ctrl_handle_portA;

	int LSB, MSB, DATA;

	struct timespec my_timer_value;
	my_timer_value.tv_nsec = 10000;    //10us

	/* Give this thread root permissions to access the hardware */
	privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
	if ( privity_err == -1 )
	{
		fprintf( stderr, "can't get root permissions\n" );
	    return -1;
	}
	ctrl_handle_commandRegLSB = mmap_device_io( PORT_LENGTH, BASE_ADDR);
	ctrl_handle_commandRegMSB = mmap_device_io( PORT_LENGTH, BASE_ADDR+1);
	ctrl_handle_chennalReg = mmap_device_io( PORT_LENGTH, BASE_ADDR + 2);
	ctrl_handle_Inputgain = mmap_device_io( PORT_LENGTH, BASE_ADDR + 3);

	ctrl_handle_portA = mmap_device_io( PORT_LENGTH, BASE_ADDR + 8);
	ctrl_handle_DIO = mmap_device_io(PORT_LENGTH, BASE_ADDR + B);

	out8(ctrl_handle_chennalReg, 0xF0);

	// Set Analog Input as -5V to +5V
	out8( ctrl_handle_Inputgain, 0x01 );

	int port_value = in8(ctrl_handle_Inputgain);
	while (port_value & 0x20)
	{
		//nanospin( &my_timer_value );
		port_value = in8(ctrl_handle_Inputgain);
	}

	out8(ctrl_handle_commandRegLSB, 0x80);
	
	while (in8(ctrl_handle_Inputgain) & 0x80);
	LSB = in8(ctrl_handle_commandRegLSB);
	MSB = in8(ctrl_handle_commandRegMSB);
	DATA = MSB*256 + LSB;
	printf("Welcome to the QNX Momentics IDE\n");
	return EXIT_SUCCESS;
}
