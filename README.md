# passiveThermostatNotifier

This project provided a new interface for notifications using the idea of Ubiquitous Computing in smart home system.
It’s realized with a Micro Controller with a Wi-fi module onboard, it’s designed for all type of sensor and show the notification in passive way with a RGB Wi-Fi bulb.
In this case the bulb is related to a temperature sensor and the notification are about how much cold or hotter the house is.
The thermostat system it is built using low-cost components and Open Source platformsto make it reproduced by everyone.

In this project are developed a phisycal device and an interface via a Telegram bot.

...in progress...

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
├── firmaware.ino                       # node mcu v1.0 firmware made with arduino ide
```
