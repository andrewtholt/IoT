import json
import network
from umqttsimple import MQTTClient
import btree
import ubinascii
import machine

from time import sleep

class iotNetwork :
    netCfg = None
    wdogTime = 5000 # ms
    tim = machine.Timer(-1)

    netConnected = False
    wdogTriggered = False

    def __init__(self, w=5000):
        self.wdogTime = w
        cfgFile = open('iotdb.db','r+b')

        self.netCfg = btree.open(cfgFile)

    def wdog(self,b):
        print("Wdog fired.")
        self.wdogTriggered = True

        machine.reset()


    def connect(self):
        print("Connect")
        self.tim.init(period=self.wdogTime, mode=machine.Timer.ONE_SHOT, callback=self.wdog)

        self.sta_if = network.WLAN(network.STA_IF)
        
        if self.sta_if.isconnected():
            print("Already connected, disconecting ...")
            self.sta_if.disconnect()
            print("... done.")

        print('connecting to network...')
        self.sta_if.active(True)

        essid    = (self.netCfg[b"ESSID"]).decode()
        password = (self.netCfg[b"PASSWD"]).decode()

        self.sta_if.connect(essid, password)

        while not self.sta_if.isconnected():
            pass

        print("... Connected")
        self.tim.init(period=-1, mode=machine.Timer.ONE_SHOT)

    def disconnect(self):
        print("Disconnect")
        self.sta_if.disconnect()
        print("Disconnect")

        count=0
        while self.sta_if.isconnected():
            count += 1
            print(count)
        print("Disconnected")

    def connectMQTT(self):
        print("MQTT...")
        self.tim.init(period=self.wdogTime, mode=machine.Timer.ONE_SHOT, callback=self.wdog)

        mqttHost = (self.netCfg[b"MQTT_HOST"]).decode()
        mqttPort = int((self.netCfg[b"PORT"]).decode())
        self.base = (self.netCfg[b"MQTT_BASE"]).decode()

        clientId = ubinascii.hexlify(machine.unique_id())

        try:
            self.client = MQTTClient(clientId, mqttHost)
            self.client.connect()
        except:
            sleep(10)


        self.tim.init(period=-1, mode=machine.Timer.ONE_SHOT)
        print("...Done")

    def checkMQTT(self):
        self.client.check_msg()

    def disconnectMQTT(self):
        print("MQTT Disconnect")
        self.client.disconnect()
        print("MQTT Disconnected")

    def publishMQTT(self,topic,message):
        print("Publish: " + topic + "->" + message)
        print(self.base + topic, message )
        self.client.publish(self.base + topic, message )

    def subscribeMQTT(self, topic, cb):
        self.client.set_callback(cb)
        self.client.subscribe( self.base + topic )
        print("Subscribing to " + self.base + topic)

    def ifconfig(self):
        print(self.sta_if.ifconfig())
    
    def getIP(self):
        n = self.sta_if.ifconfig()
        return(n[0])


if __name__ == "__main__":
    net = iotNetwork()

    net.connect()

