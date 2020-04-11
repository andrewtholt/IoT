from machine import I2C,Pin
import dht
from math import log


class environment:

    envData = {}

    envData['HUMIDITY'] =0 
    envData['TEMPERATURE'] = -100 
    envData['PRESSURE'] = -1
    envData['DEW_POINT'] = -1

    sensor = None;

    def __init__(self,dhtPin):
        self.sensor = dht.DHT11(Pin(dhtPin))

    def calcDewpoint(self):
        a = 17.271
        b = 237.7

        temp = (a * self.envData['TEMPERATURE']) / (b + self.envData['TEMPERATURE']) + log(self.envData['HUMIDITY']*0.01)

        self.envData['DEW_POINT'] = (b * temp) / (a - temp)

    def update(self):
        # 
        # Note pin number is GPIO2
        #
        self.sensor.measure()
    
        self.envData['HUMIDITY']    = self.sensor.humidity()
        self.envData['TEMPERATURE'] = self.sensor.temperature()

        self.calcDewpoint()

    def get(self,name):
        return( self.envData[ name ])

    def dump(self):
        print('Humidity   :',self.envData['HUMIDITY'], '%')
        print('Temperature:',self.envData['TEMPERATURE'], 'C')
        print('Dew Point  :',self.envData['DEW_POINT'], 'C')



