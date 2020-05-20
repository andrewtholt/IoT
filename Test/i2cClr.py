
import utime

from machine import Pin

# scl = Pin(4, Pin.IN)

# p14 = Pin(14, Pin.OUT)


def main():
    print("Setting up")

    sda = Pin(0, Pin.IN)
    scl = Pin(4, Pin.IN)

    while(scl.value() == 1 and sda.value() == 1):
        pass

    print("Idle.")

#    sda = Pin(0, Pin.OPEN_DRAIN)

    scl = Pin(4, Pin.OPEN_DRAIN)
    sda.value(1)
    utime.sleep_us(6)
    sda.value(0)

    utime.sleep_us(6)
    sda.value(1)


print("Main")

main()

