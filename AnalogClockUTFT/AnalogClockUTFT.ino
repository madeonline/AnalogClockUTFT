// UTFT_Analog_Clock (C)2010-2012 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// This program was made simply because I was bored.
//
// Hardware requirements:
//  - Arduino Mega
//  - Display module supported by UTFT
//  - DS1307 RTC
//  - Some method of connecting the display module to the Arduino
//    like the old ITDB02 Mega Shield v1.1 as it already contains
//    the DS1307 or the All in One Super Screw Shield from
//    AquaLEDSource which also has everything you need on the
//    shield.
//
// Software requirements:
//  - UTFT library    http://www.henningkarlsen.com/electronics/library.php?id=51
//  - UTouch library  http://www.henningkarlsen.com/electronics/library.php?id=55
//  - DS1307 library  http://www.henningkarlsen.com/electronics/library.php?id=34
//
// This program should work on all the supported display modules
// from the 2.4" ones and up to the 5.0" ones, but keep in mind 
// that this program was made to work on a 320x240 module, so it 
// may look a little silly on larger displays.
// It will not work on the 2.2" modules without modification as 
// the resolution on those modules are too low.
//
// Default serial speed for this sketch is 115200 baud.
//
// You can buy the AquaLEDSource All in One Super Screw Shield here:
// English:    http://www.aqualedsource.com/prods/screwshield.php
// Portuguese: http://www.aqualed-light.com/?sec=home&id=1818
//
/**************************************************************************/

// Enable (1) or disable (0) VT100 terminal mode
// Enable this if your terminal program supports VT100 control sequences.
// The Serial Monitor in the Arduino IDE does not support VT100 control sequences. 
// If using the Serial Monitor the line ending should be set to 'No line ending'.
#define VT100_MODE  1

// Enable or disable the use of the AquaLEDSource All in One Super Screw Shield
// Uncomment the following line if you are using this shield
//#define AQUALED_SHIELD 1
//#define CTE_DUE_SHIELD 1

// This code block is only needed to support multiple
// MCU architectures in a single sketch.
#if defined(__AVR__)
  #define imagedatatype  unsigned int
#elif defined(__PIC32MX__)
  #define imagedatatype  unsigned short
#elif defined(__arm__)
  #define imagedatatype  unsigned short
#endif
// End of multi-architecture block

//#include <UTFT.h>
//#include <UTouch.h>
#include <UTFT_Buttons.h>

// Declare which fonts we will be using
//extern uint8_t BigFont[];






#include <UTFT.h>
#include <DS1307.h>
#include <UTouch.h>
#include <OneWire.h>
#include "DHT.h"

#define DHTPIN 9     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);


// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
//UTFT  myGLCD(CTE50,25,26,27,28);
//UTFT  myGLCD(ITDB32S,25,26,27,28);              //Due
UTFT    myGLCD(ITDB32S,38,39,40,41);              // Remember to change the model parameter to suit your display module!
//UTFT    myGLCD(ITDB24D,38,39,40,41);            // Remember to change the model parameter to suit your display module!
//UTFT    myGLCD(ITDB,38,39,40,41);               // Remember to change the model parameter to suit your display module!
//UTFT    myGLCD(TFT01_24_8,38,39,40,41);         // Remember to change the model parameter to suit your display module!
//#ifndef AQUALED_SHIELD
UTouch  myTouch(6,5,4,3,2);

// Init the DS1307
DS1307 rtc(20, 21);
OneWire  ds(8);  // on pin 10 (a 4.7K resistor is necessary)
// Init a Time-data structure
Time  t;

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;



int clockCenterX=119;
int clockCenterY=119;
int oldsec=0;

void setup()
{
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_HI);

  // Set the clock to run-mode
  rtc.halt(false);
  
  Serial.begin(115200);
  Serial.println("Send any character to enter serial mode...");
  Serial.println();
//  setup_ds18s20();
 // dht.begin();
}

void drawDisplay()
{
  // Clear screen
  myGLCD.clrScr();
  
  // Draw Clockface
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  for (int i=0; i<5; i++)
  {
    myGLCD.drawCircle(clockCenterX, clockCenterY, 119-i);
  }
  for (int i=0; i<5; i++)
  {
    myGLCD.drawCircle(clockCenterX, clockCenterY, i);
  }
  
  myGLCD.setColor(192, 192, 255);
  myGLCD.print("3", clockCenterX+92, clockCenterY-8);
  myGLCD.print("6", clockCenterX-8, clockCenterY+95);
  myGLCD.print("9", clockCenterX-109, clockCenterY-8);
  myGLCD.print("12", clockCenterX-16, clockCenterY-109);
  for (int i=0; i<12; i++)
  {
    if ((i % 3)!=0)
      drawMark(i);
  }  
  t = rtc.getTime();
  drawMin(t.min);
  drawHour(t.hour, t.min);
  drawSec(t.sec);
  oldsec=t.sec;

  // Draw calendar
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(240, 0, 319, 85);
  myGLCD.setColor(0, 0, 0);
  for (int i=0; i<7; i++)
  {
    myGLCD.drawLine(249+(i*10), 0, 248+(i*10), 3);
    myGLCD.drawLine(250+(i*10), 0, 249+(i*10), 3);
    myGLCD.drawLine(251+(i*10), 0, 250+(i*10), 3);
  }

  // Draw SET button
  myGLCD.setColor(64, 64, 128);
  myGLCD.fillRoundRect(260, 200, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(260, 200, 319, 239);
  myGLCD.setBackColor(64, 64, 128);
  myGLCD.print("SET", 266, 212);
  myGLCD.setBackColor(0, 0, 0);
}

void drawMark(int h)
{
  float x1, y1, x2, y2;
  
  h=h*30;
  h=h+270;
  
  x1=110*cos(h*0.0175);
  y1=110*sin(h*0.0175);
  x2=100*cos(h*0.0175);
  y2=100*sin(h*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}

void drawSec(int s)
{
  float x1, y1, x2, y2;
  int ps = s-1;
  
  myGLCD.setColor(0, 0, 0);
  if (ps==-1)
    ps=59;
  ps=ps*6;
  ps=ps+270;
  
  x1=95*cos(ps*0.0175);
  y1=95*sin(ps*0.0175);
  x2=80*cos(ps*0.0175);
  y2=80*sin(ps*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);

  myGLCD.setColor(255, 0, 0);
  s=s*6;
  s=s+270;
  
  x1=95*cos(s*0.0175);
  y1=95*sin(s*0.0175);
  x2=80*cos(s*0.0175);
  y2=80*sin(s*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}

void drawMin(int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int pm = m-1;
  
  myGLCD.setColor(0, 0, 0);
  if (pm==-1)
    pm=59;
  pm=pm*6;
  pm=pm+270;
  
  x1=80*cos(pm*0.0175);
  y1=80*sin(pm*0.0175);
  x2=5*cos(pm*0.0175);
  y2=5*sin(pm*0.0175);
  x3=30*cos((pm+4)*0.0175);
  y3=30*sin((pm+4)*0.0175);
  x4=30*cos((pm-4)*0.0175);
  y4=30*sin((pm-4)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);

  myGLCD.setColor(0, 255, 0);
  m=m*6;
  m=m+270;
  
  x1=80*cos(m*0.0175);
  y1=80*sin(m*0.0175);
  x2=5*cos(m*0.0175);
  y2=5*sin(m*0.0175);
  x3=30*cos((m+4)*0.0175);
  y3=30*sin((m+4)*0.0175);
  x4=30*cos((m-4)*0.0175);
  y4=30*sin((m-4)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}

void drawHour(int h, int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int ph = h;
  
  myGLCD.setColor(0, 0, 0);
  if (m==0)
  {
    ph=((ph-1)*30)+((m+59)/2);
  }
  else
  {
    ph=(ph*30)+((m-1)/2);
  }
  ph=ph+270;
  
  x1=60*cos(ph*0.0175);
  y1=60*sin(ph*0.0175);
  x2=5*cos(ph*0.0175);
  y2=5*sin(ph*0.0175);
  x3=20*cos((ph+5)*0.0175);
  y3=20*sin((ph+5)*0.0175);
  x4=20*cos((ph-5)*0.0175);
  y4=20*sin((ph-5)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);

  myGLCD.setColor(255, 255, 0);
  h=(h*30)+(m/2);
  h=h+270;
  
  x1=60*cos(h*0.0175);
  y1=60*sin(h*0.0175);
  x2=5*cos(h*0.0175);
  y2=5*sin(h*0.0175);
  x3=20*cos((h+5)*0.0175);
  y3=20*sin((h+5)*0.0175);
  x4=20*cos((h-5)*0.0175);
  y4=20*sin((h-5)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}

void printDate()
{
  Time t_temp;
  
  t_temp = rtc.getTime();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.print(rtc.getDOWStr(FORMAT_SHORT), 256, 8);
  if (t_temp.date<10)
    myGLCD.printNumI(t_temp.date, 272, 28);
  else
    myGLCD.printNumI(t_temp.date, 264, 28);
  myGLCD.print(rtc.getMonthStr(FORMAT_SHORT), 256, 48);
  myGLCD.printNumI(t_temp.year, 248, 65);
}

void clearDate()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRect(248, 8, 312, 81);
}


void setup_ds18s20()
{
  if ( !ds.search(addr)) 
  {
//    Serial.println("No more addresses.");
//    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
//  Serial.print("ROM =");
  for( i = 0; i < 8; i++) 
  {
 //   Serial.write(' ');
 //   Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
  //    Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) 
  {
    case 0x10:
 //     Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
 //     Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
  //    Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
  //    Serial.println("Device is not a DS18x20 family device.");
      return;
  }  
  
}

void ds18s20()
{

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
 // delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

//  Serial.print("  Data = ");
//  Serial.print(present, HEX);
//  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
 //   Serial.print(data[i], HEX);
 //   Serial.print(" ");
  }
//  Serial.print(" CRC=");
//  Serial.print(OneWire::crc8(data, 8), HEX);
//  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
//  Serial.print("  Temperature = ");
//  Serial.print(celsius);
//  Serial.print(" Celsius, ");
//  Serial.print(fahrenheit);
//  Serial.println(" Fahrenheit");  
  
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumF(celsius,1, 250, 180);
}

void loop()
{
  delay(2000);
  int x, y;
 
  t = rtc.getTime();
  if (t.date==0)
  {
    setClock();
  }
  else
  {
    drawDisplay();
    printDate();
  }
  
  t = rtc.getTime();
  
  while (true)
  {
   // if (Serial.available()>0)
    //  serialMode();
   
    if (oldsec!=t.sec)
    {
      if ((t.sec==0) and (t.min==0) and (t.hour==0))
      {
        clearDate();
        printDate();
      }
      if (t.sec==0)
      {
        drawMin(t.min);
        drawHour(t.hour, t.min);
      }


      // ds18s20();
      // 
      //float HumidityDH11 = dht.readHumidity();
      //float TemperatureDH11 = dht.readTemperature();
      // 
      //myGLCD.print("B\xA0""a\x9B", 250, 95);
      //myGLCD.printNumF((float)HumidityDH11,1, 250, 115);
      //myGLCD.print("Te\xA1\xA3", 250, 140);
      //myGLCD.printNumF((float)TemperatureDH11,1, 250, 160);


      drawSec(t.sec);
      oldsec=t.sec;
  
    }
 
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if (((y>=200) && (y<=239)) && ((x>=260) && (x<=319)))
      {
        myGLCD.setColor (255, 0, 0);
        myGLCD.drawRoundRect(260, 200, 319, 239);
        setClock();
      }
    }
    
    delay(10);
    t = rtc.getTime();
  }
    delay(1000);
}



