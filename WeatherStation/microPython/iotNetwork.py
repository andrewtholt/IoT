import json
import network

class iotNetwork :
    netCfg = None

    def __init__(self):
        try:
            netFile = open('network.json','r')
        
            c = netFile.read()
        
            self.netCfg = json.loads(c)
        
            print(self.netCfg)
        except:
            print("Failed to open network file.")

    def connect(self):
        print("Connect")
        self.sta_if = network.WLAN(network.STA_IF)

        if self.sta_if.isconnected():
            self.sta_if.disconnect()

        print('connecting to network...')
        self.sta_if.active(True)

        essid = self.netCfg['ESSID']
        password = self.netCfg['PASSWD']

        self.sta_if.connect(essid, password)

        while not self.sta_if.isconnected():
            pass

        print("... Connected")

    def ifconfig(self):
        self.sta_if.ifconfig()

if __name__ == "__main__":
    net = iotNetwork()

    net.connect()

