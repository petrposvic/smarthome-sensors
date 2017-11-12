# ESP8266 (ESP-01, 512kb) Led Strip (12V) server

## Download NodeMCU

Download branch 1.5.4.1-final (frozen, for 512kb modules) from https://nodemcu-build.com

Select modules:

  * file
  * GPIO
  * net
  * node
  * PWM
  * timer
  * UART
  * WiFi

Start build and wait for email. Then download integer .bin file from email.

## Install tools

Install esptool (via pip):

```
sudo pip install esptool
```

## Upload firmware

Connect module, go to flash mode and test it:

```
esptool.py --port /dev/ttyUSB* flash_id
esptool.py v2.1
Connecting........_____....
Detecting chip type... ESP8266
Chip is ESP8266
Uploading stub...
Running stub...
Stub running...
Manufacturer: e0
Device: 4014
Detected flash size: 1MB
Hard resetting...
```

Go to flash mode again and upload firmware:

```
esptool.py --port /dev/ttyUSB* write_flash -fm qio 0x00000 Downloads/nodemcu/nodemcu-1.5.4.1-final-8-modules-2017-11-12-14-10-21-integer.bin 
esptool.py v2.1
Connecting........_____..
Detecting chip type... ESP8266
Chip is ESP8266
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Auto-detected Flash size: 1MB
Flash params set to 0x0020
Compressed 390852 bytes to 250104...
Wrote 390852 bytes (250104 compressed) at 0x00000000 in 22.1 seconds (effective 141.3 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting...
```

## Upload code

Download and use ESPlorer: https://esp8266.ru/esplorer/ Connect to right port and restart ESP-01.

## Wiring

![ESP-01 and LED strip wiring](https://user-images.githubusercontent.com/1448806/32703053-af95cc24-c7f0-11e7-88d7-e6d032d97973.jpg)

Pull-up GPIO2 to normal UART start.
