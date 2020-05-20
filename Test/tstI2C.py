import utime
from machine import Pin

class tstI2C:
    sda = None
    scl = None
    p14 = None

    def __init__(self):
        self.reset()

    def reset(self):
        self.sda = Pin(0, Pin.IN)
        self.scl = Pin(4, Pin.IN)
        self.p14 = Pin(14, Pin.OUT)
        self.p14.value(1)
        self.sda.irq(trigger=Pin.IRQ_FALLING, handler=self.nullCallback)

    def waitForIdle(self):
        while(self.scl.value() == 1 and self.sda.value() == 1):
            pass

    def callback(self,p):
        utime.sleep_us(5)
        if self.scl.value() == 0:
            self.sda = Pin(0, Pin.OPEN_DRAIN)
            self.sda.value(0)
            self.p14.value(0)
            self.sda.irq(trigger=Pin.IRQ_FALLING, handler=self.nullCallback)
        else:
            self.p14.value(1)
   
    def nullCallback(self,p):
        pass

    def pulseTrigger(self):
        self.p14.value(1)
        utime.sleep_us(60)

        self.p14.value(0)
        utime.sleep_us(60)
        self.p14.value(1)

    def state(self):
        print("sda is" , self.sda.value())
        print("scl is" , self.scl.value())
        print("p14 is" , self.p14.value())

    def lock(self):
        self.p14.value(1)
        self.waitForIdle()
        self.sda.irq(trigger=Pin.IRQ_FALLING, handler=self.callback)

    def unlock(self):
        self.p14.value(1)

        self.state()

        self.sda.irq(trigger=Pin.IRQ_FALLING, handler=self.nullCallback)
        self.sda = Pin(0, Pin.IN)

        self.scl = Pin(4, Pin.OPEN_DRAIN)

        self.scl.value(1)
        utime.sleep_us(60)

        self.p14.value(0)

        self.scl.value(0)
        utime.sleep_us(6)
        self.scl.value(1)

        print("sda is" , self.sda.value())
   
