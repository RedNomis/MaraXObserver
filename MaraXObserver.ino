/*********************************************************************

*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// Wireless Login
const char* ssid     = "SZINetzwerk";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "Blue-eyes1405";     // The password of the Wi-Fi network

// initialize webserver
ESP8266WebServer server(80);

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

// Splash settings
#define DELAY_SPLASH    75

String sWelcomeStringArray[45] = {
  "         H",
  "        Ha",
  "       Hal",
  "      Hall",
  "     Hallo",
  "    Hallo,",
  "   Hallo, ",
  "  Hallo, h",
  " Hallo, hi",
  "Hallo, hie",
  "allo, hier",
  "llo, hier ",
  "lo, hier i",
  "o, hier is",
  ", hier ist",
  " hier ist ",
  "hier ist d",
  "ier ist de",
  "er ist dei",
  "r ist dein",
  " ist deine",
  "ist deine ",
  "st deine L",
  "t deine Le",
  " deine Lel",
  "deine Leli",
  "eine Lelit",
  "ine Lelit ",
  "ne Lelit M",
  "e Lelit Ma",
  " Lelit Mar",
  "Lelit Mara",
  "elit Mara ",
  "lit Mara X",
  "it Mara X!",
  "t Mara X! ",
  " Mara X!  ",
  "Mara X!   ",
  "ara X!    ",
  "ra X!     ",
  "a X!      ",
  " X!       ",
  "X!        ",
  "!         ",
  "          ",
};



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

  
  WiFi.begin(ssid, password);  

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.clearDisplay();
  
  //display.setContrast (0); // dim display   

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
void startNetwork() {
//////////////////////////////////////////////////////////////////////////////////////
  display.clearDisplay();    
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  int iConnectTry = 0;
  display.clearDisplay();    
    display.setCursor(0, 0);
    display.println("Verbindungsaufbau");
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    display.print(".");
    delay(1000);
    display.display();
    iConnectTry++;
    if (iConnectTry > 10)
       break;
  }
  if (iConnectTry > 10) {
    display.println("");
    display.println("");
    display.println("Verbindung");
    display.println("nicht moeglich!");  
    display.display();
    delay(1000);
    return;
  }

  display.clearDisplay();    
  display.println("");
  display.println("Verbindung");
  display.println("");
  display.println("aufgebaut!");  

  server.on("/", indexHTML);
  server.on("/TempWeb", TempMessung);
  
  server.begin();
  
  display.display();
  delay(1000);
  display.clearDisplay();    
  display.setCursor(0, 0);
  display.println("");
  display.println("IP Addresse");
  display.println("");
  display.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  display.display();
  delay(3000); 
  
}

//////////////////////////////////////////////////////////////////////////////////////
void indexHTML() {    
//////////////////////////////////////////////////////////////////////////////////////
  server.send(200, "text/html", 
  "<!DOCTYPE html>\
  <html>\
  <head>\
  <title>Temperaturanzeige</title>\
  <meta http-equiv='content-type' content='text/html'; charset='utf-8'>\
  <style>\
  body { background-color: #585858; font-size: 50px; font-family: Arial, Helvetica, Sans-Serif; color: #F2F2F2; margin-left: 40px; }\
  h1 { color: #2ECCFA; margin-top: 50px; margin-bottom: 0px; }\
  h2 { font-size: 20px; margin-top: 0px; margin-bottom: 50px; }\
  #temp { width: 230px; height: 80px; border: 5px solid #F2F2F2; text-align:center; padding: 1px; color: #9AFE2E; background-color: #000000; font-size: 60px; }\
  </style>\
  </head>\
   <body>\
  <h1>Lelit Mara X</h1>\
    <table>\
  <tr><td>HX Temperatur:&nbsp;</td>\
  <td id='temp'><span id='TempWert'>-</span>°C</td></tr>\
  <tr><td>Boiler Temperatur:&nbsp;</td>\
  <td id='temp'><span id='BoTempValue'>-</span>°C</td></tr>\
  <tr><td>Prioritaet:&nbsp;</td>\
  <td id='temp'><span id='PrioValue'>-</span></td></tr>\
  <tr><td>Heizung:&nbsp;</td>\
  <td id='temp'><span id='HeaterValue'>-</span></td></tr>\
  <tr><td>Pumpe:&nbsp;</td>\
  <td id='temp'><span id='PumpValue'>-</span></td></tr>\
  </table>\
  <script>\
  setInterval(function() {\
    getData();\
  }, 1000);\
  function getData() {\
    var xhttp = new XMLHttpRequest();\
    xhttp.onreadystatechange = function() {\
      if (this.readyState == 4 && this.status == 200) {\
        document.getElementById('TempWert').innerHTML = this.responseText;\
      }\
    };\
    xhttp.open('GET', 'TempWeb', true);\
    xhttp.send();\
  }\
  </script>\
  </body>\
  </html>\
  ");
}

//////////////////////////////////////////////////////////////////////////////////////
void showSplash() {
//////////////////////////////////////////////////////////////////////////////////////
  
  // Clear the buffer.
  display.clearDisplay();
    
  // text display tests
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  
  for (int i = 1; i<46;i++) {
   display.setCursor(0, 30);
   display.print(sWelcomeStringArray[i]);    
   display.display();
   delay(DELAY_SPLASH);
   display.clearDisplay();
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
void TempMessung() {
//////////////////////////////////////////////////////////////////////////////////////  
  String TempWert = String(giHx); 
  server.send(200, "text/plane", TempWert);
}

//////////////////////////////////////////////////////////////////////////////////////
void loop() {
//////////////////////////////////////////////////////////////////////////////////////

  /*if (gbInit) {
    //startNetwork();
    showSplash();    
    gbInit = false;
  }
*/
  //server.handleClient();

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
