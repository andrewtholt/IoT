from time import sleep,sleep_ms
from machine import Pin

# create an output pin on pin #0
p0 = Pin(16, Pin.OUT)
p1 = Pin(5, Pin.IN)

print(p1.value())

# set the value low then high
while p1.value() == 1:
    p0.value(1)
    sleep_ms(500)
    p0.value(0)
    sleep_ms(500)


p0.value(1)
