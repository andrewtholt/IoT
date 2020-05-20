import machine

def i2cscan():
#    i2c = machine.I2C(scl=machine.Pin(5), sda=machine.Pin(4))
    i2c = machine.I2C(scl=machine.Pin(4), sda=machine.Pin(0))


    print('Scan i2c bus...')
    devices = i2c.scan()
    
    if len(devices) == 0:
        print("No i2c device !")
    else:
        print('i2c devices found:',len(devices))
    
    for device in devices:  
        print("Decimal address: ",device," | Hexa address: ",hex(device))
