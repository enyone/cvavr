

/****************************************/
/*
        PROJECT   "Chronometri"
        VERSION   "firmware v6.8.9"
        DATE      "date 09.08.2006"
        AUTHOR    "by Juho Tykkálá"
        
        Virtaimu:  23mA ilman taustavaloa (0%)
                   34mA sarjaliikenteellä ja lämpöanturilla
                   120mA taustavalolla (100%)
*/
/****************************************/
/*********** ä = á **** ö = ï ***********/


/* Includoidaan tarvittavat headerit */
#include <mega8.h>
#include <stdio.h>
#include <delay.h>
#include <string.h>
#include <math.h>


/* 1 Wire väylän asetukset */
#asm
   .equ __w1_port=0x15 ;PORTC   // anturi(t) on portissa C..
   .equ __w1_bit=5              // ..bitissä 5
#endasm
#include <1wire.h>
/* ds1820 lämpötila-anturin headerit */
#include <ds1820.h>


/* LCD moduulin asetukset */
#asm
   .equ __lcd_port=0x12 ;PORTD    // näyttö on portissa D
   .equ __lcd_portx=0x18 ;PORTB   // jotkut pinnit näytöstä myös portissa B (koipien puutteen vuoksi)
#endasm
#include <lcd.h>    // HOX! lcd.h ei ole oletuskamaa, sillä kaikki näytön pinnit eivät ole samassa portissa


/* MAX_DS1820 lämpötila-anturin asetukset
  jos enemmän kuin 1 kpl antureita */
// #define MAX_DS1820 8
// unsigned char ds1820_devices;
// unsigned char ds1820_rom_codes[MAX_DS1820][9];


/* Definet */
#define LEDI PORTC.4    // punainen ledi
#define LEDI2 PORTB.5   // vihreä ledi
#define LEDI3 PORTB.4   // keltainen ledi

#define PIIPPERI PORTB.0    // summeri

#define BTNS PINC       // nappuloitten portti

#define NAPPI1 0x01     // napin 1 sijainti portissa
#define NAPPI2 0x04     // napin 2 sijainti portissa
#define NAPPI3 0x02     // napin 3 sijainti portissa
#define NAPPI4 0x08     // napin 4 sijainti portissa

#define KIRKAS 0xFF     // taustavalo kirkkaimmillaan
#define HIMMEA 15       // taustavalo himmeänä (oletusarvo)
#define PIMEA 0         // taustavalo sammuksissa

#define SNOOZE 15       // torkkuajastin minuutteina (oletusarvo)


/* Sarjaliikenteen tarkistus-bitit */
#define RXB8 1          // RXB8 bitin sijainti rekisterissään (tarvittiin edellisessä versiossa)
#define TXB8 0          // TXB8 bitin sijainti rekisterissään (tarvittiin edellisessä versiossa)
#define UPE 2           // UPE bitin sijainti UCSRA rekisterissä
#define OVR 3           // OVR bitin sijainti UCSRA rekisterissä
#define FE 4            // FE bitin sijainti UCSRA rekisterissä
#define UDRE 5          // UDRE bitin sijainti rekisterissään (tarvittiin edellisessä versiossa)
#define RXC 7           // RXC bitin sijainti rekisterissään (tarvittiin edellisessä versiossa)

#define FRAMING_ERROR (1<<FE)           // kehysvirhe
#define PARITY_ERROR (1<<UPE)           // paritusvirhe ;)
#define DATA_OVERRUN (1<<OVR)           // dataa buukattu rekisteriin ennen edellisen lähetystä
#define DATA_REGISTER_EMPTY (1<<UDRE)   // data on luettu rekisteristä (tarvittiin edellisessä versiossa)
#define RX_COMPLETE (1<<RXC)            // datan vastaanotto suoritettu (tarvittiin edellisessä versiossa)


/* Pääalustusfunktio
  (CodeWizardin tuottamaa tavaraa) */
void initd(void)
{

/* Porttien asetukset */
// Port B initialization
// Func7=In Func6=In Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=T State6=T State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
DDRB=0x3F;

// Port C initialization
// Func6=In Func5=In Func4=Out Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=T State4=0 State3=P State2=P State1=P State0=P 
PORTC=0x0F;
DDRC=0x10;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=Out Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=0 State0=T 

/* Timereitten asetukset */
// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 125.000 kHz
// Mode: CTC top=OCR1A
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: Off
TCCR1B=0x0B;
OCR1AH=0x04; // OCR1AH=0x04;
OCR1AL=0xE1; // OCR1AL=0xE2;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 250.000 kHz
// Mode: Phase correct PWM top=FFh
// OC2 output: Non-Inverted PWM
TCCR2=0x62;

/* Sarjaliikenteen asetukset */
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 9600
UCSRB=0x98;
UCSRC=0x86;
UBRRL=0x33;

/* Keskeytysten asetukset */
TIMSK=0x10;

/* Analogisen vertailijan asetukset */
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;

/* LCD moduulin alustus */
lcd_init(16);

/* 1 Wire väylän alustus */
w1_init();

/* Etsitään kaikki DS1820 anturit ja niiden osoitteet */
// ds1820_devices=w1_search(0xf0,ds1820_rom_codes);

/* Sallitaan keskeytykset */
#asm("sei")

}

/* Kuukausien päivät, viikonpäivät (1&2) */
flash unsigned char monthdays[12]={31,28,31,30,31,30,31,31,30,31,30,31},
                    weekdays1[7]={'M','T','K','T','P','L','S'},
                    weekdays2[7]={'A','I','E','O','E','A','U'};

/* Globaalit muuttujat */
unsigned long int time_real, time_alarm, time_stopw, time_cdown, time_cdown_set;

register char naytto_tila, timercount, updatedelaz, rcvdatacount, debounce, xhalytys, yhalytys, zhalytys, fivesecs1, fblink, sblink;

unsigned char lcd_buff1[18], lcd_ser1[18], lcd_ser2[18], lcd_buff2[18],
              char_sett, gostopw, laps, gocdown, buttons, tjlasku,
              lcd_ser, lcd_ser_v, isset, isact, kerroin2,
              isalarm, clsnooze, gettemp, istempal, vuoroohj, wday,
              karkauscheck, getserial, autonull=1, isvoice=1, isbacklight=1,
              day=1, month=1, year=6, isbright=HIMMEA, issnooze=SNOOZE;

unsigned int re_time_h, re_time_m, re_time_s, rdebounce, redebounce, kerroin1;

int lampotila_low, lampotila_hi, storeaddr, lampotila=3000;

char lampotila_c, * rmemaddr, lampotila_alow=-40, lampotila_ahi=40, updatedelax, updatedelay;

unsigned char vale_OCR2, lastbtn, lastbtn2;

/* Funktio taustavalon asettamiseen
    Ajetaan joka kerta, kun näyttöä päivitetään
    eli noin 10x sekunnissa */
void bLight(void)
{
  if(!isbacklight) vale_OCR2 = PIMEA;   // jos taustavalo ei käytössä, taustavalo pimeäksi
  else vale_OCR2 = KIRKAS;              // muuten asetetaan taustavalo päälle 100%
  fivesecs1 = 0;                        // asetetaan 5sek lippu nollille
}

/* Sarjaportin tulevan datan keskeytys
  joka siksi, että muuttujia (tai muita asetuksia) voitaisiin muutella
  sarjaliikenteen avulla ulkoa päin 'client-ohjelmasta' */
interrupt [USART_RXC] void usart_rx_isr(void)
{
  unsigned char data;
  data = UDR;   // luetaan sana usartin rekisteristä
  
  if ((UCSRA & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)   // tark. onko virheitä?
  {
    autonull = 0;   // laitetaan data-timeout-laskurit nollille
    switch(rcvdatacount)
    {
      case 0:
        storeaddr = data; rcvdatacount++;   // luetaan 16bit osoitteen ekat 8bittiä
      break;
      case 1:
        storeaddr = (storeaddr<<4) + data; rcvdatacount++;    // luetaan 16bit osoitteen tokat 8bittiä
      break;
      case 2:
        rmemaddr = storeaddr;
        if(storeaddr == 0x1CE || getserial) {   // jos sarjaliikenne käytössä, if(getserial), hyväksytään
          if(storeaddr == 0x333) {              // jos osoite 0x333, software resetti
            #pragma optsize-
            WDTCR=0x18;   // asetetaan softan vahtikoira haukkumaan tarvittaessa
            WDTCR=0x08;
            #ifdef _OPTIMIZE_SIZE_
            #pragma optsize+
            #endif
            while(1);   // vahtikoira + ikuinen looppi = software resetti
          }
          *rmemaddr=data;   // tallennetaan data oikeaan osoitteeseen
          rcvdatacount=0;   // nollataan laskuri (saa arvot 0>1>2>0>1>2>0>1>2>0...) kts. yllä
          if(storeaddr == 0x1D1) bLight();    // jos osoite 0x1D1, ajetaan taustavalon muutosfunktio..
        }                                     // ..koska tännöin on haluttu muuttaa taustavaloa
      break;      
    }
  }
}

/* Funktio taustavalon sammuttamiseen (hox! fivesecs1) */
void bLightX(void)
{
  if(fivesecs1 == 5)    // kun 5 sekuntia on kulunut ja kukaan ei ole koskenut nappuloihin..
  {
    if(isbacklight == 2) vale_OCR2 = isbright;    // ..annetaan taustavalon olla päällä..
    else if(isbacklight != 3) vale_OCR2 = PIMEA;  // ..tai sammutetaan taustavalo riippuen asetuksista (isbacklight)
  }
}

/* Funktio tietojen lähetykseen sarjaportin kautta
  jotta 'client-ohjelma' tietäisi mitä täällä tapahtuu */
void syncSerialData()
{
  printf("%u@%d@%u@%u@%u@%u@%u@%u@%u@%u@%u@%u@%u@%u@%u@%u~", time_real/2, lampotila, time_alarm/2, time_cdown/2, tjlasku, isalarm, gocdown, day, month, year, isbacklight, isvoice, isbright, issnooze, gettemp, wday);
}

/* Funktio nappien painelun tarkistamiseen */
void checkButtons(void)
{
  if((BTNS<<4) != 0b11110000)   // nappulat (4kpl) kytketty ekoihin 4 bittiin..
  {                             // ..painettu nappi on 0-tilassa
    if(debounce == 255)
    {
      buttons |= ~PINC;         // kopioidaan nappien tilat glob. muuttujaan
      debounce = 0;             // aloitetaan debounce-esto (näppäinvärähtely)
    }
    
    if(rdebounce < 60000) rdebounce ++;
    else { debounce = 255; }    // jätetään tarpeeksi aikaa ennen 'toistotilaa' (nappi painettu alhaalla kokoajan)
  }
  else
  {
    if(debounce != 255) debounce ++;
    rdebounce = 0;              // jätetään tarpeeksi aikaa ennen seuraavaa tilan lukemista
  }
}

/* Funktio tarkasteltujen nappuloiden datojen lukemiseen
  ;kutsutaan tarvittaessa pääohjelmasta */
char getButtons(char btn)
{
  if((buttons & btn))   // jos kysytty nappula (btn) löytyy painetuista nappuloista
  {
    vuoroohj = 0;
    buttons &= ~btn;    // 'nollataan' kysytty nappula painetuista nappuloista (painettu=0, ei painettu=1)
    if(char_sett != 9) bLight();    // jos ollaan asettamassa taustavaloa, ajetaan taustavalon muutosfunktio
    LEDI3 = 1;          // vilautetaan lediä 1
    lastbtn2 = lastbtn; lastbtn = btn;
    return 1;           // palautetaan 1
  }
  else
  {
    return 0;           // muutoin palautetaan 0
  }
}

/* Funktio näyttötilan vaihtamiseen
  ;kutsutaan tarvittaessa pääohjelmasta */
void changeNayttoTila(char xila)
{
  bLight();   // ajetaan taustavalon muutosfunktio, 'valot päälle'
  if(lastbtn2 != NAPPI4) naytto_tila = 1;   // pikapalautus nättötilaan 1, jos käytettiin muuta 'toimintoa' viime kerralla
  else naytto_tila = xila;                  // jos ei, niin vaihdetaan näyttötila halutuksi
  buttons = 0x00;                           // nollataan nappien mahdolliset painallukset
  char_sett = 0;                            // näyttötilojen 'alinäyttötila' nollille
}

/* Funktio paivan vaihtamiseen
  ;kutsutaan, kun kello pyörähtää ympäri 23:59:59>00:00:00 */
void changeDate(void)
{
  day ++;
  
  karkauscheck = monthdays[month-1];    // haetaan kk-päivien lukumäärä
  
  if((2000 + year) % 4 == 0 && month == 2) karkauscheck = 29;   // tarkastetaan, onko karkausvuosi, jos on..
                                                                // ..niin helmikuussa 29 päivää
  if(day > karkauscheck)                // jos päivä meinaa mennä yli kuukauden..
  {
    day = 1; month ++;                  // ..niin nollataan päivät ja lisätään kuuta
  }
  
  if(month >= 13)                       // jos kuukauden meinaa mennä yli vuoden..
  {
    month = 1; year ++;                 // ..niin nollataan kuukaudet ja lisätään vuotta
  }
  
  wday ++;                              // lisätään viikonpäivää
  if(wday >= 7) wday = 0;               // jos viikonpäivät meinaa mennä yli, niin nollille
  
  if(tjlasku != 0) tjlasku --;          // jos käytössä päivälaskuri-toiminto, vähennetään päiviä laskurissa
}

/* Funktio ajan (h,m,s)/(m,s,100/s) laskemiseen sekunneista
  ;kerroin1 ja kerroin2 ovat glob. muuttujia ja ne asetetaan
  ennen tämän funktion kutsumista riippuen siitä lasketaanko
  kellonaikaa vai ajanottoaikaa ;re_time_* muuttujat ovat myös
  glob. ja ne vievät 'puretun' ajan takaisin sinne missä tarvitaan*/
void convertTime(unsigned long int *tuloaika)
{
  re_time_h = *tuloaika/kerroin1;   // lasketaan tunnit
  re_time_m = *tuloaika%kerroin1;   // esilasketaan minuutit
  re_time_s = re_time_m%kerroin2;   // lasketaan sekunnit
  re_time_m = re_time_m/kerroin2;   // lasketaan minuutit
}

/* Timer 1:n ylivuodon keskeytys (100x sekunnissa eli 10ms välein) */
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
  timercount++;                 // lasketaan sadasosasekunteja
  
  if(gostopw) time_stopw ++;    // jos ajanotto päällä lisätään aikaa ajanotossa (sadasosasekunteja)
  
  updatedelaz = 1;              // asetetaan 100x/sek lippu
  
  /* 10x sekunnissa */
  if((timercount % 10) == 0)
  {
    updatedelay = 1;            // asetetaan 10x/sek lippu
  }
  
  /* 5x sekunnissa */
  if((timercount % 20) == 0)
  {
    if(sblink >= 15)            // 'lyhyt vilkautus' esim. ledeille ja äänimerkille
    {
      sblink = 0;               // nollataan vilkautuslippu
      vuoroohj ++; if(vuoroohj >= 7) vuoroohj = 0;                // kelataan alinäyttötiloja näyttötilassa 1
      if(isalarm || gocdown || gostopw || istempal) LEDI3 = 1;    // vilkautetaan lediä 3 jos jokin em. toiminto käytössä
      LEDI2 = 1;                // vilkautetaan lediä 2 (power-ledi)
    }
    else {
      sblink++;                 // kasvatetaan vilkautuslippua
      LEDI = 0;                 // sammutetaan kaikki ledit
      LEDI2 = 0;
      LEDI3 = 0;
      PIIPPERI = 0;             // mahdolliset äänet pois piipperistä
    }
  }
  
  /* 2x sekunnissa */
  if((timercount % 50) == 0)
  {
    if(fblink) fblink = 0; else fblink = 1;   // fblink toimii lippuna ½sek kestävissä toiminnoissa päääohjelmassa
  }
  
  /* 1x sekunnissa */
  if(timercount >= 100)
  {
    timercount = 0; time_real ++;             // kasvatetaan reaalikellonaikaa
    
    if(gocdown && time_cdown) time_cdown --;  // jos aikapommitoiminto on päällä vähennetään aikapommiaikaa
    
    if(gettemp)                 // jos lämpötilanlukuasetus on päällä..
    {
      lampotila = ds1820_temperature_10_read(0); ds1820_temperature_10_start(0);    // ..niin luetaan lämpö anturilta
    }                           // ensin luetaan lämpötila (read), sitten aloitetaan uusi lämpötilan muunnos (start)
    
    updatedelax = 1;            // asetetaan 1x/sek lippu
  }
}

/* Näytön päivitys */
void updateScreen(void)
{
  bLightX();    // ajetaan taustavalon muutosfunktio, 'valot päälle'
  
  if ((xhalytys || yhalytys || zhalytys) && fblink) { if(isvoice) PIIPPERI = 1; LEDI = 1; }   // jos jokin hälyytys aktivoitu, piippailaan piipperiä ja lediä (fblink tahtiin)
  if(lcd_ser == 0xFF) { changeNayttoTila(7); lcd_ser --; if (lcd_ser_v && isvoice) PIIPPERI = 1; }    // jos sarjaliikenteen kautta tullut viesti, piipataan
  else if(lcd_ser > 2) lcd_ser --;                              // vähennetään viestin näyttöaikalippua
  else if(lcd_ser == 2) { changeNayttoTila(1); lcd_ser --; }    // viestiä näytetty näytössä tarpeeksi, vaihdetaan näyttötilaan 1
  
  kerroin1 = 3600; kerroin2 = 60;   // asetetaan kellonaikamuunnosfunktiolle kertoimet (nyt lasketaan tavallista aikaa)
    
  switch(naytto_tila) 
  {
    /* Näyttötila -1 (laitteen asetukset) */
    case -1:
      strcpyf(lcd_buff1, "Asetukset:");   // ylärivillä näytetään AINA haluttu toiminnon seloste..
                                          // ..siellä ei koskaan näytetä muuta (max. 12 merkkiä)
      isset = 1;
      
      if(char_sett >= 0 && char_sett <= 2)
      {
        convertTime(&time_real);                                                  // muunnetaan aikaa 'time_real'
        sprintf(lcd_buff2, "%02d:%02d:%02d", re_time_h, re_time_m, re_time_s);    // näytetään muunnettu aika
      }
      else if(char_sett >= 3 && char_sett <= 6)
      {
        sprintf(lcd_buff2, "%02d.%02d.%02d %c%c", day, month, year, weekdays1[wday], weekdays2[wday]);    // näytetään päivämäärä ja viikonpäivä
      }
      else if(char_sett >= 7 && char_sett <= 9)
      {
        sprintf(lcd_buff2, "L:%d V:%d B:%d", isbacklight, isvoice, isbright);     // näytetään muita asetuksia
        if(char_sett == 9) vale_OCR2 = isbright;    // jos muutettava asetus on taustavalon kirkkaus, näytetään kirkkaus reaaliajassa
      }
      else if(char_sett >= 10 && char_sett <= 11)
      {
        sprintf(lcd_buff2, "S:%d T:%d", issnooze, gettemp);                       // näytetään muita asetuksia
      }
      
      if(getButtons(NAPPI2))    // jos painettu nappia 2
      {
        switch(char_sett)       // asetusten muuttaminen
        {
          case 0 : if(re_time_h > 0) time_real -= 3600; break;    // tunnit
          case 1 : if(re_time_m > 0) time_real -= 60; break;      // minuutit
          case 2 : if(re_time_s > 0) time_real -= 1; break;       // sekunnit
          case 3 : if(day > 1) day --; break;                     // päivät
          case 4 : if(month > 1) month --; break;                 // kuukaudet
          case 5 : if(year > 0) year --; break;                   // vuodet
          case 6 : if(wday > 0) wday --; break;                   // viikonpäivä
          case 7 : if(isbacklight > 0) isbacklight --; break;     // taustavalo päällä/himmeä/pois
          case 8 : isvoice = 0; break;                            // äänet päällä
          case 9 : if(isbright > 0) isbright --; break;           // taustavalon kirkkaus himmeä-tilassa
          case 10 : if(issnooze > 0) issnooze --; break;          // torkkuajastin
          case 11 : gettemp = 0; break;                           // lämpötilan haku käytössä
        }
      }
      if(getButtons(NAPPI3))    // jos painettu nappia 3
      {
        switch(char_sett)       // asetusten muuttaminen
        {
          case 0 : if(re_time_h < 23) time_real += 3600; break;   // tunnit ... .. .
          case 1 : if(re_time_m < 59) time_real += 60; break;
          case 2 : if(re_time_s < 59) time_real += 1; break;
          case 3 : if(day < 31) day ++; break;
          case 4 : if(month < 12) month ++; break;
          case 5 : if(year < 255) year ++; break;
          case 6 : if(wday < 6) wday ++; break;
          case 7 : if(isbacklight < 3) isbacklight ++; break;
          case 8 : isvoice = 1; break;
          case 9 : if(isbright < 255) isbright ++; break;
          case 10 : if(issnooze < 60) issnooze ++; break;
          case 11 : gettemp = 1; break;
        }
      }      
      if(getButtons(NAPPI1))    // jos painettu nappia 1
      {
        char_sett ++;           // muutetaan asetuslippua (muutettava asetus)
        if(char_sett == 12) char_sett = 0;    // jos meinaa mennä yli asetusten määrän, nollille
      }
      if(getButtons(NAPPI4)) changeNayttoTila(1);   // jos painettu nappia 4, muutetaan näyttötilaksi 1
    break;
    
    /* Näyttötila 0 */
    case 0:
      strcpyf(lcd_buff1, "ver. 6.8.9");   // kun laite käynnistetään (näyttötila 0) tekijän nimmarit näytölle
      strcpyf(lcd_buff2, "Juho K Tykkálá");
      
      if(getButtons(NAPPI4)) changeNayttoTila(1);   // nappia painamalla pääsee jatkamaan

// Tämän tarkoituksena on tarvittaessa selvittää mitä ascii-merkkejä LCD-näyttö pystyy muististaan tuottamaan      
//       lcd_gotoxy(0,0);
//       sprintf(lcd_buff, "%d:%c %d:%c", day, day, day+1, day+1);
//       lcd_puts(lcd_buff);
//       lcd_gotoxy(0,1);
//       sprintf(lcd_buff, "%d:%c %d:%c", day+2, day+2, day+3, day+3);
//       lcd_puts(lcd_buff);
//       
//       if(getButtons(NAPPI2)) day += 4;
//       if(getButtons(NAPPI3)) day -= 4;
    break;
    
    /* Näyttötila 1 */
    case 1:
    
      convertTime(&time_real);    // muunnetaan aikaa 'time_real'
    
      sprintf(lcd_buff1, "Fi %02d:%02d:%02d", re_time_h, re_time_m, re_time_s);   // näytetään muunnettu aika
      
      lcd_gotoxy(1,1);    // LCD-näytön kursori riville 1 merkkiin 1 (eli toinen rivi merkki 2)
      if(vuoroohj == 0)   // vuorotellaan alinäyttötiloja
      {
        if(gettemp)           // onko lämpötilan näyttö päällä ??¿?
        {
          if(lampotila == 3000) strcpyf(lcd_buff2, "Anturi?");    // jos ei löydy lämpöanturia (lampotila=3000), ihmetellään sitä ja näytetään näytöllä
          else sprintf(lcd_buff2, "Lámpï %+i.%-i%cC", lampotila/10, abs(lampotila%10), 0xDF);   // muutoin näytetään näytöllä lämpötila
        } else vuoroohj ++;   // ??¿? jos ei niin hypätään tämän alinäyttötilan yli
      }
      if(vuoroohj == 1)   // vuorotellaan alinäyttötiloja ... .. .
      {
        if(isalarm)
        {
          convertTime(&time_alarm);    // muunnetaan aikaa 'time_alarm'
          sprintf(lcd_buff2, "Heráá %02d:%02d:%02d", re_time_h, re_time_m, re_time_s);    // näytetään herätysaika ... .. .
        } else vuoroohj ++; 
      }
      if(vuoroohj == 2)
      {
        if(gocdown)
        {
          convertTime(&time_cdown);
          sprintf(lcd_buff2, "Aikaa %02d:%02d:%02d", re_time_h, re_time_m, re_time_s);
        } else vuoroohj ++; 
      }
      if(vuoroohj == 3)
      {
        if(gostopw)
        {
          kerroin1 = 6000; kerroin2 = 100;   // asetetaan kellonaikamuunnosfunktiolle kertoimet (nyt lasketaan ajanottoaikaa)
          convertTime(&time_stopw);
          sprintf(lcd_buff2, "Aotto %02d:%02d.%02d", re_time_h, re_time_m, re_time_s);    // HOX! minuutit, sekunnit ja SADASOSASEKUNNIT!
        } else vuoroohj ++; 
      }
      if(vuoroohj == 4)
      {
        if(tjlasku)
        {
          sprintf(lcd_buff2, "Páiviá: %d", tjlasku);
        } else vuoroohj ++; 
      }
      if(vuoroohj == 5)
      {
        if(lcd_ser)   // jos tullu sarjaliikenteen kautta viesti, näytetään se
        {
          strcpy(lcd_buff2,lcd_ser2);   // kopioidaan viesti näyttöpuskuriin
        } else vuoroohj ++; 
      }
      if(vuoroohj == 6)
      {
        sprintf(lcd_buff2, "%c%c %02d.%02d.20%02d", weekdays1[wday], weekdays2[wday], day, month, year);   // jos ei muuta näytettävää niin näytetään sitten päivämäärää
      }
      
      if(getButtons(NAPPI1)) changeNayttoTila(-1);          // nappi 1 vaihtaa näyttötilan -1 (laitteen asetukset)
      if(getButtons(NAPPI2)) { tjlasku = 0; }               // nappi 2, nollataan päivälaskuri
      if(getButtons(NAPPI3)) { tjlasku ++; vuoroohj=61; }   // nappi 3, lisätään päivälaskuria (sen toiminnalle ei ole omaa näyttötilaa)
      if(getButtons(NAPPI4)) changeNayttoTila(2);           // nappi 4 vaihtaa näyttötilan 2 (ajanotto)
    break;
    
    /* Näyttötila 2 (ajanotto) */
    case 2:
      strcpyf(lcd_buff1, "Ajanotto:");
      
      kerroin1 = 6000; kerroin2 = 100;    // muunnetaan aikaa ... .. .
      if(vuoroohj) vuoroohj --; else { convertTime(&time_stopw); }    // näytetään kierrosaikaa hetken verran aina lapituksen yhteydessä
      sprintf(lcd_buff2, "%02d:%02d.%02d L:%d", re_time_h, re_time_m, re_time_s, laps);   // tulostetaan näytölle ... .. .

      // Pysäytys ja käynnistys ja lapitus ja nollaus hoidetaan pääohjelmasta viiveen minimoimiseksi !!! !! !
      if(getButtons(NAPPI4)) changeNayttoTila(3);
    break;
    
    /* Näyttötila 3 (aikapommi) */
    case 3:
      convertTime(&time_cdown);

      strcpyf(lcd_buff1, "Aikapommi:");
      
      
      sprintf(lcd_buff2, "%02d:%02d:%02d", re_time_h, re_time_m, re_time_s);
      
      if(char_sett) { isset = 1; }      // jos asetusten muutto käynnissä, näytetään asetusnuoli ('<')
      else if(gocdown) { isact = 1; }   // jos aikapommi käytössä, näytetään sen symboli
      
      if(getButtons(NAPPI1) && !gocdown)
      {
        char_sett ++;
        if(char_sett == 4) { char_sett = 0; time_cdown_set = time_cdown; }    // tallennetaan asetettu aika myös toiseen muuttujaan, jos se halutaan myöhemmin palauttaa
      }
      if(getButtons(NAPPI2))
      {
        switch(char_sett)
        {
          case 0 : time_cdown = time_cdown_set; break;            // siirretään asetettu aika takaisin eli 'nollataan' aikapommi
          case 1 : if(re_time_h > 0) time_cdown -= 3600; break;   // tunnit
          case 2 : if(re_time_m > 0) time_cdown -= 60; break;     // minuutit
          case 3 : if(re_time_s > 0) time_cdown -= 1; break;      // sekunnit
        }
      }
      if(getButtons(NAPPI3))
      {
        switch(char_sett)
        {
          case 0 : if(gocdown) gocdown = 0; else gocdown = 1; break;    // käynnistetään aikapommi
          case 1 : if(re_time_h < 23) time_cdown += 3600; break;
          case 2 : if(re_time_m < 59) time_cdown += 60; break;
          case 3 : if(re_time_s < 59) time_cdown += 1; break;
        }
      }
      
      if(getButtons(NAPPI4)) changeNayttoTila(4);
    break;

    /* Näyttötila 4 (herätyskello) */
    case 4:
      convertTime(&time_alarm);

      strcpyf(lcd_buff1, "Herátysklo:");
      
      sprintf(lcd_buff2, "%02d:%02d:%02d", re_time_h, re_time_m, re_time_s);
      
      if(char_sett) { isset = 1; }    // jos asetusten muutto käynnissä, näytetään asetusnuoli ('<')
      else if(isalarm) { isact = 1; } // jos herätys päällä, näytetään sen symboli
      
      if(getButtons(NAPPI1) && !isalarm)
      {
        char_sett ++;
        if(char_sett == 4) { char_sett = 0; clsnooze=0; }   // nollataan myös snooze eli torkkuajastin (käyttöergonomiaa..)
      }
      if(getButtons(NAPPI2))
      {
        switch(char_sett)
        {
          case 1 : if(re_time_h > 0) time_alarm -= 3600; break;
          case 2 : if(re_time_m > 0) time_alarm -= 60; break;
          case 3 : if(re_time_s > 0) time_alarm -= 1; break;
        }
      }
      if(getButtons(NAPPI3))
      {
        switch(char_sett)
        {
          case 0 : if(isalarm) isalarm = 0; else isalarm = 1; break;    // aktivoidaan herätys
          case 1 : if(re_time_h < 23) time_alarm += 3600; break;
          case 2 : if(re_time_m < 59) time_alarm += 60; break;
          case 3 : if(re_time_s < 59) time_alarm += 1; break;
        }
      }
      
      if(getButtons(NAPPI4)) changeNayttoTila(6);
    break;
    
    /* Näyttötila 6 (lämpötilan näyttö) */
    case 6:
      strcpyf(lcd_buff1, "Lámpïtila:");
      
      if(lampotila == 3000) strcpyf(lcd_buff2, "Anturi?");    // jos ei löydy lämpöanturia (lampotila=3000), ihmetellään sitä ja näytetään näytöllä
      else if(char_sett == 0)
      {
        if(istempal) isact = 1;   // jos lämpötilan raja-hälyytys päällä, näytetään sen symboli
        sprintf(lcd_buff2, "%+i.%-i%cC", lampotila/10, abs(lampotila%10), 0xDF);   // näytetään lämpötila
      }
      else if(char_sett == 1)
      {
        sprintf(lcd_buff2, "L%+i.%-i H%+i.%-i", lampotila_low/10, abs(lampotila_low%10), lampotila_hi/10, abs(lampotila_hi%10));   // näytetään ylin/alin lämpötila
      }
      else if(char_sett >= 2 && char_sett <= 4)
      {
        isset = 1;    // asetusten muutto käynnissä, näytetään asetusnuoli ('<')
        sprintf(lcd_buff2, "%+d %+d", lampotila_alow, lampotila_ahi);   // näytetään ylä/ala-rajan asetukset
      }
      
      if(lampotila != 3000)   // jos lämpöanturi löydetty, hyväksytään asetusten/alinäyttötilojen muuttaminen
      {
        if(getButtons(NAPPI1))
        {
          char_sett ++;
          if(char_sett == 4) char_sett = 0;
        }
        if(getButtons(NAPPI2))
        {
          switch(char_sett)
          {
            case 1 : lampotila_low = lampotila; lampotila_hi = lampotila; break;    // 'nollataan' ylin/alin-läpötilanayttö
            case 2 : if(lampotila_alow > -128) lampotila_alow --; break;            // alin raja-lämpötila
            case 3 : if(lampotila_ahi > -128) lampotila_ahi --; break;              // ylin raja-lämpötila
          }
        }
        if(getButtons(NAPPI3))
        {
          switch(char_sett)
          {
            case 0 : if(istempal) istempal=0; else istempal=1; break;
            case 2 : if(lampotila_alow < 127) lampotila_alow ++; break;
            case 3 : if(lampotila_ahi < 127) lampotila_ahi ++; break;
          }
        }
      }
      if(getButtons(NAPPI4)) { if(lcd_ser) changeNayttoTila(7); else changeNayttoTila(1); }    // jos sarjaliikenteen kautta on tullut viesti, tilaan 7, muutoin tilaan 1
    break;
    
    /* Näyttötila 7 (näytetään mahd. sarjaliikenteen mukana tullut viesti) */
    case 7:
      
      strcpy(lcd_buff1,lcd_ser1);   // rivi 1
      strcpy(lcd_buff2,lcd_ser2);   // rivi 2
      
      if(getButtons(NAPPI3)) { lcd_ser = 0x00; changeNayttoTila(1); }   // nappi 3, 'poistetaan' viesti (lcd_ser -> 0) ja mennään näyttötilaan 1
      if(getButtons(NAPPI4)) changeNayttoTila(1);                       // mennään näyttötilaan 1 'poistamatta' viestiä
      
    break;
  }
  
  // LCD-näytön koko on 16 merkkiä X 2 riviä (16x2), 2. rivin eka merkki on aina tyhjä (lukemisen selventämiseksi)
  sprintf(lcd_buff1, "%-16s", lcd_buff1);   // varmistutaan siitä, että LCD-näytölle (rivi 1) menevä rivi on enintään 16 merkkiä pitkä
  sprintf(lcd_buff2, "%-15s", lcd_buff2);   // varmistutaan siitä, että LCD-näytölle (rivi 2) menevä rivi on enintään 15 merkkiä pitkä

  if(isvoice) lcd_buff1[15] = 244;              // jos äänet päällä, näytetään näytöllä sen symboli
  if(isalarm) lcd_buff1[14] = 224;              // jos herätys päällä, näytetään -||-
  if(clsnooze) lcd_buff1[14] = 226;             // jos torkkuajastin päälllä, -||-
  if(gostopw || gocdown) lcd_buff1[13] = 252;   // jos ajanotto tai aikapommi -||-
  if(istempal) lcd_buff1[12] = 226;             // jos lämpötilan rajahälyytys -||-
  if(fblink && getserial) lcd_buff1[15] = 243;  // jos sarjaliikenne on avattu 'client-ohjelma(an/sta)' -||- (fblink tahdissa) 'äänet päällä' merkin kohdalla vuorotellen
  
  if(isset) { lcd_buff2[13] = '<'; lcd_buff2[14] = char_sett+65; }    // jos jonkin näyttötilan asetusnayttö on päällä näytetään sen symboli
  if(isact) { lcd_buff2[13] = '*'; }                                  // jos jonkin näyttötilan toiminto on käytössä näytetään sen symboli
  isset = 0; isact = 0;

  lcd_gotoxy(0,0);        // asetetaan LCD-näytön kursori ekalle riville
  lcd_puts(lcd_buff1);    // syötetään teksti näytölle
  lcd_gotoxy(1,1);        // asetetaan LCD-näytön kursori tokalle riville
  lcd_puts(lcd_buff2);    // syötetään teksti näytölle
  
  strcpyf(lcd_buff1, "                ");   // nollataan rivi 1 jotta seuraavalla 'kierrolla' sinne ei jäisi edellisen kierron merkkejä
  strcpy(lcd_buff2,lcd_buff1);              // nollataan rivi 2 jotta seuraavalla -||-
  
  if(lcd_ser == 0xFE) { PIIPPERI = 0; }     // jos sarjaliikenteen mukana on tullut käsky piipittää piipperiä viestin saapuessa (lcd_ser=0xFE), niin sitten piipitetään
}

/* Pääohjelma */
void main(void)
{

  initd();            // ajetaan mikro-ohjaimen alustusvunktio
  naytto_tila = 0;    // kun virrat päälle, aloitetaan näyttötilasta 0
  
  while (1)
  {
    checkButtons();   // tarkistetaan onko nappuloita paineltu
    
    if(naytto_tila == 2) { if(getButtons(NAPPI3)) { if(gostopw) gostopw = 0; else gostopw = 1; } if(getButtons(NAPPI2)) { if(gostopw) { laps ++; vuoroohj = 50; time_stopw = 0; } else { time_stopw = 0; laps = 0; } } if(getButtons(NAPPI1)) { if(gostopw) { laps ++; vuoroohj = 50; } } }    // pysäytellään ja käynnistellään tarvittaessa ajanottoa (naytto_tila=2)
    if(zhalytys) { if(getButtons(NAPPI4)) { zhalytys = 0; istempal = 0; } }   // kuitataan lämpötila hälyytys
    if(xhalytys) { if(getButtons(NAPPI4)) { xhalytys = 0; } }                 // kuitataan aikapommin hälyytys
    if(yhalytys)   // kuitataan herätyskellon hälyytys
    { 
      if(getButtons(NAPPI3)) { isalarm = 1; yhalytys = 0; time_alarm += ((unsigned int)issnooze) * 60; clsnooze=1; }   // kuitataan, mutta asetetaan lisää 'nukkumaaikaa' (issnooze verran), jatketaan herätystoimintoa
      if(getButtons(NAPPI4)) { isalarm = 0; yhalytys = 0; clsnooze=0; }    // kuitataan, lopetetaan herätystoiminto
    }

    /* 100x sekunnissa */
    if(updatedelaz)
    { 
      updatedelaz = 0;                      // nollataan 100x/sek lippu
      if(vale_OCR2 < OCR2) OCR2 --;         // himmennetään taustavaloa ('portaaton') (PWM, pulssinleveysmodulaatio, jonka ohjausrekisteri OCR2 on)
      else if(vale_OCR2 > OCR2) OCR2 ++;    // kirkastetaan taustavaloa ('portaaton')
    }
    
    /* 10x sekunnissa */
    if(updatedelay)
    {    
      updatedelay = 0;    // nollataan 10x/sek lippu
      updateScreen();     // päivitä LCD-näytön sisältö
    }
    
    /* 1x sekunnissa */
    if(updatedelax)
    {
      updatedelax = 0;    // nollataan 1x/sek lippu
      
      if(time_real >= 86400) { time_real = 0; changeDate(); }   // jos reaaliaika näyttää 23:59:59 niin nollataan se ja vaihdetaan päivää
      
      if(time_alarm == time_real && isalarm) { yhalytys = 1; }  // jos herätyskellon aika on sama kuin reaaliaika niin hälyt päälle (yhalytys)
    
      if(!time_cdown && gocdown) { gocdown = 0; xhalytys = 1; } // jos aikapommi on kulunut nollille niin hälyt päälle (xhalytys)
      
      lampotila_c = lampotila/10;                               // lämpötila tulee anturilta 10x, niin palautetaan oikeaksi
    
      if((lampotila_c >= lampotila_ahi || lampotila_c <= lampotila_alow) && istempal) { zhalytys = 1; }    // jos lämpötilan jompikumpi raja on rikottu, hälyt päälle (zhalytys)
      
      if(lampotila < lampotila_low) { lampotila_low = lampotila; }    // jos lämpö oli kylmempi kuin kylmin, niin päivitetään
      if(lampotila > lampotila_hi) { lampotila_hi = lampotila; }      // jos lämpö oli kuumempi kuin kuumin, niin päivitetään
      
      if(autonull && rcvdatacount) rcvdatacount = 0;            // sarjaliikenteen timeoutti, jos dataa ei ole tullut 1 sekuntiin 'client-ohjelmasta', niin nollataan datan osalaskuri
      autonull = 1;
      
      if(fivesecs1 < 6) fivesecs1 ++;                           // jos 5 sekunnin laskuri on alle 6 niin kasvatetaan laskuria (käytetään taustavalon aikasammutukseen)
    
      if(getserial) { getserial --; syncSerialData(); }         // jos 'client-ohjelmasta' on tullut dataa (getserial=1) niin lähetetään vastauksena lisää dataa
    }
  };
}
