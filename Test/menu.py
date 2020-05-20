import tstI2C
import i2cLock

def cls():
    print("\033[2J")


def menu():

    runFlag=True

    tst=tstI2C.tstI2C()

    while runFlag:
        cls()

        print("\n\n")
    
        print("\t\tLock   I2C : 1")
        print("\t\tUnlock I2C : 2")
        print("\t\tPulse Trig : 3")
        print("\t\tState      : 4")
        print("\t\tReset      : 5")
        print("\t\tQuit       : q")
    
        print("\n\n")
    
        res=input("\tOption:")

        print(res)

        if res == 'q':
            runFlag=False
        elif res == '1':
            tst.lock()
        elif res == '2':
            tst.unlock()
        elif res == '3':
            tst.pulseTrigger()
        elif res == '4':
            tst.state()
            input('Press return to continue')
        elif res == '5':
            tst.reset()
    
