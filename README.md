# 1. Another giant 7 segments clock
- Clock will connect to wifi network after booting, pulling the time from NTP.org
- Weather data is from api.openweathermap.org
- 2 leds per segment, each number has 14 leds, total 58 leds
- Led is on D3 of controller (NodeMCU)

# 2. 3D printing parts
- 3D model in STL format. Slicing & printing using your own software/3d printer.

# 3. Web Server
### Web Server for other features
- Scoreboard
- Count down
- Changing color/brightness

### Install ESP8266 support to Arduino
Arduino > Preferences > Additonal Board Management link
```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

### Install the FS tool
Download the 0.5.0 which supports python3 
https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.5.0/ESP8266FS-0.5.0.zip

Copy the jar file to sketch folder of Arduino
Arduino IDE > Sketch > Show Sketch Folder
```
~/Documents/Arduino/tools/ESP8266FS/tool/esp8266fs.jar
```
Copy data to current sketch folder
```
~/Documents/Arduino/NodeMCUGiantClock
```
Then Adruino IDE > Tools > ESP8266 Sketch Data Upload

# 3. Ref
Original design from
https://github.com/leonvandenbeukel/7-Segment-Digital-Clock-V2
Changes in this repo:
- Time is updated directly from ntp.org server
- Weather data is updated from openweathermap.org
- Day/night color enabled 

