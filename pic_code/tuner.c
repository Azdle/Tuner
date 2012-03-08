#include <p18f4550.h> 


#pragma config FOSC=HSPLL_HS //Using XMHz crystal(above 4 MHz, assuming 8 MHz)
#pragma config PLLDIV=2 //Divide 8 by 2 provide 4 MHz to the 96 MHz PLL
#pragma config CPUDIV=OSC1_PLL2 //Divide 96 MHz by 2 to get 48 MHz for system clock
#pragma config USBDIV=2 //USB clock comes from 96MHz PLL/2
#pragma config IESO=ON
#pragma config WDT=OFF, PWRT=ON, BOR=OFF//Disable Watchdog timer and Brown Out Rest, Enable Power up timer
#pragma config LVP=OFF, MCLRE=ON, PBADEN=OFF, STVREN=ON
#pragma config FCMEN = ON

void hi_isr(void);
void lo_isr(void);

#define CCP1IF PIR1bits.CCP1IF 
//using 16 MHz Fosc (48 MHz Clock)
#define COUNT_1 35790	//lowest cycle count (highest frequency)
#define COUNT_2 35893
#define COUNT_3 35996
#define COUNT_4 36099
#define COUNT_5 36207
#define COUNT_6 36312
#define COUNT_7 36416
#define COUNT_8 36521
#define COUNT_9 36627
#define COUNT_10 36733
#define COUNT_11 36840
#define COUNT_12 36946 	//highest cycle count (lowest frequency)

#define size 5 //size of the array being used.
int tmr_overflow;//used for tmr overflow counts
unsigned long int values[size];//May not need to have the items be this large
unsigned char count = 0; //used for keeping track of place in array
unsigned long int sum;
unsigned long int avg;// could not define this in the isr?
unsigned char init_counter = 0;
unsigned int ledAverageChoice(unsigned long int avg);


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
		if(init_counter < size)//first size iterations will not yield an output
		{
				int spot = count++;
				CCPIF = 0;
				count = count % size;
				values[spot] = CCPR1
				sum = sum + CCPR1;
		
		}
		//This will ensure that only 1 time in every 5 will output, not necessarily bad, but is it what we want?
		/*if(count < 5)//on the occurance of size iterations 
		{
			CCP1IF = 0;
			count++;
			sum += CCPR1;
		}
		*/
		else		
		{
			int place = count++;
			count = count % 5;//this will break the above if statement, since count will never be 5
			CCP1IF = 0;
			sum = sum - values[place];//remove the previous value of the running sum
			values[place] = CCPR1;
			sum = sum + values[place];
			//need to check for tmr1 overflows and calculate those in this section.
			avg = sum/size;

			switch(ledAverageChoice(avg))
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
			case 8:
				//set appropriate LED's
				return;
			
			case 9:
				//set appropriate LED's
				return;
			case 10:
				//set appropriate LED's
				return;
			case 11:
				//set appropriate LED's
				return;
			case 12:
				//set appropriate LED's
				return;

			default:
				PORTD = 0xFF;//set all LED's to indicate an error
				PORTA = 0xFF;
				return;
			}											
		}
		OSCCONbits.IDLEN = 1; //PRI_IDLE modes
		OSCCONbits.SCS1 = 0;
		OSCCONbits.SCS0 = 0;
		_asm SLEEP _endasm
		
}



#pragma code low_vector = 0x18
void lo_isr_entry(void)
{
    _asm goto lo_isr _endasm
}



#pragma code
#pragma interruptlow lo_isr
void lo_isr(void){
	//This section of code handles when TMR0 overflows. We made need to put this in the High_ISR or disable interuppts to prevent interruption
	int i;
	INTCONbits.GIEH = 0;
	count = 0;
	PORTA = 0x00;
	PORTD = 0x00;
	
	for(i = 0; i <(size -1); i++)
	{
		values[i] = 0;
	}
	INTCONbits.GIEH = 0;
}

int ledAverageChoice( unsigned long int avg)
{
	//translates the avg value from CCP to a case value for led io
	//30 cents must be displayed through the cases inside the interrupt, below and above
	if(COUNT_6 < avg && avg < COUNT_7)
		return 13; // perfect (+0 cents)!
		
	if(COUNT_5 < avg && avg < COUNT_6)
		return 1; // +5 cents!
	
	if(COUNT_4 < avg && avg < COUNT_5 )
		return 2; // +10 cents!
	
	if( COUNT_3< avg && avg <COUNT_4 )
		return 3; // +15 cents!
	
	if( COUNT_2<avg && avg < COUNT_3)
		return 4; // +20 cents!
	
	if( COUNT_1< avg && avg < COUNT_2 )
		return 5; // +25 cents!
	
	if( (avg < COUNT_1))
		return 6; // +30 cents!
	
	if( COUNT_7 < avg && avg < COUNT_8)
		return 7; // -5 cents!
	
	if( COUNT_8 < avg && avg < COUNT_9 )
		return 8; // -10 cents!
	
	if( COUNT_9< avg && avg < COUNT_10 )
		return 9; // -15 cents!
	
	if( COUNT_10< avg && avg <COUNT_11 )
		return 10; // -20 cents!
	
	if( COUNT_11< avg && avg <COUNT_12 )
		return 11; // -25 cents!
	
	if(	avg > COUNT_12 )
		return 12; // -30 cents!
	else return 15;
}


void setup(void){
	/*timer set up in this function, we'll need to configure tmr0 as a low priority 
	interrupt to handle cases when it overruns before a CCP even occurs*/
	T1CON = 0x49;
	PIR1bits.TMR1IF = 0;
	
	IPR1bits.TMR1IP = 0;
	//timer 1 must be in timer mode or synchronized mode


	//Setup LED ports
	TRISD = 0x00;
	TRISA = 0x00;
	PORTD = 0x00;
	PORTA = 0x00;//initially output nothing
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
	
}


void main(void){
	setup();
	while(1);
}