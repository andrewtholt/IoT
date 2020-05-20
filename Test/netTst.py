from iotNetwork import iotNetwork
import time
import machine

def deepSleep(msecs):
    print("Nap", msecs)
    #configure RTC.ALARM0 to be able to wake the device
    rtc = machine.RTC()
    rtc.irq(trigger=rtc.ALARM0, wake=machine.DEEPSLEEP)
  # set RTC.ALARM0 to fire after Xmilliseconds, waking the device
    rtc.alarm(rtc.ALARM0, msecs)
  #put the device to sleep
    machine.deepsleep()

def netTst():

    net = iotNetwork()
    net.connect()

    net.ifconfig()

    net.connectMQTT()

    net.checkMQTT()

    time.sleep(1)

    net.disconnectMQTT()
    net.disconnect()

    time.sleep(1)

    deepSleep(1000)


netTst()

