#include <p18f4550.h> 


//3102 probable configurations
/*
#pragma config FOSC=HSPLL_HS //Using XMHz crystal(above 4 MHz, assuming 8 MHz)
#pragma config PLLDIV=2 //Divide 8 by 2 provide 4 MHz to the 96 MHz PLL
#pragma config CPUDIV=OSC1_PLL2 //Divide 96 MHz by 2 to get 48 MHz for system clock
#pragma config USBDIV=2 //USB clock comes from 96MHz PLL/2
#pragma config IESO=ON
#pragma config WDT=OFF, PWRT=ON, BOR=OFF//Disable Watchdog timer and Brown Out Rest, Enable Power up timer
#pragma config LVP=OFF, MCLRE=ON, PBADEN=OFF, STVREN=ON
#pragma config FCMEN = ON
*/
//2361 configurations
#pragma config PLLDIV=2, CPUDIV=OSC1_PLL2, USBDIV=2, IESO=ON, WDT=OFF, BOR=OFF, PWRT=ON, LVP=OFF
#pragma config FOSC=HSPLL_HS, FCMEN=OFF, VREGEN=OFF, MCLRE=ON, LPT1OSC=ON, PBADEN=OFF
#pragma config STVREN=ON
void hi_isr(void);
void lo_isr(void);

#define CCP1IF PIR1bits.CCP1IF 
//adjust to 4 MHz Instruction 
#define COUNT_0 8922		//lowest cycle count (highest frequency)
#define COUNT_1 8948
#define COUNT_2 8974
#define COUNT_3 8999
#define COUNT_4 9025
#define COUNT_5 9052
#define COUNT_6 9078
#define COUNT_7 9104
#define COUNT_8 9130
#define COUNT_9 9157
#define COUNT_10 9184
#define COUNT_11 9210
#define COUNT_12 9236 	
#define COUNT_13 9263 //highest cycle count (lowest frequency)
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
	{
		//TMR1REG = 0x00;//reset the tmr1 register
		if(init_counter < size)//first size iterations will not yield an output
		//if(count < size)//average value will update only when size = count
		{
				int spot = count++;
				CCP1IF = 0;
				init_counter++;
				count = count % size;
				values[spot] = CCPR1;
				sum = sum + values[spot];
		
		}
		else		
		{
			int place = count++;
			count = count % size;
			CCP1IF = 0;
			sum = sum - values[place];//remove the previous value of the running sum
			values[place] = CCPR1;
			sum = sum + values[place];
			avg = sum/size;
			//reset LED's to off
			PORTD = 0x00;
			PORTA &= 0xC0;
			PORTEbits.RE0 = 0;
			switch(ledAverageChoice(avg))
			{
			case 0:
				PORTEbits.RE0 = 1; //Less than -30 cents
				return;				
			case 1:
				PORTAbits.RA5 = 1; //-30 cents
				return;	
			case 2:
				PORTAbits.RA4 = 1; //-25 cents
				return;
			case 3:
				PORTAbits.RA3 = 1; //-20 cents
				return;
			case 4:
				PORTAbits.RA2 = 1; //-15 cents
				return;
			case 5:
				PORTAbits.RA1 = 1; //-10 cents
				return;
			case 6:
				PORTAbits.RA0 = 1; //-5 cents
				return;
			case 7:
				PORTDbits.RD0 = 1; ;//0 Cents
				return;
			case 8:
				PORTDbits.RD1 = 1; //+5 cents
				return;
			case 9:
				PORTDbits.RD2 = 1; //+10 cents
				return;
			case 10:
				PORTDbits.RD3 = 1; //+15 cents
				return;
			case 11:
				PORTDbits.RD4 = 1; //+20 cents
				return;
			case 12:
				PORTDbits.RD5 = 1; //+25 cents
				return;
			case 13:
				PORTDbits.RD6 = 1; //+30 cents
				return;
			case 14:
				PORTDbits.RD7 = 1; //More than 30 Cents
				return;
			default:
				PORTD = 0xC0;//set LED's to indicate an error
				//PORTA |= 0x3F;
				PORTEbits.RE0 = 1;
				
				return;
			}											
			//reset average (for fixed sampling)
			//avg = 0;
		}
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
	int i;
	//probably want to add an if check to ensure that the timer1 interrupt occurs
	//TMR1REG = 0x00;//reset the tmr1 register
	//This section of code handles when TMR1 overflows. We made need to put this in the High_ISR or disable interuppts to prevent interruption
	INTCONbits.GIEH = 0;
	count = 0;
	PORTA = 0x00;
	PORTD = 0x00;
	init_counter = 0;
	
	for(i = 0; i <(size -1); i++)
	{
		values[i] = 0;
	}
	INTCONbits.GIEH = 0;
}

unsigned int ledAverageChoice( unsigned long int avg)
{
	//translates the avg value from CCP to a case value for led io
	//30 cents must be displayed through the cases inside the interrupt, below and above
	

	
	if( avg < COUNT_0 )
		return 14; //more than 30 cents
		
	if( (COUNT_0 <= avg && avg < COUNT_1))
		return 13; // +30 cents!
		
	if( COUNT_1 <= avg && avg < COUNT_2 )
		return 12; // +25 cents!

	if( COUNT_2 <= avg && avg < COUNT_3)
		return 11; // +20 cents!
		
	if( COUNT_3 <= avg && avg <COUNT_4 )
		return 10; // +15 cents!
		
	if(COUNT_4 <= avg && avg < COUNT_5 )
		return 9; // +10 cents!
	
	if(COUNT_5 <= avg && avg < COUNT_6)
		return 8; // +5 cents!

	if(COUNT_6 <= avg && avg < COUNT_7)
		return 7; // perfect (+0 cents)!
		
	if( COUNT_7<= avg && avg < COUNT_8)
		return 6; // -5 cents!
	
	if( COUNT_8 <= avg && avg < COUNT_9 )
		return 5; // -10 cents!
	
	if( COUNT_9 <= avg && avg < COUNT_10 )
		return 4; // -15 cents!
	
	if( COUNT_10 <= avg && avg <COUNT_11 )
		return 3; // -20 cents!
	
	if( COUNT_11 <= avg && avg <COUNT_12 )
		return 2; // -25 cents!
	
	if(	COUNT_12 <= avg && avg < COUNT_13 )
		return 1; // -30 cents!
	
	if( avg > COUNT_13)
		return 0;// less than 30 cents
		
	else return 15;
}


void setup(void){
	/*timer set up in this function, we'll need to configure tmr1 as a low priority 
	interrupt to handle cases when it overruns before a CCP even occurs*/
	UCFGbits.UPUEN = 0;
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
	OSCCONbits.IDLEN = 1; //PRI_IDLE modes
	OSCCONbits.SCS1 = 0;
	OSCCONbits.SCS0 = 0;
	while(1)
	{
		_asm SLEEP _endasm
	}
}