
/****************************************/
/*
        PROJECT   "MPCOMP_GPS"
        VERSION   "firmware v7.1.11"
        DATE      "date 11.01.2007"
        AUTHOR    "by Juho Tykk�l�"
        MISC      "risc 11.0592MHz cpu 1"
        
        Current:  --mA ilman taustavaloa (0%)
                  --mA sarjaliikenteell� ja l�mp�anturilla
                  --mA taustavalolla (100%)
*/
/****************************************/
/*********** � = � **** � = � ***********/
/*********** � = � **** � = � ***********/
/**********  asteen merkki = � **********/


/* Includoidaan tarvittavat headerit */
#include <mega8.h>
#include <delay.h>
#include <stdio.h>
#include <string.h>


/* Definet */
#define UPE 2
#define OVR 3
#define FE 4

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)

#define GPS_NMEA PORTB.1


/* P��alustusfunktio */
void initd(void)
{
// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=Out Func4=In Func3=Out Func2=Out Func1=Out Func0=In 
// State7=T State6=T State5=0 State4=T State3=0 State2=0 State1=1 State0=T 
PORTB=0x02;
DDRB=0x2E;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=1 State2=T State1=T State0=T 
PORTD=0x08;
DDRD=0x08;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
TCCR0=0x00;
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 10.800 kHz
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
TCCR1B=0x0D;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x6C;
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

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x10;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 4800
UCSRA=0x00;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x8F;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 86.400 kHz
// SPI Clock Phase: Cycle Half
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
SPCR=0xD3;
SPSR=0x00;

// Clear the SPI interrupt flag
// #asm
//     in   r30,spsr
//     in   r30,spdr
// #endasm

// Global enable interrupts
#asm("sei")
}

typedef struct
{
  unsigned char sn, ew, magnetic_ew, time_s, time_m, time_h, date_d, date_m, date_y, status;
  unsigned int lat_d, lat_m, lat_s, lat_ss, lon_d, lon_m, lon_s, lon_ss, speed, magnetic, course;
} GPSD;

GPSD gpsd;

register unsigned char j, z;

unsigned char timeout, secmark, dlen, ydone, xdone, gps_data_status, tahti=' ';

unsigned char gps_buffer[512], send_data[3][40];

unsigned int i;

/* USART-datan keskeytys */
interrupt [USART_RXC] void usart_rx_isr(void)
{
  unsigned char status,data;
  status=UCSRA;
  data=UDR;
  if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
  {
    if(data == '\n' || data == '\r') data = 20;
    gps_buffer[i] = data;
    i ++;
    timeout = 0;
    xdone = 1;
    if(i > 511) i = 0;
  };
}


/* SPI-datan keskeytys */
interrupt [SPI_STC] void spi_isr(void)
{
  unsigned char data;
  data=SPDR;
}


/* Timer1 keskeytys */
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
  if(timeout >= 10 && xdone)
  {
    dlen = i;
    i = 0;
    ydone = 1;
    xdone = 0;
  }
  else timeout ++;
  
  secmark ++;
}


/* Funktio SPI-datan l�hetykseen */
void putspi(char d) {
	SPDR = d;
	while(!(SPSR & 0b10000000));
}


/* Funktio tietojen l�hetykseen sarjaportin kautta */
void initGps(void)
{
  printf("$PRWIILOG,???,V,,,,\r\n");
  delay_ms(100);
  printf("$PRWIILOG,RMC,A,,,,\r\n");
  delay_ms(100);
}


/* Funktio stringien yht�l�isyyksien parsintaan */
// char strEquals(char *source, char flash *mask, unsigned int start, unsigned int stop)
// {
//   for(k=start;k<=stop;k++)
//   {
//     if(*source[k] == *mask[k])
//     {
//       equals_count ++;
//     }
//   }
//   
//   if(equals_count == (stop-start+1))
//   {
//     equals_count = 0;
//     return 1;
//   }
//   
//   return 0;
// }


void syncData(void)
{
  for(z=0;z<3;z++)
  {      
    for(j=0;j<40;j++)
    {
      putspi(send_data[z][j]);
      send_data[z][j] = ' ';
    }
    putspi('\n');
  }
}


void main(void)
{

  initd();
  
  //delay_ms(100);
  //GPS_NMEA = 0;
  //delay_ms(100);
  
  //initGps();
  
  /*gps_data_status = 0;

  strcpyf(send_data[1], "GPS driver ver. 7.1.11");
  strcpyf(send_data[2], "Odota...");
  syncData();*/
  
  PORTD.3 = 1;
  
  while (1)
  {
    if(ydone)
    {
      ydone = 0;
      
      gps_data_status = 0;
      
      if(
        gps_buffer[0] == '$' &&
        gps_buffer[1] == 'P' &&
        gps_buffer[2] == 'R' &&
        gps_buffer[3] == 'W' &&
        gps_buffer[4] == 'I' &&
        gps_buffer[5] == 'R' &&
        gps_buffer[6] == 'I' &&
        gps_buffer[7] == 'D'
      ) gps_data_status = 1;
      else if(
        gps_buffer[0] == '$' &&
        gps_buffer[1] == 'G' &&
        gps_buffer[2] == 'P' &&
        gps_buffer[3] == 'R' &&
        gps_buffer[4] == 'M' &&
        gps_buffer[5] == 'C'
      ) gps_data_status = 2;
      
      if(gps_data_status == 0 || gps_data_status == 1) {}
      else if(gps_data_status == 2)
      {
        gpsd.lat_d = ((gps_buffer[16]-48)*10)+(gps_buffer[17]-48);
        gpsd.lat_m = ((gps_buffer[18]-48)*10)+(gps_buffer[19]-48);
        gpsd.lat_s = ((gps_buffer[21]-48)*1000)+((gps_buffer[22]-48)*100)+((gps_buffer[23]-48)*10)+(gps_buffer[24]-48)+256;
        gpsd.lat_ss = (gpsd.lat_s*6)%1000;
        gpsd.lat_s = (gpsd.lat_s*6)/1000;
        
        gpsd.lon_d = ((gps_buffer[28]-48)*100)+((gps_buffer[29]-48)*10)+(gps_buffer[30]-48);
        gpsd.lon_m = ((gps_buffer[31]-48)*10)+(gps_buffer[32]-48);
        gpsd.lon_s = ((gps_buffer[34]-48)*1000)+((gps_buffer[35]-48)*100)+((gps_buffer[36]-48)*10)+(gps_buffer[37]-48)+768;
        gpsd.lon_ss = (gpsd.lon_s*6)%1000;
        gpsd.lon_s = (gpsd.lon_s*6)/1000;
        
        gpsd.time_h = ((gps_buffer[7]-48)*10)+((gps_buffer[8]-48));
        gpsd.time_m = ((gps_buffer[9]-48)*10)+((gps_buffer[10]-48));
        gpsd.time_s = ((gps_buffer[11]-48)*10)+((gps_buffer[12]-48));
        
        gpsd.date_d = ((gps_buffer[51]-48)*10)+((gps_buffer[52]-48));
        gpsd.date_m = ((gps_buffer[53]-48)*10)+((gps_buffer[54]-48));
        gpsd.date_y = ((gps_buffer[55]-48)*10)+((gps_buffer[56]-48));
        
        gpsd.status = gps_buffer[14];
        
        gpsd.speed = ((gps_buffer[41]-48)*1000)+((gps_buffer[43]-48)*100)+((gps_buffer[44]-48)*10)+(gps_buffer[45]-48);
        
        gpsd.course = ((gps_buffer[47]-48)*10)+((gps_buffer[49]-48));
        
        gpsd.magnetic = ((gps_buffer[58]-48)*10)+((gps_buffer[60]-48));
        gpsd.magnetic_ew = gps_buffer[62];
        
        gpsd.sn = gps_buffer[26];
        gpsd.ew = gps_buffer[39];
        
        if(gpsd.status == 'V') sprintf(send_data[2], "%c Haetaan sijaintia...", tahti);
        else
        {
          sprintf(send_data[0], "Pit: %2u�%02u\'%02u.%-03u\"%c  Lev: %2u�%02u\'%02u.%-03u\"%c", gpsd.lat_d, gpsd.lat_m, gpsd.lat_s, gpsd.lat_ss, gpsd.sn, gpsd.lon_d, gpsd.lon_m, gpsd.lon_s, gpsd.lon_ss, gpsd.ew);
          sprintf(send_data[1], "Aika: %02u:%02u:%02u %02u.%02u.%02u  Nopeus: %01u.%03uK", gpsd.time_h, gpsd.time_m, gpsd.time_s, gpsd.date_d, gpsd.date_m, gpsd.date_y, gpsd.speed/100, gpsd.speed%100);
          sprintf(send_data[2], "Kurssi: %01u.%01u�  Magv: %01u.%01u�%c", gpsd.course/10, gpsd.course%10, gpsd.magnetic/10, gpsd.magnetic%10, gpsd.magnetic_ew);
        }
      }
      else sprintf(send_data[2], "%c Virheellinen gps-vastaus...", tahti);
    }
    
    if(secmark >= 100)
    {
      if(tahti == ' ') tahti = '*'; else tahti = ' ';
      
      if(gps_data_status == 0)
      {
        sprintf(send_data[2], "%c Ladataan gps j�rjestelm��...", tahti);
      }
      else if(gps_data_status == 1)
      {
        sprintf(send_data[2], "%c Odotetaan gps-laitetta...", tahti);
      }
      
      PORTD.3 = ~PIND.3;
      
      syncData();
      secmark = 0;
    }
  };

}
