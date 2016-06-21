#!/usr/bin/python2.7

from time import sleep
import Adafruit_DHT
import requests
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_UP)

while True:
    humidity, temperature = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, 4)

    if humidity is not None and temperature is not None:
        print('Temp={0:0.1f}*  Humidity={1:0.1f}%'.format(temperature, humidity))
        try:
            r = requests.post(
                'http://192.168.1.203:3000/api/measurements',
                data = {'device': 'livingroom', 'temperature': temperature, 'humidity': humidity}
            )
            print(r)
        except requests.exceptions.ConnectionError:
            print('Connection error')
    else:
        print('Failed to get reading. Try again!')
    sleep(60)

