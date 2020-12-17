# 1. Giant 7 segments clock.
2 leds per segment, each number has 14 leds, total 
Led is on D3 of NodeMCU

# 2. 3D printing parts
- StL files are in STL folder

# 3. Webserver
- Copy files in data folder to device.
http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system

Before uploading the files to your ESP board you have to gzip them with the command:

gzip -r ./data/

Afterwards if you want to change something to the html files, just unzip with:

gunzip -r ./data/

# 3. Ref
Original design from
https://github.com/leonvandenbeukel/7-Segment-Digital-Clock-V2

