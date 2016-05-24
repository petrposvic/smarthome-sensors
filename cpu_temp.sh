#!/bin/bash

if [ ! -f /usr/bin/cut ]; then
  echo "/usr/bin/cut doesn't exist"
  exit 1
fi

if [ ! -f /usr/bin/curl ]; then
  echo "/usr/bin/curl doesn't exist"
  exit 2
fi

if [ ! -f /opt/vc/bin/vcgencmd ]; then
  echo "/opt/vc/bin/vcgencmd doesn't exist"
  exit 3
fi

# Expected string in format temp=?.?'C (where ? is number)
T=`/opt/vc/bin/vcgencmd measure_temp | /usr/bin/cut -d"=" -f 2 | /usr/bin/cut -d"'" -f1`

/usr/bin/curl --silent --request POST --header "Content-Type: application/json" --data "{\"device\":\"livingroom_cpu\",\"temperature\":\"$T\"}" http://192.168.1.203:3000/api/measurements
