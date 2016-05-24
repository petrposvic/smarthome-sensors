# smarthome-sensors

sudo vim /etc/rc.local
/usr/bin/screen -d -m /home/pi/smarthome/sensors/main.py

crontab -e

# Send CPU temperature each 5 minutes
*/5 * * * * /home/pi/smarthome/sensors/cpu_temp.sh 2>&1 1>/dev/null
