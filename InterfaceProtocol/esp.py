import json

class esp:
    CONNECTED = False
    COMMAND_LOOP = True
    LISTEN = False
    CONNECT = False
    errorList = ['OK','INVALID_PACKET','INVALID_CMD','WIFI_FAILED','BUG']

    validCmds = ['WIFI_CONNECT','WIFI_DISCONNECT','LISTEN','CONNECT','SLEEP']

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
                print("Valid command");
                rc = self.errorList.index('OK')
            else:
                rc = self.errorList.index('INVALID_CMD')

            print("Here",rc)
            return rc

        except KeyError:
            rc = self.errorList.index('INVALID_PACKET')
            return rc

        return rc

    def queryConnected(self):
        return self.CONNECTED

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
            try:
                cmd = data['CMD']['NAME']
    
                if cmd == "WIFI_CONNECT":
                    out=self.wifiConnect(data)
                elif cmd == "SLEEP":
                    out = self.machineSleep()
    
            except KeyError:
                print("Unkown key")

        return out

    def wifiConnect(self,cmdDict):
        cmd = cmdDict['CMD']['NAME']

        print("wifi connect")
        essid = cmdDict['CMD']['ESSID']
        passwd = cmdDict['CMD']['PASSWD']

        print(essid)
        print(passwd)

        return self.errorResponse(0)
#        print(self.errorResponse(1))

    def machineSleep(self):
        return self.errorResponse(0)



if __name__ == "__main__":

    conn = esp()

    print("e.g. cut and paste")

    print('{ "CMD": { "NAME":"WIFI_CONNECT", "ESSID":"HoltAtHome4","PASSWD":"password"}}')


    tst = input()
    print(conn.command(tst))

