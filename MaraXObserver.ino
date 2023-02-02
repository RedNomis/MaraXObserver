// Copyright (C) 2023 Simon Ziehme
// This file is part of MaraXObserver <https://github.com/RedNomis/MaraXObserver>.
//
// MaraXObserver is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MaraXObserver is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with dogtag.  If not, see <http://www.gnu.org/licenses/>.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <SoftwareSerial.h>

// Serial Port PINS ESP8266
#define D5              14
#define D6              12

// Serial Port Buffer Size
#define BUFFER_SIZE     32


// Display stuff
#define SCREEN_WIDTH   128 // OLED display width, in pixels
#define SCREEN_HEIGHT   64 // OLED display height, in pixels
#define OLED_RESET      -1 // QT-PY / XIAO
#define i2c_Address   0x3c // initialize with the I2C addr 0x3C Typically eBay OLED's

// initialize the 1.3 inch OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// initialize the serial port
SoftwareSerial mySerial(D5, D6);

// variables
bool gbInit = true;
bool gbPumpOn = true;
bool gbCoffeeMode = false;
bool gbHeaterOn = false;
int giHx = 32;
int giBo = 110;
int giCurrentCounter = 1;
int giLastCounter = 0;
long lLastMillis = 0;

//Internals
long lastMillis;
int seconds = 0;
int lastTimer = 0;
long serialTimeout = 0;
char buffer[BUFFER_SIZE];
int iIndex = 0;

//Mara Data
String maraData[7];
String* lastMaraData = NULL;

const byte numChars = 32;
char receivedChars[numChars];
static byte ndx = 0;
char endMarker = '\n';
char rc;

//////////////////////////////////////////////////////////////////////////////////////
void setup()   {
//////////////////////////////////////////////////////////////////////////////////////
  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.clearDisplay();

  Serial.begin(9600);
  mySerial.begin(9600);
  memset(buffer, 0, BUFFER_SIZE);
  mySerial.write(0x11);

  showMessage("Waiting for the Mara X");
}

//////////////////////////////////////////////////////////////////////////////////////
void showMessage(String message)  {
//////////////////////////////////////////////////////////////////////////////////////
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  delay(1000);
}

//////////////////////////////////////////////////////////////////////////////////////
void getMaraData()
//////////////////////////////////////////////////////////////////////////////////////
{
  /*
    Example Data: C1.10,116,124,093,0840,1,0\n every ~400-500ms
    Length: 26
    [Pos] [Data] [Describtion]
    0)      C     Coffee Mode (C) or SteamMode (V)
    -        1.10 Software Version
    1)      116   current steam temperature (Celsisus)
    2)      124   target steam temperature (Celsisus)
    3)      093   current hx temperature (Celsisus)
    4)      0840  countdown for 'boost-mode'
    5)      1     heating element on or off
    6)      0     pump on or off
  */

  // read serial interface
  while (mySerial.available())
  {
    serialTimeout = millis();
    char rcv = mySerial.read();
    if (rcv != '\n')
      buffer[iIndex++] = rcv;
    else {
      iIndex = 0;
      char* ptr = strtok(buffer, ",");
      int idx = 0;
      while (ptr != NULL)
      {
        maraData[idx++] = String(ptr);
        ptr = strtok(NULL, ",");
      }
      lastMaraData = maraData;
    }
  }

  if (millis() - serialTimeout > 6000)
  {

    serialTimeout = millis();
    mySerial.write(0x11);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// show the main screen with the temperature information
void showMain(bool bCoffeeMode, bool bHeaterOn, String sHXTemp, String sBoTemp)   {
//////////////////////////////////////////////////////////////////////////////////////
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  if (bCoffeeMode == true)
    display.print("Kaffee-Mode   |||:");
  else
    display.print("Dampf-Mode    |||:");

  if (bHeaterOn == true)
    display.println("An");
  else
    display.println("Aus");

  display.println("---------------------");
  //display.println("");
  display.setTextSize(2);

  display.print("HX: ");
  display.print(sHXTemp);
  display.print(" ");
  display.print((char)247);
  display.println("C");

  display.print("Bo: ");

  display.print(sBoTemp);
  display.print(" ");
  display.print((char)247);
  display.println("C");

  display.setTextSize(1);
  display.println("---------------------");
  display.print("letzter Bezug: ");
  display.print(giLastCounter);
  display.println(" s");
  display.display();
}

//////////////////////////////////////////////////////////////////////////////////////
// show the counter screen; if pump is on this screen is active
void showCounter(bool bCoffeeMode, bool bHeaterOn, String sCounter)   {
//////////////////////////////////////////////////////////////////////////////////////
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  if (bCoffeeMode == true)
    display.print("Kaffee-Mode   |||:");
  else
    display.print("Dampf-Mode    |||:");

  if (bHeaterOn == true)
    display.println("An");
  else
    display.println("Aus");

  display.println("---------------------");
  display.println("");
  display.setTextSize(3);

  display.print("   ");
  if (sCounter.toInt() < 10)
    display.print(" ");
  display.println(sCounter);

  display.setTextSize(1);
  //display.println("");
  display.println("---------------------");
  display.print("letzter Bezug: ");
  display.print(giLastCounter);
  display.println(" s");
  display.display();
}

//////////////////////////////////////////////////////////////////////////////////////
void loop() {
//////////////////////////////////////////////////////////////////////////////////////
  getMaraData();

  if (lastMaraData != NULL) {
    if (lastMaraData[0].startsWith("C"))
      gbCoffeeMode = true;
    else
      gbCoffeeMode = false;

    if (lastMaraData[5].toInt() > 0)
      gbHeaterOn = true;
    else
      gbHeaterOn = false;

    if (lastMaraData[6].toInt() > 0)
      gbPumpOn = true;
    else
      gbPumpOn = false;

    if (gbPumpOn)  {

      showCounter(gbCoffeeMode, gbHeaterOn, String(giCurrentCounter));
      if ( millis() - lastMillis >= 1000) {
        lastMillis = millis();
        ++giCurrentCounter;
        if (giCurrentCounter > 99)
          giCurrentCounter = 0;
      }
    }
    else {
      if (giCurrentCounter > 0)
        giLastCounter = giCurrentCounter;
      giCurrentCounter = 0;
      showMain(gbCoffeeMode, gbHeaterOn, lastMaraData[3], lastMaraData[1]);
    }
  }
}
