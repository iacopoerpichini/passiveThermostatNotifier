/*
  Iacopo Erpichini

  IoT node for passive notification about temperature with a remote control using telegram bot

  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

  Programmed on a node mcu v1 board with esp8266: https://github.com/esp8266/Arduino
*/

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include "DHT.h"


// network declaration
/*
  const char* SSID = "Home&Life SuperWiFi-F829";
  const char* PASSWORD = "G44B477ACAA34DK7";

  const char* SSID = "Covid Free";      // Replace with your network name
  const char* PASSWORD = "cosimino";    // Replace with your network psw

  const char* SSID = "xxxxxxxxxx";      // Replace with your network name
  const char* PASSWORD = "xxxxxxxxxxx";    // Replace with your network psw
*/


const char* SSID = "Iacopo wifi";        // Replace with your network name
const char* PASSWORD = "trottola";       // Replace with your network psw

// Initialize Telegram BOT
#define BOTtoken "1735101347:AAFVXkpBJkKWLETT39tuZycR59EcPM-_Ooo"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "1040419680"
// used for security, this bot is yours no one can access to it only with the name
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// declaration pin
const int ledRGBred = 12; // D6 on board
const int ledRGBgreen = 14; // D5 on board
const int ledRGBblue = 13; // D7 on board
const int buzzer = 5; // D1 on board
const int DHT_PIN_DATA = 0; // D3 on board
DHT dht(DHT_PIN_DATA);


// variables for set the desired temperature
int lower_bound = 16;
int upper_bound = 24;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
    // serial monitor 115200 baud
    Serial.begin(115200);

#ifdef ESP8266
    client.setInsecure();
#endif

    // connect led rgb pin
    pinMode(ledRGBred, OUTPUT);
    pinMode(ledRGBgreen, OUTPUT);
    pinMode(ledRGBblue, OUTPUT);

    // aggiungo pin buzzer
    pinMode(buzzer, OUTPUT);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    // Print ESP32/ESP8266 Local IP Address
    Serial.println(WiFi.localIP());
}

// set a rgb color on led
void RGB_color(int red, int green, int blue) {
    analogWrite(ledRGBred, red);
    analogWrite(ledRGBgreen, green);
    analogWrite(ledRGBblue, blue);
}


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

    for (int i = 0; i < numNewMessages; i++) {
        // Chat id of the requester
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID) {
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.println(text);

        String from_name = bot.messages[i].from_name;
        String welcome = "Welcome, " + from_name + ".\n";

        String keyboardJson = F("[[{ \"text\" : \"Home temperature\", \"callback_data\" : \"/temperature\" },{ \"text\" : \"Home humidity\", \"callback_data\" : \"/humidity\" }],");
        keyboardJson += F("[{ \"text\" : \"Set lower bound temperature\", \"callback_data\" : \"/lower_bound\" }], ");
        keyboardJson += F("[{ \"text\" : \"Set upper bound temperature\", \"callback_data\" : \"/upper_bound\" }], ");
        keyboardJson += F("[{ \"text\" : \"Check bounds\", \"callback_data\" : \"/state\" }, { \"text\" : \"Alert people\", \"callback_data\" : \"/alert\" }]]");

        // emit an alert sound
        if (text == "/alert") {
            bot.sendMessage(chat_id, "Sound alert", "");
            tone(buzzer, 1000); // Send 1KHz sound signal...
            delay(1000);        // ...for 1 sec
            noTone(buzzer);     // Stop sound...
        }
        // return the current temperature
        if (text == "/temperature") {
            float dhtTempC = dht.readTempC(); // Read temperature in Celsius, for Fahrenheit use .readTempF()
            String temp = String(dhtTempC);
            Serial.print(F("Temp: ")); Serial.print(dhtTempC); Serial.println(F(" [C]"));
            bot.sendMessage(chat_id, ("Home temperature: " + temp + " C"), "");
        }
        // return the current humidity
        if (text == "/humidity") {
            float dhtHumidity = dht.readHumidity(); // Reading humidity in %
            String humidity = String(dhtHumidity);
            Serial.print(F("Humidity: ")); Serial.print(dhtHumidity); Serial.print(F(" [%]\t"));
            bot.sendMessage(chat_id, ("Home humidity: " + humidity + " %\t"), "");
        }

        if (text == "/testRGB") {
            bot.sendMessage(chat_id, "Test rgb led...", "");
            RGB_color(255, 0, 0); // Red
            delay(1000);
            RGB_color(0, 255, 0); // Green
            delay(1000);
            RGB_color(0, 0, 255); // Blue
            delay(1000);
            RGB_color(0, 0, 0);
        }

        if (text == "/upper_bound" || text == "/lower_bound") {
            if (text == "/upper_bound") {
                bot.sendMessage(chat_id, "Write /ub=<integer number> to set the Upper bound of temperature", "");
            } else {
                bot.sendMessage(chat_id, "Write /lb=<integer number> to set the Upper bound of temperature", "");
            }
            bot.sendMessage(chat_id, "Remember to check the bound to see if it works.\nWarning: if you write a string value the bound is setted to 0.", "");
        }

        // extract the substring for change the bound
        String action = text.substring(0, 3);
        String value = text.substring(4, 7);
        int bound = value.toInt();

        if (action == "/ub") {
            if (bound <= lower_bound) {
                bot.sendMessage(chat_id, "Warning: upper bound cannot be smaller than the lower bound.", "");
            } else {
                upper_bound = bound;
                bot.sendMessage(chat_id, ("Temperature bound: [" + String(lower_bound) + ", " + String(upper_bound) + "] C"), "");
            }
        }

        if (action == "/lb") {
            if (bound >= upper_bound) {
                bot.sendMessage(chat_id, "Warning: lower bound cannot be bigger than the upper bound.", "");
            } else {
                lower_bound = bound;
                bot.sendMessage(chat_id, ("Temperature bound: [" + String(lower_bound) + ", " + String(upper_bound) + "] C"), "");
            }
        }

        if (text == "/state") {
            bot.sendMessage(chat_id, ("Temperature bound: [" + String(lower_bound) + ", " + String(upper_bound) + "] C"), "");
        }

        if (text == "/start" || text == "/command")
        {
            bot.sendMessage(chat_id, welcome, "");
            bot.sendMessageWithInlineKeyboard(chat_id, "Use the following command to control the iot passive thermostat or write /help.", "", keyboardJson);
        }

        if (text == "/help") {
            welcome += "The passive notification of this thermostat system is provided by a RGB lamp connected to the micro-controller.\n";
            welcome += "It is possible to set the bounds of home desired temperature and the RGB lamp will change color based on them.\n";
            welcome += "When the temperature is below the lower bound the light turns to blue instead when is over the upper bound light turns to red.\n";
            welcome += "There is the possibility to actively notify people in the house with an alert sound.";
            welcome += "\n\nFor the textual command list write: /command_list";
            welcome += "\n\nFor the command button list write: /command";
            bot.sendMessage(chat_id, welcome, "");
        }

        if (text == "/command_list") {
            String command_list = "Textual commands list:\n";
            command_list += "/alert to alert people actively \n";
            command_list += "/temperature to get home tempemperature \n";
            command_list += "/humidity to get tempemperature home humidity \n";
            command_list += "/testRGB test all rgb color on led \n";
            command_list += "/upper_bound instruction to set upper bound\n";
            command_list += "/lower_bound instruction to set lower bound\n";
            command_list += "/state to get the temperature bounds \n";
            command_list += "/help to recive help about iot passive thermostat\n";
            command_list += "/command to display the command buttons \n";
            bot.sendMessage(chat_id, command_list, "");
        }

    }
}

void trasactionRGB(unsigned int rgbColourEnd[3]) {
    unsigned int rgbColour[3];
    // Start off with red.
    rgbColour[0] = rgbColourEnd[0];
    rgbColour[1] = rgbColourEnd[1];
    rgbColour[2] = rgbColourEnd[2];
    // Choose the colours to increment and decrement.
    for (int decColour = 0; decColour < 3; decColour += 1) {
        int incColour = decColour == 2 ? 0 : decColour + 1;
        // cross-fade the two colours.
        for (int i = 0; i < 255; i += 1) {
            rgbColour[decColour] -= 1;
            rgbColour[incColour] += 1;
            RGB_color(rgbColour[0], rgbColour[1], rgbColour[2]);
            delay(50); // speed of change led on rgb lamp
        }
    }
}

// loop function listen the message on the bot and pilot the passive notification on the rgb led
void loop() {
    float dhtTempC = dht.readTempC();

    if (lower_bound < dhtTempC && dhtTempC < upper_bound) {
        unsigned int white[3] = {255, 255, 255};
        trasactionRGB(white);
    }
    if (dhtTempC > upper_bound) {
        unsigned int red[3] = {255, 0, 0};
        trasactionRGB(red);
    }
    if (lower_bound > dhtTempC) {
        unsigned int blue[3] = {0, 0, 255};
        trasactionRGB(blue);
    }

    if (millis() > lastTimeBotRan + botRequestDelay)  {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}