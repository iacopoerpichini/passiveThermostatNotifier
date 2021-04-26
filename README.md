# passiveThermostatNotifier

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This project provided a new interface for notifications using the idea of Ubiquitous Computing in smart home system.
It’s realized with a Micro Controller with a Wi-fi module onboard, it’s designed for all type of sensor and show the notification in passive way with a RGB Wi-Fi bulb.
In this case the bulb is related to a temperature sensor and the notification are about how much cold or hotter the house is.
The thermostat system it is built using low-cost components and Open Source platformsto make it reproduced by everyone.

In this project are developed a physical device and an interface via a Telegram bot.

### Directories Layout

```bash
├── 3DModel                             # contains two file tipe of box model
│   ├── stl
│   ├── ctb
├── Sensor Library and Examples         # a series of sensors library to remake this project
│   ├── DHT                             # sensor used in thermostat device          
│   ├── ...
├── UniversalTelegramBot-1.3.0 Library  # used to make the Telegram bot
├── img                                 # image used in readme/paper
│   ├── ...
├── documentation.pdf                   # guide to reproduce all the work
├── firmaware.ino                       # node mcu v1.0 firmware made with arduino ide
```

#### Device
The device can be installed in an environment connected to a smart bulb that changes color and light intensity in relation to the temperature measured by a sensor.
The light bulb color changes slowly.
The device has a temperature sensor, a WiFi connection, a buzzer to receive active notification via interaction with an interface and a button to stop the change of color bulb.
There is also a build-in led that displays in real time the state of the temperature sensor.
#### Render of 3D model
In this repo are provided all the 3D file used to print the cover of the device.

<img src="https://github.com/iacopoerpichini/passiveThermostatNotifier/blob/master/img/animation.gif" height="240" width="320">

#### Telegram bot
This allows users to get real time information about temperature and humidity and to select a bound of desired temperature to change the color of the bulb.
By default the temperature bounds are set on [16,24] Celsius degree.
The bulb turns to red slowly if the temperature is higher than 24 degree and turns blue if is lower than 16 degree.
It is also possible to notify people near the sensor with a buzzer.

<img src="https://github.com/iacopoerpichini/passiveThermostatNotifier/blob/master/img/logo.PNG" height="100" width="100">

To create a Telegram bot:
  1. Open BotFather available at this [link](https://t.me/botfather)
  2. Click the start button and type `/newbot` and follow the instructions to create your bot. Give it a name and username.
  3. If your bot is successfully created, you’ll receive a message with a link to access the bot and the bot token.

Anyone that knows your bot username can interact with it. 

To make sure that the bot ignore messages that are not from any authorized users, you can get your Telegram User ID.
Then, when your telegram bot receives a message, the ESP can check whether the sender ID corresponds to your User ID and handle the message or ignore it.
Open [IDBot](t.me/myidbot) and type `/getid` and use it in the firmware.

#### ThingSpeak

The device also use a ThingSpeak channel available [here](https://thingspeak.com/channels/1367465) to display all the data collected.

To make a Channel follow the[ThingSpeak Official Documentation](https://www.mathworks.com/help/thingspeak/collect-data-in-a-new-channel.html).

### Requirements
| Software                           | Version             |
| -----------------------------------|---------------------|
| **Arduino IDE**                    | tested on v1.8.3    | 
| **ESP8266**                        | tested on v2.5.2    | 
| **ArduinoJson**                    | tested on v6.17.3   | 
| **Universal Telegram Bot Library** | tested on v1.3.0    | 

To install ESP8266 Add-on in Arduino IDE:
  1. Go to File > Preferences
  2. Enter `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into the “Additional Boards Manager URLs” field. Then, click the “OK” button.
  3. Open the Boards Manager. Go to Tools > Board > Boards Manager
  4. Search and install ESP8266 by ESP8266 Community library
  5. Use Generic ESP8266 Board in Tools
  
Also install the ArduinoJson and Universal Telegram Bot Librarys.

  1. Go to Skech > Include Library > Manage Libraries.
  2. Search library.
  3. Check the version and install the library.

### Usage

#### Thanks
I want to thank my childhood friend Andrea for giving me the opportunity to 3D print the model.
