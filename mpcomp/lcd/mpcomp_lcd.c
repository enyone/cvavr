
/****************************************/
/*
        PROJECT   "MPCOMP_LCD"
        VERSION   "firmware v6.7.7"
        DATE      "date 07.07.2006"
        AUTHOR    "by Juho Tykkálá"
        MISC      "risc 8MHz cpu 1"
        
        Current:  --mA ilman taustavaloa (0%)
                  --mA sarjaliikenteellä ja lämpöanturilla
                  --mA taustavalolla (100%)
*/
/****************************************/
/*********** ä = „ **** ö = ” ***********/
/*********** Ä = Ž **** Ö = ™ ***********/
/**********  asteen merkki = ² **********/


/* Includoidaan tarvittavat headerit */
#include <mega162.h>
#include <stdio.h>
#include <delay.h>
#include <string.h>
#include <math.h>


/* LCD moduulin asetukset */
#asm
   .equ __lcd_port=0x15 ;PORTC
#endasm
#include <lcd4x40.h>


/* Definet */
#define LCD_BL PORTE.2
#define LCD_PWR PORTA.6

#define BTNS PINA

#define NAPPI1 0x02
#define NAPPI2 0x04
#define NAPPI3 0x08

#define LED1 PORTB.0
#define LED2 PORTD.2
#define LED3 PORTB.1
#define LED4 PORTA.4
#define LED5 PORTA.5

#define RADIO_CHAN PORTD.5
#define RADIO_PWR PORTD.6
#define RADIO_RxTx PORTD.7

#define GPS_PWR PORTD.4


/************************************/
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define EEMWE 2
#define EEWE 1
#define EERE 0

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
/************************************/


/* Pääalustusfunktio */
void initd(void)
{
// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port A initialization
// Func7=In Func6=Out Func5=Out Func4=Out Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=1 State5=1 State4=1 State3=P State2=P State1=P State0=T 
PORTA=0x7E;
DDRA=0x70;

// Port B initialization
// Func7=In Func6=Out Func5=In Func4=In Func3=In Func2=In Func1=Out Func0=Out 
// State7=T State6=0 State5=T State4=T State3=T State2=T State1=1 State0=1 
PORTB=0x03;
DDRB=0x43;

// Port C initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=In Func2=Out Func1=In Func0=In 
// State7=1 State6=0 State5=1 State4=0 State3=T State2=1 State1=T State0=T 
PORTD=0xA4;
DDRD=0xF4;

// Port E initialization
// Func2=Out Func1=In Func0=In 
// State2=1 State1=T State0=T 
PORTE=0x04;
DDRE=0x04;

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
// Clock value: 115.200 kHz
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
TCCR1B=0x0B;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x04;
OCR1AL=0x80;
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

// Timer/Counter 3 initialization
// Clock value: Timer 3 Stopped
// Mode: Normal top=FFFFh
// Noise Canceler: Off
// Input Capture on Falling Edge
// OC3A output: Discon.
// OC3B output: Discon.
// Timer 3 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR3A=0x00;
TCCR3B=0x00;
TCNT3H=0x00;
TCNT3L=0x00;
ICR3H=0x00;
ICR3L=0x00;
OCR3AH=0x00;
OCR3AL=0x00;
OCR3BH=0x00;
OCR3BL=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
// Interrupt on any change on pins PCINT0-7: On
// Interrupt on any change on pins PCINT8-15: Off
GICR|=0x08;
PCMSK0=0x0E;
MCUCR=0x00;
EMCUCR=0x00;
GIFR=0x08;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x40;
ETIMSK=0x00;

// USART0 initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART0 Receiver: On
// USART0 Transmitter: On
// USART0 Mode: Asynchronous
// USART0 Baud rate: 2400
UCSR0A=0x00;
UCSR0B=0x98;
UCSR0C=0x86;
UBRR0H=0x00;
UBRR0L=0xBF;

// USART1 initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART1 Receiver: On
// USART1 Transmitter: On
// USART1 Mode: Asynchronous
// USART1 Baud rate: 2400
UCSR1A=0x00;
UCSR1B=0x98;
UCSR1C=0x86;
UBRR1H=0x00;
UBRR1L=0xBF;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;

// SPI initialization
// SPI Type: Slave
// SPI Clock Rate: 57.600 kHz
// SPI Clock Phase: Cycle Half
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
SPCR=0xC3;
SPSR=0x00;

// Clear the SPI interrupt flag
#asm
    in   r30,spsr
    in   r30,spdr
#endasm

lcd_init();

#asm("sei");
}


typedef struct
{
  unsigned char backlight, sleeptimer, outbox, ajovalot, tempshield, gpspower, gpssleep, gpsnmea, radiopower, radiochan, radioway, radiotest;
  unsigned int kehamitta;
} ASETUKSET;

#define ASET_TAVU_MAARA 14

ASETUKSET aset;

typedef struct
{
  unsigned char nopeus, rpm, jaahdytys, oljy, vaihteisto, kaasu, polttoaine, kmhbar[39], rpmbar[39];
  unsigned long matka;
} TIEDOT;

TIEDOT tied;

register char debounce, i, j, k, z, u0, u1, timercount, updatedelaz;

unsigned long int time_real, time_stopw, time_vstopw, time_bstopw=0xFFFF, time_mbstopw, time_wstopw, time_running;

char naytto_tila;

unsigned char char_sett, buttons, kerroin2, lcd_buff1[42], lcd_buff2[42],
              lcd_buff3[42], lcd_buff4[42], day, updatedelax, u_radio_string[6], u_comp_string[8],
              fblink, spi_data, spi_string[3][42], asetstamp[6], asetcount,
              sleepon_sb, sleepon_sd, gostopw, laps, updatedelay, reset, outbox,
              serial1_null, scount, wakecount;

unsigned int re_time_h, re_time_m, re_time_s, re2_time_h, re2_time_m, re2_time_s, kerroin1, rdebounce;



/* Funktio tavun lattaamiseen eepromista */
unsigned char xeepromRead(unsigned int uiAddress)
{
  while(EECR & (1<<EEWE));
  EEAR = uiAddress;
  #asm("cli");
  EECR |= (1<<EERE);
  #asm("sei");
  return EEDR;  
}


/* Funktio tavun tallentamiseen eepromiin */
void xeepromWrite(unsigned int uiAddress, unsigned char ucData)
{
  while(EECR & (1<<EEWE));
  EEAR = uiAddress;
  EEDR = ucData;
  #asm("cli");
  EECR |= (1<<EEMWE);
  EECR |= (1<<EEWE);
  #asm("sei");
}


/* Funktio asetusten lattaamiseen eepromista */
void loadAset(void)
{
  unsigned int eepaddr=1;
  unsigned char *asetaddr=&aset.backlight;
  for(z=0;z<ASET_TAVU_MAARA;z++)
  {
    *asetaddr = xeepromRead(eepaddr);
    eepaddr ++; asetaddr ++;
  }
  eepaddr = 1;
  asetaddr = &aset.backlight;
  
  if(aset.backlight != 0 && aset.backlight != 1) aset.backlight = 1;
  if(aset.outbox != 0 && aset.outbox != 1) aset.outbox = 0;
  if(aset.ajovalot != 0 && aset.ajovalot != 1 && aset.ajovalot != 2) aset.ajovalot = 0;
  if(aset.gpspower != 0 && aset.gpspower != 1) aset.gpspower = 0;
  if(aset.gpssleep != 0 && aset.gpssleep != 1) aset.gpssleep = 0;
  if(aset.gpsnmea != 0 && aset.gpsnmea != 1) aset.gpsnmea = 0;
  if(aset.radiopower != 0 && aset.radiopower != 1) aset.radiopower = 0;
  if(aset.radiochan != 0 && aset.radiochan != 1) aset.radiochan = 0;
  if(aset.radioway != 0 && aset.radioway != 1) aset.radioway = 0;
  if(aset.radiotest != 0 && aset.radiotest != 1) aset.radiotest = 0;
  if(aset.sleeptimer == 255) aset.sleeptimer = 60;
  if(aset.tempshield == 255) aset.tempshield = 80;
  if(aset.kehamitta == 65535) aset.kehamitta = 1500;
}


/* Funktio asetusten tallentamiseen eepromiin */
void saveAset(void)
{
  unsigned int eepaddr=1;
  unsigned char *asetaddr=&aset.backlight;
  for(z=0;z<ASET_TAVU_MAARA;z++)
  {        
  
    xeepromWrite(eepaddr, *asetaddr);
    eepaddr ++; asetaddr ++;
  }
  eepaddr = 1;
  asetaddr = &aset.backlight;
}


/* Funktio radion alustamiseen */
void initRadio(void)
{
  RADIO_PWR = 1;
  delay_ms(10);
  RADIO_CHAN = 1;
  RADIO_RxTx = 1;
  delay_ms(10);
}


/* Funktio näytön alustamiseen */
void initLcd(void)
{
  LCD_PWR = 0;
  lcd_clear();

  LCD_BL = 0;
}


/* Funktio GPS:n alustamiseen */
void initGps(void)
{
  //GPS_PWR = 1;
}


/* Funktio datan lähettämiseen radiolla */
#pragma used+
void sendRadioChar(char c)
{
  while(!(UCSR0A & DATA_REGISTER_EMPTY));
  UDR0 = c;
}
#pragma used-


/* Funktio datan lähettämiseen keskusyksikköön */
#pragma used+
void sendCompChar(char c)
{
  while(!(UCSR1A & DATA_REGISTER_EMPTY));
  UDR1 = c;
}
#pragma used-


/* Funktio asetusten käyttöönottoon */
void syncComp(void)
{
  sendCompChar(reset); // reset
  sendCompChar(outbox); // onoff
  sendCompChar(aset.ajovalot); // rele1
  sendCompChar(0); // rele2
  sendCompChar(aset.kehamitta>>8); // kehamitta_h
  sendCompChar(aset.kehamitta); // kehamitta_l
}


/* Funktio etälaitteen hereillä pitämiseen */
void pingComp(void)
{
  outbox = aset.outbox;
  reset = 0;
  syncComp();
}


/* Funktio asetusten käyttöönottoon */
void syncAsetukset(void)
{
  if(aset.backlight) LCD_BL = 0; else LCD_BL = 1;
  if(aset.gpspower) GPS_PWR = 1; else GPS_PWR = 0;
  if(aset.radiopower) RADIO_PWR = 1; else RADIO_PWR = 0;
  if(aset.radiochan) RADIO_CHAN = 1; else RADIO_CHAN = 0;
  if(aset.radioway) RADIO_RxTx = 1; else RADIO_RxTx = 0;
  
  if(aset.ajovalot != 0) LED4 = 0;
  else LED4 = 1;
  
  if(tied.vaihteisto == 0) LED3 = 0;
  else LED3 = 1;
  
  outbox = aset.outbox;
  reset = 0;
  syncComp();
}


void syncFromRadio(void)
{

}


void syncFromComp(void)
{
  int temp1;

  tied.nopeus = u_comp_string[0];
  tied.rpm = u_comp_string[1];
  tied.jaahdytys = u_comp_string[2];
  tied.oljy = u_comp_string[3];
  tied.vaihteisto = u_comp_string[4];
  tied.kaasu = u_comp_string[5];

  if(tied.vaihteisto == 0) LED3 = 0;
  else LED3 = 1; 

  temp1 = u_comp_string[6];
  tied.matka += (temp1<<8) + u_comp_string[7];
}


/* Usart0:n tulokeskeytys */
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
  unsigned char u0_data;
  u0_data = UDR0;
  
  if ((UCSR0A & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)   // tark. onko virheitä?
  {
    u_radio_string[u0] = u0_data;
    u0 ++;
    
    if(u0 >= 5) { u0 = 0; syncFromRadio(); }
  }
  else LED2 = 1;
}


/* Usart1:n tulokeskeytys */
interrupt [USART1_RXC] void usart1_rx_isr(void)
{
  unsigned char u1_data;
  u1_data = UDR1;
  
  if(serial1_null > 1 && u1) LED2 = 1;
  
  scount = 0;
  serial1_null = 0;
  
  if ((UCSR1A & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)   // tark. onko virheitä?
  {
    u_comp_string[u1] = u1_data;
    u1 ++;
    
    if(u1 >= 7) { u1 = 0; syncFromComp(); }
  }
  else LED2 = 1;
}


/* Timer 1:n ylivuodon keskeytys (100x sekunnissa tai 10ms välein) */
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
  /* 100x sekunnissa */
  timercount++;
  updatedelaz = 1;
  
  if(gostopw) time_stopw ++;
  
  /* 10x sekunnissa */
  if((timercount % 10) == 0)
  {
    updatedelay = 1;
  }
  
  /* 5x sekunnissa */
  if((timercount % 20) == 0)
  {
  }
  
  /* 2x sekunnissa */
  if((timercount % 50) == 0)
  {
    if(fblink) fblink = 0; else fblink = 1;
  }
  
  /* 1x sekunnissa */
  if(timercount >= 100)
  {
    timercount = 0;
    updatedelax = 1;
    if(tied.rpm) time_running ++;
  }
}


/* SPI-datan keskeytys */
interrupt [SPI_STC] void spi_isr(void)
{
  spi_data = SPDR;
  
  if(spi_data == '\n') { k = 0; spi_string[z][40] = '\0'; z++; if(z == 3) z = 0; }
  else
  {
    spi_string[z][k] = spi_data;
    k ++;
  }
}


/* Funktio näyttötilan vaihtamiseen */
void changeNayttoTila(char xila)
{
  naytto_tila = xila;
  asetcount = 0;
  char_sett = 0;
  buttons = 0x00;
}


/* Nappuloiden keskeytysrutiini */
interrupt [PCINT0] void pin_change_isr0(void)
{
  if(sleepon_sb || sleepon_sd)
  {
    LED2 = 0;
    
    if(sleepon_sd)
    {
      changeNayttoTila(0);
    }
    if(sleepon_sb)
    {
      changeNayttoTila(1);
    }
    initLcd();  
    
    sleepon_sb = 0;
    sleepon_sd = 0;
    
    delay_ms(500);
    
    syncAsetukset();
  }
}


/* Funktio nappien painelun tarkistamiseen */
void checkButtons(void)
{
  if(BTNS.1 == 0 || BTNS.2 == 0 || BTNS.3 == 0)
  {
    if(debounce == 255)
    {
      buttons |= ~BTNS;
      debounce = 0;
    }
    
    if(rdebounce < 60000) rdebounce ++;
    else { debounce = 255; }
  }
  else
  {
    if(debounce != 255) debounce ++;
    rdebounce = 0;
  }
}


/* Funktio tarkasteltujen nappuloiden datojen lukemiseen */
char getButtons(char btn)
{
  if((buttons & btn))
  {
    buttons &= ~btn;
    return 1;
  }
  else
  {
    return 0;
  }
}


/* Funktio ajan (h,m,s)/(m,s,100/s) laskemiseen sekunneista */
void convertTime(unsigned long int *tuloaika)
{
  re_time_h = *tuloaika/kerroin1;
  re_time_m = *tuloaika%kerroin1;
  re_time_s = re_time_m%kerroin2;
  re_time_m = re_time_m/kerroin2;
}


/* Funktio laittees asettamiseen valmiustilaan */
void initStandby(void)
{
  lcd_clear();
  lcd_gotoxy(15,1);
  lcd_putsf("valmiustila");
  if(aset.gpssleep) GPS_PWR = 0;
  LED1 = 1;
  LED2 = 1;
  LED5 = 1;
  delay_ms(1500);
  LCD_BL = 1;
  sleepon_sb = 1;
  
  outbox = 0;
  reset = 0;
  syncComp();
  
  MCUCR |= 0b00110000;    // 00 SE SM1 0000
  MCUCSR |= 0b00100000;   // 00 SM2 00000
  EMCUCR |= 0b00000000;   // SM0 0000000
  
  #asm("sleep");
  
  LED2 = 1;
}


/* Funktio laitteen sammuttamiseen */
void initShutdown(void)
{
  lcd_clear();
  LCD_PWR = 1;
  RADIO_PWR = 0;
  GPS_PWR = 0;
  LED1 = 1;
  LED2 = 1;
  LED3 = 1;
  LED4 = 1;
  LED5 = 1;
  delay_ms(500);
  LCD_BL = 1;
  sleepon_sd = 1;
  
  outbox = 0;
  reset = 1;
  syncComp();
  reset = 0;
  
  MCUCR |= 0b00110000;    // 00 SE SM1 0000
  MCUCSR |= 0b00000000;   // 00 SM2 00000
  EMCUCR |= 0b00000000;   // SM0 0000000
  
  #asm("sleep");
  
  LED2 = 1;
}


/* Funktio laitteen resetoimiseen */
void initReset(void)
{
  outbox = 0;
  reset = 1;
  syncComp();
  reset = 0;
  
  #pragma optsize-
  WDTCR=0x18;
  WDTCR=0x08;
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif
  
  while(1);
  
  LED2 = 1;
}


char valikkochar(char luku)
{
  switch(luku)
  {
    case 0: return '-'; break;
    case 1: return '+'; break;
    case 2: return 'A'; break;
    default: return '?'; break;
  }
}


/* Näytön päivitys */
void updateScreen(void)
{
  switch(naytto_tila)
  {
    /* Näyttötila -3 */
    case -3:
        strcpyf(lcd_buff1, "             \xBCLaiteasetukset\xBB");
        
        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }
        
        sprintf(lcd_buff2, "%cTaustavalo: (%c)", asetstamp[0], valikkochar(aset.backlight));
        sprintf(lcd_buff3, "%cUniajastin: (%-3usek)", asetstamp[1], aset.sleeptimer);
        sprintf(lcd_buff4, "%cUlkoyksikk”: (%c)             %cPoistu...", asetstamp[2], valikkochar(aset.outbox), asetstamp[3]);
        
        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 4) asetcount = 0; else if(asetcount == -1) asetcount = 3;
        if(getButtons(NAPPI2))
        {
          if(asetcount == 0) { aset.backlight ++; if(aset.backlight >= 2) aset.backlight = 0; }
          else if(asetcount == 1) { aset.sleeptimer = aset.sleeptimer+30; if(aset.sleeptimer >= 241) aset.sleeptimer = 0; }
          else if(asetcount == 2) { aset.outbox ++; if(aset.outbox >= 2) aset.outbox = 0; }  
          else changeNayttoTila(-2);
          syncAsetukset();
        }  
    break;

    /* Näyttötila -4 */
    case -4:
        strcpyf(lcd_buff1, "           \xBCAjoneuvoasetukset\xBB");
        
        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }
        
        sprintf(lcd_buff2, "%cKeh„mitta: (%-4umm)", asetstamp[0], aset.kehamitta);
        sprintf(lcd_buff3, "%cAjovalot: (%c)", asetstamp[1], valikkochar(aset.ajovalot));
        sprintf(lcd_buff4, "%cL„mp”suoja: (%-2u²c)           %cPoistu...", asetstamp[2], aset.tempshield, asetstamp[3]);
        
        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 4) asetcount = 0; else if(asetcount == -1) asetcount = 3;
        if(getButtons(NAPPI2))
        {
          if(asetcount == 0) { aset.kehamitta ++; if(aset.kehamitta > 9999) aset.kehamitta = 0; }
          else if(asetcount == 1) { aset.ajovalot ++; if(aset.ajovalot >= 3) aset.ajovalot = 0; }
          else if(asetcount == 2) { aset.tempshield ++; if(aset.tempshield >= 100) aset.tempshield = 0; }  
          else changeNayttoTila(-2);
          syncAsetukset();
        }  
    break;

    /* Näyttötila -5 */
    case -5:
        strcpyf(lcd_buff1, "             \xBCGPS-asetukset\xBB");
        
        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }
        
        sprintf(lcd_buff2, "%cGPS k„yt”ss„: (%c)", asetstamp[0], valikkochar(aset.gpspower));
        sprintf(lcd_buff3, "%cSammuta valmiustilassa: (%c)", asetstamp[1], valikkochar(aset.gpssleep));
        sprintf(lcd_buff4, "%cNMEA-protokolla: (%c)         %cPoistu...", asetstamp[2], valikkochar(aset.gpsnmea), asetstamp[3]);
        
        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 4) asetcount = 0; else if(asetcount == -1) asetcount = 3;
        if(getButtons(NAPPI2))
        {
          if(asetcount == 0) { aset.gpspower ++; if(aset.gpspower >= 2) aset.gpspower = 0; }
          else if(asetcount == 1) { aset.gpssleep ++; if(aset.gpssleep >= 2) aset.gpssleep = 0; }
          else if(asetcount == 2) { aset.gpsnmea ++; if(aset.gpsnmea >= 2) aset.gpsnmea = 0; }
          else changeNayttoTila(-2);
          syncAsetukset();
        }  
    break;

    /* Näyttötila -6 */
    case -6:
        strcpyf(lcd_buff1, "             \xBCRadioasetukset\xBB");
        
        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }
        
        sprintf(lcd_buff2, "%cRadio k„yt”ss„: (%c)      %cTestiajo: (%c)", asetstamp[0], valikkochar(aset.radiopower), asetstamp[3], valikkochar(aset.radiotest));
        sprintf(lcd_buff3, "%cKanava: (%d)", asetstamp[1], aset.radiochan+1);
        sprintf(lcd_buff4, "%cL„hetystila: (%c)             %cPoistu...", asetstamp[2], valikkochar(aset.radioway), asetstamp[4]);
        
        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 5) asetcount = 0; else if(asetcount == -1) asetcount = 4;
        if(getButtons(NAPPI2))
        {
          if(asetcount == 0) { aset.radiopower ++; if(aset.radiopower >= 2) aset.radiopower = 0; }
          else if(asetcount == 1) { aset.radiochan ++; if(aset.radiochan >= 2) aset.radiochan = 0; }
          else if(asetcount == 2) { aset.radioway ++; if(aset.radioway >= 2) aset.radioway = 0; }
          else if(asetcount == 3) { aset.radiotest ++; if(aset.radiotest >= 2) aset.radiotest = 0; }
          else changeNayttoTila(-2);
          syncAsetukset();
        }  
    break;

    /* Näyttötila -7 */
    case -7:
        strcpyf(lcd_buff2, "Paina nappia tallentaaksesi...");
        
        if(getButtons(NAPPI1) || getButtons(NAPPI2) || getButtons(NAPPI3))
        {
          
          lcd_clear();
          lcd_gotoxy(0,1);
          lcd_putsf("Tallennetaan asetuksia...");
          
          saveAset();

          syncAsetukset();
          
          changeNayttoTila(-2);
        }
    break;

    /* Näyttötila -2 */
    case -2:
        strcpyf(lcd_buff1, "             \xBCAsetusvalikko\xBB");
        
        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }
        
        sprintf(lcd_buff2, "%cLaiteasetukset          %cRadioasetukset", asetstamp[0], asetstamp[3]);
        sprintf(lcd_buff3, "%cAjoneuvoasetukset          %cTallenna...", asetstamp[1], asetstamp[4]);
        sprintf(lcd_buff4, "%cGPS-asetukset                %cPoistu...", asetstamp[2], asetstamp[5]);
        
        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 6) asetcount = 0; else if(asetcount == -1) asetcount = 5;
        if(getButtons(NAPPI2))
        {
          if(asetcount < 5) changeNayttoTila((-3)-asetcount); 
          else changeNayttoTila(1);
        }  
    break;

    /* Näyttötila -1 */
    case -1:
        strcpyf(lcd_buff1, "             \xBCSammuta laite\xBB");

        for(i=0;i<6;i++)
        {
          if(asetcount == i) asetstamp[i] = 246;
          else asetstamp[i] = 205;
        }

        sprintf(lcd_buff2, "%cValmiustila", asetstamp[0]);
        sprintf(lcd_buff3, "%cSammuta laite", asetstamp[1]);
        sprintf(lcd_buff4, "%cResetoi laite                %cPoistu...", asetstamp[2], asetstamp[3]);

        if(getButtons(NAPPI1)) asetcount --;
        if(getButtons(NAPPI3)) asetcount ++;
        if(asetcount == 4) asetcount = 0; else if(asetcount == -1) asetcount = 3;
        if(getButtons(NAPPI2))
        {
          if(asetcount == 0) initStandby();
          else if(asetcount == 1) initShutdown();
          else if(asetcount == 2) initReset();
          else changeNayttoTila(1);
        }  
    break;
  
    /* Näyttötila 0 */
    case 0:
       strcpyf(lcd_buff1, "RC MPCOMP RACING COMPUTER &");
       strcpyf(lcd_buff2, " GPS NAVIGATION SYSTEM");
       strcpyf(lcd_buff3, "  firmware v. v7.1.7 / by: Juho Tykk„l„");
       
       if(!fblink) strcpyf(lcd_buff4, "Ladataan j„rjestelm„„...");

       /*sprintf(lcd_buff1, "%d:%c %d:%c", day, day, day+1, day+1);
       sprintf(lcd_buff2, "%d:%c %d:%c", day+2, day+2, day+3, day+3);
       if(getButtons(NAPPI1)) day += 4;
       if(getButtons(NAPPI3)) day -= 4;*/

       if(getButtons(NAPPI2)) changeNayttoTila(1);

    break;

    /* Näyttötila 1 */
    case 1:
        strcpyf(lcd_buff1, "         \xBCAjonaikaiset tiedot\xBB");
        kerroin1 = 3600; kerroin2 = 60;
        convertTime(&time_running);
        sprintf(lcd_buff2, "%3ukm/h  %3u00rpm  %05um  #%u  %02u:%02u:%02u", tied.nopeus, tied.rpm, (tied.matka / 1000), tied.vaihteisto, re_time_h, re_time_m, re_time_s);

        for(i=0;i<38;i++)
        {
          tied.kmhbar[i] = ' ';
          tied.rpmbar[i] = ' ';
        }

        for(i=0;i<(tied.nopeus / 4);i++)
        {
          tied.kmhbar[i] = 246;
        }

        for(i=0;i<(tied.rpm / 3);i++)
        {
          tied.rpmbar[i] = 246;
        }
        
        tied.kmhbar[38] = '\0';
        tied.rpmbar[38] = '\0';
        
        sprintf(lcd_buff3, "S%c%-38s", 246, tied.kmhbar);
        sprintf(lcd_buff4, "R%c%-38s", 246, tied.rpmbar);
        
        if(getButtons(NAPPI1)) changeNayttoTila(-2);
        if(getButtons(NAPPI2)) changeNayttoTila(2);
        if(getButtons(NAPPI3)) changeNayttoTila(-1);  
    break;
    
    /* Näyttötila 2 */
    case 2:
        strcpyf(lcd_buff1, "           \xBCAjoneuvon tiedot\xBB");
        kerroin1 = 3600; kerroin2 = 60;
        convertTime(&time_running);
        sprintf(lcd_buff2, "%3ukm/h  %3u00rpm  %05um  #%u  %02u:%02u:%02u", tied.nopeus, tied.rpm, (tied.matka / 1000), tied.vaihteisto, re_time_h, re_time_m, re_time_s);
        sprintf(lcd_buff3, "J„„hdytys: %2u²c (+ok)  ™ljy %2u²c (+ok)", tied.jaahdytys, tied.oljy);
        sprintf(lcd_buff4, "Moottori: -STOP-  Polttoaine: %2u%c (+hi)", tied.polttoaine, 37);
        
        if(getButtons(NAPPI2)) changeNayttoTila(3);
    break;
    
    /* Näyttötila 3 */
    case 3:
        strcpyf(lcd_buff1, "         \xBCAjanottoj„rjestelm„\xBB");
        
        kerroin1 = 6000; kerroin2 = 100;
        if(time_bstopw != 0xFFFF) time_mbstopw = time_bstopw; else time_mbstopw = 0;
        convertTime(&time_mbstopw);
        re2_time_h = re_time_h; re2_time_m = re_time_m; re2_time_s = re_time_s;
        convertTime(&time_stopw);
        sprintf(lcd_buff2, "Aika: %02u:%02u.%02u           Paras: %02u:%02u.%02u", re_time_h, re_time_m, re_time_s, re2_time_h, re2_time_m, re2_time_s);
        convertTime(&time_wstopw);
        sprintf(lcd_buff3, "Kierrokset: %2u kpl     Huonoin: %02u:%02u.%02u", laps, re_time_h, re_time_m, re_time_s);
        convertTime(&time_vstopw);
        sprintf(lcd_buff4, "V„liaika: %02u:%02u.%02u", re_time_h, re_time_m, re_time_s);
        
        if(getButtons(NAPPI2)) changeNayttoTila(4);  
    break;
    
    /* Näyttötila 4 */
    case 4:
        strcpyf(lcd_buff1, "            \xBCGPS-tilan„ytt”\xBB");
        
        if(!aset.gpspower) { strcpyf(lcd_buff4, "GPS ei k„yt”ss„ !"); spi_string[1][0] = 0; }
        // else if(spi_string[2][2] == 0) strcpyf(lcd_buff4, "Odotetaan GPS-laitetta ...");
        else
        {
          sprintf(lcd_buff2, "%-40s", spi_string[0]);
          sprintf(lcd_buff3, "%-40s", spi_string[1]);
          sprintf(lcd_buff4, "%-40s", spi_string[2]);
        }
        
        if(getButtons(NAPPI2)) changeNayttoTila(1);  
    break;
  }
  
  sprintf(lcd_buff1, "%-40s", lcd_buff1);
  sprintf(lcd_buff2, "%-40s", lcd_buff2);
  sprintf(lcd_buff3, "%-40s", lcd_buff3);
  sprintf(lcd_buff4, "%-40s", lcd_buff4);
  
  lcd_gotoxy(0,0);
  lcd_puts(lcd_buff1);
  lcd_gotoxy(0,1);
  lcd_puts(lcd_buff2);
  lcd_gotoxy(0,2);
  lcd_puts(lcd_buff3);
  lcd_gotoxy(0,3);
  lcd_puts(lcd_buff4);
  
  strcpyf(lcd_buff1, "                                        ");
  strcpy(lcd_buff2, lcd_buff1);
  strcpy(lcd_buff3, lcd_buff1);
  strcpy(lcd_buff4, lcd_buff1);
}


void main(void)
{

  initd();
  
  initShutdown();
  
  changeNayttoTila(0);

  initLcd();  
  initRadio();
  initGps();
  
  loadAset();
  syncAsetukset();
  
  while (1)
  {
    checkButtons();
    
    if(naytto_tila == 3)
    {
      if(getButtons(NAPPI1))
      {
        if(gostopw) gostopw = 0; else gostopw = 1;
      }
      if(getButtons(NAPPI3))
      {
        if(gostopw)
        {
          laps ++;
          time_vstopw = time_stopw;
          if(time_stopw < time_bstopw) time_bstopw = time_stopw;
          if(time_stopw >= time_wstopw) time_wstopw = time_stopw;
          time_stopw = 0;
        }
        else
        {
          if(time_stopw == 0)
          {
            time_vstopw = 0;
            time_bstopw = 0xFFFF;
            time_wstopw = 0;
            laps = 0;
          }
          time_stopw = 0;
        }
      }
    }
    
    /* 100x sekunnissa */
    if(updatedelaz)
    { 
      updatedelaz = 0;
      
      if(serial1_null <= 4) serial1_null ++;
      else u1 = 0;
      
      if(scount <= 5) { scount ++; LED1 = 0; }
      else if(aset.outbox) LED1 = 1;
    }
    
    /* 10x sekunnissa */
    if(updatedelay)
    {    
      updatedelay = 0;
      updateScreen();
      
      LED5 = ~PINA.5;
    }
    
    /* 1x sekunnissa */
    if(updatedelax)
    {
      if(aset.radiotest)
      {
        sendRadioChar('T');
        sendRadioChar('x');
      }
      
      if(wakecount >= 10) { wakecount = 0; pingComp(); }
      else wakecount ++;
      
      if(!aset.outbox) LED1 = ~PINB.0;
      updatedelax = 0;
    }
  };

}


