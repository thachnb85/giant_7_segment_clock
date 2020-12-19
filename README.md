# 1. Giant 7 segments clock.
2 leds per segment, each number has 14 leds, total 
Led is on D3 of NodeMCU

# 2. 3D printing parts
- StL files are in STL folder

# 3. Webserver
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

