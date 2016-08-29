# Smarthome Sensors

```
sudo vim /etc/rc.local
```

```
/usr/bin/screen -d -m /home/pi/smarthome/sensors/main.py
```

```
crontab -e
```

```
# Send CPU temperature each 5 minutes
*/5 * * * * /home/pi/smarthome/sensors/cpu_temp.sh 2>&1 1>/dev/null
```

# ESP-01 pinout

## Antenna on left

```
rx  -> tx  vcc -> 3.3
gp0 -> fls rst -> rst
gp2        chp -> 3.3
gnd -> gnd tx  -> rx
```

## How to upload firmware

1. Click on Upload button
2. Press and hold RESET
3. Press and hold FLASH
4. Wait until compilation is done
5. Release RESET
6. Wait until uploading is done
7. Release FLASH

