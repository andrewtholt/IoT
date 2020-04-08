import time
from umqttsimple import MQTTClient
import ubinascii
import machine
import micropython
import network
import esp
import bh1750fvi
from machine import I2C,Pin
from bmp180 import BMP180
import dht

from math import log

def netStatus(net):
    print('network config:', net.ifconfig())

def do_connect(ssid, password):

    essid = "HoltAtHome4"
    password = "anthony050192"
    sta_if = network.WLAN(network.STA_IF)

    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(essid, password)
        while not sta_if.isconnected():
            pass
    netStatus(sta_if)

    return sta_if

def sub_cb(topic, msg):
    print(topic, msg)

def calcRH(celsius, humidity):
    a = 17.271
    b = 237.7

    temp = (a * celsius) / (b + celsius) + log(humidity*0.01)

    Td = (b * temp) / (a - temp)
    return Td


def main():
    esp.osdebug(None)
    import gc
    gc.collect()
    i2c = I2C(scl=machine.Pin(5), sda=machine.Pin(4))

    sensor = dht.DHT11(Pin(12))

    bmp180 = BMP180(i2c)
    bmp180.oversample_sett = 2
    bmp180.baseline = 101325

    mqtt_server = '192.168.10.124'

    net = do_connect('HoltAtHome4','anthony050192')
    client_id = ubinascii.hexlify(machine.unique_id())

    client = MQTTClient(client_id, mqtt_server)
    client.connect()
    
    count = 255
    base = '/home/office/'

    print("Free : ", gc.mem_free())
    while True:
        lightLevel = bh1750fvi.sample(i2c)
        print("Light Level : ",lightLevel)
        print("Count       : ",count)
        client.publish(base + 'light', str(lightLevel))

        count = count+1

        if count > 2:
            sensor.measure()
            humidity = sensor.humidity()
            temp1 = sensor.temperature()

            temp = bmp180.temperature
            p = bmp180.pressure
            mbar = p/100
            freeMem = gc.mem_free()

            dewPoint = calcRH(temp,humidity)
            
            print("Temperature :", temp)
            print("Temperature1:", temp1)
            print("Pressure    :", mbar)
            print("Humidity    :", humidity)
            print("Dew Point   :", dewPoint)
            print("Free : ", freeMem)
            
            client.publish(base + 'temperature', str(temp))
            client.publish(base + 'pressure', str(mbar))
            client.publish(base + 'humidity', str(humidity))
            client.publish(base + 'dewpoint', str(dewPoint))
            client.publish(base + 'envNode/freemem' , str(freeMem))
            
            count = 0
            gc.collect()
            
        time.sleep(15)

print("Start")
stop = machine.Pin(14,Pin.IN, Pin.PULL_UP)

if stop.value() == 0:
    print("ABORT !!!")
else:
    main()



