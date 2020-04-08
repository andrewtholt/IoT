
from machine import Pin

pin = Pin(14,Pin.IN, Pin.PULL_UP)

print(pin.value())
