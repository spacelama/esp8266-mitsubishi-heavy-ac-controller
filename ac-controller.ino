/*
 *  Originally based on:
 *  https://www.sysrun.io/2017/02/22/use-a-esp8266-to-control-your-ac-via-mqtt/
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// Include the appropriate header for your A/C
#include <MitsubishiHeavyHeatpumpIR.h>

#include "Switch.h"
#include "template.h"
String CODE_VERSION = "$Revision: 1.6 $";

#include <Syslog.h>

extern Syslog syslog;
// defines pins numbers 
const byte IRLedPin = D1;
const byte led = LED_BUILTIN;

ESP8266WebServer server(80);

// Define a ir sender at port D2
IRSenderBitBang irSender(IRLedPin);
 
// Define the heatpump 
MitsubishiHeavyZMHeatpumpIR heatpump;
 
void http_ac_on() {
    uint8_t fanSpeed;
    int temp = getArgValue("temp");
    if (temp < 0) {
        temp = 27;
    }
    if (temp >= 27) {
        fanSpeed = FAN_1;
    } else if (temp >= 26) {
        fanSpeed = FAN_2;
    } else if (temp >= 25) {
        fanSpeed = FAN_3;
    } else
        fanSpeed = FAN_AUTO;

    String content;
    content = "ac_on: " + String(temp);
 
    // Send the command
    heatpump.send(irSender, POWER_ON, MODE_COOL, fanSpeed, temp, VDIR_MANUAL, HDIR_MANUAL, 0, 0, 0);
   
    server.send(200, "text/html", content+"\n");
}

void http_heater_on() {
    uint8_t fanSpeed;
    int temp = getArgValue("temp");
    if (temp < 0) {
        temp = 18;
    }    
    if (temp <= 18) {
        fanSpeed = FAN_1;
    /* } else if (temp <= 19) { */
    /*     fanSpeed = FAN_2; */
    /* } else if (temp <= 20) { */
    /*     fanSpeed = FAN_3; */
    } else
        fanSpeed = FAN_AUTO;

    String content;
    content = "heater_on: " + String(temp);
 
    // Send the command
    heatpump.send(irSender, POWER_ON, MODE_HEAT, fanSpeed, temp, VDIR_MANUAL, HDIR_MANUAL, 0, 0, 0);
   
    server.send(200, "text/html", content+"\n");
}

void http_fan_on() {
    uint8_t fanSpeed;
    int temp = getArgValue("temp");
    if (temp < 0) {
        temp = 22;
    }    
    if (temp >= 27) {
        fanSpeed = FAN_1;
    } else if (temp >= 26) {
        fanSpeed = FAN_2;
    } else if (temp >= 25) {
        fanSpeed = FAN_3;
    } else
        fanSpeed = FAN_AUTO;

    String content;
    content = "fan_on: " + String(temp);
 
    // Send the command
    heatpump.send(irSender, POWER_ON, MODE_FAN, fanSpeed, temp, VDIR_MANUAL, HDIR_MANUAL, 0, 0, 0);
   
    server.send(200, "text/html", content+"\n");
}

void http_off() {
    int temp = getArgValue("temp");
    if (temp < 0) {
        temp = 22;
    }    

    String content;
    content = "off: " + String(temp);
 
    // Send the command
    heatpump.send(irSender, POWER_OFF, MODE_COOL, FAN_AUTO, 27, VDIR_MANUAL, HDIR_MANUAL, 0, 0, 0);
   
    server.send(200, "text/html", content+"\n");
}

String handleRoot_stub() {
    return "";
}

String http_uptime_stub() {
    return "";
}

void http_handle_not_found() {
    server.send(404, "text/plain", "File Not Found\n");
}

void setup_stub(void) {
    digitalWrite(D8, LOW);  // should be unused, reset pin, assign to known state
    digitalWrite(D4, HIGH); // should be unused, reset pin, assign to known state
//    digitalWrite(D3, HIGH); // should be unused, reset pin, assign to known state - overriden by input_pullup below
//    pinMode(D3, OUTPUT);  // switch installed on this pin - maybe configure as input PULLUP instead?

    pinMode(D3, INPUT_PULLUP);
    pinMode(D4, OUTPUT);
    pinMode(D8, OUTPUT);

    Serial.println("");

    //find it as http://lights.local
    /*if (MDNS.begin("lights")) 
      {
      Serial.println("MDNS responder started");
      }*/
  
    ledRamp(led_range,0,1000,30);

//    server.on("/", handleRoot);

    pinMode(IRLedPin, OUTPUT); // Sets the trigPin as an Output

    Serial.println("HTTP server extra setup");

    server.on("/ac_on",       http_ac_on);
    server.on("/heater_on",   http_heater_on);
    server.on("/fan_on",      http_fan_on);
    server.on("/off",         http_off);

    ledRamp(0,led_range,80,30);

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    ledBright(1);
}

void loop_stub(void) {
    //pollButtons();
}
