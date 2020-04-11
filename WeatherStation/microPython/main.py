import sys
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
import json

from iotNetwork import iotNetwork

from math import log

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

#    net = iotNetwork()
#    net.connect()

main()

