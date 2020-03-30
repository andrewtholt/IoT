
# i2c = machine.I2C(scl=machine.Pin(5), sda=machine.Pin(4))

import bh1750fvi
from machine import I2C,Pin
i2c = I2C(scl=machine.Pin(5), sda=machine.Pin(4))
result = bh1750fvi.sample(i2c)
print(result)

