import machine
import dht
from math import log
from bmp180 import BMP180
import bh1750fvi
from time import sleep

class environment:

    envData = {}

    envData['HUMIDITY'] =0 
    envData['TEMPERATURE'] = -100 
    envData['DEW_POINT'] = -1
    envData['LIGHT_LEVEL'] = -1

    envData['BMP_TEMPERATURE'] = -100 
    envData['BMP_PRESSURE'] = -0 

    bmp180 = None
    i2c = None

    def __init__(self,dhtPin):
#        d = dht.DHT22(Pin(4, Pin.IN, Pin.PULL_UP))
        self.sensor = dht.DHT11(machine.Pin(dhtPin, machine.Pin.IN, machine.Pin.PULL_UP))
#        self.sensor = dht.DHT11(machine.Pin(dhtPin),)
        sleep(2)

    def getI2C(self):

        if self.i2c == None:
            self.i2c = machine.I2C(scl=machine.Pin(5,machine.Pin.OUT, machine.Pin.PULL_UP), sda=machine.Pin(4,machine.Pin.OUT, machine.Pin.PULL_UP))
            self.bmp180 = BMP180(self.i2c)
            self.bmp180.oversample_sett = 2
            self.bmp180.baseline = 101325

        self.envData['LIGHT_LEVEL']     = bh1750fvi.sample(self.i2c)
        self.envData['BMP_TEMPERATURE'] = self.bmp180.temperature
        self.envData['BMP_PRESSURE']    = (self.bmp180.pressure)/100


    def calcDewpoint(self):
        a = 17.271
        b = 237.7

        temp = (a * self.envData['TEMPERATURE']) / (b + self.envData['TEMPERATURE']) + log(self.envData['HUMIDITY']*0.01)

#        self.envData['DEW_POINT'] = (b * temp) / (a - temp)
        dp = (b * temp) / (a - temp)

        if dp < 0:
            dp = 0

        self.envData['DEW_POINT'] = dp

    def update(self):
        # 
        # Note pin number is GPIO2
        #
        sleep(0.25)
        self.sensor.measure()
    
        self.envData['HUMIDITY']    = self.sensor.humidity()
        self.envData['TEMPERATURE'] = self.sensor.temperature()

        self.calcDewpoint()

    def get(self,name):
        return( self.envData[ name ])

    def dump(self):
        print('Humidity       :',self.envData['HUMIDITY'], '%')
        print('Temperature    :',self.envData['TEMPERATURE'], 'C')
        print('Dew Point      :',self.envData['DEW_POINT'], 'C')

        print('Light Level    :',self.envData['LIGHT_LEVEL'])
        print('bmp Temperature:',self.envData['BMP_TEMPERATURE'],'C')
        print('bmp Pressure   :',self.envData['BMP_PRESSURE'],'mBar')



