/******************************************************************************
* Timer Output Compare Demo
*
* Description:
*
* This demo configures the timer to a rate of 1 MHz, and the Output Compare
* Channel 1 to toggle PORT T, Bit 1 at rate of 10 Hz.
*
* The toggling of the PORT T, Bit 1 output is done via the Compare Result Output
* Action bits. 
*
* The Output Compare Channel 1 Interrupt is used to refresh the Timer Compare
* value at each interrupt
*
* Author:
*  Rajeev Verma
*  Pratheep Joe
*
*****************************************************************************/


// system includes
#include <hidef.h>      /* common defines and macros */
#include <stdio.h>      /* Standard I/O Library */
#include <ctype.h>
#include <stdlib.h>

// project includes
#include "types.h"
#include "derivative.h" /* derivative-specific definitions */

// Definitions

// Change this value to change the frequency of the output compare signal.
// The value is in Hz.
#define OC_FREQ_HZ    ((UINT16)10)

// Macro definitions for determining the TC1 value for the desired frequency
// in Hz (OC_FREQ_HZ). The formula is:
//
// TC1_VAL = ((Bus Clock Frequency / Prescaler value) / 2) / Desired Freq in Hz
//
// Where:
//        Bus Clock Frequency     = 2 MHz
//        Prescaler Value         = 2 (Effectively giving us a 1 MHz timer)
//        2 --> Since we want to toggle the output at half of the period
//        Desired Frequency in Hz = The value you put in OC_FREQ_HZ
//
#define BUS_CLK_FREQ  ((UINT32) 2000000)  
#define PRESCALE      ((UINT16)  2)        
#define TC1_VAL       ((UINT16)  (((BUS_CLK_FREQ / PRESCALE) / 2) / OC_FREQ_HZ))

// Boolean Definitions to make the code more readable.
#define FALSE 0
#define TRUE 1
#define MAXINPUTVALUES 1001


// Normally I'd use something awesome like a bool but we're stuck with this err
// limited system.
// This is used to let the program know when to capture values.
UINT16 captureValues = FALSE;

// holds the timer values captured on the rising edge.
UINT16 timerValuesUs [1001] = { 0 };

// holds the time inteval between rising edges.
UINT16 pulseIntervalsUs [1000] = { 0 };


UINT16 getUINT16Input(void);
UINT16 post_function(void);
void processTimerMeasurements(UINT16 lowerBoundaryUs, UINT16 upperBoundaryUs);
UINT16 MaxInputValue;
UINT16 MinInputValue;

// Initializes SCI0 for 8N1, 9600 baud, polled I/O
// The value for the baud selection registers is determined
// using the formula:
//
// SCI0 Baud Rate = ( 2 MHz Bus Clock ) / ( 16 * SCI0BD[12:0] )
//--------------------------------------------------------------
void InitializeSerialPort(void)
{
    // Set baud rate to ~9600 (See above formula)
    SCI0BD = 13;         
    
    // 8N1 is default, so we don't have to touch SCI0CR1.
    // Enable the transmitter and receiver.
    SCI0CR2_TE = 1;
    SCI0CR2_RE = 1;
}


// Initializes I/O and timer settings for the demo.
//--------------------------------------------------------------      
void InitializeTimer(void)
{
  // Set the timer prescaler to %2, since the bus clock is at 2 MHz,
  // and we want the timer running at 1 MHz
  TSCR2_PR0 = 1;
  TSCR2_PR1 = 0;
  TSCR2_PR2 = 0;      
    
  // Change to an input compare. HR
  // Enable input capture on Channel 1 
  TIOS_IOS1 = 0;
  
  
  // Set up input capture edge control to capture on a rising edge.
  TCTL4_EDG1A = 1;
  TCTL4_EDG1B = 0;
   
  // from here down we want this code. HR.
  // Clear the input capture Interrupt Flag (Channel 1)
  TFLG1 = TFLG1_C1F_MASK;
  
  // Enable the input capture interrupt on Channel 1;
  TIE_C1I = 1; 
  
  //
  // Enable the timer
  //
  TSCR1_TEN = 1;
   
  //
  // Enable interrupts via macro provided by hidef.h
  //
  EnableInterrupts;
}


// Output Compare Channel 1 Interrupt Service Routine
// Refreshes TC1 and clears the interrupt flag.
//         
// The first CODE_SEG pragma is needed to ensure that the ISR
// is placed in non-banked memory. The following CODE_SEG
// pragma returns to the default scheme. This is neccessary
// when non-ISR code follows.
//
// The TRAP_PROC tells the compiler to implement an
// interrupt funcion. Alternitively, one could use
// the __interrupt keyword instead.
//
// The following line must be added to the Project.prm
// file in order for this ISR to be placed in the correct
// location:
//		VECTOR ADDRESS 0xFFEC OC1_isr
#pragma push
#pragma CODE_SEG __SHORT_SEG NON_BANKED
//--------------------------------------------------------------      
void interrupt 9 OC1_isr( void )
{
   // This interrupt stores the values from the table into the array.
   // we don't want to do any calculations because we are dealing with
   // Us and want the reads to be as accurate as possible.
  
  
   
   // set the interrupt enable flag for that port because it is cleared every
   // everytime an interrupt fires.
   TFLG1   =   TFLG1_C1F_MASK;
}
#pragma pop



// Entry point of our application code
//--------------------------------------------------------------      
void main(void)
{

  UINT8 userInput = 0;
  UINT16 lowerBoundaryUs = 0;
  UINT16 upperBoundaryUs = 0;
  
  InitializeSerialPort();
  InitializeTimer();
   
  // Post function
  
  // if we pass post run the program otherwise go home.
  if(post_function())
  {
 
     //start of main loop
     for(;;)
     {
        // Check to see if the user wants another set of readings
        (void) printf("Press s key to capture the readings or e to end the program. ");
        
     }
  }
  
  (void) printf("\r\n\r\nOk I'm outa here!!!\r\n\r\n");
}




//*****************************************************************************
// This unmitigated piece of crap will test to make sure the timer is running
// on the board.  If it is not running it will print an error message and fail.
//
//
// Parameters:  None.
//
// Return: None.
//*****************************************************************************
UINT16 post_function(void){
  UINT16 timer_check_value1, timer_check_value2;   //Two values to check whether timer is running or not.
  UINT8 i;
  timer_check_value1 =  TCNT;                      //Take first value at some time.
  for(i=0;i<200;i++) {                             //Take some rest before reading second value
  }
  timer_check_value2 =  TCNT;                      //Take second value at some time
  if (timer_check_value2 == timer_check_value1) {
    (void)printf("POST failed! You buggy man.\r\n");        //POST get failed. Big reason to worry!
    return FALSE;
  }
  return TRUE;
}

