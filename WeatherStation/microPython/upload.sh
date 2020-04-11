#!/bin/sh

PORT=/dev/ttyUSB3

# FILES="bh1750fvi.py bmp180.py umqttsimple.py wemos.py iotNetwork.py iot_config.json main.py"
FILES="iotNetwork.py iot_config.json main.py"

for F in $FILES; do
    echo $F
    ampy -p $PORT put $F
done
ampy -p $PORT reset
