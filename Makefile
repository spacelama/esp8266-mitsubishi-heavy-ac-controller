ESP_ADDR=192.168.1.95
#ac
#VERBOSE=true

# doesn't just let me have ac.ajaxy.js.cpp depend only on ac.ajaxy.js
# just gets ignored.  Need to do this indirection thing
USER_SRC_PATTERN = |ac.js
USER_RULES=Makefile.extras

EXCLUDE_DIRS=IRremoteESP8266/test/
# EXPAND_LIBS=

LIBS=*.ac.js

#ac.ajaxy.js.cpp : ac.ajaxy.js.cpp.include

include $(HOME)/Arduino/template/Makefile

.DEFAULT_GOAL := ota
#.DEFAULT_GOAL := flash
