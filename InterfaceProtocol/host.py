
def wifiConnectAP(essid,passwd):

    print("HERE")
    out = '{ "CMD": { "NAME":'
    out += '"WIFI_CONNECT", "ESSID":"' + essid + '",'

    out += '"PASSWD":"' + passwd 
    out += '"}}'

    return out

def wifiDisconnect():
    out = '{ "CMD": { "NAME":"WIFI_DISCONNECT'
    out += '"}}'
    return out

def wifiListen(port):
    out = '{ "CMD": { "NAME":'
    out += '"LISTEN",'
    out += '"PORT":' + str(port)
    out += '}}'

    return out

def wifiConnect(host,port):
    out = '{ "CMD": { "NAME":'
    out += '"CONNECT",'
    out += '"HOST":"' + host + '",'
    out += '"PORT":' + str(port)
    out += '}}'

    return out

def hwSleep():
    out = '{ "CMD": { "NAME":'
    out += '"SLEEP"'
    out += '}}'

    return out


if __name__ == "__main__":
    ret=wifiConnectAP("HoltAtHome4","password")
    print(ret)

#    ret=wifiDisconnect()
#    print(ret)
#
#    ret=wifiListen(123)
#    print(ret)
#
#    ret=wifiConnect("192.168.10.124",123)
#    print(ret)
#
#    ret=hwSleep()
#    print(ret)
#
