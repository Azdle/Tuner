#include <p18f4550.h> 

void hi_isr(void);
void lo_isr(void);

#define CCP1IF PIR1bits.CCP1IF 


#define size 5 //size of the array being used.
int tmr_overflow;//used for tmr overflow counts
unsigned long int values[size];
unsigned char count = 0; //used for keeping track of place in array
unsigned long int sum;
unsigned long int avg;// could not define this in the isr?


#pragma code high_vector = 0x08
void hi_isr_entry(void)
{
    _asm goto hi_isr _endasm
}
 
#pragma code
#pragma interrupt hi_isr
void hi_isr(void)
{ 
	if(CCP1IF)
	{//maybe should write a calculate function?
		int place = count++;
		count = count % 5;
		CCP1IF = 0;
		sum = sum - values[place];//remove the previous value of the running sum
		values[place] = CCPR1;
		sum = sum + values[place];
		//need to check for tmr1 overflows and calculate those in this section.
		avg = sum/size;
		
	}
	switch( avg )
	{
		case 1:
			//set appropriate LED's
			return;
		
		case 2:
			//set appropriate LED's
			return;
		case 3:
			//set appropriate LED's
			return;
		case 4:
			//set appropriate LED's
			return;
		case 5:
			//set appropriate LED's
			return;
		case 6:
			//set appropriate LED's
			return;
		case 7:
			//set appropriate LED's
			return;
		default:
			//set appropriate LED's
			return;
	}											



}



#pragma code low_vector = 0x18
void lo_isr_entry(void)
{
    _asm goto lo_isr _endasm
}



#pragma code
#pragma interruptlow lo_isr
void lo_isr(void){

}


void setup(void){
	/*timer set up in this function, we'll need to configure tmr0 as a low priority 
	interrupt to handle cases when it overruns before a CCP even occurs*/
	//PIR1bits.TMR1 = 0;
	//IPR1bits.TMR1IP = 0;
	//timer 1 must be in timer mode or synchronized mode

	/*make sure to set the appropriate pins for outputs using the TRIS
	registers
	*/
	tmr_overflow = 0; /* this will be used to track the number of times 
						that tmr1 overflows. Used for calculation only*/
	
	T3CON = 0x00;//bits 6 and 3 are used to select tmr1 as the counter for the CCP

	RCONbits.IPEN = 1; //enable priority with interrupts
	INTCONbits.GIEH = 1; //enable high priority interrupts
	INTCONbits. GIEL = 1; // enable low priority interrupts
	CCP1IF = 0; 
	PIE1bits.CCP1IE = 1;
	IPR1 = 0b00000100; // set the CCP to high priority
	TRISCbits.TRISC2 = 1; // set the CCP1 pin as an input
//	CCP1CON &= 0xF0;//reset the CCP module (possibly unecessary)
	CCP1CON |= 0x04; //select capture mode, event on every falling edge
	
	//
	
	



}


void main(void){
	setup();
}