import json
import network
import time
import socket
import select
import select

class espSide:
    CONNECTED = False
    COMMAND_LOOP = True
    LISTEN = False
    CONNECT = False
    errorList = ['OK','INVALID_PACKET','INVALID_CMD','WIFI_FAILED','BUG']
    sta_if = None

    parameters = { 'IP' : '0.0.0.0' }

    validCmds = ['EXIT','WIFI_CONNECT','WIFI_DISCONNECT','GET','LISTEN','CONNECT','SLEEP']

    def cmdValidate(self, cmdDict):
        rc = self.errorList.index('BUG')
        try:
            cmdType = cmdDict['CMD']
        except KeyError:
            rc = self.errorList.index('INVALID_PACKET')
            return rc

        try:
            cmd = cmdDict['CMD']['NAME']
            if cmd in self.validCmds:
                rc = self.errorList.index('OK')
            else:
                rc = self.errorList.index('INVALID_CMD')

            return rc

        except KeyError:
            rc = self.errorList.index('INVALID_PACKET')
            return rc

        return rc

    def queryConnected(self):
        return self.CONNECTED

    def queryCommandLoop(self):
        return self.COMMAND_LOOP

    def errorResponse(self,err):

        out = '{"STATUS":"' + self.errorList[err] + '","CODE":'+str(err)+ '}'

        return out

    def command(self,cmd):
        data= json.loads(cmd)

        out = ""

        error = self.cmdValidate(data)

        if error != 0:
            out = self.errorResponse(error)
        else:
            cmd = data['CMD']['NAME']
    
            if cmd == "WIFI_CONNECT":
                out=self.wifiConnect(data)
            elif cmd == "WIFI_DISCONNECT":
                out=self.wifiDisconnect(data)
            elif cmd == "EXIT":
                out = self.debugExit(data)
            elif cmd == "GET":
                put = self.get(data)
            elif cmd == "LISTEN":
                out = self.netListen(data)
            elif cmd == "SLEEP":
                out = self.machineSleep(data)
    

        return out

    def netListen(self,cmdDict):
        rc = 0

        alert = select.poll()

        print("Listen")
        port = 0
        port = cmdDict['CMD']['PORT']
        print("Port=",port)

        addr = socket.getaddrinfo('0.0.0.0', port)[0][-1]

        s = socket.socket()
        s.bind(addr)
        s.listen(1)

        cl, addr = s.accept()
        print('client connected from', addr)
        cl_file = cl.makefile('rwb', 0)

        poll=select.poll()
        poll.register(cl_file, select.POLLIN)
        poll.register(uart, select.POLLIN)

        lineCount=0

        output = ""
        while True:
            print("Waiting...")
            poll.poll()
            print(".. done")
            line = cl_file.readline()

            lineCount += 1

            if not line or line == b'\r\n':
                break
            else:
                tmp = line[:-2]
                output += tmp.decode('utf-8')

        response = "Hello " + str(lineCount) +"\r\n" + output
        print(response)
        cl.send(response)
        cl.close()

        print('listening on', addr)

        return self.errorResponse(rc)



    def get(self,cmdDict):
        rc = 0
        cmd = cmdDict['CMD']['NAME']
        paramName = cmdDict['CMD']['ITEM']

        if paramName in self.parameters :
            rc = 0
            data = self.parameters[ paramName ]

            out = '{ "CMD": { "NAME":"DATA","TOPIC":'
            out += '"' + paramName  + '", "PAYLOAD":"' + data + '" }}'

            print(out)



    def debugExit(self,cmdDict):
        self.COMMAND_LOOP=False

    def wifiConnect(self,cmdDict):
        rc = 0
        cmd = cmdDict['CMD']['NAME']

        essid = cmdDict['CMD']['ESSID']
        passwd = cmdDict['CMD']['PASSWD']

        if self.CONNECTED == False:
            self.sta_if = network.WLAN(network.STA_IF)
            if not self.sta_if.isconnected():
                self.sta_if.active(True)
                self.sta_if.connect(essid, passwd)

                count=10

                while not self.sta_if.isconnected():
                    time.sleep(1)
                    count -= 1
                    if count <= 0:
                        break;

        if self.sta_if.isconnected():

            tmp = self.sta_if.ifconfig()
            self.parameters['IP'] = tmp[0]

            rc = self.errorList.index('OK')
            self.CONNECTED=True
        else:
            rc = self.errorList.index('WIFI_FAILED')

            self.CONNECTED=False

        return self.errorResponse(rc)

    def wifiDisconnect(self,cmdDict):
        if self.CONNECTED :
            if self.sta_if.isconnected():
                self.CONNECTED=False
                self.sta_if.disconnect()

        return self.errorResponse(0)


    def machineSleep(self):
        return self.errorResponse(0)

