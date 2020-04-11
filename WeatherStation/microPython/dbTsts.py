import json

import btree

def main():
    try:
        netFile = open('iot_config.json','r')

        c = netFile.read()
        netCfg = json.loads(c)

        print(netCfg)

        f = open('iotdb.db','r+b')
    except OSError:
        f = open('iotdb.db','w+b')
    except:
        print("config error")

    db = btree.open(f)

    for key, value in netCfg.items():
        print(key,"->", value)

        db[ key.encode()] = value.encode()


main()


