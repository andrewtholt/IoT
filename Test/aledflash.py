# aledflash.py Demo/test program for MicroPython asyncio
# Author: Peter Hinch
# Copyright Peter Hinch 2017 Released under the MIT license
# Flashes the onboard LED's each at a different rate. Stops after ten seconds.
# Run on MicroPython board bare hardware

from machine import Pin
import uasyncio as asyncio

duration = 10

async def killer():

    global duration

    while duration > 0:
        await asyncio.sleep(1)
        duration -= 1
        print("Duration ",duration)

async def toggle(objLED, time_ms):
    while True:
        await asyncio.sleep_ms(time_ms)

        if objLED.value() == 0:
            print("ON")
            objLED.value(1) 
        else:
            print("OFF")
            objLED.value(0) 

# TEST FUNCTION

def test(d):

    global duration

    duration = d

    led = Pin(16,Pin.OUT)
    loop = asyncio.get_event_loop()
    duration = int(duration)

    if duration > 0:
        print("Flash LED's for {:3d} seconds".format(duration))

    t = 500
    loop.create_task(toggle(led, t))

    loop.run_until_complete(killer())
    loop.close()

test(10)
