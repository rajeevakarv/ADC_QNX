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



// Normally I'd use something awesome like a bool but we're stuck with this err
// limited system.
// This is used to let the program know when to capture values.
//UINT16 captureValues = FALSE;

// holds the timer values captured on the rising edge.
//UINT16 timerValuesUs [1001] = { 0 };

// holds the time inteval between rising edges.
//UINT16 pulseIntervalsUs [1000] = { 0 };

UINT8 motorControlLogic = FALSE;         //This variable will enable the logic for motor driving.
UINT8 motorPosition = 0;                 //This variable will set the position of servo.
UINT8 servoA = FALSE;
UINT8 servoB = FALSE;


UINT16 getUINT16Input(void);
void processTimerMeasurements(UINT16 lowerBoundaryUs, UINT16 upperBoundaryUs);
//UINT16 MaxInputValue;
//UINT16 MinInputValue;

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



void InitializePWM0() {

 PWMCLK_PCLK0 = 1;
 PWMPOL_PPOL0 = 1;
 PWMSCLA = 0x50;
 PWMPER0 = 0xFF; 
 PWME_PWME0 = 1;
}

void InitializePWM2() {
 
 PWMCLK_PCLK2 = 1;
 PWMPOL_PPOL2 = 1;   
 PWMSCLB = 0x50;
 PWMPER2 = 0xFF;
 PWME_PWME2 = 1;
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
    
  // Enable output compare on Channel 1
  TIOS_IOS1 = 1;
  
  // Set up output compare action to toggle Port T, bit 1
  TCTL2_OM1 = 0;
  TCTL2_OL1 = 1;
  
  // Set up timer compare value
  TC1 = TC1_VAL;
  
  // Clear the Output Compare Interrupt Flag (Channel 1) 
  TFLG1 = TFLG1_C1F_MASK;
  
  // Enable the output compare interrupt on Channel 1;
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


/*
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
void interrupt 8 OC1_isr( void )
{
   // This interrupt stores the values from the table into the array.
   // we don't want to do any calculations because we are dealing with
   // Us and want the reads to be as accurate as possible.
   motorControlLogic = ~motorControlLogic;           // Switch the Flag as user push the button
  
   
   // set the interrupt enable flag for that port because it is cleared every
   // everytime an interrupt fires.
   TFLG1   =   TFLG1_C1F_MASK;
}
#pragma pop

*/

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


// This function is called by printf in order to
// output data. Our implementation will use polled
// serial I/O on SCI0 to output the character.
//
// Remember to call InitializeSerialPort() before using printf!
//
// Parameters: character to output
//--------------------------------------------------------------      
void TERMIO_PutChar(INT8 ch)
{
    // Poll for the last transmit to be complete
    do
    {
      // Nothing 
    } while (SCI0SR1_TC == 0);
    
    // write the data to the output shift register
    SCI0DRL = ch;
}


// Polls for a character on the serial port.
//
// Returns: Received character
//--------------------------------------------------------------      
UINT8 GetChar(void)
{
  // Poll for data
  do
  {
    // Nothing
  } while(SCI0SR1_RDRF == 0);
   
  // Fetch and return data from SCI0
  return SCI0DRL;
}


void run(){

   if(servoA == TRUE) {
       
//   InitializePWM0();
   PWMDTY0 = motorPosition;
   } else if(servoB == TRUE) {
   
  // InitializePWM2();
   PWMDTY2 = motorPosition;
   }
   
}






// Entry point of our application code
//--------------------------------------------------------------    

#pragma push
#pragma CODE_SEG __SHORT_SEG NON_BANKED
//--------------------------------------------------------------      
void interrupt 9 OC1_isr( void ) {

        TC1     +=  TC1_VAL;      
        TFLG1   =   TFLG1_C1F_MASK; 
         if(motorControlLogic) {
                motorPosition = PORTB ;
                if (motorPosition & 0x80){
                  
                    servoB = TRUE;                         // Enable the ServoB if it is negetive value
                    servoA = FALSE;
                }else {
                  
                    servoA = TRUE;                         // Enable servoA if it is positive value
                    servoB = FALSE;
                }
                motorPosition = motorPosition & 0x7F;      // Set position for servo motor to run
         } 
         else {
             servoB = FALSE;                           // If push button is pushed, diable both servos.
             servoA = FALSE;
         }
          run();
   
   
   
   // set the interrupt enable flag for that port because it is cleared every
   // everytime an interrupt fires
}
#pragma pop


  
void main(void)
{

  UINT8 userInput = 0;
  UINT16 lowerBoundaryUs = 0;
  UINT16 upperBoundaryUs = 0;
  UINT8 temp = 0;
  INT16 i=0;
  
  DDRB = 0x00;
  DDRA = 0x00;
  
  InitializeSerialPort();
  InitializeTimer();
  InitializePWM0();
  InitializePWM2();
  
  temp = PORTA;
  (void)printf("temp = %d \n\r", temp ); 

  (void) printf("\r\n\r\n Motors are running.!!!\r\n\r\n");   
  
  // if we pass post run the program otherwise go home.
 //start of main loop
    for(;;) {
          if( (PORTA & 0x01) == 0) {
            
              motorControlLogic = ~motorControlLogic;
              for(i=32000; i>0; --i){
              }
              (void)printf("Control = %d,  motorPosition = %d\n\r", motorControlLogic, motorPosition ); 
          }
     }
     
/*     for(;;)
     {
         (void)printf("Control = %d \n\r", motorControlLogic ); 
         userInput = GetChar();
         if(motorControlLogic) {
                motorPosition = PORTB ;
                if (motorPosition & 0x80){
                  
                    servoB = TRUE;                         // Enable the ServoB if it is negetive value
                    servoA = FALSE;
                }else {
                  
                    servoA = TRUE;                         // Enable servoA if it is positive value
                    servoB = FALSE;
                }
                motorPosition = motorPosition & 0x7F;      // Set position for servo motor to run. 
                (void)printf("Position = %d \n\r", motorPosition );
         } 
         else {
             servoB = FALSE;                           // If push button is pushed, diable both servos.
             servoA = FALSE;
         }
         
     }
*/ 
  (void) printf("\r\n\r\nOk I'm outa here!!!\r\n\r\n");
}


