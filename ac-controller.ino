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
String CODE_VERSION = "$Revision: 1.9 $";

#include <Syslog.h>
#include <ESP_EEPROM.h>
unsigned long write_eeprom_time = 0;

extern Syslog syslog;
// defines pins numbers
const byte IRLedPin = D1;
const byte led = LED_BUILTIN;

ESP8266WebServer server(80);

// Define a ir sender at port D2
IRSenderBitBang irSender(IRLedPin);

// Define the heatpump
MitsubishiHeavyZMHeatpumpIR heatpump;

struct State {
    uint8_t power=POWER_ON;
    uint8_t mode=MODE_COOL;
    uint8_t temp=27;
    uint8_t vdir=VDIR_MANUAL;
    uint8_t hdir=HDIR_MANUAL;
    uint8_t fanspeed=FAN_AUTO;
    bool silent=0;
    bool _3d=0;
} state;


String error_str="";

void sendIndexHTML() {
    String mode_str, mode_iscool_htmlstr="", mode_isfan_htmlstr="", mode_isheat_htmlstr="";
    String power_str="", power_on_htmlstr="", power_off_htmlstr="";
    String silent_str="", silent_on_htmlstr="", silent_off_htmlstr="";
    String _3d_str="", _3d_on_htmlstr="", _3d_off_htmlstr="";
    String fanspeed_str="";
    String buf="";
    String bgcolor="";

    switch (state.power) {
      case POWER_OFF:
          power_str="off";
          power_off_htmlstr="checked=\"checked\"";
          bgcolor="#777777";
          break;
      case POWER_ON:
          power_str="on";
          power_on_htmlstr="checked=\"checked\"";
          break;
      default:
          power_str="unknown";
          bgcolor="#444444";
    }

    switch (state.mode) {
      case MODE_COOL:
          mode_str="cool";
          mode_iscool_htmlstr="selected=\"selected\"";
          if (state.power == POWER_ON) {
              bgcolor="#4444CC";
          }
          break;
      case MODE_FAN:
          mode_str="fan";
          mode_isfan_htmlstr="selected=\"selected\"";
          if (state.power == POWER_ON) {
              bgcolor="#44CC44";
          }
          break;
      case MODE_HEAT:
          mode_str="heat";
          mode_isheat_htmlstr="selected=\"selected\"";
          if (state.power == POWER_ON) {
              bgcolor="#CC4444";
          }
          break;
      default:
          mode_str="unknown";
    }

    switch (state.silent) {
      case 0:
          silent_str="normal";
          silent_off_htmlstr="checked=\"checked\"";
          break;
      case 1:
          silent_str="silent";
          silent_on_htmlstr="checked=\"checked\"";
          break;
      default:
          silent_str="unknown";
    }

    switch (state._3d) {
      case 0:
          _3d_str="normal";
          _3d_off_htmlstr="checked=\"checked\"";
          break;
      case 1:
          _3d_str="3d";
          _3d_on_htmlstr="checked=\"checked\"";
          break;
      default:
          _3d_str="unknown";
    }

    switch (state.fanspeed) {
      case 0:
          fanspeed_str="auto";
          break;
      default:
          fanspeed_str=String(state.fanspeed);
    }

    buf = "<html>"
        "<head>"
        "<title>Loungeroom Air Conditioner</title>"
        "<meta name=\"viewport\" content=\"width=max-device-width, user-scalable=yes, initial-scale=1.0\" />"
        "<meta content=\"no-cache\" http-equiv=\"Pragma\" />"
        "</head>"
        "<body bgcolor=\""+bgcolor+"\"><h1>Loungeroom Air Conditioner</h1>"+
        error_str+

        "<form action=\"do\" method=\"POST\">\n"
        "Power: "
        "<input type=\"radio\" id=\"0\" name=\"power\" value=\"0\" "+power_off_htmlstr+"onchange=\"this.form.submit()\">"
        "<label for=\"0\">Off</label>"
        "<input type=\"radio\" id=\"1\" name=\"power\" value=\"1\" "+power_on_htmlstr+"onchange=\"this.form.submit()\">"
        "<label for=\"1\">On</label>"
        "</form>"
        "<br>\n"

        "<form action=\"do\" method=\"POST\">\n"
        "<label for=\"mode\">Mode:</label>"
        "<select name=\"mode\" id=\"mode\" onchange=\"this.form.submit()\">"
        "<option value=\"cool\" "+mode_iscool_htmlstr+">Cool</option>"
        "<option value=\"fan\" "+mode_isfan_htmlstr+">Fan</option>"
        "<option value=\"heat\" "+mode_isheat_htmlstr+">Heat</option>"
        "</select><br>\n"
        "</form>"

        "<form action=\"do\" method=\"POST\">\n"
        "<label for=\"slider_temp\">Temperature <span id=\"val_temp_html\">"+state.temp+"</span>:</label>"
        "<input type=\"range\" id=\"slider_temp\" name=\"temp\" min=\"18\" max=\"30\" value=\""+state.temp+"\" onmouseup=\"this.form.submit()\" ontouchend=\"this.form.submit()\">"
        "</form>"

        //https://stackoverflow.com/questions/15935837/how-to-display-a-range-input-slider-vertically - but making it look OK would be a challenge
        "<form action=\"do\" method=\"POST\">\n"
        "<label for=\"slider_vdir\">Vdir <span id=\"val_vdir_html\" style=\"width:2em;display:inline-block;\">"+state.vdir+"</span>:</label>"
        "<input type=\"range\" "
        //style=\"-webkit-transform: rotate(90deg);-moz-transform: rotate(90deg);-o-transform: rotate(90deg);-ms-transform: rotate(90deg);transform: rotate(90deg);\"
        "id=\"slider_vdir\" name=\"vdir\" min=\"0\" max=\"6\" value=\""+state.vdir+"\" onmouseup=\"this.form.submit()\" ontouchend=\"this.form.submit()\">"
        "</form>"

        "<form action=\"do\" method=\"POST\">\n"
        "<label for=\"slider_hdir\">Hdir <span id=\"val_hdir_html\" style=\"width:2em;display:inline-block;\">"+state.hdir+"</span>:</label>"
        "<input type=\"range\" id=\"slider_hdir\" name=\"hdir\" min=\"0\" max=\"6\" value=\""+state.hdir+"\" onmouseup=\"this.form.submit()\" ontouchend=\"this.form.submit()\">"
        "</form>"

        "<form action=\"do\" method=\"POST\">\n"
        "<label for=\"slider_fan\">Fan speed <span id=\"val_fan_html\" style=\"width:2em;display:inline-block;\">"+fanspeed_str+"</span>:</label>"
        "<input type=\"range\" id=\"slider_fan\" name=\"fan\" min=\"0\" max=\"3\" value=\""+state.fanspeed+"\" onmouseup=\"this.form.submit()\" ontouchend=\"this.form.submit()\">"
        "</form>"

        "<form action=\"do\" method=\"POST\">\n"
        "Silent: "
        "<input type=\"radio\" id=\"0\" name=\"silent\" value=\"0\" "+silent_off_htmlstr+" onchange=\"this.form.submit()\">"
        "<label for=\"0\">Normal</label>"
        "<input type=\"radio\" id=\"1\" name=\"silent\" value=\"1\" "+silent_on_htmlstr+" onchange=\"this.form.submit()\">"
        "<label for=\"1\">Silent</label>"
        "</form>"
        "<br>\n"

        "<form action=\"do\" method=\"POST\">\n"
        "3D: "
        "<input type=\"radio\" id=\"0\" name=\"3d\" value=\"0\" "+_3d_off_htmlstr+" onchange=\"this.form.submit()\">"
        "<label for=\"0\">Normal</label>"
        "<input type=\"radio\" id=\"1\" name=\"3d\" value=\"1\" "+_3d_on_htmlstr+" onchange=\"this.form.submit()\">"
        "<label for=\"1\">3D</label>"
        "</form>"
        "<br>\n"

        "<script>\n\n"
        "var slider_temp = document.getElementById(\"slider_temp\");\n"
        "var val_temp_html = document.getElementById(\"val_temp_html\");\n"
        "function set_temp_html(v) {\n"
        "    val_temp_html.innerHTML = v;\n"
        "}\n"
        "slider_temp.oninput = function() {\n"
        "    set_temp_html(this.value);\n"
        "}\n"
        "set_temp_html(slider_temp.value);\n\n"

        "var slider_vdir = document.getElementById(\"slider_vdir\");\n"
        "var val_vdir_html = document.getElementById(\"val_vdir_html\");\n"
        "function set_vdir_html(v) {\n"
        "    switch(v) {\n"
        "      case \"0\":\n"
        "        val_vdir_html.innerHTML = \"auto/manual\";\nbreak;\n"
        "      case \"1\":\n"
        "        val_vdir_html.innerHTML = \"swing\";\nbreak;\n"
        "      case \"2\":\n"
        "        val_vdir_html.innerHTML = \"up\";\nbreak;\n"
        "      case \"3\":\n"
        "        val_vdir_html.innerHTML = \"mup\";\nbreak;\n"
        "      case \"4\":\n"
        "        val_vdir_html.innerHTML = \"middle\";\nbreak;\n"
        "      case \"5\":\n"
        "        val_vdir_html.innerHTML = \"mdown\";\nbreak;\n"
        "      case \"6\":\n"
        "        val_vdir_html.innerHTML = \"down\";\nbreak;\n"
        "      default:\n"
        "        val_vdir_html.innerHTML = \"unknown\";\nbreak;\n"
        "    }\n"
        "}\n"
        "slider_vdir.oninput = function() {\n"
        "    set_vdir_html(this.value);\n"
        "}\n"
        "set_vdir_html(slider_vdir.value);\n\n"

        "var slider_hdir = document.getElementById(\"slider_hdir\");\n"
        "var val_hdir_html = document.getElementById(\"val_hdir_html\");\n"
        "function set_hdir_html(v) {\n"
        "    switch(v) {\n"
        "      case \"0\":\n"
        "        val_hdir_html.innerHTML = \"auto/manual\";\nbreak;\n"
        "      case \"1\":\n"
        "        val_hdir_html.innerHTML = \"swing\";\nbreak;\n"
        "      case \"2\":\n"
        "        val_hdir_html.innerHTML = \"middle\";\nbreak;\n"
        "      case \"3\":\n"
        "        val_hdir_html.innerHTML = \"left\";\nbreak;\n"
        "      case \"4\":\n"
        "        val_hdir_html.innerHTML = \"mleft\";\nbreak;\n"
        "      case \"5\":\n"
        "        val_hdir_html.innerHTML = \"mright\";\nbreak;\n"
        "      case \"6\":\n"
        "        val_hdir_html.innerHTML = \"right\";\nbreak;\n"
        "      default:\n"
        "        val_hdir_html.innerHTML = \"unknown\";\nbreak;\n"
        "    }\n"
        "}\n"
        "slider_hdir.oninput = function() {\n"
        "    set_hdir_html(this.value);\n"
        "}\n"
        "set_hdir_html(slider_hdir.value);\n\n"

        "var slider_fan = document.getElementById(\"slider_fan\");\n"
        "var val_fan_html = document.getElementById(\"val_fan_html\");\n"
        "function set_fan_html(v) {\n"
        "    if (v == 0) {\n"
        "        val_fan_html.innerHTML = \"auto\";\n"
        "    } else {\n"
        "        val_fan_html.innerHTML = v;\n"
        "    }\n"
        "}\n"
        "slider_fan.oninput = function() {\n"
        "    set_fan_html(this.value);\n"
        "}\n"
        "set_fan_html(slider_fan.value);\n\n"

        "</script>"
        "</body></html>\n";
    error_str="";
    server.send(200, "text/html", buf);
}

void updateAC() {
    char *mode_str;
    switch (state.mode) {
      case MODE_COOL:
          mode_str="cool";
          break;
      case MODE_FAN:
          mode_str="fan";
          break;
      case MODE_HEAT:
          mode_str="heat";
          break;
      default:
          mode_str="unknown";
    }
    syslog.logf(LOG_INFO, "Power: %i, Mode: %s, Temp: %i, Vdir: %i, Hdir: %i, FanSpeed: %i, Silent: %i, 3D: %i", state.power, mode_str, state.temp, state.vdir, state.hdir, state.fanspeed, state.silent, state._3d);
    // FIXME: write these params to flash
// Send the command
    heatpump.send(irSender, state.power, state.mode, state.fanspeed, state.temp, state.vdir, state.hdir, 0, state.silent, state._3d);
}

bool setParameters(bool force=false) {
    int temp_p = getArgValue("temp");
    int fan_p = getArgValue("fan");
    String mode_p = getArgValueStr("mode");
    int power_p = getArgValue("power");
    int vdir_p = getArgValue("vdir");
    int hdir_p = getArgValue("hdir");
    int silent_p = getArgValue("silent");
    int _3d_p = getArgValue("3d");

    bool changed=force;

    if (mode_p != "") {
        syslog.log(LOG_INFO, "mode supplied");
        if (mode_p ==  "cool") {
            state.mode=MODE_COOL;
            state.temp=27;
            changed=true;
        } else if (mode_p == "fan") {
            state.mode=MODE_FAN;
            changed=true;
        } else if (mode_p == "heat") {
            state.mode=MODE_HEAT;
            state.temp=18;
            changed=true;
        } else {
            error_str="Unknown mode supplied<br>\n"+error_str;
        }
    }

    if (power_p >= 0) {
        syslog.log(LOG_INFO, "power supplied");
        if (power_p == 0 || power_p == 1) {
            state.power = power_p;
            changed=true;
        } else {
            error_str="Unknown power supplied<br>\n"+error_str;
        }
    }

    if (temp_p >= 0) {
        syslog.log(LOG_INFO, "temp supplied");
        if (temp_p >= 18 && temp_p <= 30) {
            state.temp = temp_p;
            changed=true;
        } else {
            error_str="Unknown temp supplied<br>\n"+error_str;
        }
    }

    if (fan_p >= 0 ) {
        syslog.log(LOG_INFO, "fan supplied");
        if (fan_p >= 0 && fan_p <= 3) {
            state.fanspeed = fan_p;
            changed=true;
        } else {
            error_str="Unknown power supplied<br>\n"+error_str;
        }
    } else {
        if (temp_p >= 0) {
            if (state.temp >= 27) {
                state.fanspeed = FAN_1;
            } else if (state.temp >= 26) {
                state.fanspeed = FAN_2;
            } else if (state.temp >= 25) {
                state.fanspeed = FAN_3;
            } else if (state.temp <= 18) {
                state.fanspeed = FAN_1;
                /* } else if (state.temp <= 19) {
                   state.fanspeed = FAN_2;
                   } else if (state.temp <= 20) {
                   state.fanspeed = FAN_3; */
            } else {
                state.fanspeed = FAN_AUTO;
            }
        }
    }

    if (vdir_p >= 0) {
        syslog.log(LOG_INFO, "vdir supplied");
        if (vdir_p >= 0 && vdir_p <= 6) {
            state.vdir = vdir_p;
            changed=true;
        } else {
            error_str="Unknown vdir supplied<br>\n"+error_str;
        }
    }

    if (hdir_p >= 0) {
        syslog.log(LOG_INFO, "hdir supplied");
        if (hdir_p >= 0 && hdir_p <= 6) {
            state.hdir = hdir_p;
            changed=true;
        } else {
            error_str="Unknown hdir supplied<br>\n"+error_str;
        }
    }

    if (silent_p >= 0) {
        syslog.log(LOG_INFO, "silent supplied");
        if (silent_p == 0 || silent_p == 1) {
            state.silent = silent_p;
            changed=true;
        } else {
            error_str="Unknown silent supplied<br>\n"+error_str;
        }
    }

    if (_3d_p >= 0) {
        syslog.log(LOG_INFO, "3d supplied");
        if (_3d_p == 0 || _3d_p == 1) {
            state._3d = _3d_p;
            changed=true;
        } else {
            error_str="Unknown 3d supplied<br>\n"+error_str;
        }
    }

    if (changed) {
        write_eeprom_time=millis()+10000;
    }
    return changed;
}

void http_do() {
    if (setParameters()) {
        updateAC();
    }
    sendIndexHTML();
}

void http_do_and_redirect() {
    if (setParameters()) {
        updateAC();
    }
    server.sendHeader("Location",".");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void http_forcedo_and_redirect() {
    if (setParameters(true)) {
        updateAC(); // always true with true above
    }
    server.sendHeader("Location",".");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void http_ac_on() {
    state.mode=MODE_COOL;
    state.power=POWER_ON;

    http_forcedo_and_redirect();
}

void http_heater_on() {
    state.mode=MODE_HEAT;
    state.power=POWER_ON;

    http_forcedo_and_redirect();
}

void http_fan_on() {
    state.mode=MODE_FAN;
    state.power=POWER_ON;

    http_forcedo_and_redirect();
}

void http_off() {
    state.power=POWER_OFF;

    http_forcedo_and_redirect();
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

    pinMode(IRLedPin, OUTPUT); // Sets the trigPin as an Output

    Serial.println("HTTP server extra setup");

    server.on("/ac_on",       http_ac_on);
    server.on("/heater_on",   http_heater_on);
    server.on("/fan_on",      http_fan_on);
    server.on("/off",         http_off);
    server.on("/",            http_do);
    server.on("/do",          http_do_and_redirect);

    ledRamp(0,led_range,80,30);

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    EEPROM.begin(sizeof(State));
    if(EEPROM.percentUsed()>=0) {
        Serial.println("EEPROM has data from a previous run.");
        Serial.printf("%i%% of ESP flash space currently used\n", EEPROM.percentUsed());
        syslog_buffer="EEPROM has data from a previous run: "+String(EEPROM.percentUsed())+"% of ESP flash space currently used";
        EEPROM.get(0, state);
    }
    ledBright(1);
}

void loop_stub(void) {
    //pollButtons();
    if (write_eeprom_time && (millis() > write_eeprom_time)) {
        write_eeprom_time = 0;
        EEPROM.put(0, state);
        boolean ok = EEPROM.commit();
        syslog.log(LOG_INFO, (ok) ? "Commit OK" : "Commit failed");
    }
}
