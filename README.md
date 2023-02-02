# Lelit Mara X (V2) data observer with shot-timer
This small project makes it possible to query and visualize the data of the Lelit Mara X (V2) via its serial interface.
In addition, a timer is started and displayed to indicate the time the pump runs as coffee is drawn.

## todo
* Case (3D Print)
* Webserver
* English translation (switchable via Web access)
* Perstistence (timer, language setting)

## Used Hardware
* AZDelivery NodeMCU Amica Modul V2 ESP8266 ESP-12F
* AZDelivery 1,3 Zoll OLED Display I2C SSH1106 Chip 128 x 64 Pixel I2C

Costs ~ 15 € (Germany)

## Used Software
* Arduino IDE 1.8.8

### Libraries
* Wire.h
* Adafruit_GFX.h
* Adafruit_SH110X.h
* SoftwareSerial.h
