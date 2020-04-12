import json
import network
from umqttsimple import MQTTClient
import btree
import ubinascii
import machine

class iotNetwork :
    netCfg = None

    def __init__(self):
        cfgFile = open('iotdb.db','r+b')

        self.netCfg = btree.open(cfgFile)

    def connect(self):
        print("Connect")
        self.sta_if = network.WLAN(network.STA_IF)

        if self.sta_if.isconnected():
            self.sta_if.disconnect()

        print('connecting to network...')
        self.sta_if.active(True)

        essid    = (self.netCfg[b"ESSID"]).decode()
        password = (self.netCfg[b"PASSWD"]).decode()

        self.sta_if.connect(essid, password)

        while not self.sta_if.isconnected():
            pass

        print("... Connected")

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
        print("MQTT")
        mqttHost = (self.netCfg[b"MQTT_HOST"]).decode()
        mqttPort = int((self.netCfg[b"PORT"]).decode())
        self.base = (self.netCfg[b"MQTT_BASE"]).decode()

        clientId = ubinascii.hexlify(machine.unique_id())

        self.client = MQTTClient(clientId, mqttHost)
        self.client.connect()

    def disconnectMQTT(self):
        print("MQTT Disconnect")
        self.client.disconnect()
        print("MQTT Disconnected")

    def publishMQTT(self,topic,message):
        print("Publish: " + topic + "->" + message)
        print(self.base + topic, message )
        self.client.publish(self.base + topic, message )

    def ifconfig(self):
        print(self.sta_if.ifconfig())

if __name__ == "__main__":
    net = iotNetwork()

    net.connect()

