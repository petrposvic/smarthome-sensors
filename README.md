# smarthome-sensors

crontab -e

# Send CPU temperature each 5 minutes
*/5 * * * * /home/pi/smarthome/sensors/cpu_temp.sh 2>&1 1>/dev/null
