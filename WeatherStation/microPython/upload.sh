#!/bin/sh

# set -x

if [ $# -lt 2 ]; then
    echo "Usage: upload.sh <port> <type>"
    echo "e.g. ./upload.sh /dev/ttyUSB9 wemos"
    exit 0
fi
PORT=$1
TARGET=$2

cp iot_config_${TARGET}.json iot_config.json

FILES="dht12.py bh1750fvi.py bmp180.py umqttsimple.py wemos.py iotNetwork.py iot_config.json main.py"
# FILES="iotNetwork.py iot_config.json main.py"

for F in $FILES; do
    echo $F
    ampy -p $PORT put $F
done
ampy -p $PORT reset
