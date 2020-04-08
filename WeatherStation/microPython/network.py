import network

def netStatus(net):
    print('network config:', net.ifconfig())

def do_connect(ssid, password):
    sta_if = network.WLAN(network.STA_IF)
    netStatus(sta_if)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
#        sta_if.connect('HoltAtHome4', 'anthony050192')
        sta_if.connect(essid, password)
        while not sta_if.isconnected():
            pass
    netStatus(sta_if)
