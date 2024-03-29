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
#include <Arduino_JSON.h>
unsigned long write_eeprom_time = 0;

extern Syslog syslog;
// defines pins numbers
const byte IRLedPin = D1;
const byte led = LED_BUILTIN;
extern unsigned char ajaxy_ac_js[];
extern unsigned char feedback_ac_js[];

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
    bool debug=0;
} state, previous_state;

String error_str="";

void sendIndexHTML() {
    String buf="";

    buf = "<html>"
        "<head>"
        "<title>Loungeroom Air Conditioner</title>"
        "<meta name=\"viewport\" content=\"width=max-device-width, user-scalable=yes, initial-scale=1.0\" />"
        "<meta content=\"no-cache\" http-equiv=\"Pragma\" />"
        "</head>"
        "<body><h1>Loungeroom Air Conditioner</h1>"+
        error_str+

        "Updated: "
        "<div id=\"ajaxtime\" style=\"display:inline\"></div>"
        "<br>\n"

        "Current temperature: "
        "<div id=\"ajaxtemp\" style=\"display:inline\"></div>"
        "<br>\n"

        "Power: "
        "<div id=\"ajaxpower\" style=\"display:inline\">"+state.power+"</div>"
        "<p></p>\n"

        "<label for=\"mode\">Mode:</label>"
        "<div id=\"ajaxmode\" style=\"display:inline\">"+state.mode+"</div>"
        "<p></p>\n"

        "<label for=\"slider_temp\">Temperature <span id=\"val_temp_html\">"+state.temp+"</span>:</label>"
        "<button onclick=\"temp_down()\">-</button>"
        "<div id=\"ajaxsetpointtemp\" style=\"display:inline\"></div>"
        "<button onclick=\"temp_up()\">+</button>"
        "<p></p>\n"

        "<label for=\"slider_vdir\">Vdir <span id=\"val_vdir_html\" style=\"width:5em;display:inline-block;\">"+state.vdir+"</span>:</label>"
        "<button onclick=\"vdir_down()\">-</button>"
        "<div id=\"ajaxvdir\" style=\"display:inline\"></div>"
        "<button onclick=\"vdir_up()\">+</button>"
        "<p></p>\n"

        "<label for=\"slider_hdir\">Hdir <span id=\"val_hdir_html\" style=\"width:5em;display:inline-block;\">"+state.hdir+"</span>:</label>"
        "<button onclick=\"hdir_down()\">-</button>"
        "<div id=\"ajaxhdir\" style=\"display:inline\"></div>"
        "<button onclick=\"hdir_up()\">+</button>"
        "<p></p>\n"

        "<label for=\"slider_fan\">Fan speed <span id=\"val_fan_html\" style=\"width:3em;display:inline-block;\">"+state.fanspeed+"</span>:</label>"
        "<button onclick=\"fan_down()\">-</button>"
        "<div id=\"ajaxfanspeed\" style=\"display:inline\"></div>"
        "<button onclick=\"fan_up()\">+</button>"
        "<p></p>\n"

        "Silent: "
        "<div id=\"ajaxsilent\" style=\"display:inline\">"+state.silent+"</div>"
        "<p></p>\n"

        "3D: "
        "<div id=\"ajax3d\" style=\"display:inline\">"+state._3d+"</div>"

        "<script type='text/javascript' src='ajaxy.js'></script>\n"
        "<script type='text/javascript' src='feedback.js'></script>\n"
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
      case MODE_DRY:
          mode_str="dehumidify";
          break;
      case MODE_AUTO:
          mode_str="auto (temporary only)";
          break;
      default:
          mode_str="unknown";
    }
    syslog.logf(LOG_INFO, "Power: %i, Mode: %s, Temp: %i, Vdir: %i, Hdir: %i, FanSpeed: %i, Silent: %s, 3D: %s, debug: %s", state.power, mode_str, state.temp, state.vdir, state.hdir, state.fanspeed, state.silent?"true":"false", state._3d?"true":"false", state.debug?"true":"false");
    // Send the command
    if (! state.debug) {
        heatpump.send(irSender, state.power, state.mode, state.fanspeed, state.temp, state.vdir, state.hdir, 0, state.silent, state._3d);
    }
}

bool setParameters(bool force=false) {
    logQuery();

    int power_p = getArgValue("power");
    String mode_p = getArgValueStr("mode");
    int temp_p = getArgValue("temp");
    int vdir_p = getArgValue("vdir");
    int hdir_p = getArgValue("hdir");
    int fan_p = getArgValue("fan");
    int silent_p = getArgValue("silent");
    int _3d_p = getArgValue("3d");

    bool changed=force;

    previous_state = state;

    if (mode_p != "") {
        if (mode_p ==  "cool") {
            state.mode=MODE_COOL;
            state.temp=27;
            changed=true;
            syslog.log(LOG_INFO, "mode=cool supplied (mode: " + String(previous_state.mode) + "->" + String(state.mode)+")");
        } else if (mode_p == "fan") {
            state.mode=MODE_FAN;
            changed=true;
            syslog.log(LOG_INFO, "mode=fan supplied (mode: " + String(previous_state.mode) + "->" + String(state.mode)+")");
        } else if (mode_p == "heat") {
            state.mode=MODE_HEAT;
            state.temp=18;
            changed=true;
            syslog.log(LOG_INFO, "mode=heat supplied (mode: " + String(previous_state.mode) + "->" + String(state.mode)+")");
        } else if (mode_p == "dehumidify") {
            state.mode=MODE_DRY;
            state.temp=27;
            changed=true;
            syslog.log(LOG_INFO, "mode=dehumidify supplied (mode: " + String(previous_state.mode) + "->" + String(state.mode)+")");
        } else if (mode_p == "auto") {
            // We run the calculation ourselves and set an explicit
            // mode rather than letting the AC go into actual auto
            // mode
            state.mode=MODE_AUTO;
            changed=true;
            syslog.log(LOG_INFO, "mode=auto supplied (mode: " + String(previous_state.mode) + "->" + String(state.mode)+")");
// LOG LATER:            syslog.log(LOG_INFO, "mode=auto supplied");
        } else {
            error_str="Unknown mode supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid mode supplied");
        }
    }

    if (power_p >= 0) {
        if (power_p == 0 || power_p == 1) {
            state.power = power_p;
            changed=true;
            syslog.logf(LOG_INFO, "power=%s supplied", power_p ? "on":"off");
        } else {
            error_str="Unknown power supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid power supplied");
        }
    }

    if (temp_p >= 0) {
        if (temp_p >= 18 && temp_p <= 30) {
            state.temp = temp_p;
            changed=true;
            syslog.logf(LOG_INFO, "temp=%d supplied",temp_p);
        } else {
            error_str="Unknown temp supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid temp supplied");
        }
    }

    if (fan_p >= 0 ) {
        syslog.log(LOG_INFO, "fan supplied");
        if (fan_p >= 0 && fan_p <= 3) {
            state.fanspeed = fan_p;
            changed=true;
            syslog.logf(LOG_INFO, "fan=%d supplied",fan_p);
        } else {
            error_str="Unknown fan supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid fan supplied");
        }
    } else {
        if (temp_p >= 0) {
            if (state.temp >= 27) {
                if (state.temp > previous_state.temp) {
                    state.fanspeed = FAN_1; // when ramping up temperature, slow the speed down to minimum
                }
            } else if (state.temp >= 26) {
                if ((state.temp > previous_state.temp) ||
                    (previous_state.fanspeed == FAN_1)) {
                    state.fanspeed = FAN_2;
                }
            } else if (state.temp >= 25) {
                if ((state.temp > previous_state.temp) ||
                    (previous_state.fanspeed == FAN_1) ||
                    (previous_state.fanspeed == FAN_2)) {
                    state.fanspeed = FAN_3;
                }
            } else if (state.temp <= 18) {
                if ((state.temp < previous_state.temp) ||
                    (previous_state.fanspeed == FAN_2) ||
                    (previous_state.fanspeed == FAN_3)) {
                    state.fanspeed = FAN_1;
                }
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
        if (vdir_p >= 0 && vdir_p <= 6) {
            state.vdir = vdir_p;
            changed=true;
            syslog.logf(LOG_INFO, "vdir=%d supplied", vdir_p);
        } else {
            error_str="Unknown vdir supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid vdir supplied");
        }
    }

    if (hdir_p >= 0) {
        if (hdir_p >= 0 && hdir_p <= 6) {
            state.hdir = hdir_p;
            changed=true;
            syslog.logf(LOG_INFO, "hdir=%d supplied",hdir_p);
        } else {
            error_str="Unknown hdir supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid hdir supplied");
        }
    }

    if (silent_p >= 0) {
        if (silent_p == 0 || silent_p == 1) {
            state.silent = silent_p;
            changed=true;
            syslog.logf(LOG_INFO, "silent=%d supplied", silent_p);
       } else {
            error_str="Unknown silent supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid silent supplied");
        }
    }

    if (_3d_p >= 0) {
        if (_3d_p == 0 || _3d_p == 1) {
            state._3d = _3d_p;
            changed=true;
            syslog.logf(LOG_INFO, "3d=%d supplied", _3d_p);
        } else {
            error_str="Unknown 3d supplied<br>\n"+error_str;
            syslog.log(LOG_INFO, "invalid 3d supplied");
        }
    }

    if (state.mode == MODE_AUTO) {
        if ((state.temp <= 22) && (previous_state.mode == MODE_COOL)) {
            state.mode = MODE_HEAT;
            syslog.log(LOG_INFO, "mode=auto was supplied, heating at " + String(state.temp));
            changed=true;
        } else if ((state.temp >= 25) && (previous_state.mode == MODE_HEAT)) {
            state.mode = MODE_COOL;
            syslog.log(LOG_INFO, "mode=auto was supplied, cooling at " + String(state.temp));
            changed=true;
        } else {
            state.mode = previous_state.mode;
            syslog.log(LOG_INFO, "mode=was auto supplied, keeping mode at " + String(state.mode) + " with temp " + String(state.temp));
        }
    }
    if (changed) {
        write_eeprom_time=millis()+10000;
    }
    return changed;
}

void srv_handle_ajax_js() {
    server.send(200, "application/javascript", (char*)ajaxy_ac_js);
}

void srv_handle_feedback_js() {
    server.send(200, "application/javascript", (char*)feedback_ac_js);
}

void srv_handle_ajax_get() {
//    String res="{\"power\":\""+state.power+"\",\"mode\":
    JSONVar state_json;

    state_json["power"]        = state.power;
    state_json["mode"]         = state.mode;
    state_json["setpointtemp"] = state.temp;
    state_json["vdir"]         = state.vdir;
    state_json["hdir"]         = state.hdir;
    state_json["fanspeed"]     = state.fanspeed;
    state_json["silent"]       = state.silent;
    state_json["3d"]           = state._3d;
    state_json["debug"]        = state.debug;

    String jsonString = JSON.stringify(state_json);

    //syslog.log(LOG_INFO, "get: "+jsonString);

    server.send(200, "application/json", jsonString);
}

void http_do() {
    if (setParameters()) {
        updateAC();
    }
    sendIndexHTML();
}

void http_do_action() {
    Serial.println("do action called");
    if (setParameters()) {
        updateAC();
    }
    srv_handle_ajax_get();
}

void http_forcedo_and_redirect() {
    if (setParameters(true)) {
        updateAC(); // always true with true above
    }
    server.sendHeader("Location",".");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void http_on() {
    state.power=POWER_ON;

    http_forcedo_and_redirect();
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

void http_dehumidify_on() {
    state.mode=MODE_DRY;
    state.power=POWER_ON;

    http_forcedo_and_redirect();
}

void http_toggle() {
    if (state.power == POWER_OFF) {
        state.power=POWER_ON;
    } else {
        state.power=POWER_OFF;
    }
    http_forcedo_and_redirect();
}

void http_up() {
    state.temp++;

    http_forcedo_and_redirect();
}

void http_down() {
    state.temp--;

    http_forcedo_and_redirect();
}

void http_off() {
    state.power=POWER_OFF;

    http_forcedo_and_redirect();
}

void http_debug_toggle() {
    state.debug = !state.debug;
    http_forcedo_and_redirect();
}

String http_uptime_stub() {
    return "debug: " + String(state.debug?"true":"false") + "\n";
}

void http_handle_not_found() {
    server.send(404, "text/plain", "File Not Found\n");
}

void http_start_stub() {
    Serial.println("HTTP server extra setup");

    server.on("/toggle",      http_toggle);
    server.on("/on",          http_on);
    server.on("/ac_on",       http_ac_on);
    server.on("/heater_on",   http_heater_on);
    server.on("/fan_on",      http_fan_on);
    server.on("/dry_on",      http_dehumidify_on);
    server.on("/up",          http_up);
    server.on("/down",        http_down);
    server.on("/off",         http_off);
    server.on("/",            http_do);
    server.on("/do",          http_do_action);
    server.on("/ajaxy.js",    srv_handle_ajax_js);
    server.on("/feedback.js", srv_handle_feedback_js);
    server.on("/get",         srv_handle_ajax_get);
    server.on("/debug",       http_debug_toggle);
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
