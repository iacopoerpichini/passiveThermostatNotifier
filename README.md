# passiveThermostatNotifier

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

###
...in progress...

This project provided a new interface for notifications using the idea of Ubiquitous Computing in smart home system.
It’s realized with a Micro Controller with a Wi-fi module onboard, it’s designed for all type of sensor and show the notification in passive way with a RGB Wi-Fi bulb.
In this case the bulb is related to a temperature sensor and the notification are about how much cold or hotter the house is.
The thermostat system it is built using low-cost components and Open Source platformsto make it reproduced by everyone.

In this project are developed a physical device and an interface via a Telegram bot.

### Features
#### Device
#### Telegram bot
#### 

### Requirements
| Software        | Version             |
| ----------------|---------------------|
| **Arduino IDE** | tested on v1.8.3    | 

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

### Usage

#### Thanks
I want to thank my childhood friend Andrea for giving me the opportunity to 3D print the model.
