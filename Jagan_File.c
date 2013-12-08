#include <stdio.h>
#include <unistd.h>			/* for sleep() */
#include <stdlib.h>			/* for EXIT_* */
#include <stdint.h>			/* for uintptr_t */
#include <hw/inout.h>		/* for in*() and out*() functions */
#include <sys/neutrino.h>	/* for ThreadCtl() */
#include <sys/syspage.h>	/* for for cycles_per_second */
#include <sys/mman.h>		/* for mmap_device_io() */
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <math.h>

#define PORT_LENGTH 1
#define COMMAND_LSB_CTRL_ADDRESS 0x280
#define COMMAND_MSB_CTRL_ADDRESS 0x281
#define CHANNEL_CTRL_ADDRESS 0x282
#define GAIN_CTRL_ADDRESS 0x283

//types.h-------------------------------------------------------
// Signed 8-bit Type
#ifndef INT8
typedef signed char INT8;
#endif

// Signed 16-bit Type
#ifndef INT16
typedef signed int INT16;
#endif

// Signed 32-bit Type
#ifndef INT32
typedef signed long int INT32;
#endif

// Unsigned 8-bit Type
#ifndef UINT8
typedef unsigned char UINT8;
#endif


// Unsigned 16-bit Type
#ifndef UINT16
typedef unsigned int UINT16;
#endif

// Unsigned 32-bit Type
#ifndef UINT32
typedef unsigned long int UINT32;
#endif
//--------------------------------------------------------------

uintptr_t command_LSB_ctrl_handle;
uintptr_t command_MSB_ctrl_handle;
uintptr_t channel_ctrl_handle;
uintptr_t gain_ctrl_handle;
uintptr_t output_ctrl_handle;
uintptr_t output_data_handle;

UINT16 wait_bit;
INT8 LSB, MSB;
INT16 data;
float voltage;


int main(void)
{
	printf("THU\n");
	int privity_err;
	/* Give this thread root permissions to access the hardware */
	privity_err = ThreadCtl(_NTO_TCTL_IO, NULL);
	if (privity_err == -1) {
	    fprintf(stderr, "can't get root permissions\n");
	    return -1;
	}

for(;;){
       // Get a handle to the parallel port's Control register
       command_LSB_ctrl_handle = mmap_device_io(PORT_LENGTH, COMMAND_LSB_CTRL_ADDRESS);
       command_MSB_ctrl_handle = mmap_device_io(PORT_LENGTH, COMMAND_MSB_CTRL_ADDRESS);
       channel_ctrl_handle = mmap_device_io(PORT_LENGTH, CHANNEL_CTRL_ADDRESS);
       gain_ctrl_handle = mmap_device_io(PORT_LENGTH, GAIN_CTRL_ADDRESS);

       //Selecting the range
       out8(channel_ctrl_handle, 0x44);
       out8(gain_ctrl_handle, 0x01);

     //  uint x=in8(gain_ctrl_handle);


       //Wait for analog input circuit to settle
       wait_bit = (in8(gain_ctrl_handle) & 0x20);
       while(wait_bit !=0)
       {
           wait_bit = (in8(gain_ctrl_handle) & 0x20);
       }


       //Starting the A/D Conversion
       //AINTE = 0;
       out8(command_LSB_ctrl_handle, 0x80);

       //Wait for conversions to finish
       while((in8(gain_ctrl_handle) & 0x80) !=0)
       {

       }

       //Read the data
       LSB = in8(command_LSB_ctrl_handle);
       MSB = in8(command_MSB_ctrl_handle);
       //printf("%d\n",LSB);
       //printf("%d\n",MSB);
       data = (MSB*256) + LSB;
       //printf("Data : %d\n",data);

       //Converting the data
       voltage = (float)(data*5)/32768;

       printf("voltage %f\n",voltage);
       printf("abs voltage %f\n",fabsf(voltage));
       printf("int voltage %d\n",(int)fabsf(voltage));
       float abs_voltage = fabsf(voltage);
       int solid = (int)fabsf(voltage);
       int frac = (int)((abs_voltage-solid)*10);
       int position = (solid*4) + (frac/2);
       printf("solid = %d, frac = %d, POS = %d\n\n", solid, frac, position);

       //assigning digital signals to output port
       output_ctrl_handle = mmap_device_io(PORT_LENGTH, 0x28B);
       out8(output_ctrl_handle, 0x00);
       output_data_handle = mmap_device_io(PORT_LENGTH, 0x288);
	   out8(output_data_handle, voltage);

	   //int temp = (voltage & 0x8000) >> 15;
	   //printf("Temp = %d", temp)



	}

}
