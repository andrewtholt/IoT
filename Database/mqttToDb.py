#!/usr/bin/env python3

import sqlite3
import paho.mqtt.client as mqtt

import time


db = None

mqttBroker = "192.168.10.124"
mqttPort = 1883


def on_message(client, userdata, msg):
    global db

    topic = ((msg.topic)[1:]).replace('/','_')
    payload = (msg.payload).decode("utf-8")

    seconds = int(time.time())

    print(seconds)
    print(topic)
    print(payload)

    if db == None:
        print("No database")
    else:
        sql = 'insert into weather (time, name, data ) values( ' + str(seconds) +  ',"' + topic + '",'  + payload + ');'

        print(">" + sql + "<")

        cur = db.cursor()
        rows = cur.execute( sql )
        db.commit()
        cur.close()

    print("=======")

def on_connect(client, userdata, flags, rc):
    print("Connected")

def main():
    global db
    db = sqlite3.connect('weather.db')

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(mqttBroker, mqttPort, 60)
    client.subscribe('/home/office/light')
    client.subscribe('/home/office/temperature')
    client.subscribe('/home/office/pressure')
    client.subscribe('/home/office/humidity')
    client.subscribe('/home/office/dewpoint')

#    db = sqlite3.connect('weather.db')

    client.loop_forever()


main()
