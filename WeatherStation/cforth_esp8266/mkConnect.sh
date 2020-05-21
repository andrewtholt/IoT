#!/bin/bash

# set -x

if [ $# -ne 2 ]; then
    echo "Usage:./mkConnect <ssid> <password>"
    echo "And redirect the output to a file"
    exit 1
fi

printf "\" %s\" \" %s\" station-connect \n" $1 $2
printf "ipaddr@ .ipaddr cr\n"
