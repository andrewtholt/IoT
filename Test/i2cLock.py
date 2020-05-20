
import utime

from machine import Pin

sda = Pin(0, Pin.IN)
scl = Pin(4, Pin.IN)
p14 = Pin(14, Pin.OUT)

def callback(p):
    utime.sleep_us(5)
    if scl.value() == 0:
        sda = Pin(0, Pin.OPEN_DRAIN)
        sda.value(0)
        p14.value(0)
    else:
        p14.value(1)

def waitForIdle():
    while(scl.value() == 1 and sda.value() == 1):
        pass


def main():
    print("Setting up")
    sda = Pin(0, Pin.IN)
    p14.value(1)
    waitForIdle()
    sda.irq(trigger=Pin.IRQ_FALLING, handler=callback)

print("Main")

main()

