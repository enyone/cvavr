#include <mega8.h>

// Standard Input/Output functions
#include <stdio.h>
#include <delay.h>

          /* Sarjaliikenteen tarkistus-bitit */
#define RXB8 1          // RXB8 bitin sijainti rekisteriss‰‰n (tarvittiin edellisess‰ versiossa)
#define TXB8 0          // TXB8 bitin sijainti rekisteriss‰‰n (tarvittiin edellisess‰ versiossa)
#define UPE 2           // UPE bitin sijainti UCSRA rekisteriss‰
#define OVR 3           // OVR bitin sijainti UCSRA rekisteriss‰
#define FE 4            // FE bitin sijainti UCSRA rekisteriss‰
#define UDRE 5          // UDRE bitin sijainti rekisteriss‰‰n (tarvittiin edellisess‰ versiossa)
#define RXC 7           // RXC bitin sijainti rekisteriss‰‰n (tarvittiin edellisess‰ versiossa)

#define FRAMING_ERROR (1<<FE)           // kehysvirhe
#define PARITY_ERROR (1<<UPE)           // paritusvirhe ;)
#define DATA_OVERRUN (1<<OVR)           // dataa buukattu rekisteriin ennen edellisen l‰hetyst‰
#define DATA_REGISTER_EMPTY (1<<UDRE)   // data on luettu rekisterist‰ (tarvittiin edellisess‰ versiossa)
#define RX_COMPLETE (1<<RXC)            // datan vastaanotto suoritettu (tarvittiin edellisess‰ versiossa)


register char rcvdatacount, timerflag, timercounter;
unsigned char vale1_OCR1A=0xFF,vale1_OCR1B=0xFF,vale1_OCR2=0xFF,
              temp1_OCR1A=0xFF,temp1_OCR1B=0xFF,temp1_OCR2=0xFF,
              vale2_OCR1A=0xFF,vale2_OCR1B=0xFF,vale2_OCR2=0xFF,
              temp2_OCR1A=0xFF,temp2_OCR1B=0xFF,temp2_OCR2=0xFF,
              vale3_OCR1A=0xFF,vale3_OCR1B=0xFF,vale3_OCR2=0xFF,
              temp3_OCR1A=0xFF,temp3_OCR1B=0xFF,temp3_OCR2=0xFF,ddspeed;
int bigtimercounter=0;
unsigned int subtimercounter=0;

bit auto,moniajo,RYHMA1,RYHMA2,RYHMA3;


/* Sarjaportin tulevan datan keskeytys
  joka siksi, ett‰ muuttujia (tai muita asetuksia) voitaisiin muutella
  sarjaliikenteen avulla ulkoa p‰in 'client-ohjelmasta' */
interrupt [USART_RXC] void usart_rx_isr(void)
{
  unsigned char data;
  data = UDR;
  auto = 0;
  
  if ((UCSRA & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)   // tark. onko virheit‰?
  {
    switch(rcvdatacount)
    {
      case 0:
        temp1_OCR1A = data; rcvdatacount ++;
      break;
      case 1:
        temp1_OCR1B = data; rcvdatacount ++;
      break;
      case 2:
        temp1_OCR2 = data; rcvdatacount ++;
      break;
      case 3:
        if(data & 0b00000010)
          RYHMA1 = 1;
        else
          RYHMA1 = 0;
        if(data & 0b00000100)
          RYHMA2 = 1;
        else
          RYHMA2 = 0;
        if(data & 0b00001000)
          RYHMA3 = 1;
        else
          RYHMA3 = 0;
        
        if(data > 0x0E)
          printf("E");
        
        rcvdatacount ++;
      break;
      case 4:
        if(data == 0x00)
        {
          printf("C");
        }
        else if(data == 0x01)
        {
          printf("A");
          auto = 1;
        }
        else if(data == 0x02)
        {
          printf("R");
          #pragma optsize-
          WDTCR=0x18;
          WDTCR=0x08;
          #ifdef _OPTIMIZE_SIZE_
          #pragma optsize+
          #endif
          while(1);   // software resetti
        }
        else
        {
          printf("E");
        }
        rcvdatacount = 0;
      break;           
    }
  }
}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    timerflag = 1;
}

void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=Out Func1=Out Func0=In 
// State7=T State6=T State5=T State4=T State3=0 State2=0 State1=0 State0=T 
PORTB=0x00;
DDRB=0x0E;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=Out Func2=Out Func1=Out Func0=In 
// State6=T State5=T State4=T State3=0 State2=0 State1=0 State0=T 
PORTC=0x00;
DDRC=0x0E;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 10.800 kHz
TCCR0=0x05;
TCNT0=0xFF;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 43.200 kHz
// Mode: Ph. correct PWM top=00FFh
// OC1A output: Non-Inv.
// OC1B output: Non-Inv.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0xA1;
TCCR1B=0x04;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 43.200 kHz
// Mode: Phase correct PWM top=FFh
// OC2 output: Non-Inverted PWM
ASSR=0x00;
TCCR2=0x66;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x01;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 115200
UCSRA=0x00;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x05;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// Global enable interrupts
#asm("sei")

  RYHMA1 = 0;
  RYHMA2 = 0;
  RYHMA3 = 0;
  auto = 1;
  moniajo = 0;
  ddspeed = 1;

while (1)
      {
if(auto && timerflag)
  {
  if(timercounter % 2 == 0)
  {
    subtimercounter ++;
    switch(bigtimercounter)
    {
      case 0: // Kaikki p‰‰lle
        vale1_OCR1B = 0x00;
        if(temp1_OCR1B <= 200 && vale1_OCR1A == 0xFF)
          vale1_OCR1A = 0x00;
        if(temp1_OCR1A <= 200 && vale1_OCR2 == 0xFF)
          vale1_OCR2 = 0x00;
        if(temp1_OCR2 == 0x00)
          bigtimercounter ++;
      break;
      case 1: // Vihre‰ pois
        vale1_OCR2 = 0xFF;
        if(temp1_OCR2 == 0xFF)
          bigtimercounter ++;
      break;
      case 2: // Sininen pois
        vale1_OCR1A = 0xFF;
        if(temp1_OCR1A == 0xFF)
          bigtimercounter ++;
      break;
      case 3: // Vihre‰ p‰‰lle
        vale1_OCR2 = 0x00;
        if(temp1_OCR2 == 0x00)
          bigtimercounter ++;
      break;
      case 4: // Punainen pois
        vale1_OCR1B = 0xFF;
        if(temp1_OCR1B == 0xFF)
          bigtimercounter ++;
      break;
      case 5: // Sininen p‰‰lle
        vale1_OCR1A = 0x00;
        if(temp1_OCR1A == 0x00)
          bigtimercounter ++;
      break;
      case 6: // Vihre‰ pois
        vale1_OCR2 = 0xFF;
        if(temp1_OCR2 == 0xFF)
          bigtimercounter ++;
      break;
      case 7: // Punainen p‰‰lle
        vale1_OCR1B = 0x00;
        if(temp1_OCR1B == 0x00)
          bigtimercounter ++;
      break;
      case 8: // Sininen pois
        vale1_OCR1A = 0xFF;
        if(temp1_OCR1A == 0xFF)
          bigtimercounter ++;
      break;
      case 9: // Punainen pois
        vale1_OCR1B = 0xFF;
        if(temp1_OCR1B == 0xFF)
          bigtimercounter ++;
      break;
      case 10: // Sininen p‰‰lle
        vale1_OCR1A = 0x00;
        if(temp1_OCR1A == 0x00)
          bigtimercounter ++;
      break;
      case 11: // Sininen pois
        vale1_OCR1A = 0xFF;
        if(temp1_OCR1A == 0xFF)
          bigtimercounter ++;
      break;
      case 12: // Punainen p‰‰lle
        vale1_OCR1B = 0x00;
        if(temp1_OCR1B == 0x00)
          bigtimercounter ++;
      break;
      case 13: // Punainen pois
        vale1_OCR1B = 0xFF;
        if(temp1_OCR1B == 0xFF)
          bigtimercounter ++;
      break;
      case 14: // Vihre‰ p‰‰lle
        vale1_OCR2 = 0x00;
        if(temp1_OCR2 == 0x00)
          bigtimercounter ++;
      break;
      case 15: // Vihre‰ pois
        vale1_OCR2 = 0xFF;
        if(temp1_OCR2 == 0xFF)
          bigtimercounter ++;
      break;
      default: // Loppu
        bigtimercounter = 0;
        switch(ddspeed)
        {
          case 1:
            ddspeed = 3;
          break;
          case 3:
            ddspeed = 5;
          break;
          case 5:
            ddspeed = 15;
          break;
          default:
            ddspeed = 1;
          break;
        }
      break;
    }
    switch(subtimercounter)
    {
      case 3:
        RYHMA1 = 0;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 6:
        RYHMA1 = 1;
        RYHMA2 = 0;
        RYHMA3 = 1;
      break;
      case 9:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 0;
      break;
      case 12:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 300:
        RYHMA1 = 1;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 301:
        RYHMA1 = 0;
        RYHMA2 = 1;
        RYHMA3 = 0;
      break;
      case 302:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 1;
      break;
      case 303:
        RYHMA1 = 1;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 304:
        RYHMA1 = 0;
        RYHMA2 = 1;
        RYHMA3 = 0;
      break;
      case 305:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 1;
      break;
      case 306:
        RYHMA1 = 1;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 307:
        RYHMA1 = 0;
        RYHMA2 = 1;
        RYHMA3 = 0;
      break;
      case 308:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 1;
      break;
      case 309:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 600:
        RYHMA1 = 0;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 605:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 1;
      break;
      case 610:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 615:
        RYHMA1 = 1;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 620:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 0;
      break;
      case 625:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 900:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 901:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 902:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 903:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 904:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 905:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 906:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 907:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 908:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 909:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
      case 910:
        RYHMA1 = 0;
        RYHMA2 = 0;
        RYHMA3 = 0;
      break;
      case 911:
        RYHMA1 = 1;
        RYHMA2 = 1;
        RYHMA3 = 1;
      break;
    }
    if(subtimercounter==1200)
    {
      subtimercounter = 0;
    }
  }
    
  timercounter ++;

  if(vale1_OCR1A < temp1_OCR1A) temp1_OCR1A -= ddspeed;
  else if(vale1_OCR1A > temp1_OCR1A) temp1_OCR1A += ddspeed;
  
  if(vale1_OCR1B < temp1_OCR1B) temp1_OCR1B -= ddspeed;
  else if(vale1_OCR1B > temp1_OCR1B) temp1_OCR1B += ddspeed;

  if(vale1_OCR2 < temp1_OCR2) temp1_OCR2 -= ddspeed;
  else if(vale1_OCR2 > temp1_OCR2) temp1_OCR2 += ddspeed;
  
  timerflag = 0; 
  }

  if(moniajo)
  {
        PORTC.3 = RYHMA1;
        PORTC.2 = 0;
        PORTC.1 = 0;
    OCR1A = temp1_OCR1A;
    OCR1B = temp1_OCR1B;
    OCR2 = temp1_OCR2;
    delay_us(100);
        PORTC.3 = 0;
        PORTC.2 = RYHMA2;
        PORTC.1 = 0;
    OCR1A = temp2_OCR1A;
    OCR1B = temp2_OCR1B;
    OCR2 = temp2_OCR2;
    delay_us(100);
        PORTC.3 = 0;
        PORTC.2 = 0;
        PORTC.1 = RYHMA3;
    OCR1A = temp3_OCR1A;
    OCR1B = temp3_OCR1B;
    OCR2 = temp3_OCR2;
    delay_us(100);
  }
  else
  {
        PORTC.3 = RYHMA1;
        PORTC.2 = RYHMA2;
        PORTC.1 = RYHMA3;
    OCR1A = temp1_OCR1A;
    OCR1B = temp1_OCR1B;
    OCR2 = temp1_OCR2;
  }
 
}

}
