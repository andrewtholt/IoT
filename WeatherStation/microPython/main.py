import sys
import time

import ubinascii
import machine
import micropython
import network
import esp

from machine import I2C,Pin

import json
import btree

from iotNetwork import iotNetwork
from wemos import environment

# from math import log

def main():
    esp.osdebug(None)
    gc.collect()

    print("Free : ", gc.mem_free())

    try:
        cfgFile = open('iot_config.json','r')
        c = cfgFile.read()
        netCfg = json.loads(c)
        
        print("Opening db ... ")
        f = open('iotdb.db','r+b')
    except OSError:
        print("... No creating db ...")
        f = open('iotdb.db','w+b')
    except :
        print("config error")
        sys.exit(1)

    print("... Done")
    print("Free : ", gc.mem_free())
    gc.collect()

    db = btree.open(f)

    for key, value in netCfg.items():
        print(key,"->", value)
        db[ key.encode()] = value.encode()

    db.close()
    f.close()


    dht11Pin = int(netCfg['DHT11_PIN'])

    net = iotNetwork()
    net.connect()
    net.connectMQTT()

    env = environment(dht11Pin)

    while True:
        env.update()


        temp = env.get('TEMPERATURE')
        net.publishMQTT("temperature", str(temp))

        humid = env.get('HUMIDITY')
        net.publishMQTT("humidity", str(humid))

        dew = env.get('DEW_POINT')
        net.publishMQTT("dewpoint", str(dew))

        # 
        # Comment this out if you dont gave bmp120 or light level
        #
        env.getI2C()

        mbar = env.get('BMP_PRESSURE')
        net.publishMQTT("pressure", str(mbar))

        light = env.get('LIGHT_LEVEL')
        net.publishMQTT("light", str(light))
        # 
        # HERE
        #

        gc.collect()
        time.sleep(15)

    net.publishMQTT("fred","1234")

main()

