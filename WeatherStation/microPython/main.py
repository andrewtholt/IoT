import sys
import time

import ubinascii
import machine
import micropython
import network
import esp

# from machine import I2C,Pin

import json
import btree

from iotNetwork import iotNetwork
from wemos import environment

# from math import log
def deepSleep(msecs):
    print("Nap", msecs)
    #configure RTC.ALARM0 to be able to wake the device
    rtc = machine.RTC()
    rtc.irq(trigger=rtc.ALARM0, wake=machine.DEEPSLEEP)
  # set RTC.ALARM0 to fire after Xmilliseconds, waking the device
    rtc.alarm(rtc.ALARM0, msecs)
  #put the device to sleep
    machine.deepsleep()


def main():
    led = machine.Pin(15, machine.Pin.OUT)

    led.value(1)

    esp.osdebug(None)
    gc.collect()

    resetDb = False

    if machine.reset_cause() == machine.DEEPSLEEP_RESET:
        hardReset = False
    else:
        hardReset = True

    try:
        cfgFile = open('iot_config.json','r')
        c = cfgFile.read()
        netCfg = json.loads(c)

        print("Opening db ... ")
        f = open('iotdb.db','r+b')
        resetDb = False
    except OSError:
        print("... No creating db ...")
        f = open('iotdb.db','w+b')
        resetDb = True
    except :
        print("config error")
        sys.exit(1)

    print("... Done")
    print("Free : ", gc.mem_free())
    gc.collect()

    db = btree.open(f)

    if resetDb or hardReset:
        print("Populating db ...")
        for key, value in netCfg.items():
            print(key,"->", value)
            db[ key.encode()] = value.encode()

        print("... done")

#    db[ key.encode()] = value.encode()
#    db[ b"MEM_USAGE" ] = str(freeMem).encode()

    db.close()
    f.close()


    dht11Pin = int(netCfg['DHT11_PIN'])
    sleepTime=15000

    sleepTime = (int(netCfg['SLEEP_TIME']) * 1000)

    net = iotNetwork()
    net.connect()

    net.ifconfig()
    net.connectMQTT()

    env = environment(dht11Pin)
    env.getI2C()

    env.update()

    temp = env.get('TEMPERATURE')
    net.publishMQTT("temperature", str(temp))

    humid = env.get('HUMIDITY')
    net.publishMQTT("humidity", str(humid))

    dew = env.get('DEW_POINT')
    net.publishMQTT("dewpoint", str(dew))
    # 
    # Comment this out if you dont have bmp120 or light level,
    # from HERE
    #
    iam = netCfg["IAM"]

    print('iam', iam)
#    net.publishMQTT("free_mem", str(freeMem))
#    if iam == 'ESP8266':
#    env.getI2C()

    mbar = env.get('BMP_PRESSURE')
    net.publishMQTT("pressure", str(mbar))

    light = env.get('LIGHT_LEVEL')
    if light >= 0:
        net.publishMQTT("light", str(light))
    # 
    # to HERE
    #
    time.sleep(1)
    gc.collect()

    net.disconnectMQTT()
    net.disconnect()

    led.value(0)
    deepSleep(sleepTime)
    deepSleep(15000)

main()

