/**
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

/*************************************************************************+
 ****************** Node mcu v1.0 pin definitions *************************
 **************************************************************************/
const int ledRGBred = 12; // D6 on board
const int ledRGBgreen = 14; // D5 on board
const int ledRGBblue = 13; // D7 on board
const int buzzer = 5; // D1 on board
const int DHT_PIN_DATA = 0; // D3 on board
const int button = 15; //D8 on board
DHT dht(DHT_PIN_DATA);

/*********************************************************
 ****************** User setting *************************
 *********************************************************/

const char* SSID = "network_name";        // Replace with your network name
const char* PASSWORD = "password";       // Replace with your network psw


// Initialize Telegram BOT
#define BOTtoken "xxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can message you
#define CHAT_ID "1040419680"
// used for security, this bot is yours no one can access to it only with the name
WiFiClientSecure client_bot;
UniversalTelegramBot bot(BOTtoken, client_bot);

//Hue constants // need to rename
const char hueHubIP[] = "192.168.0.50";  //Hue Bridge IP
const char hueUsername[] = "myhue";  //Hue username
const int hueHubPort = 80;

//Hue variables
IPAddress server(192, 168, 0, 50); // insert your hue ip
const int DELAY_HUE = 100000; //transaction time for hue

// Hue color in JSON format using hue, saturation and brightness
const String hueWhite = "{\"on\": true,\"hue\": 50100,\"sat\":255,\"bri\":255,\"transitiontime\":" + String(0) + "}";
const String hueRed = "{\"on\": true,\"hue\": 0,\"sat\":255,\"bri\":124,\"transitiontime\":" + String(DELAY_HUE) + "}";
const String hueBlue = "{\"on\": true,\"hue\": 200,\"sat\":255,\"bri\":124,\"transitiontime\":" + String(DELAY_HUE) + "}";
WiFiClient client_hue;

// THINGSPEAK integration
const char *host_thingspeak = "api.thingspeak.com";        //IP address of the thingspeak server
const char *api_key_thingspeak = "replace with yours";     //Your own thingspeak api_key
const int httpPort = 80;                                   //thingspeak port
long uploadTime = 60000;                                   //upload time for thingspeak every 60 sec
long differenceUpload = 0;
WiFiClient client_thingspeak;

/*********************************************************
 ************* Micro controller parameters ***************
 *********************************************************/

// progressive time in millis for transaction of change color to notify change state
const int CHANGE_DELAY = 5; // if higher the change is slower

// Checks for new messages every 1 second
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// default variables for set the desired temperature in celsious
int lowerBound = 16;
int upperBound = 24;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

// led color
unsigned int green[3] = {0, 255, 0};
unsigned int red[3] = {255, 0, 0};
unsigned int blue[3] = {0, 0, 255};

/****************************************
 **************** CODE ******************
 ****************************************/

// setup of the micro controller
void setup() {
    Serial.begin(115200); // serial monitor 115200 baud
#ifdef ESP8266
    client_bot.setInsecure();
#endif
    // connect led rgb pin
    pinMode(ledRGBred, OUTPUT);
    pinMode(ledRGBgreen, OUTPUT);
    pinMode(ledRGBblue, OUTPUT);

    // add pin buzzer
    pinMode(buzzer, OUTPUT);

    // add pin button as input
    pinMode(button, INPUT);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println(WiFi.localIP());     // Print ESP8266 Local IP Address

    while (!client_thingspeak.connect(host_thingspeak, httpPort)) {
        Serial.println("Connection Failed");
    }
}

// set a rgb color on led with my function it is possible to use RGB-1.0.5 libraries provided
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
            for (int i = 0; i < 3; i++) {
                tone(buzzer, 1000); // Send 1KHz sound signal...
                delay(1000);        // ...for 1 sec
                noTone(buzzer);     // Stop sound...
            }
        }
            // return the current temperature
        else if (text == "/temperature") {
            float dhtTempC = dht.readTempC(); // Read temperature in Celsius, for Fahrenheit use .readTempF()
            String temp = String(dhtTempC);
            Serial.print(F("Temp: ")); Serial.print(dhtTempC); Serial.println(F(" [C]"));
            bot.sendMessage(chat_id, ("Home temperature: " + temp + " C"), "");
        }
            // return the current humidity
        else if (text == "/humidity") {
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
            if (bound <= lowerBound) {
                bot.sendMessage(chat_id, "Warning: upper bound cannot be smaller than the lower bound.", "");
            } else {
                upperBound = bound;
                bot.sendMessage(chat_id, ("Temperature bound: [" + String(lowerBound) + ", " + String(upperBound) + "] C"), "");
            }
        }

        if (action == "/lb") {
            if (bound >= upperBound) {
                bot.sendMessage(chat_id, "Warning: lower bound cannot be bigger than the upper bound.", "");
            } else {
                lowerBound = bound;
                bot.sendMessage(chat_id, ("Temperature bound: [" + String(lowerBound) + ", " + String(upperBound) + "] C"), "");
            }
        }

        if (text == "/state") {
            bot.sendMessage(chat_id, ("Temperature bound: [" + String(lowerBound) + ", " + String(upperBound) + "] C"), "");
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

        /*if u want to add a new command on telegram follow this pattern
          if (text == "<action>") {
          // connect other sensor like a audio sensor for measuring a noise in a room
          if(sensor.sense() > limit){
            //do something
           }
          bot.sendMessage(chat_id, "Hello i'm a message", "");
          }
        */
    }
}

// function for change color for passive notification
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
            delay(CHANGE_DELAY); // speed of change led on rgb lamp
        }
    }
}

//upload temperature humidity data to thingspeak.com
void uploadTemperatureHumidity() {
    float dhtTempC = dht.readTempC();
    float dhtHumidity = dht.readHumidity(); // Reading humidity in %
    if (!client_thingspeak.connect(host_thingspeak, httpPort)) {
        Serial.println("connection failed thingspeak");
        return;
    }
    Serial.println("upload");
    // Four values (field1 field2 field3 field4) have been set in thingspeak.com
    client_thingspeak.print(String("GET ") + "/update?api_key=" + api_key_thingspeak + "&field1=" + dhtTempC + "&field2=" + dhtHumidity + " HTTP/1.1\r\n" + "Host: " + host_thingspeak + "\r\n" + "Connection: close\r\n\r\n");
    while (client_thingspeak.available()) {
        String line = client_thingspeak.readStringUntil('\r');
        Serial.print(line);
    }
}

// HUE INTEGRATION

/* setHue() is a command function, which needs to be passed a light number and a
   properly formatted command string in JSON format.
   It then makes a simple HTTP PUT request to the Bridge at the IP specified at the start.

   Made according to philips hue api site: https://developers.meethue.com/develop/get-started-2/
*/
boolean setHue(int lightNum, String command)
{
    Serial.println("setHue");
    if (client_hue.connect(server, 80))
    {
        Serial.println(F("connected to server"));
        while (client_hue.connected() && WiFi.status() == WL_CONNECTED)
        {
            client_hue.print(F("PUT /api/"));
            client_hue.print(hueUsername);
            client_hue.print(F("/lights/"));
            client_hue.print(lightNum);  // hueLight zero based, add 1
            client_hue.println(F("/state HTTP/1.1"));
            client_hue.println(F("keep-alive"));
            client_hue.print(F("Host: "));
            client_hue.println(hueHubIP);
            client_hue.print(F("Content-Length: "));
            client_hue.println(command.length());
            client_hue.println(F("Content-Type: text/plain;charset=UTF-8"));
            client_hue.println();  // blank line before body
            client_hue.println(command);  // Hue command
        }
        client_hue.stop();
        return true;  // command executed
    }
    else
        return false;  // command failed
}


// loop function listen the message on the bot and pilot the passive notification on the rgb led
void loop() {

    // section that scan new messages on telegram bot
    if (millis() > lastTimeBotRan + botRequestDelay)  {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
    // action related to sensor
    float dhtTempC = dht.readTempC();
    if (lowerBound < dhtTempC && dhtTempC < upperBound) {
        trasactionRGB(green); // led on node mcu
        //setHue(1, hueWhite); // philips hue lamp
    }
    if (dhtTempC > upperBound) {
        trasactionRGB(red); // led on node mcu
        //setHue(1, hueRed); // philips hue lamp 1
    }
    if (lowerBound > dhtTempC) {
        trasactionRGB(blue); // led on node mcu
        //setHue(1, hueBlue); // philips hue lamp
    }
    // action related on board button
    buttonState = digitalRead(button); // read the state of the button
    Serial.println(buttonState);
    // reset the bound of temperature and the light of consequence turn to white
    if (buttonState == HIGH) {
        Serial.println("Reset pressed");
        for (int i = 0; i < 6; i++) { // notify the user that have press the button
            RGB_color(255, 0, 0); // Red
            delay(200);
            RGB_color(0, 0, 0); // Red
            delay(200);
        }
        // set the bound for have the white lamp
        lowerBound -= 5;
        upperBound += 5;
        bot.sendMessage(CHAT_ID, "Warning: User play reset button.\nRemember to change the temperature bound!", "");
    }
    //Upload Temperature Humidity every 60 seconds in thingspeak.com
    if (millis() - differenceUpload > uploadTime) {
        uploadTemperatureHumidity();
        differenceUpload = millis();
    }
}