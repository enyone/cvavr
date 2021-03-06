
/****************************************/
/*
        PROJECT   "MPCOMP_COMP"
        VERSION   "firmware v7.1.7"
        DATE      "date 07.01.2007"
        AUTHOR    "by Juho Tykk�l�"
        MISC      "risc 8MHz cpu 1"
        
        Current:  --mA ilman taustavaloa (0%)
                  --mA sarjaliikenteell� ja l�mp�anturilla
                  --mA taustavalolla (100%)
*/
/****************************************/
/*********** � = � **** � = � ***********/
/*********** � = � **** � = � ***********/
/**********  asteen merkki = � **********/

#include <mega16.h>
#include <string.h>

#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define RXC 7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

#define INPUT_WHEEL PINC.0
#define INPUT_MOTOR PINC.1

#define LED1 PORTB.0
#define LED2 PORTB.1

#include <stdio.h>
#include <delay.h>


/* P��alustusfunktio */
void initd(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port A initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTA=0x00;
DDRA=0x00;

// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=Out Func0=Out 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=1 State0=1 
PORTB=0x03;
DDRB=0x03;

// Port C initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=P State6=P State5=P State4=P State3=P State2=P State1=P State0=P 
PORTC=0xFF;
DDRC=0x00;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
MCUCR=0x00;
MCUCSR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x10;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 2400
UCSRA=0x00;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0xBF;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// Global enable interrupts
#asm("sei")
}

void initTimers(void)
{
// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Normal top=FFh
// OC0 output: Disconnected
TCCR0=0x00;
TCNT0=0x00;
OCR0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 28.800 kHz
// Mode: CTC top=OCR1A
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x0C;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x01;
OCR1AL=0x20;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;
}

typedef struct
{
  unsigned char onoff, rele1, rele2;
} TIEDOT;

TIEDOT tied;

register char u0, blink;

char u_lcd_string[6];

unsigned char sendcount=0,reset=0,serial0_null,bcount,scount,timerflag,blinkspeed=100;

unsigned int timercount=0, ring=1885, rpmcount=0, rpmtcount=0, matka=0, wakecheck;

bit check_wheel, check_motor;

unsigned char kmh=0,rpm=0;


/* Funktio laittees asettamiseen valmiustilaan */
void initIdle(void)
{
  delay_ms(100);
  
  LED1 = 1;
  LED2 = 1;
  wakecheck = 0;
  
  MCUCR |= 0b01000000;    // SM2 SE SM1 SM0 0000
  
  #asm("sleep");
  
  blinkspeed = 10;
}


/* Funktio laitteen resetoimiseen */
void initReset(void)
{
  #pragma optsize-
  WDTCR=0x18;
  WDTCR=0x08;
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif
  
  while(1);
}


void syncAset(void)
{
  reset = u_lcd_string[0];
  tied.onoff = u_lcd_string[1];
  tied.rele1 = u_lcd_string[2];
  tied.rele2 = u_lcd_string[3];
  ring = u_lcd_string[4];
  ring = (ring<<8) + u_lcd_string[5];
  
  if(reset) initReset();
}


void wheelInterrupt(void)
{
  if(timercount >= 100) kmh = 0;
  else kmh = ((ring*(360000/timercount))/1000000);
  timercount = 0;
  
  matka = matka + ring; 
}


void motorInterrupt(void)
{
  rpmcount++;
}


/* Timer1 vertailukeskeytys (10ms v�lein) */
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
  if(timercount < 65534) timercount++;
  rpmtcount++;
  timerflag = 1;
  
  if(rpmtcount >= 50)
  {
    rpmtcount = 0;
    rpm = (rpmcount * 120) / 100;
    rpmcount = 0;
  }
  if(timercount >= 100) kmh = 0; 
}


/* Usart tulokeskeytys */
interrupt [USART_RXC] void usart_rx_isr(void)
{
  unsigned char u0_data;
  scount = 0;
  u0_data = UDR;
  wakecheck = 1100;
  
  if(serial0_null > 1 && u0) blinkspeed = 10;
  
  blinkspeed = 100;
  
  serial0_null = 0;
  
  if ((UCSRA & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)   // tark. onko virheit�?
  {
    u_lcd_string[u0] = 0x00;
    u_lcd_string[u0] = u0_data;
    u0 ++;
    
    if(u0 >= 5) { u0 = 0; syncAset(); }
  }
  else blinkspeed = 10;
}


/* Funktio datan l�hett�miseen n�yt�lle */
#pragma used+
void sendLcdChar(char c)
{
  scount = 0;
  while(!(UCSRA & DATA_REGISTER_EMPTY));
  UDR = c;
}
#pragma used-


void readInputs(void)
{
  if(!INPUT_WHEEL && check_wheel == 0) { wheelInterrupt(); check_wheel = 1; }
  else if(check_wheel == 1 && INPUT_WHEEL) { check_wheel = 0; }
  
  if(!INPUT_MOTOR && check_motor == 0) { motorInterrupt(); check_motor = 1; }
  else if(check_motor == 1 && INPUT_MOTOR) { check_motor = 0; }
}


void main(void)
{
  initd();
       
  LED1 = 0;
  LED2 = 0;
  initIdle();
  
  initTimers();
  
  while (1)
  {
    readInputs();
    
    if(timerflag)
    {
      bcount ++;
      if(bcount >= blinkspeed)
      {
        if(blink == 0) blink = 1; else blink = 0;
        LED1 = blink;
        bcount = 0;
      }
      
      if(scount <= 1) { scount ++; LED2 = 0; }
      else LED2 = 1;
      
      sendcount ++;
      
      if(serial0_null <= 4) serial0_null ++;
      else u0 = 0;
      
      if(wakecheck > 0) wakecheck --;
      else initReset();
      
      timerflag = 0;
    }
    
    if(sendcount >= 25)
    {
      if(tied.onoff)
      {
        sendLcdChar(kmh);
        sendLcdChar(rpm);
        sendLcdChar(20);
        sendLcdChar(20);
        sendLcdChar(1);
        sendLcdChar(1);
        sendLcdChar(matka>>8);
        sendLcdChar(matka);
        sendcount = 0;
        matka = 0;
      }
    }    
  }
}
